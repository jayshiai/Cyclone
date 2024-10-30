#include "CodeAnalysis/Lexer.h"
#include "CodeAnalysis/SyntaxTree.h"
#include <iostream>
#include <cctype>
#include <stdexcept>
#include "CodeAnalysis/Diagnostic.h"
#include "Utils.h"
std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;
    while (currentChar != '\0')
    {
        switch (currentChar)
        {
        case '+':
            tokens.push_back(Token{SyntaxKind::PLUS, "+", pos});
            advance();
            break;

        case '-':
            tokens.push_back(Token{SyntaxKind::MINUS, "-", pos});
            advance();
            break;

        case '*':
            tokens.push_back(Token{SyntaxKind::MULTIPLY, "*", pos});
            advance();
            break;

        case '/':
            tokens.push_back(Token{SyntaxKind::DIVIDE, "/", pos});
            advance();
            break;

        case '(':
            tokens.push_back(Token{SyntaxKind::LPAREN, "(", pos});
            advance();
            break;

        case ')':
            tokens.push_back(Token{SyntaxKind::RPAREN, ")", pos});
            advance();
            break;
        case '{':
            tokens.push_back(Token{SyntaxKind::OPEN_BRACE, "{", pos});
            advance();
            break;
        case '}':
            tokens.push_back(Token{SyntaxKind::CLOSE_BRACE, "}", pos});
            advance();
            break;
        case '~':
            tokens.push_back(Token{SyntaxKind::TILDE, "~", pos});
            advance();
            break;
        case '^':
            tokens.push_back(Token{SyntaxKind::HAT, "^", pos});
            advance();
            break;
        case '&':
            if (lookAhead == '&')
            {
                tokens.push_back(Token{SyntaxKind::AMPERSAND_AMPERSAND, "&&", pos});
                advance();
                advance();
            }
            else
            {
                tokens.push_back(Token{SyntaxKind::AMPERSAND, "&", pos});
                advance();
            }
            break;
        case '|':
            if (lookAhead == '|')
            {
                tokens.push_back(Token{SyntaxKind::PIPE_PIPE, "||", pos});
                advance();
                advance();
            }
            else
            {
                tokens.push_back(Token{SyntaxKind::PIPE, "|", pos});
                advance();
            }
            break;
        case '=':
            if (lookAhead == '=')
            {
                tokens.push_back(Token{SyntaxKind::EQUALS_EQUALS, "==", pos});
                advance();
                advance();
            }
            else
            {
                tokens.push_back(Token{SyntaxKind::EQUALS, "=", pos});
                advance();
            }
            break;
        case '!':
            if (lookAhead == '=')
            {
                tokens.push_back(Token{SyntaxKind::BANG_EQUALS, "!=", pos});
                advance();
                advance();
            }
            else
            {
                tokens.push_back(Token{SyntaxKind::BANG, "!", pos});
                advance();
            }
            break;
        case '<':
            if (lookAhead == '=')
            {
                tokens.push_back(Token{SyntaxKind::LESS_EQUALS, "<=", pos});
                advance();
                advance();
            }
            else
            {
                tokens.push_back(Token{SyntaxKind::LESS, "<", pos});
                advance();
            }
            break;
        case '>':
            if (lookAhead == '=')
            {
                tokens.push_back(Token{SyntaxKind::GREATER_EQUALS, ">=", pos});
                advance();
                advance();
            }
            else
            {
                tokens.push_back(Token{SyntaxKind::GREATER, ">", pos});
                advance();
            }
            break;
        case '"':
            tokens.push_back(GenerateStringToken());
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            tokens.push_back(GenerateNumberToken());
            break;
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            GenerateWhitespaceToken();
            break;
        default:

            // if (isspace(currentChar))
            // {
            //     skipWhitespace();
            // }

            if (isalpha(currentChar))
            {
                tokens.push_back(GenerateIdentifierToken());
            }
            else if (isspace(currentChar))
            {
                GenerateWhitespaceToken();
            }
            else
            {
                _diagnostics.ReportBadCharacter(pos, currentChar);
                // tokens.push_back(Token{SyntaxKind::BAD_TOKEN, std::string(1, currentChar), pos});
                advance();
            }

            break;
        }
    }
    tokens.push_back(Token{SyntaxKind::END_OF_FILE, "", pos});
    // std::cout << "TOKENS: " << std::endl;
    // for (auto token : tokens)
    // {
    //     std::cout << "  " << convertSyntaxKindToString(token.Kind) << " " << token.value << std::endl;
    // }
    return tokens;
}

void Lexer::advance()
{
    pos++;
    if (pos < input.Length())
    {
        currentChar = input[pos];
        lookAhead = input[pos + 1];
    }
    else
    {
        currentChar = '\0';
    }
}

Token Lexer::GenerateStringToken()
{
    size_t start = pos;
    advance();
    while (currentChar != '"')
    {
        if (currentChar == '\0')
        {
            _diagnostics.ReportUnterminatedString(TextSpan(start, pos - start));
            return Token{SyntaxKind::BAD_TOKEN, input.ToString(start, pos - start), start};
        }
        advance();
    }
    advance();
    return Token{SyntaxKind::STRING, input.ToString(start, pos - start), start};
}

Token Lexer::GenerateWhitespaceToken()
{
    while (isspace(currentChar))
    {
        advance();
    }
    return Token{SyntaxKind::WHITESPACE, " ", pos};
}
Token Lexer::GenerateNumberToken()
{
    size_t start = pos;
    std::string result;
    while (pos < input.Length() && isdigit(currentChar))
    {
        result += currentChar;
        advance();
    }

    int length = pos - start;
    int value;

    if (isalpha(currentChar))
    {
        while (isalnum(currentChar))
        {
            result += currentChar;
            advance();
        }

        _diagnostics.ReportInvalidNumber(TextSpan(start, pos - start), result, "int");
        return Token{SyntaxKind::BAD_TOKEN, result, start};
    }
    return Token{SyntaxKind::NUMBER, result, pos};
}

Token Lexer::GenerateIdentifierToken()
{
    size_t start = pos;
    while (isalnum(currentChar))
    {
        advance();
    }
    int length = pos - start;
    std::string text = input.ToString(start, length);
    SyntaxKind kind = checkKeyword(text);
    return Token{kind, text, start};
}

SyntaxKind Lexer::checkKeyword(const std::string &keyword)
{
    if (keyword == "true")
        return SyntaxKind::TRUE_KEYWORD;
    else if (keyword == "false")
        return SyntaxKind::FALSE_KEYWORD;
    else if (keyword == "let")
        return SyntaxKind::LET_KEYWORD;
    else if (keyword == "var")
        return SyntaxKind::VAR_KEYWORD;
    else if (keyword == "if")
        return SyntaxKind::IF_KEYWORD;
    else if (keyword == "else")
        return SyntaxKind::ELSE_KEYWORD;
    else if (keyword == "while")
        return SyntaxKind::WHILE_KEYWORD;
    else if (keyword == "for")
        return SyntaxKind::FOR_KEYWORD;
    else if (keyword == "to")
        return SyntaxKind::TO_KEYWORD;
    return SyntaxKind::IDENTIFIER;
}