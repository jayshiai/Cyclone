#include "CodeAnalysis/SyntaxTree.h"
#include "CodeAnalysis/Binder.h"
#include <string>

std::string convertTypetoString(Type type)
{
    switch (type)
    {
    case Type::Boolean:
        return "Boolean";
    case Type::Integer:
        return "Integer";
    default:
        return "Unknown";
    }
}

std::string convertSyntaxKindToString(SyntaxKind kind)
{
    switch (kind)
    {
    case SyntaxKind::PLUS:
    case SyntaxKind::MINUS:
    case SyntaxKind::MULTIPLY:
    case SyntaxKind::DIVIDE:
        return "OpetatorToken";
    case SyntaxKind::NUMBER:
        return "NumberToken";
    case SyntaxKind::TRUE:
    case SyntaxKind::FALSE:
        return "BooleanToken";
    case SyntaxKind::LiteralExpression:
        return "LiteralExpression";
    case SyntaxKind::UnaryExpression:
        return "UnaryExpression";
    case SyntaxKind::BinaryExpression:
        return "BinaryExpression";
    case SyntaxKind::ParenthesizedExpression:
        return "ParenthesizedExpression";
    case SyntaxKind::IDENTIFIER:
        return "IdentifierToken";
    case SyntaxKind::LPAREN:
    case SyntaxKind::RPAREN:
        return "ParenthesisToken";
    case SyntaxKind::BANG:
        return "BangToken";
    case SyntaxKind::EQUALS:
        return "EqualsToken";
    case SyntaxKind::EQUALS_EQUALS:
        return "EqualsEqualsToken";
    case SyntaxKind::BANG_EQUALS:
        return "BangEqualsToken";
    case SyntaxKind::AMPERSAND_AMPERSAND:
        return "AmpersandAmpersandToken";
    case SyntaxKind::PIPE_PIPE:
        return "PipePipeToken";
    case SyntaxKind::END_OF_FILE:
        return "EndOfFileToken";
    default:
        return "Unknown";
    }
}