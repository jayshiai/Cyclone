#include "Lexer.h"
#include <iostream>
#include <cctype>

Lexer::Lexer(const std::string &input) : input(input), pos(0), currentChar(input[pos]) {}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;
    while (currentChar != '\0')
    {
        switch (currentChar)
        {
        case '+':
            tokens.push_back(Token{TokenType::PLUS, "+", pos});
            advance();
            break;

        case '-':
            tokens.push_back(Token{TokenType::MINUS, "-", pos});
            advance();
            break;

        case '*':
            tokens.push_back(Token{TokenType::MULTIPLY, "*", pos});
            advance();
            break;

        case '/':
            tokens.push_back(Token{TokenType::DIVIDE, "/", pos});
            advance();
            break;

        case '(':
            tokens.push_back(Token{TokenType::LPAREN, "(", pos});
            advance();
            break;

        case ')':
            tokens.push_back(Token{TokenType::RPAREN, ")", pos});
            advance();
            break;

        default:

            if (isdigit(currentChar))
            {
                tokens.push_back(Token{TokenType::NUMBER, number(), pos});
            }

            else if (isspace(currentChar))
            {
                skipWhitespace();
            }

            else
            {
                std::cerr << "Unknown character: " << currentChar << std::endl;
                exit(1);
            }

            break;
        }
    }
    tokens.push_back(Token{TokenType::END_OF_FILE, "", pos});
    return tokens;
}

void Lexer::advance()
{
    pos++;
    if (pos < input.size())
    {
        currentChar = input[pos];
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
