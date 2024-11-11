#include "CodeAnalysis/Binder.h"
#include "CodeAnalysis/SyntaxTree.h"

BoundUnaryOperator *BoundUnaryOperator::Bind(SyntaxKind syntaxKind, TypeSymbol operandType)
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
    BoundUnaryOperator(SyntaxKind::BANG, BoundUnaryOperatorKind::LogicalNegation, TypeSymbol::Boolean),
    BoundUnaryOperator(SyntaxKind::PLUS, BoundUnaryOperatorKind::Identity, TypeSymbol::Integer),
    BoundUnaryOperator(SyntaxKind::MINUS, BoundUnaryOperatorKind::Negation, TypeSymbol::Integer),
    BoundUnaryOperator(SyntaxKind::TILDE, BoundUnaryOperatorKind::OnesComplement, TypeSymbol::Integer),

    BoundUnaryOperator(SyntaxKind::PLUS, BoundUnaryOperatorKind::Identity, TypeSymbol::Float),
    BoundUnaryOperator(SyntaxKind::MINUS, BoundUnaryOperatorKind::Negation, TypeSymbol::Float),

};

std::string BoundUnaryOperator::ToString() const
{
    switch (Kind)
    {
    case BoundUnaryOperatorKind::Identity:
        return "+";
    case BoundUnaryOperatorKind::Negation:
        return "-";
    case BoundUnaryOperatorKind::LogicalNegation:
        return "!";
    case BoundUnaryOperatorKind::OnesComplement:
        return "~";
    default:
        return "Unknown";
    }
}