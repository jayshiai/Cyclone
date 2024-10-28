#include "CodeAnalysis/Evaluator.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>

std::string convertBoundNodeKind(BoundNodeKind kind)
{
    switch (kind)
    {
    case BoundNodeKind::LiteralExpression:
        return "LiteralExpression";
    case BoundNodeKind::UnaryExpression:
        return "UnaryExpression";
    case BoundNodeKind::BinaryExpression:
        return "BinaryExpression";
    case BoundNodeKind::ParenthesizedExpression:
        return "ParenthesizedExpression";
    case BoundNodeKind::VariableExpression:
        return "VariableExpression";
    case BoundNodeKind::AssignmentExpression:
        return "AssignmentExpression";
    case BoundNodeKind::ExpressionStatement:
        return "ExpressionStatement";
    case BoundNodeKind::VariableDeclaration:
        return "VariableDeclaration";
    case BoundNodeKind::BlockStatement:
        return "BlockStatement";
    default:
        return "Unknown";
    }
}
void Evaluator::EvaluateStatement(BoundStatement *node)
{
    switch (node->GetKind())
    {
    case BoundNodeKind::BlockStatement:
        EvaluateBlockStatement((BoundBlockStatement *)node);
        break;
    case BoundNodeKind::ExpressionStatement:
        EvaluateExpressionStatement((BoundExpressionStatement *)node);
        break;
    case BoundNodeKind::VariableDeclaration:
        EvaluateVariableDeclaration((BoundVariableDeclaration *)node);
        break;
    default:
        throw std::runtime_error("Unexpected node kind::Statement");
    }
}

void Evaluator::EvaluateVariableDeclaration(BoundVariableDeclaration *node)
{
    std::any value = EvaluateExpression(node->Initializer);
    _variables[node->Variable] = value;
    _lastValue = value;
}

void Evaluator::EvaluateBlockStatement(BoundBlockStatement *node)
{
    for (auto &statement : node->Statements)
    {
        EvaluateStatement(statement);
    }
}

void Evaluator::EvaluateExpressionStatement(BoundExpressionStatement *node)
{

    _lastValue = EvaluateExpression(node->Expression);
}

std::any Evaluator::EvaluateExpression(BoundExpression *node)
{

    switch (node->GetKind())
    {
    case BoundNodeKind::LiteralExpression:
        return EvaluateLiteralExpression((BoundLiteralExpression *)node);
    case BoundNodeKind::VariableExpression:
        return EvaluateVariableExpression((BoundVariableExpression *)node);
    case BoundNodeKind::AssignmentExpression:
        return EvaluateAssignmentExpression((BoundAssignmentExpression *)node);
    case BoundNodeKind::UnaryExpression:
        return EvaluateUnaryExpression((BoundUnaryExpression *)node);
    case BoundNodeKind::BinaryExpression:
        std::cout << "WOWO" << std::endl;
        return EvaluateBinaryExpression((BoundBinaryExpression *)node);
    default:
        throw std::runtime_error("Unexpected node kind");
    }
}

std::any Evaluator::EvaluateLiteralExpression(BoundLiteralExpression *n)
{
    if (n->Value == "true")
    {
        return std::any(true);
    }
    else if (n->Value == "false")
        return std::any(false);
    else
        return std::any(std::stoi(n->Value));
}

std::any Evaluator::EvaluateVariableExpression(BoundVariableExpression *n)
{
    return _variables[n->Variable];
}

std::any Evaluator::EvaluateAssignmentExpression(BoundAssignmentExpression *n)
{
    auto value = EvaluateExpression(n->Expression);
    _variables[n->Variable] = value;
    return value;
}

std::any Evaluator::EvaluateUnaryExpression(BoundUnaryExpression *n)
{
    auto operand = EvaluateExpression(n->Operand);

    switch (n->Op->Kind)
    {
    case BoundUnaryOperatorKind::Identity:
        return std::any_cast<int>(operand);
    case BoundUnaryOperatorKind::Negation:
        return -std::any_cast<int>(operand);
    case BoundUnaryOperatorKind::LogicalNegation:
        return !std::any_cast<bool>(operand);
    default:
        throw std::runtime_error("Unexpected unary operator");
    }
}

std::any Evaluator::EvaluateBinaryExpression(BoundBinaryExpression *n)
{
    auto left = EvaluateExpression(n->Left);
    auto right = EvaluateExpression(n->Right);

    switch (n->Op->Kind)
    {
    case BoundBinaryOperatorKind::Addition:
        return std::any_cast<int>(left) + std::any_cast<int>(right);
    case BoundBinaryOperatorKind::Subtraction:
        return std::any_cast<int>(left) - std::any_cast<int>(right);
    case BoundBinaryOperatorKind::Multiplication:
        return std::any_cast<int>(left) * std::any_cast<int>(right);
    case BoundBinaryOperatorKind::Division:
        return std::any_cast<int>(left) / std::any_cast<int>(right);
    case BoundBinaryOperatorKind::LogicalAnd:
        return std::any_cast<bool>(left) && std::any_cast<bool>(right);
    case BoundBinaryOperatorKind::LogicalOr:
        return std::any_cast<bool>(left) || std::any_cast<bool>(right);
    case BoundBinaryOperatorKind::Equals:
        if (left.type() == typeid(int) && right.type() == typeid(int))
            return std::any_cast<int>(left) == std::any_cast<int>(right);
        if (left.type() == typeid(bool) && right.type() == typeid(bool))
            return std::any_cast<bool>(left) == std::any_cast<bool>(right);
        throw std::runtime_error("Unexpected types for equality comparison");
    case BoundBinaryOperatorKind::NotEquals:
        if (left.type() == typeid(int) && right.type() == typeid(int))
            return std::any_cast<int>(left) != std::any_cast<int>(right);
        if (left.type() == typeid(bool) && right.type() == typeid(bool))
            return std::any_cast<bool>(left) != std::any_cast<bool>(right);
        throw std::runtime_error("Unexpected types for inequality comparison");
    default:
        throw std::runtime_error("Unexpected binary operator");
    }
}