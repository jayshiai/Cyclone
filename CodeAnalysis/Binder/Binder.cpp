#include "CodeAnalysis/Binder.h"
#include "CodeAnalysis/SyntaxTree.h"
#include "Utils.h"
#include <iostream>
#include <algorithm>
#include <stack>

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
    default:
        std::cerr << "Unexpected syntax kind: {" << convertSyntaxKindToString(node->Kind) << "}" << std::endl;
        return nullptr;
    }
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
    BoundStatement *body = BindStatement(node->Body);
    return new BoundWhileStatement(condition, body);
}

BoundStatement *Binder::BindForStatement(ForStatementSyntax *node)
{
    BoundExpression *lowerBound = BindExpression(node->LowerBound, TypeSymbol::Integer);
    BoundExpression *upperBound = BindExpression(node->UpperBound, TypeSymbol::Integer);
    _scope = new BoundScope(_scope);
    VariableSymbol *variable = BindVariable(node->Identifier, false, TypeSymbol::Integer);
    BoundStatement *body = BindStatement(node->Body);
    _scope = _scope->Parent;
    return new BoundForStatement(*variable, lowerBound, upperBound, body);
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

BoundStatement *Binder::BindVariableDeclaration(VariableDeclarationSyntax *node)
{

    bool isReadOnly = node->Keyword.Kind == SyntaxKind::LET_KEYWORD;
    BoundExpression *initializer = BindExpression(node->Initializer);
    VariableSymbol *variable = BindVariable(node->Identifier, isReadOnly, initializer->type);

    return new BoundVariableDeclaration(*variable, initializer);
}

VariableSymbol *Binder::BindVariable(Token identifier, bool isReadOnly, TypeSymbol type)
{
    std::string name = identifier.value;
    bool declare = name != "" || !name.empty();
    VariableSymbol *variable = new VariableSymbol(name, isReadOnly, type);

    if (declare && !_scope->TryDeclare(*variable))
    {
        _diagnostics.ReportVariableAlreadyDeclared(identifier.Span, name);
    }

    return variable;
}
BoundStatement *Binder::BindExpressionStatement(ExpressionStatementSyntax *node)
{
    BoundExpression *expression = BindExpression(node->Expression);
    return new BoundExpressionStatement(expression);
}

BoundExpression *Binder::BindExpression(SyntaxNode *node, TypeSymbol type)
{
    BoundExpression *result = BindExpression(node);
    if (type != TypeSymbol::Error && result->type != type && result->type != TypeSymbol::Error)
    {
        _diagnostics.ReportCannotConvert(node->Span(), result->type.ToString(), type.ToString());
    }
    return result;
}
BoundExpression *Binder::BindExpression(SyntaxNode *node)
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
    default:
        _diagnostics.ReportUnexpectedToken(node->LiteralToken.Span, convertSyntaxKindToString(node->LiteralToken.Kind), "Literal Expression");
        return nullptr;
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
    if (!_scope->TryLookup(name, variable))
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
    if (!_scope->TryLookup(name, variable))
    {
        _diagnostics.ReportUndefinedName(node->IdentifierToken.Span, name);
        return boundExpression;
    }
    if (variable.IsReadOnly)
    {
        _diagnostics.ReportCannotAssign(node->IdentifierToken.Span, name);
    }

    if (boundExpression->type != variable.Type)
    {
        _diagnostics.ReportCannotConvert(node->Expression->Span(), boundExpression->type.ToString(), variable.Type.ToString());
    }

    return new BoundAssignmentExpression(variable, boundExpression);
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

BoundGlobalScope Binder::BindGlobalScope(BoundGlobalScope *previous, CompilationUnitNode *tree)
{
    BoundScope *parentScope = Binder::CreateParentScope(previous);
    Binder binder(parentScope);

    BoundStatement *statement = binder.BindStatement(tree->Statement);
    std::vector<VariableSymbol> variables = binder._scope->GetDeclaredVariables();
    std::vector<Diagnostic> diagnostics = binder.GetDiagnostics().GetDiagnostics();

    if (previous != nullptr)
    {
        diagnostics.insert(diagnostics.begin(), previous->Diagnostics.begin(), previous->Diagnostics.end());
    }
    return BoundGlobalScope(previous, diagnostics, variables, statement);
}

BoundScope *Binder::CreateParentScope(BoundGlobalScope *previous)
{
    std::stack<BoundGlobalScope *> stack;
    while (previous != nullptr)
    {
        stack.push(previous);
        previous = previous->Previous;
    }

    BoundScope *parent = nullptr;

    while (!stack.empty())
    {
        previous = stack.top();
        stack.pop();
        BoundScope *scope = new BoundScope(parent);
        for (auto &variable : previous->Variables)
        {
            scope->TryDeclare(variable);
        }
        parent = scope;
    }
    return parent;
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
