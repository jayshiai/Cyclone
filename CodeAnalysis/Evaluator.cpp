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
    case BoundNodeKind::IfStatement:
        return "IfStatement";
    case BoundNodeKind::WhileStatement:
        return "WhileStatement";
    case BoundNodeKind::ForStatement:
        return "ForStatement";
    default:
        return "Unknown";
    }
}

std::any Evaluator::Evaluate()
{
    std::unordered_map<BoundLabel, int> labelToIndex;

    for (int i = 0; i < _root->Statements.size(); i++)
    {
        if (_root->Statements[i]->GetKind() == BoundNodeKind::LabelStatement)
        {
            BoundLabelStatement *label = static_cast<BoundLabelStatement *>(_root->Statements[i]);
            labelToIndex[label->Label] = i + 1;
        }
    }

    int index = 0;
    while (index < _root->Statements.size())
    {
        BoundStatement *s = _root->Statements[index];

        switch (s->GetKind())
        {
        case BoundNodeKind::VariableDeclaration:
            EvaluateVariableDeclaration((BoundVariableDeclaration *)s);
            index++;
            break;
        case BoundNodeKind::ExpressionStatement:
            EvaluateExpressionStatement((BoundExpressionStatement *)s);
            index++;
            break;
        case BoundNodeKind::GotoStatement:
        {
            BoundGotoStatement *gs = (BoundGotoStatement *)s;
            index = labelToIndex[gs->Label];
            break;
        }
        case BoundNodeKind::ConditionalGotoStatement:
        {
            BoundConditionalGotoStatement *cgs = (BoundConditionalGotoStatement *)s;
            bool condition = std::any_cast<bool>(EvaluateExpression(cgs->Condition));
            if ((condition == cgs->JumpIfTrue))
            {
                index = labelToIndex[cgs->Label];
            }
            else
            {
                index++;
            }
            break;
        }
        case BoundNodeKind::LabelStatement:
            index++;
            break;
        default:
            throw std::runtime_error("Unexpected node kind");
        }
    }

    return _lastValue;
}

void Evaluator::EvaluateVariableDeclaration(BoundVariableDeclaration *node)
{
    std::any value = EvaluateExpression(node->Initializer);
    _variables[node->Variable] = value;
    _lastValue = value;
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
        return EvaluateBinaryExpression((BoundBinaryExpression *)node);
    default:
        throw std::runtime_error("Unexpected node kind");
    }
}

std::any Evaluator::EvaluateLiteralExpression(BoundLiteralExpression *n)
{
    if (n->type == TypeSymbol::Boolean)
        return n->Value == "true";
    if (n->type == TypeSymbol::String)
        return n->Value;
    return std::stoi(n->Value);
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
    case BoundUnaryOperatorKind::OnesComplement:
        return ~std::any_cast<int>(operand);
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
    case BoundBinaryOperatorKind::BitwiseAnd:
        if (n->Op->LeftType == TypeSymbol::Boolean && n->Op->RightType == TypeSymbol::Boolean)
        {
            return std::any_cast<bool>(left) & std::any_cast<bool>(right);
        }
        return std::any_cast<int>(left) & std::any_cast<int>(right);
    case BoundBinaryOperatorKind::BitwiseOr:
        if (n->Op->LeftType == TypeSymbol::Boolean && n->Op->RightType == TypeSymbol::Boolean)
            return std::any_cast<bool>(left) | std::any_cast<bool>(right);
        return std::any_cast<int>(left) | std::any_cast<int>(right);
    case BoundBinaryOperatorKind::BitwiseXor:
        if (n->Op->LeftType == TypeSymbol::Boolean && n->Op->RightType == TypeSymbol::Boolean)
            return std::any_cast<bool>(left) ^ std::any_cast<bool>(right);
        return std::any_cast<int>(left) ^ std::any_cast<int>(right);
    case BoundBinaryOperatorKind::LogicalAnd:
        return std::any_cast<bool>(left) && std::any_cast<bool>(right);
    case BoundBinaryOperatorKind::LogicalOr:
        return std::any_cast<bool>(left) || std::any_cast<bool>(right);
    case BoundBinaryOperatorKind::Less:
        return std::any_cast<int>(left) < std::any_cast<int>(right);
    case BoundBinaryOperatorKind::LessOrEquals:
        return std::any_cast<int>(left) <= std::any_cast<int>(right);
    case BoundBinaryOperatorKind::Greater:
        return std::any_cast<int>(left) > std::any_cast<int>(right);
    case BoundBinaryOperatorKind::GreaterOrEquals:
        return std::any_cast<int>(left) >= std::any_cast<int>(right);
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