#include "CodeAnalysis/Evaluator.h"
#include "CodeAnalysis/Symbol.h"
#include "Utils.h"
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
    case BoundNodeKind::GotoStatement:
        return "GotoStatement";
    case BoundNodeKind::LabelStatement:
        return "LabelStatement";
    case BoundNodeKind::ConditionalGotoStatement:
        return "ConditionalGotoStatement";
    case BoundNodeKind::ReturnStatement:
        return "ReturnStatement";
    default:
        return "Unknown";
    }
}

std::any Evaluator::EvaluateStatement(BoundBlockStatement *body)
{
    std::unordered_map<BoundLabel, long long> labelToIndex;

    for (long long i = 0; i < body->Statements.size(); i++)
    {
        if (body->Statements[i]->GetKind() == BoundNodeKind::LabelStatement)
        {
            BoundLabelStatement *label = static_cast<BoundLabelStatement *>(body->Statements[i]);
            labelToIndex[label->Label] = i + 1;
        }
    }

    long long index = 0;
    while (index < body->Statements.size())
    {
        BoundStatement *s = body->Statements[index];

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
        case BoundNodeKind::ReturnStatement:
        {
            BoundReturnStatement *rs = (BoundReturnStatement *)s;
            _lastValue = rs->Expression == nullptr ? std::any() : EvaluateExpression(rs->Expression);
            return _lastValue;
        }
        case BoundNodeKind::BlockStatement:
        {
            BoundBlockStatement *block = (BoundBlockStatement *)s;
            std::any value = EvaluateStatement(block);
            if (value.has_value())
            {
                return value;
            }
            index++;
            break;
        }
        default:
            throw std::runtime_error("Unexpected node kind2: " + convertBoundNodeKindToString(s->GetKind()));
        }
    }

    return _lastValue;
}

void Evaluator::EvaluateVariableDeclaration(BoundVariableDeclaration *node)
{
    std::any value = EvaluateExpression(node->Initializer);
    _lastValue = value;
    Assign(node->Variable, value);
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
    case BoundNodeKind::CallExpression:
        return EvaluateCallExpression((BoundCallExpression *)node);
    case BoundNodeKind::ConversionExpression:
        return EvaluateConversionExpression((BoundConversionExpression *)node);
    case BoundNodeKind::ArrayInitializerExpression:
        return EvaluateArrayInitializerExpression((BoundArrayInitializerExpression *)node);
    case BoundNodeKind::ArrayAccessExpression:
        return EvaluateArrayAccessExpression((BoundArrayAccessExpression *)node);
    case BoundNodeKind::ArrayAssignmentExpression:
        return EvaluateArrayAssignmentExpression((BoundArrayAssignmentExpression *)node);
    default:
        throw std::runtime_error("Unexpected node kind1: " + convertBoundNodeKindToString(node->GetKind()));
    }
}

std::any Evaluator::EvaluateLiteralExpression(BoundLiteralExpression *n)
{
    if (n->type == TypeSymbol::Boolean)
        return std::any(n->Value == "true");
    if (n->type == TypeSymbol::String)
        return std::any(n->Value);
    if (n->type == TypeSymbol::Float)
        return std::any(std::stod(n->Value));

    return std::any(std::stoll(n->Value));
}

std::any Evaluator::EvaluateVariableExpression(BoundVariableExpression *n)
{

    if (n->Variable.GetKind() == SymbolKind::GlobalVariable)
    {

        return _globals[n->Variable];
    }
    else if (n->Variable.GetKind() == SymbolKind::LocalVariable)
    {
        std::unordered_map<VariableSymbol, std::any> locals = _locals.top();
        return locals[n->Variable];
    }
    else
    {
        throw std::runtime_error("Unexpected variable kind");
    }
}

std::any Evaluator::EvaluateAssignmentExpression(BoundAssignmentExpression *n)
{
    std::any value = EvaluateExpression(n->Expression);
    Assign(n->Variable, value);
    return value;
}

std::any Evaluator::EvaluateUnaryExpression(BoundUnaryExpression *n)
{
    auto operand = EvaluateExpression(n->Operand);

    switch (n->Op->Kind)
    {
    case BoundUnaryOperatorKind::Identity:
        if (n->Op->OperandType == TypeSymbol::Integer)
            return std::any_cast<long long>(operand);
        else if (n->Op->OperandType == TypeSymbol::Float)
            return std::any_cast<double>(operand);
        else
            return std::any_cast<std::string>(operand);
    case BoundUnaryOperatorKind::Negation:
        if (n->Op->OperandType == TypeSymbol::Integer)
            return -std::any_cast<long long>(operand);
        else if (n->Op->OperandType == TypeSymbol::Float)
            return -std::any_cast<double>(operand);
        else
            throw std::runtime_error("Unexpected unary operator");
    case BoundUnaryOperatorKind::LogicalNegation:
        return !std::any_cast<bool>(operand);
    case BoundUnaryOperatorKind::OnesComplement:
        return ~std::any_cast<long long>(operand);
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
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::Integer)
            return std::any_cast<long long>(left) + std::any_cast<long long>(right);
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::Float)
            return std::any_cast<long long>(left) + std::any_cast<double>(right);
        if (n->Op->LeftType == TypeSymbol::Float && n->Op->RightType == TypeSymbol::Integer)
            return std::any_cast<double>(left) + std::any_cast<long long>(right);
        if (n->Op->LeftType == TypeSymbol::String && n->Op->RightType == TypeSymbol::String)
            return std::any_cast<std::string>(left) + std::any_cast<std::string>(right);
        if (n->Op->LeftType == TypeSymbol::String && n->Op->RightType == TypeSymbol::Integer)
            return std::any_cast<std::string>(left) + std::to_string(std::any_cast<long long>(right));
        if (n->Op->LeftType == TypeSymbol::String && n->Op->RightType == TypeSymbol::Float)
            return std::any_cast<std::string>(left) + std::to_string(std::any_cast<double>(right));
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::String)
            return std::to_string(std::any_cast<long long>(left)) + std::any_cast<std::string>(right);
        if (n->Op->LeftType == TypeSymbol::Float && n->Op->RightType == TypeSymbol::String)
            return std::to_string(std::any_cast<double>(left)) + std::any_cast<std::string>(right);
        return std::any_cast<double>(left) + std::any_cast<double>(right);
    case BoundBinaryOperatorKind::Subtraction:
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::Integer)
        {
            if (left.type() == typeid(long long) && right.type() == typeid(long long))
                return std::any_cast<long long>(left) - std::any_cast<long long>(right);
            if (left.type() == typeid(int) && right.type() == typeid(int))
                return (long long)std::any_cast<int>(left) - std::any_cast<int>(right);
            if (left.type() == typeid(long long) && right.type() == typeid(int))
                return (long long)std::any_cast<long long>(left) - std::any_cast<int>(right);
            if (left.type() == typeid(int) && right.type() == typeid(long long))
                return (long long)std::any_cast<int>(left) - std::any_cast<long long>(right);
        }
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::Float)
            return std::any_cast<long long>(left) - std::any_cast<double>(right);
        if (n->Op->LeftType == TypeSymbol::Float && n->Op->RightType == TypeSymbol::Integer)
            return std::any_cast<double>(left) - std::any_cast<long long>(right);
        return std::any_cast<double>(left) - std::any_cast<double>(right);
    case BoundBinaryOperatorKind::Multiplication:
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::Integer)
            return std::any_cast<long long>(left) * std::any_cast<long long>(right);
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::Float)
            return std::any_cast<long long>(left) * std::any_cast<double>(right);
        if (n->Op->LeftType == TypeSymbol::Float && n->Op->RightType == TypeSymbol::Integer)
            return std::any_cast<double>(left) * std::any_cast<long long>(right);
        return std::any_cast<double>(left) * std::any_cast<double>(right);
    case BoundBinaryOperatorKind::Division:
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::Integer)
            return std::any_cast<long long>(left) / std::any_cast<long long>(right);
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::Float)
            return std::any_cast<long long>(left) / std::any_cast<double>(right);
        if (n->Op->LeftType == TypeSymbol::Float && n->Op->RightType == TypeSymbol::Integer)
            return std::any_cast<double>(left) / std::any_cast<long long>(right);
        return std::any_cast<double>(left) / std::any_cast<double>(right);
    case BoundBinaryOperatorKind::BitwiseAnd:
        if (n->Op->LeftType == TypeSymbol::Boolean && n->Op->RightType == TypeSymbol::Boolean)
        {
            return std::any_cast<bool>(left) & std::any_cast<bool>(right);
        }
        return std::any_cast<long long>(left) & std::any_cast<long long>(right);
    case BoundBinaryOperatorKind::BitwiseOr:
        if (n->Op->LeftType == TypeSymbol::Boolean && n->Op->RightType == TypeSymbol::Boolean)
            return std::any_cast<bool>(left) | std::any_cast<bool>(right);
        return std::any_cast<long long>(left) | std::any_cast<long long>(right);
    case BoundBinaryOperatorKind::BitwiseXor:
        if (n->Op->LeftType == TypeSymbol::Boolean && n->Op->RightType == TypeSymbol::Boolean)
            return std::any_cast<bool>(left) ^ std::any_cast<bool>(right);
        return std::any_cast<long long>(left) ^ std::any_cast<long long>(right);
    case BoundBinaryOperatorKind::LogicalAnd:
        return std::any_cast<bool>(left) && std::any_cast<bool>(right);
    case BoundBinaryOperatorKind::LogicalOr:
        return std::any_cast<bool>(left) || std::any_cast<bool>(right);
    case BoundBinaryOperatorKind::Less:
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::Integer)
            return std::any_cast<long long>(left) < std::any_cast<long long>(right);
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::Float)
            return std::any_cast<long long>(left) < std::any_cast<double>(right);
        if (n->Op->LeftType == TypeSymbol::Float && n->Op->RightType == TypeSymbol::Integer)
            return std::any_cast<double>(left) < std::any_cast<long long>(right);
        return std::any_cast<double>(left) < std::any_cast<double>(right);
    case BoundBinaryOperatorKind::LessOrEquals:
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::Integer)
            return std::any_cast<long long>(left) <= std::any_cast<long long>(right);
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::Float)
            return std::any_cast<long long>(left) <= std::any_cast<double>(right);
        if (n->Op->LeftType == TypeSymbol::Float && n->Op->RightType == TypeSymbol::Integer)
            return std::any_cast<double>(left) <= std::any_cast<long long>(right);
        return std::any_cast<double>(left) <= std::any_cast<double>(right);
    case BoundBinaryOperatorKind::Greater:
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::Integer)
            return std::any_cast<long long>(left) > std::any_cast<long long>(right);
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::Float)
            return std::any_cast<long long>(left) > std::any_cast<double>(right);
        if (n->Op->LeftType == TypeSymbol::Float && n->Op->RightType == TypeSymbol::Integer)
            return std::any_cast<double>(left) > std::any_cast<long long>(right);
        return std::any_cast<double>(left) > std::any_cast<double>(right);
    case BoundBinaryOperatorKind::GreaterOrEquals:
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::Integer)
            return std::any_cast<long long>(left) >= std::any_cast<long long>(right);
        if (n->Op->LeftType == TypeSymbol::Integer && n->Op->RightType == TypeSymbol::Float)
            return std::any_cast<long long>(left) >= std::any_cast<double>(right);
        if (n->Op->LeftType == TypeSymbol::Float && n->Op->RightType == TypeSymbol::Integer)
            return std::any_cast<double>(left) >= std::any_cast<long long>(right);
        return std::any_cast<double>(left) >= std::any_cast<double>(right);
    case BoundBinaryOperatorKind::Equals:
        if (left.type() == typeid(long long) && right.type() == typeid(long long))
            return std::any_cast<long long>(left) == std::any_cast<long long>(right);
        if (left.type() == typeid(double) && right.type() == typeid(double))
            return std::any_cast<double>(left) == std::any_cast<double>(right);
        if (left.type() == typeid(long long) && right.type() == typeid(double))
            return std::any_cast<long long>(left) == std::any_cast<double>(right);
        if (left.type() == typeid(double) && right.type() == typeid(long long))
            return std::any_cast<double>(left) == std::any_cast<long long>(right);
        if (left.type() == typeid(bool) && right.type() == typeid(bool))
            return std::any_cast<bool>(left) == std::any_cast<bool>(right);
        if (left.type() == typeid(std::string) && right.type() == typeid(std::string))
            return std::any_cast<std::string>(left) == std::any_cast<std::string>(right);
        throw std::runtime_error("Unexpected types for equality comparison");
    case BoundBinaryOperatorKind::NotEquals:
        if (left.type() == typeid(long long) && right.type() == typeid(long long))
            return std::any_cast<long long>(left) != std::any_cast<long long>(right);
        if (left.type() == typeid(double) && right.type() == typeid(double))
            return std::any_cast<double>(left) != std::any_cast<double>(right);
        if (left.type() == typeid(long long) && right.type() == typeid(double))
            return std::any_cast<long long>(left) != std::any_cast<double>(right);
        if (left.type() == typeid(double) && right.type() == typeid(long long))
            return std::any_cast<double>(left) != std::any_cast<long long>(right);
        if (left.type() == typeid(bool) && right.type() == typeid(bool))
            return std::any_cast<bool>(left) != std::any_cast<bool>(right);
        if (left.type() == typeid(std::string) && right.type() == typeid(std::string))
            return std::any_cast<std::string>(left) != std::any_cast<std::string>(right);
        throw std::runtime_error("Unexpected types for inequality comparison");
    default:
        throw std::runtime_error("Unexpected binary operator");
    }
}

std::any Evaluator::EvaluateCallExpression(BoundCallExpression *n)
{
    if (n->Function == BuiltInFunctions::Input)
    {
        std::string input;
        std::cin >> input;
        std::cout << "\n";
        return input;
    }
    else if (n->Function == BuiltInFunctions::Print)
    {
        std::string output = std::any_cast<std::string>(EvaluateExpression(n->Arguments[0]));
        std::cout << output;
        return output;
    }
    else if (n->Function == BuiltInFunctions::ArrayLength)
    {
        std::vector<std::any> array = std::any_cast<std::vector<std::any>>(EvaluateExpression(n->Arguments[0]));
        long long size = array.size();
        return size;
    }
    else if (n->Function == BuiltInFunctions::StringLength)
    {
        std::string text = std::any_cast<std::string>(EvaluateExpression(n->Arguments[0]));
        long long size = text.size();
        return size;
    }
    else if (n->Function == BuiltInFunctions::Random)
    {
        long long max = std::any_cast<long long>(EvaluateExpression(n->Arguments[0]));
        return rand() % max;
    }
    else
    {
        std::unordered_map<VariableSymbol, std::any> locals;

        for (long long i = 0; i < n->Arguments.size(); i++)
        {
            locals[n->Function.Parameters[i]] = EvaluateExpression(n->Arguments[i]);
        }

        _locals.push(locals);

        BoundBlockStatement *statement = _program->Functions[n->Function];
        std::any result = EvaluateStatement(statement);

        _locals.pop();

        return result;
    }
}

std::any Evaluator::EvaluateConversionExpression(BoundConversionExpression *n)
{
    auto value = EvaluateExpression(n->Expression);
    if (n->type == TypeSymbol::Boolean)
    {
        if (n->Expression->type == TypeSymbol::Boolean)
            return std::any_cast<bool>(value);
        else if (n->Expression->type == TypeSymbol::Integer)
            return std::any_cast<long long>(value) != 0;
        else if (n->Expression->type == TypeSymbol::Float)
            return std::any_cast<double>(value) != 0.0;
        else if (n->Expression->type == TypeSymbol::String)
            return std::any_cast<std::string>(value) != "";
        else if (n->Expression->type == TypeSymbol::Any)
            return value.has_value();
    }
    else if (n->type == TypeSymbol::String)
    {
        if (n->Expression->type == TypeSymbol::Boolean)
            return std::to_string(std::any_cast<bool>(value));
        else if (n->Expression->type == TypeSymbol::Integer)
            return std::to_string(std::any_cast<long long>(value));
        else if (n->Expression->type == TypeSymbol::Float)
            return std::to_string(std::any_cast<double>(value));
        else if (n->Expression->type == TypeSymbol::String)
            return std::any_cast<std::string>(value);
        else if (n->Expression->type == TypeSymbol::Any)
        {
            if (value.type() == typeid(bool))
                return std::to_string(std::any_cast<bool>(value));
            else if (value.type() == typeid(int))
                return std::to_string(std::any_cast<int>(value));
            else if (value.type() == typeid(long long))
                return std::to_string(std::any_cast<long long>(value));
            else if (value.type() == typeid(float))
                return std::to_string(std::any_cast<double>(value));
            else if (value.type() == typeid(double))
                return std::to_string(std::any_cast<double>(value));
            else if (value.type() == typeid(std::string))
                return std::any_cast<std::string>(value);
            else if (value.type() == typeid(std::any))
                return "any";
        }
    }
    else if (n->type == TypeSymbol::Integer)
    {
        if (n->Expression->type == TypeSymbol::Boolean)
            return std::any_cast<bool>(value);
        else if (n->Expression->type == TypeSymbol::String)
            return std::stoll(std::any_cast<std::string>(value));
        else if (n->Expression->type == TypeSymbol::Integer)
            return std::any_cast<long long>(value);
        else if (n->Expression->type == TypeSymbol::Float)
            return (long long)std::any_cast<double>(value);
        else if (n->Expression->type == TypeSymbol::Any)
            return std::any_cast<long long>(value);
    }
    else if (n->type == TypeSymbol::Float)
    {
        if (n->Expression->type == TypeSymbol::Boolean)
            return std::any_cast<bool>(value);
        else if (n->Expression->type == TypeSymbol::String)
            return std::stod(std::any_cast<std::string>(value));
        else if (n->Expression->type == TypeSymbol::Integer)
            return (double)std::any_cast<long long>(value);
        else if (n->Expression->type == TypeSymbol::Float)
            return std::any_cast<double>(value);
        else if (n->Expression->type == TypeSymbol::Any)
            return std::any_cast<double>(value);
    }
    else if (n->type == TypeSymbol::Any)
    {
        if (n->Expression->type == TypeSymbol::Boolean)
            return std::any_cast<bool>(value);
        else if (n->Expression->type == TypeSymbol::String)
            return std::any_cast<std::string>(value);
        else if (n->Expression->type == TypeSymbol::Integer)
            return std::any_cast<long long>(value);
        else if (n->Expression->type == TypeSymbol::Float)
            return std::any_cast<double>(value);
        else if (n->Expression->type == TypeSymbol::Any)
            return value;
    }
    else if (n->type == TypeSymbol::ArrayAny)
    {
        if (n->Expression->type.IsArray())
            return std::any_cast<std::vector<std::any>>(value);
    }
    else
        throw std::runtime_error("Unexpected conversion: " + n->type.Name);

    return std::any();
}

std::any Evaluator::EvaluateArrayAccessExpression(BoundArrayAccessExpression *n)
{
    long long index = std::any_cast<long long>(EvaluateExpression(n->Index));
    if (n->type == TypeSymbol::String)
    {
        std::string array = std::any_cast<std::string>(EvaluateExpression(n->Variable));
        if (index < 0 || index >= array.size())
            throw std::runtime_error("Index out of bounds");
        return std::string(1, array[index]);
    }
    else
    {

        std::vector<std::any> array = std::any_cast<std::vector<std::any>>(EvaluateExpression(n->Variable));
        if (index < 0 || index >= array.size())
            throw std::runtime_error("Index out of bounds");
        return array[index];
    }
}

std::any Evaluator::EvaluateArrayInitializerExpression(BoundArrayInitializerExpression *n)
{
    std::vector<std::any> values;
    for (auto &expression : n->Elements)
    {
        values.push_back(EvaluateExpression(expression));
    }
    return values;
}

std::any Evaluator::EvaluateArrayAssignmentExpression(BoundArrayAssignmentExpression *n)
{
    long long index = std::any_cast<long long>(EvaluateExpression(n->Index));
    std::any value = EvaluateExpression(n->Expression);

    if (n->type == TypeSymbol::String)
    {
        std::string array = std::any_cast<std::string>(EvaluateExpression(n->Identifier));
        std::string newValue = std::any_cast<std::string>(value);
        if (index < 0 || index >= array.size())
            throw std::runtime_error("Index out of bounds");
        if (newValue.size() != 1)
            throw std::runtime_error("Expected a character");
        array[index] = newValue[0];
        Assign(n->Variable, array);
        return value;
    }
    else
    {
        std::vector<std::any> array = std::any_cast<std::vector<std::any>>(EvaluateExpression(n->Identifier));
        if (index < 0 || index >= array.size())
            throw std::runtime_error("Index out of bounds");
        array[index] = value;
        Assign(n->Variable, array);
        return value;
    }
}

void Evaluator::Assign(VariableSymbol variable, std::any value)
{
    if (variable.GetKind() == SymbolKind::GlobalVariable)
    {
        _globals[variable] = value;
    }
    else
    {
        std::unordered_map<VariableSymbol, std::any> locals = _locals.top();
        _locals.pop();
        locals[variable] = value;
        _locals.push(locals);
    }
}