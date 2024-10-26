#include "CodeAnalysis/Evaluator.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>
std::any Evaluator::EvaluateExpression(BoundExpression *node)
{
    if (auto n = dynamic_cast<BoundLiteralExpression *>(node))
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
    if (auto v = dynamic_cast<BoundVariableExpression *>(node))
    {
        return _variables[v->Name];
    }

    if (auto a = dynamic_cast<BoundAssignmentExpression *>(node))
    {
        auto value = EvaluateExpression(a->Expression);
        _variables[a->Name] = value;
        return value;
    }
    if (auto u = dynamic_cast<BoundUnaryExpression *>(node))
    {
        auto operand = EvaluateExpression(u->Operand);

        switch (u->Op->Kind)
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

    if (auto b = dynamic_cast<BoundBinaryExpression *>(node))
    {
        auto left = EvaluateExpression(b->Left);
        auto right = EvaluateExpression(b->Right);

        switch (b->Op->Kind)
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

    throw std::runtime_error("Unexpected node kind");
}