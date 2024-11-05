#include "CodeAnalysis/Binder.h"
#include "CodeAnalysis/SyntaxTree.h"
#include "CodeAnalysis/Symbol.h"
#include "CodeAnalysis/Lowerer.h"
#include "Utils.h"
#include <iostream>
#include <algorithm>
#include <stack>
#include <unordered_set>

BoundStatement *Binder::BindStatement(StatementSyntax *node)
{
    switch (node->Kind)
    {
    case SyntaxKind::ExpressionStatement:
        return BindExpressionStatement((ExpressionStatementSyntax *)node);
    case SyntaxKind::VariableDeclaration:
        return BindVariableDeclaration((VariableDeclarationSyntax *)node);
    case SyntaxKind::BlockStatement:
        return BindBlockStatement((BlockStatementSyntax *)node);
    case SyntaxKind::IfStatement:
        return BindIfStatement((IfStatementSyntax *)node);
    case SyntaxKind::WhileStatement:
        return BindWhileStatement((WhileStatementSyntax *)node);
    case SyntaxKind::ForStatement:
        return BindForStatement((ForStatementSyntax *)node);
    case SyntaxKind::BreakStatement:
        return BindBreakStatement((BreakStatementSyntax *)node);
    case SyntaxKind::ContinueStatement:
        return BindContinueStatement((ContinueStatementSyntax *)node);
    case SyntaxKind::ReturnStatement:
        return BindReturnStatement((ReturnStatementSyntax *)node);
    default:
        std::cerr << "Unexpected syntax kind: {" << convertSyntaxKindToString(node->Kind) << "}" << std::endl;
        return nullptr;
    }
}

BoundStatement *Binder::BindErrorStatement()
{
    return new BoundExpressionStatement(new BoundErrorExpression());
}

BoundStatement *Binder::BindIfStatement(IfStatementSyntax *node)
{

    BoundExpression *condition = BindExpression(node->Condition, TypeSymbol::Boolean);

    BoundStatement *thenStatement = BindStatement(node->ThenStatement);
    BoundStatement *elseStatement = node->ElseClause == nullptr ? nullptr : BindStatement(node->ElseClause->ElseStatement);
    return new BoundIfStatement(condition, thenStatement, elseStatement);
}

BoundStatement *Binder::BindWhileStatement(WhileStatementSyntax *node)
{
    BoundExpression *condition = BindExpression(node->Condition, TypeSymbol::Boolean);

    BoundLabel *breakLabel = nullptr;
    BoundLabel *continueLabel = nullptr;

    BoundStatement *body = BindLoopBody(node->Body, breakLabel, continueLabel);

    return new BoundWhileStatement(condition, body, breakLabel, continueLabel);
}

BoundStatement *Binder::BindForStatement(ForStatementSyntax *node)
{
    BoundExpression *lowerBound = BindExpression(node->LowerBound, TypeSymbol::Integer);
    BoundExpression *upperBound = BindExpression(node->UpperBound, TypeSymbol::Integer);
    _scope = new BoundScope(_scope);
    VariableSymbol *variable = BindVariable(node->Identifier, false, TypeSymbol::Integer);

    BoundLabel *breakLabel = nullptr;
    BoundLabel *continueLabel = nullptr;
    BoundStatement *body = BindLoopBody(node->Body, breakLabel, continueLabel);

    std::cout << breakLabel->Name << std::endl;
    _scope = _scope->Parent;
    return new BoundForStatement(*variable, lowerBound, upperBound, body, breakLabel, continueLabel);
}

BoundStatement *Binder::BindLoopBody(StatementSyntax *body, BoundLabel *&breakLabel, BoundLabel *&continueLabel)
{
    _labelCounter++;

    breakLabel = new BoundLabel("break" + std::to_string(_labelCounter));
    continueLabel = new BoundLabel("continue" + std::to_string(_labelCounter));

    _loopStack.push(std::make_pair(*breakLabel, *continueLabel));
    BoundStatement *boundBody = BindStatement(body);
    _loopStack.pop();

    return boundBody;
}

BoundStatement *Binder::BindBreakStatement(BreakStatementSyntax *node)
{
    if (_loopStack.empty())
    {
        _diagnostics.ReportInvalidBreakOrContinue(node->Keyword.Span, node->Keyword.value);
        return BindErrorStatement();
    }
    return new BoundGotoStatement(_loopStack.top().first);
}

BoundStatement *Binder::BindContinueStatement(ContinueStatementSyntax *node)
{
    if (_loopStack.empty())
    {
        _diagnostics.ReportInvalidBreakOrContinue(node->Keyword.Span, node->Keyword.value);
        return BindErrorStatement();
    }
    return new BoundGotoStatement(_loopStack.top().second);
}

BoundStatement *Binder::BindReturnStatement(ReturnStatementSyntax *node)
{
    BoundExpression *expression = node->Expression == nullptr ? nullptr : BindExpression(node->Expression);
    if (_function == nullptr)
    {
        _diagnostics.ReportInvalidReturn(node->Keyword.Span);
    }
    else
    {
        if (_function->Type == TypeSymbol::Void)
        {
            if (expression != nullptr)
            {
                _diagnostics.ReportInvalidReturnExpression(node->Keyword.Span, _function->Name);
            }
        }
        else
        {
            if (expression == nullptr)
            {
                _diagnostics.ReportMissingReturnExpression(node->Keyword.Span, _function->Type.ToString());
            }
            else
            {
                expression = BindConversion(node->Expression->Span(), expression, _function->Type);
            }
        }
    }
    return new BoundReturnStatement(expression);
}

BoundStatement *Binder::BindBlockStatement(BlockStatementSyntax *node)
{
    _scope = new BoundScope(_scope);
    std::vector<BoundStatement *> statements;

    for (auto &statement : node->Statements)
    {
        BoundStatement *boundStatement = BindStatement(statement);
        statements.push_back(boundStatement);
    }
    return new BoundBlockStatement(statements);
}

VariableSymbol *Binder::BindVariable(Token identifier, bool isReadOnly, TypeSymbol type)
{
    std::string name = identifier.value;
    bool declare = name != "" || !name.empty();
    VariableSymbol *variable = _function == nullptr
                                   ? static_cast<VariableSymbol *>(new GlobalVariableSymbol(name, isReadOnly, type))
                                   : static_cast<VariableSymbol *>(new LocalVariableSymbol(name, isReadOnly, type));

    if (declare && !_scope->TryDeclareVariable(*variable))
    {
        _diagnostics.ReportSymbolAlreadyDeclared(identifier.Span, name);
    }

    return variable;
}
BoundStatement *Binder::BindExpressionStatement(ExpressionStatementSyntax *node)
{
    BoundExpression *expression = BindExpression(node->Expression, true);
    return new BoundExpressionStatement(expression);
}

BoundExpression *Binder::BindExpression(SyntaxNode *node, TypeSymbol type)
{

    return BindConversion(node, type);
}
BoundExpression *Binder::BindExpression(SyntaxNode *node, bool canBeVoid)
{
    BoundExpression *result = BindExpressionInternal(node);

    if (!canBeVoid && result->type == TypeSymbol::Void)
    {
        _diagnostics.ReportExpressionMustHaveValue(node->Span());
        return new BoundErrorExpression();
    }
    return result;
}

BoundExpression *Binder::BindExpressionInternal(SyntaxNode *node)
{
    switch (node->Kind)
    {
    case SyntaxKind::LiteralExpression:
        return BindLiteralExpression((LiteralExpressionNode *)node);
    case SyntaxKind::NameExpression:
        return BindNameExpression((NameExpressionNode *)node);
    case SyntaxKind::AssignmentExpression:
        return BindAssignmentExpression((AssignmentExpressionNode *)node);
    case SyntaxKind::UnaryExpression:
        return BindUnaryExpression((UnaryExpressionNode *)node);
    case SyntaxKind::BinaryExpression:
        return BindBinaryExpression((BinaryExpressionNode *)node);
    case SyntaxKind::ParenthesizedExpression:
        return BindExpression(((ParenthesizedExpressionNode *)node)->expression);
    case SyntaxKind::CallExpression:
        return BindCallExpression((CallExpressionNode *)node);
    default:
        std::cerr << "Unexpected syntax kind: {" << convertSyntaxKindToString(node->Kind) << "}" << std::endl;
        return nullptr;
    }
}

BoundExpression *Binder::BindLiteralExpression(LiteralExpressionNode *node)
{

    switch (node->LiteralToken.Kind)
    {
    case SyntaxKind::TRUE_KEYWORD:
    case SyntaxKind::FALSE_KEYWORD:
        return new BoundLiteralExpression(node->LiteralToken.value, TypeSymbol::Boolean);
    case SyntaxKind::NUMBER:
        return new BoundLiteralExpression(node->LiteralToken.value, TypeSymbol::Integer);
    case SyntaxKind::STRING:
        return new BoundLiteralExpression(node->LiteralToken.value, TypeSymbol::String);
    default:
        _diagnostics.ReportUnexpectedToken(node->LiteralToken.Span, convertSyntaxKindToString(node->LiteralToken.Kind), "Literal Expression");
        return new BoundErrorExpression();
    }
}
BoundExpression *Binder::BindNameExpression(NameExpressionNode *node)
{
    std::string name = node->IdentifierToken.value;
    if (name.empty())
    {

        return new BoundErrorExpression();
    }
    VariableSymbol variable(name, false, TypeSymbol::Error);
    if (!_scope->TryLookupVariable(name, variable))
    {
        _diagnostics.ReportUndefinedName(node->IdentifierToken.Span, name);
        return new BoundErrorExpression();
    }
    return new BoundVariableExpression(variable);
}

BoundExpression *Binder::BindAssignmentExpression(AssignmentExpressionNode *node)
{
    std::string name = node->IdentifierToken.value;
    BoundExpression *boundExpression = BindExpression(node->Expression);
    VariableSymbol variable(name, false, TypeSymbol::Error);
    if (!_scope->TryLookupVariable(name, variable))
    {
        _diagnostics.ReportUndefinedName(node->IdentifierToken.Span, name);
        return boundExpression;
    }
    if (variable.IsReadOnly)
    {
        _diagnostics.ReportCannotAssign(node->IdentifierToken.Span, name);
    }

    BoundExpression *convertedExpression = BindConversion(node->Expression->Span(), boundExpression, variable.Type);

    return new BoundAssignmentExpression(variable, convertedExpression);
}
BoundExpression *Binder::BindUnaryExpression(UnaryExpressionNode *node)
{
    BoundExpression *boundOperand = BindExpression(node->expression);

    if (boundOperand->type == TypeSymbol::Error)
    {
        return new BoundErrorExpression();
    }
    BoundUnaryOperator *boundOperator = BoundUnaryOperator::Bind(node->OperatorToken.Kind, boundOperand->type);
    if (boundOperator == nullptr)
    {
        _diagnostics.ReportUndefinedUnaryOperator(node->OperatorToken.Span, node->OperatorToken.value, boundOperand->type.ToString());
        return new BoundErrorExpression();
    }
    return new BoundUnaryExpression(boundOperator, boundOperand);
}

BoundExpression *Binder::BindBinaryExpression(BinaryExpressionNode *node)
{
    BoundExpression *boundLeft = BindExpression(node->left);
    BoundExpression *boundRight = BindExpression(node->right);

    if (boundLeft->type == TypeSymbol::Error || boundRight->type == TypeSymbol::Error)
    {
        return new BoundErrorExpression();
    }
    BoundBinaryOperator *boundOperator = BoundBinaryOperator::Bind(node->OperatorToken.Kind, boundLeft->type, boundRight->type);

    if (boundOperator == nullptr)
    {

        _diagnostics.ReportUndefinedBinaryOperator(node->OperatorToken.Span, node->OperatorToken.value, boundLeft->type.ToString(), boundRight->type.ToString());
        return new BoundErrorExpression();
    }

    return new BoundBinaryExpression(boundLeft, boundOperator, boundRight);
}

BoundExpression *Binder::BindCallExpression(CallExpressionNode *node)
{
    if (node->Arguments.Count() == 1)
    {
        TypeSymbol type = LookupType(node->IdentifierToken.value);
        if (type != TypeSymbol::Null)
            return BindConversion(node->Arguments[0], type, true);
    }

    std::vector<BoundExpression *> boundArguments;

    for (auto argument : node->Arguments)
    {
        boundArguments.push_back(BindExpression(argument));
    }

    FunctionSymbol function;
    if (!_scope->TryLookupFunction(node->IdentifierToken.value, function))
    {
        _diagnostics.ReportUndefinedFunction(node->IdentifierToken.Span, node->IdentifierToken.value);
        return new BoundErrorExpression();
    }

    if (node->Arguments.Count() != function.Parameters.size())
    {
        TextSpan Span(0, 0);
        if (node->Arguments.Count() > function.Parameters.size())
        {
            TextSpan firstExceedingNode(0, 0);
            if (function.Parameters.size() > 0)
            {
                firstExceedingNode = node->Arguments.GetSeparator(function.Parameters.size() - 1)->Span;
            }
            else
            {
                firstExceedingNode = node->Arguments[0]->Span();
            }

            Span = TextSpan::FromBounds(firstExceedingNode.Start, node->CloseParenthesisToken.Span.End);
        }
        else
        {
            Span = node->CloseParenthesisToken.Span;
        }
        _diagnostics.ReportWrongArgumentCount(Span, function.Name, function.Parameters.size(), node->Arguments.Count());
        return new BoundErrorExpression();
    }

    bool hasErrors = false;

    for (int i = 0; i < node->Arguments.Count(); i++)
    {
        BoundExpression *argument = boundArguments[i];
        ParameterSymbol parameter = function.Parameters[i];
        if (argument->type != parameter.Type)
        {
            if (argument->type != TypeSymbol::Error)
            {
                _diagnostics.ReportWrongArgumentType(node->Arguments[i]->Span(), parameter.Name, parameter.Type.ToString(), argument->type.ToString());
            }

            hasErrors = true;
        }
    }

    if (hasErrors)
        return new BoundErrorExpression();
    return new BoundCallExpression(function, boundArguments);
}
BoundGlobalScope Binder::BindGlobalScope(BoundGlobalScope *previous, CompilationUnitNode *tree)
{
    BoundScope *parentScope = Binder::CreateParentScope(previous);
    Binder binder(parentScope, nullptr);

    std::vector<BoundStatement *> statements;
    for (auto *member : tree->Members)
    {

        if (auto *function = dynamic_cast<FunctionDeclarationSyntax *>(member))
        {
            binder.BindFunctionDeclaration(function);
        }
        if (auto *memberStatement = dynamic_cast<GlobalStatementSyntax *>(member))
        {
            BoundStatement *statement = binder.BindStatement(memberStatement->Statement);
            statements.push_back(statement);
        }
    }
    std::vector<FunctionSymbol> functions = binder._scope->GetDeclaredFunctions();
    std::vector<VariableSymbol> variables = binder._scope->GetDeclaredVariables();
    std::vector<Diagnostic> diagnostics = binder.GetDiagnostics().GetDiagnostics();

    if (previous != nullptr)
    {
        diagnostics.insert(diagnostics.begin(), previous->Diagnostics.begin(), previous->Diagnostics.end());
    }
    return BoundGlobalScope(previous, diagnostics, variables, functions, statements);
}

BoundProgram *Binder::BindProgram(BoundGlobalScope *globalScope)
{
    BoundScope parentScope = CreateParentScope(globalScope);
    std::unordered_map<FunctionSymbol, BoundBlockStatement *> functions = {};
    std::vector<Diagnostic> diagnostics = {};

    BoundGlobalScope *scope = globalScope;

    while (scope != nullptr)
    {
        for (auto &function : scope->Functions)
        {
            Binder binder(&parentScope, &function);
            BoundStatement *body = binder.BindStatement(function.Declaration->Body);
            BoundBlockStatement *loweredBody = Lowerer::Lower(body);
            functions[function] = loweredBody;

            diagnostics.insert(diagnostics.end(), binder.GetDiagnostics().GetDiagnostics().begin(), binder.GetDiagnostics().GetDiagnostics().end());
        }

        scope = scope->Previous;
    }

    BoundBlockStatement *statement = Lowerer::Lower(new BoundBlockStatement(globalScope->Statements));
    return new BoundProgram(diagnostics, functions, statement);
}

void Binder::BindFunctionDeclaration(FunctionDeclarationSyntax *node)
{
    std::vector<ParameterSymbol> parameters;
    std::unordered_set<std::string> seenParameterNames;

    for (const auto &parameter : node->Parameters)
    {
        std::string parameterName = parameter->IdentifierToken.value;
        TypeSymbol boundType = BindTypeClause(parameter->Type);
        TypeSymbol parameterType = boundType != TypeSymbol::Null ? boundType : TypeSymbol::Void;

        if (seenParameterNames.find(parameterName) != seenParameterNames.end())
        {
            _diagnostics.ReportParameterAlreadyDeclared(parameter->IdentifierToken.Span, parameterName);
        }
        else
        {
            seenParameterNames.insert(parameterName);
            parameters.push_back(ParameterSymbol(parameterName, parameterType));
        }
    }

    TypeSymbol boundResultType = BindTypeClause(node->Type);
    TypeSymbol returnType = boundResultType != TypeSymbol::Null ? boundResultType : TypeSymbol::Void;

    FunctionSymbol function(node->Identifier.value, parameters, returnType, node);

    if (!_scope->TryDeclareFunction(function))
    {
        _diagnostics.ReportSymbolAlreadyDeclared(node->Identifier.Span, function.Name);
    }
}

BoundScope *Binder::CreateRootScope()
{
    BoundScope *result = new BoundScope(nullptr);
    for (auto &function : BuiltInFunctions::GetAll())
    {
        result->TryDeclareFunction(function);
    }

    return result;
}
BoundScope *Binder::CreateParentScope(BoundGlobalScope *previous)
{
    std::stack<BoundGlobalScope *> stack;
    while (previous != nullptr)
    {
        stack.push(previous);
        previous = previous->Previous;
    }

    BoundScope *parent = CreateRootScope();

    while (!stack.empty())
    {
        previous = stack.top();
        stack.pop();
        BoundScope *scope = new BoundScope(parent);

        for (auto &function : previous->Functions)
        {
            scope->TryDeclareFunction(function);
        }
        for (auto &variable : previous->Variables)
        {
            scope->TryDeclareVariable(variable);
        }
        parent = scope;
    }
    return parent;
}

BoundStatement *Binder::BindVariableDeclaration(VariableDeclarationSyntax *node)
{

    bool isReadOnly = node->Keyword.Kind == SyntaxKind::LET_KEYWORD;
    TypeSymbol type = BindTypeClause(node->TypeClause);
    BoundExpression *initializer = BindExpression(node->Initializer);

    TypeSymbol variableType = type != TypeSymbol::Null ? type : initializer->type;
    VariableSymbol *variable = BindVariable(node->Identifier, isReadOnly, variableType);
    BoundExpression *convertedInitializer = BindConversion(node->Initializer->Span(), initializer, variableType);

    return new BoundVariableDeclaration(*variable, initializer);
}

TypeSymbol Binder::LookupType(std::string name)
{
    if (name == "bool")
    {
        return TypeSymbol::Boolean;
    }
    if (name == "int")
    {
        return TypeSymbol::Integer;
    }
    if (name == "string")
    {
        return TypeSymbol::String;
    }
    return TypeSymbol::Null;
}

TypeSymbol Binder::BindTypeClause(TypeClauseNode *node)
{
    if (node == nullptr)
    {
        return TypeSymbol::Null;
    }
    TypeSymbol type = LookupType(node->IdentifierToken.value);

    if (type == TypeSymbol::Null)
    {
        _diagnostics.ReportUndefinedType(node->IdentifierToken.Span, node->IdentifierToken.value);
    }
    return type;
}
BoundExpression *Binder::BindConversion(SyntaxNode *node, TypeSymbol type, bool allowExplicit)
{
    BoundExpression *expression = BindExpression(node);
    return BindConversion(node->Span(), expression, type, node);
}
BoundExpression *Binder::BindConversion(TextSpan diagnosticSpan, BoundExpression *expression, TypeSymbol type, bool allowExplicit)
{

    Conversion conversion = Conversion::Classify(expression->type, type);
    if (!conversion.Exists)
    {
        if (expression->type != TypeSymbol::Error && type != TypeSymbol::Error)
            _diagnostics.ReportCannotConvert(diagnosticSpan, expression->type.ToString(), type.ToString());
        return new BoundErrorExpression();
    }

    if (!allowExplicit && conversion.IsExplicit)
    {
        _diagnostics.ReportCannotConvertImplicitly(diagnosticSpan, expression->type.ToString(), type.ToString());
    }

    if (conversion.IsIdentity)
    {
        return expression;
    }
    return new BoundConversionExpression(type, expression);
}
// enum class BoundNodeKind
// {
//     LiteralExpression,
//     UnaryExpression,
//     BinaryExpression,
//     ParenthesizedExpression,
//     VariableExpression,
//     AssignmentExpression,

//     ExpressionStatement,
//     VariableDeclaration,
//     BlockStatement,
//     IfStatement,
//     WhileStatement,
//     ForStatement,

//     GotoStatement,
//     LabelStatement,
//     ConditionalGotoStatement,
// };

std::string convertBoundNodeKindToString(BoundNodeKind kind)
{
    switch (kind)
    {
    case BoundNodeKind::VariableDeclaration:
        return "VariableDeclaration";
    case BoundNodeKind::ExpressionStatement:
        return "ExpressionStatement";
    case BoundNodeKind::BlockStatement:
        return "BlockStatement";
    case BoundNodeKind::IfStatement:
        return "IfStatement";
    case BoundNodeKind::WhileStatement:
        return "WhileStatement";
    case BoundNodeKind::ForStatement:
        return "ForStatement";
    case BoundNodeKind::LiteralExpression:
        return "LiteralExpression";
    case BoundNodeKind::VariableExpression:
        return "VariableExpression";
    case BoundNodeKind::AssignmentExpression:
        return "AssignmentExpression";
    case BoundNodeKind::UnaryExpression:
        return "UnaryExpression";
    case BoundNodeKind::BinaryExpression:
        return "BinaryExpression";
    case BoundNodeKind::ParenthesizedExpression:
        return "ParenthesizedExpression";
    case BoundNodeKind::GotoStatement:
        return "GotoStatement";
    case BoundNodeKind::LabelStatement:
        return "LabelStatement";
    case BoundNodeKind::ConditionalGotoStatement:
        return "ConditionalGotoStatement";
    case BoundNodeKind::CallExpression:
        return "CallExpression";
    case BoundNodeKind::ConversionExpression:
        return "ConversionExpression";

    case BoundNodeKind::ErrorExpression:
        return "ErrorExpression";
    default:
        return "Unknown";
    }
}

void BoundNode::PrettyPrint(std::ostream &os, BoundNode *node, std::string indent, bool isLast)
{
    const std::string RESET_COLOR = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string GRAY = "\033[90m";

    os << indent << GRAY << "|--";

    std::string kind = convertBoundNodeKindToString(node->GetKind());

    if (kind.find("Expression") != std::string::npos)
    {
        os << CYAN << " " << kind;
    }
    else if (kind.find("Statement") != std::string::npos)
    {
        os << MAGENTA << " " << kind;
    }
    else
    {
        os << RESET_COLOR << " " << kind;
    }

    bool firstProperty = true;
    for (const auto &prop : node->GetProperties())
    {
        if (!firstProperty)
            os << ",";
        os << YELLOW << " " << prop.first << " = " << GREEN << prop.second;
        firstProperty = false;
    }

    os << GRAY << std::endl;

    std::string childIndent = indent + (isLast ? "   " : "|  ");
    const auto &children = node->GetChildren();
    for (size_t i = 0; i < children.size(); ++i)
    {
        PrettyPrint(os, children[i], childIndent, i == children.size() - 1);
    }
    os << RESET_COLOR;
}

const Conversion Conversion::None = Conversion(false /*exists*/, false /*isIdentity*/, false /*isImplicit*/);
const Conversion Conversion::Identity = Conversion(true /*exists*/, true /*isIdentity*/, true /*isImplicit*/);
const Conversion Conversion::Implicit = Conversion(true /*exists*/, false /*isIdentity*/, true /*isImplicit*/);
const Conversion Conversion::Explicit = Conversion(true /*exists*/, false /*isIdentity*/, false /*isImplicit*/);

Conversion Conversion::Classify(TypeSymbol from, TypeSymbol to)
{
    if (from == to)
    {
        return Conversion::Identity;
    }

    if (from == TypeSymbol::String)
    {
        if (to == TypeSymbol::Integer || to == TypeSymbol::Boolean)
        {
            return Conversion::Explicit;
        }
    }

    if (from == TypeSymbol::Integer || from == TypeSymbol::Boolean)
    {
        if (to == TypeSymbol::String)
        {
            return Conversion::Explicit;
        }
    }

    if (from == TypeSymbol::Error || to == TypeSymbol::Error)
    {
        return Conversion::None;
    }
    return Conversion::None;
}