#include "CodeAnalysis/Lexer.h"
#include "CodeAnalysis/SyntaxTree.h"
#include <iostream>
#include <cctype>
#include <stdexcept>
#include "CodeAnalysis/Diagnostic.h"
Lexer::Lexer(const std::string &input) : input(input), pos(0), currentChar(input[pos]), lookAhead(input[pos + 1]) {}

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
        case '&':
            if (lookAhead == '&')
            {
                tokens.push_back(Token{SyntaxKind::AMPERSAND_AMPERSAND, "&&", pos});
                advance();
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
        default:

            if (isdigit(currentChar))
            {
                int start = pos;
                std::string numberText = number();
                int length = pos - start;
                int value;

                try
                {
                    value = std::stoi(numberText);
                }
                catch (const std::invalid_argument &)
                {
                    _diagnostics.ReportInvalidNumber(TextSpan(start, length), numberText, "int");
                    value = 0;
                }

                tokens.push_back(Token{SyntaxKind::NUMBER, numberText, pos});
            }

            else if (isspace(currentChar))
            {
                skipWhitespace();
            }

            else if (isalpha(currentChar))
            {
                std::string result;
                while (isalpha(currentChar))
                {
                    result += currentChar;
                    advance();
                }

                if (result == "true")
                {
                    tokens.push_back(Token{SyntaxKind::TRUE, "true", pos});
                }
                else if (result == "false")
                {
                    tokens.push_back(Token{SyntaxKind::FALSE, "false", pos});
                }
                else
                {
                    tokens.push_back(Token{SyntaxKind::IDENTIFIER, result, pos});
                }
            }

            else
            {
                _diagnostics.ReportBadCharacter(pos, currentChar);
                tokens.push_back(Token{SyntaxKind::BAD_TOKEN, std::string(1, currentChar), pos});
                advance();
            }

            break;
        }
    }
    tokens.push_back(Token{SyntaxKind::END_OF_FILE, "", pos});
    return tokens;
}

void Lexer::advance()
{
    pos++;
    if (pos < input.size())
    {
        currentChar = input[pos];
        lookAhead = input[pos + 1];
    }
    else
    {
        currentChar = '\0';
    }
}

void Lexer::skipWhitespace()
{
    while (isspace(currentChar))
    {
        advance();
    }
}

std::string Lexer::number()
{
    std::string result;
    while (pos < input.size() && isdigit(currentChar))
    {
        result += currentChar;
        advance();
    }
    return result;
}

SyntaxKind Lexer::checkKeyword(const std::string &keyword)
{
    if (keyword == "true")
    {
        return SyntaxKind::TRUE;
    }
    else if (keyword == "false")
    {
        return SyntaxKind::FALSE;
    }
    return SyntaxKind::IDENTIFIER;
}