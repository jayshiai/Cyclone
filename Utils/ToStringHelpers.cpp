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
    case SyntaxKind::NameExpression:
        return "NameExpression";
    case SyntaxKind::AssignmentExpression:
        return "AssignmentExpression";
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
    case SyntaxKind::WHITESPACE:
        return "WhitespaceToken";
    case SyntaxKind::BAD_TOKEN:
        return "BadToken";
    case SyntaxKind::END_OF_FILE:
        return "EndOfFileToken";
    default:
        return "Unknown";
    }
}

void PrintDiagnostic(Diagnostic diagnostic, SourceText Text)
{
    const std::string RESET_COLOR = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";

    int lineIndex = Text.GetLineIndex(diagnostic.Span.Start);
    TextLine line = Text._lines[lineIndex];
    int lineNumber = lineIndex + 1;
    int character = diagnostic.Span.Start - line.Start + 1;

    std::cout << YELLOW << "(" << lineNumber << "," << character << "): " << RESET_COLOR;

    std::cout << RED << diagnostic.ToString() << RESET_COLOR << std::endl;

    TextSpan prefixSpan = TextSpan::FromBounds(line.Start, diagnostic.Span.Start);
    TextSpan suffixSpan = TextSpan::FromBounds(diagnostic.Span.End, line.End);

    std::string prefix = Text.ToString(prefixSpan);
    std::string error = Text.ToString(diagnostic.Span);
    std::string suffix = Text.ToString(suffixSpan);

    std::cout << "    " << RESET_COLOR << prefix
              << RED << error << RESET_COLOR
              << suffix << std::endl;
}

void PrintVariables(const std::unordered_map<VariableSymbol, std::any> &variables)
{
    for (const auto &[key, value] : variables)
    {
        std::cout << key << ": ";
        if (value.type() == typeid(int))
        {
            std::cout << std::any_cast<int>(value);
        }
        else if (value.type() == typeid(double))
        {
            std::cout << std::any_cast<double>(value);
        }
        else if (value.type() == typeid(std::string))
        {
            std::cout << std::any_cast<std::string>(value);
        }
        else
        {
            std::cout << "Unknown type";
        }
        std::cout << '\n';
    }
}