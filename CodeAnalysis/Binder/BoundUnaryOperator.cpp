#include "CodeAnalysis/Binder.h"
#include "CodeAnalysis/SyntaxTree.h"

BoundUnaryOperator *BoundUnaryOperator::Bind(SyntaxKind syntaxKind, Type operandType)
{
    for (auto op : operators)
    {
        if (op.syntaxKind == syntaxKind && op.OperandType == operandType)
        {
            return new BoundUnaryOperator(op.syntaxKind, op.Kind, op.OperandType, op.ResultType);
        }
    }
    return nullptr;
}

const std::vector<BoundUnaryOperator> BoundUnaryOperator::operators{
    BoundUnaryOperator(SyntaxKind::BANG, BoundUnaryOperatorKind::LogicalNegation, Type::Boolean),
    BoundUnaryOperator(SyntaxKind::PLUS, BoundUnaryOperatorKind::Identity, Type::Integer),
    BoundUnaryOperator(SyntaxKind::MINUS, BoundUnaryOperatorKind::Negation, Type::Integer)};
