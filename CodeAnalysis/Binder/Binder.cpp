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
    default:
        std::cerr << "Unexpected syntax kind: {" << convertSyntaxKindToString(node->Kind) << "}" << std::endl;
        return nullptr;
    }
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
    std::string name = node->Identifier.value;
    bool isReadOnly = node->Keyword.Kind == SyntaxKind::LET_KEYWORD;
    BoundExpression *initializer = BindExpression(node->Initializer);
    VariableSymbol variable = VariableSymbol(name, isReadOnly, initializer->type);
    if (!_scope->TryDeclare(variable))
    {
        _diagnostics.ReportVariableAlreadyDeclared(node->Identifier.Span, name);
    }

    return new BoundVariableDeclaration(variable, initializer);
}

BoundStatement *Binder::BindExpressionStatement(ExpressionStatementSyntax *node)
{
    BoundExpression *expression = BindExpression(node->Expression);
    return new BoundExpressionStatement(expression);
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
    case SyntaxKind::TRUE:
    case SyntaxKind::FALSE:
        return new BoundLiteralExpression(node->LiteralToken.value, Type::Boolean);
    case SyntaxKind::NUMBER:
        return new BoundLiteralExpression(node->LiteralToken.value, Type::Integer);
    default:
        _diagnostics.ReportUnexpectedToken(node->LiteralToken.Span, convertSyntaxKindToString(node->LiteralToken.Kind), "Literal Expression");
        return nullptr;
    }
}
BoundExpression *Binder::BindNameExpression(NameExpressionNode *node)
{
    std::string name = node->IdentifierToken.value;
    std::cout << "Binding name expression: " << name << std::endl;
    VariableSymbol variable(name, false, Type::Unknown);
    if (!_scope->TryLookup(name, variable))
    {
        _diagnostics.ReportUndefinedName(node->IdentifierToken.Span, name);
        return new BoundLiteralExpression("0", Type::Integer);
    }
    return new BoundVariableExpression(variable);
}

BoundExpression *Binder::BindAssignmentExpression(AssignmentExpressionNode *node)
{
    std::string name = node->IdentifierToken.value;
    BoundExpression *boundExpression = BindExpression(node->Expression);
    VariableSymbol variable(name, false, Type::Unknown);
    if (!_scope->TryLookup(name, variable))
    {
        _diagnostics.ReportUndefinedName(node->IdentifierToken.Span, name);
        return boundExpression;
    }
    if (variable.IsReadOnly)
    {
        _diagnostics.ReportCannotAssign(node->IdentifierToken.Span, name);
    }

    if (boundExpression->type != variable.type)
    {
        _diagnostics.ReportCannotConvert(node->Expression->Span(), convertTypetoString(boundExpression->type), convertTypetoString(variable.type));
    }

    return new BoundAssignmentExpression(variable, boundExpression);
}
BoundExpression *Binder::BindUnaryExpression(UnaryExpressionNode *node)
{
    BoundExpression *boundOperand = BindExpression(node->expression);
    BoundUnaryOperator *boundOperator = BoundUnaryOperator::Bind(node->OperatorToken.Kind, boundOperand->type);
    if (boundOperator == nullptr)
    {
        _diagnostics.ReportUndefinedUnaryOperator(node->OperatorToken.Span, node->OperatorToken.value, convertTypetoString(boundOperand->type));
        return boundOperand;
    }
    return new BoundUnaryExpression(boundOperator, boundOperand);
}

BoundExpression *Binder::BindBinaryExpression(BinaryExpressionNode *node)
{
    BoundExpression *boundLeft = BindExpression(node->left);
    BoundExpression *boundRight = BindExpression(node->right);
    BoundBinaryOperator *boundOperator = BoundBinaryOperator::Bind(node->OperatorToken.Kind, boundLeft->type, boundRight->type);

    if (boundOperator == nullptr)
    {

        _diagnostics.ReportUndefinedBinaryOperator(node->OperatorToken.Span, node->OperatorToken.value, convertTypetoString(boundLeft->type), convertTypetoString(boundRight->type));
        return boundLeft;
    }
    BoundBinaryExpression *bi = new BoundBinaryExpression(boundLeft, boundOperator, boundRight);
    return bi;
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