#include "CodeAnalysis/Binder.h"
#include "CodeAnalysis/SyntaxTree.h"
#include "Utils.h"
#include <iostream>
#include <algorithm>

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
    auto it = std::find_if(_variables.begin(), _variables.end(),
                           [&name](const auto &variable)
                           {
                               return variable.first.Name == name;
                           });
    if (it == _variables.end())
    {
        _diagnostics.ReportUndefinedName(node->IdentifierToken.Span, name);
        return new BoundLiteralExpression("0", Type::Integer);
    }
    return new BoundVariableExpression(it->first);
}

BoundExpression *Binder::BindAssignmentExpression(AssignmentExpressionNode *node)
{
    std::string name = node->IdentifierToken.value;
    BoundExpression *boundExpression = BindExpression(node->Expression);
    auto existingVariable = std::find_if(_variables.begin(), _variables.end(),
                                         [&name](const auto &variable)
                                         {
                                             return variable.first.Name == name;
                                         });
    if (existingVariable != _variables.end())
    {
        _variables.erase(existingVariable);
    }
    VariableSymbol variable = VariableSymbol(name, boundExpression->type);

    _variables[variable] = NULL;
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
    return new BoundBinaryExpression(boundLeft, boundOperator, boundRight);
}