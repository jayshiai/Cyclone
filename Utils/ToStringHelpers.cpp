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

// enum class SyntaxKind
// {
//     NUMBER,
//     PLUS,
//     MINUS,
//     MULTIPLY,
//     DIVIDE,
//     LPAREN, // (
//     RPAREN, // )
//     BANG,
//     EQUALS,
//     LESS,
//     LESS_EQUALS,
//     GREATER,
//     GREATER_EQUALS,
//     EQUALS_EQUALS,
//     BANG_EQUALS,
//     AMPERSAND,
//     TILDE,
//     HAT,
//     PIPE,

//     AMPERSAND_AMPERSAND,
//     PIPE_PIPE,
//     IDENTIFIER,
//     WHITESPACE,
//     END_OF_FILE,
//     BAD_TOKEN,
//     OPEN_BRACE,
//     CLOSE_BRACE,

//     TRUE_KEYWORD,
//     FALSE_KEYWORD,
//     LET_KEYWORD,
//     VAR_KEYWORD,
//     ELSE_KEYWORD,
//     IF_KEYWORD,
//     WHILE_KEYWORD,
//     FOR_KEYWORD,
//     TO_KEYWORD,

//     LiteralExpression,
//     UnaryExpression,
//     BinaryExpression,
//     ParenthesizedExpression,
//     NameExpression,
//     AssignmentExpression,

//     CompilationUnit,
//     ElseClause,

//     ExpressionStatement,
//     VariableDeclaration,
//     BlockStatement,
//     IfStatement,
//     WhileStatement,
//     ForStatement,

// };
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
    case SyntaxKind::TRUE_KEYWORD:
    case SyntaxKind::FALSE_KEYWORD:
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

    case SyntaxKind::LESS:
        return "LessToken";
    case SyntaxKind::LESS_EQUALS:
        return "LessEqualsToken";
    case SyntaxKind::GREATER:
        return "GreaterToken";
    case SyntaxKind::GREATER_EQUALS:
        return "GreaterEqualsToken";

    case SyntaxKind::AMPERSAND:
        return "AmpersandToken";
    case SyntaxKind::TILDE:
        return "TildeToken";
    case SyntaxKind::HAT:
        return "HatToken";
    case SyntaxKind::PIPE:
        return "PipeToken";
    case SyntaxKind::AMPERSAND_AMPERSAND:
        return "AmpersandAmpersandToken";
    case SyntaxKind::PIPE_PIPE:
        return "PipePipeToken";
    case SyntaxKind::WHITESPACE:
        return "WhitespaceToken";

    case SyntaxKind::LET_KEYWORD:
        return "LetKeyword";
    case SyntaxKind::VAR_KEYWORD:
        return "VarKeyword";
    case SyntaxKind::IF_KEYWORD:
        return "IfKeyword";
    case SyntaxKind::ELSE_KEYWORD:
        return "ElseKeyword";
    case SyntaxKind::WHILE_KEYWORD:
        return "WhileKeyword";
    case SyntaxKind::FOR_KEYWORD:
        return "ForKeyword";
    case SyntaxKind::TO_KEYWORD:
        return "ToKeyword";

    case SyntaxKind::OPEN_BRACE:
        return "OpenBraceToken";
    case SyntaxKind::CLOSE_BRACE:
        return "CloseBraceToken";
    case SyntaxKind::CompilationUnit:
        return "CompilationUnit";
    case SyntaxKind::ExpressionStatement:
        return "ExpressionStatement";
    case SyntaxKind::VariableDeclaration:
        return "VariableDeclaration";
    case SyntaxKind::BlockStatement:
        return "BlockStatement";
    case SyntaxKind::IfStatement:
        return "IfStatement";
    case SyntaxKind::WhileStatement:
        return "WhileStatement";
    case SyntaxKind::ForStatement:
        return "ForStatement";

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