#include "CodeAnalysis/Binder.h"
#include "CodeAnalysis/SyntaxTree.h"
#include <iostream>

BoundBinaryOperator *BoundBinaryOperator::Bind(SyntaxKind syntaxKind, Type leftType, Type rightType)
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
    BoundBinaryOperator(SyntaxKind::PLUS, BoundBinaryOperatorKind::Addition, Type::Integer),
    BoundBinaryOperator(SyntaxKind::MINUS, BoundBinaryOperatorKind::Subtraction, Type::Integer),
    BoundBinaryOperator(SyntaxKind::MULTIPLY, BoundBinaryOperatorKind::Multiplication, Type::Integer),
    BoundBinaryOperator(SyntaxKind::DIVIDE, BoundBinaryOperatorKind::Division, Type::Integer),

    BoundBinaryOperator(SyntaxKind::EQUALS_EQUALS, BoundBinaryOperatorKind::Equals, Type::Integer, Type::Boolean),
    BoundBinaryOperator(SyntaxKind::BANG_EQUALS, BoundBinaryOperatorKind::NotEquals, Type::Integer, Type::Boolean),

    BoundBinaryOperator(SyntaxKind::LESS, BoundBinaryOperatorKind::Less, Type::Integer, Type::Boolean),
    BoundBinaryOperator(SyntaxKind::LESS_EQUALS, BoundBinaryOperatorKind::LessOrEquals, Type::Integer, Type::Boolean),
    BoundBinaryOperator(SyntaxKind::GREATER, BoundBinaryOperatorKind::Greater, Type::Integer, Type::Boolean),
    BoundBinaryOperator(SyntaxKind::GREATER_EQUALS, BoundBinaryOperatorKind::GreaterOrEquals, Type::Integer, Type::Boolean),

    BoundBinaryOperator(SyntaxKind::AMPERSAND_AMPERSAND, BoundBinaryOperatorKind::LogicalAnd, Type::Boolean),
    BoundBinaryOperator(SyntaxKind::PIPE_PIPE, BoundBinaryOperatorKind::LogicalOr, Type::Boolean),
    BoundBinaryOperator(SyntaxKind::EQUALS_EQUALS, BoundBinaryOperatorKind::Equals, Type::Boolean),
    BoundBinaryOperator(SyntaxKind::BANG_EQUALS, BoundBinaryOperatorKind::NotEquals, Type::Boolean),

};
