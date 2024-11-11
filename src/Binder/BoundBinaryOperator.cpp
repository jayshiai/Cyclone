#include "CodeAnalysis/Binder.h"
#include "CodeAnalysis/SyntaxTree.h"
#include <iostream>

BoundBinaryOperator *BoundBinaryOperator::Bind(SyntaxKind syntaxKind, TypeSymbol leftType, TypeSymbol rightType)
{

    for (auto op : operators)
    {
        if (op.syntaxKind == syntaxKind && op.LeftType == leftType && op.RightType == rightType)
        {

            return new BoundBinaryOperator(op.syntaxKind, op.Kind, op.LeftType, op.RightType, op.ResultType);
        }
    }
    return nullptr;
}

const std::vector<BoundBinaryOperator> BoundBinaryOperator::operators{
    BoundBinaryOperator(SyntaxKind::PLUS, BoundBinaryOperatorKind::Addition, TypeSymbol::Integer),
    BoundBinaryOperator(SyntaxKind::MINUS, BoundBinaryOperatorKind::Subtraction, TypeSymbol::Integer),
    BoundBinaryOperator(SyntaxKind::MULTIPLY, BoundBinaryOperatorKind::Multiplication, TypeSymbol::Integer),
    BoundBinaryOperator(SyntaxKind::DIVIDE, BoundBinaryOperatorKind::Division, TypeSymbol::Integer),

    BoundBinaryOperator(SyntaxKind::PLUS, BoundBinaryOperatorKind::Addition, TypeSymbol::String),
    BoundBinaryOperator(SyntaxKind::EQUALS_EQUALS, BoundBinaryOperatorKind::Equals, TypeSymbol::String, TypeSymbol::Boolean),
    BoundBinaryOperator(SyntaxKind::BANG_EQUALS, BoundBinaryOperatorKind::NotEquals, TypeSymbol::String, TypeSymbol::Boolean),

    BoundBinaryOperator(SyntaxKind::AMPERSAND, BoundBinaryOperatorKind::BitwiseAnd, TypeSymbol::Integer),
    BoundBinaryOperator(SyntaxKind::PIPE, BoundBinaryOperatorKind::BitwiseOr, TypeSymbol::Integer),
    BoundBinaryOperator(SyntaxKind::HAT, BoundBinaryOperatorKind::BitwiseXor, TypeSymbol::Integer),

    BoundBinaryOperator(SyntaxKind::EQUALS_EQUALS, BoundBinaryOperatorKind::Equals, TypeSymbol::Integer, TypeSymbol::Boolean),
    BoundBinaryOperator(SyntaxKind::BANG_EQUALS, BoundBinaryOperatorKind::NotEquals, TypeSymbol::Integer, TypeSymbol::Boolean),

    BoundBinaryOperator(SyntaxKind::LESS, BoundBinaryOperatorKind::Less, TypeSymbol::Integer, TypeSymbol::Boolean),
    BoundBinaryOperator(SyntaxKind::LESS_EQUALS, BoundBinaryOperatorKind::LessOrEquals, TypeSymbol::Integer, TypeSymbol::Boolean),
    BoundBinaryOperator(SyntaxKind::GREATER, BoundBinaryOperatorKind::Greater, TypeSymbol::Integer, TypeSymbol::Boolean),
    BoundBinaryOperator(SyntaxKind::GREATER_EQUALS, BoundBinaryOperatorKind::GreaterOrEquals, TypeSymbol::Integer, TypeSymbol::Boolean),

    BoundBinaryOperator(SyntaxKind::AMPERSAND, BoundBinaryOperatorKind::BitwiseAnd, TypeSymbol::Boolean),
    BoundBinaryOperator(SyntaxKind::PIPE, BoundBinaryOperatorKind::BitwiseOr, TypeSymbol::Boolean),
    BoundBinaryOperator(SyntaxKind::HAT, BoundBinaryOperatorKind::BitwiseXor, TypeSymbol::Boolean),

    BoundBinaryOperator(SyntaxKind::AMPERSAND_AMPERSAND, BoundBinaryOperatorKind::LogicalAnd, TypeSymbol::Boolean),
    BoundBinaryOperator(SyntaxKind::PIPE_PIPE, BoundBinaryOperatorKind::LogicalOr, TypeSymbol::Boolean),
    BoundBinaryOperator(SyntaxKind::EQUALS_EQUALS, BoundBinaryOperatorKind::Equals, TypeSymbol::Boolean),
    BoundBinaryOperator(SyntaxKind::BANG_EQUALS, BoundBinaryOperatorKind::NotEquals, TypeSymbol::Boolean),

};

std::string BoundBinaryOperator::ToString() const
{
    switch (Kind)
    {
    case BoundBinaryOperatorKind::Addition:
        return "+";
    case BoundBinaryOperatorKind::Subtraction:
        return "-";
    case BoundBinaryOperatorKind::Multiplication:
        return "*";
    case BoundBinaryOperatorKind::Division:
        return "/";
    case BoundBinaryOperatorKind::BitwiseAnd:
        return "&";
    case BoundBinaryOperatorKind::BitwiseOr:
        return "|";
    case BoundBinaryOperatorKind::BitwiseXor:
        return "^";
    case BoundBinaryOperatorKind::Equals:
        return "==";
    case BoundBinaryOperatorKind::NotEquals:
        return "!=";
    case BoundBinaryOperatorKind::Less:
        return "<";
    case BoundBinaryOperatorKind::LessOrEquals:
        return "<=";
    case BoundBinaryOperatorKind::Greater:
        return ">";
    case BoundBinaryOperatorKind::GreaterOrEquals:
        return ">=";
    case BoundBinaryOperatorKind::LogicalAnd:
        return "&&";
    case BoundBinaryOperatorKind::LogicalOr:
        return "||";
    default:
        return "Unknown";
    }
}