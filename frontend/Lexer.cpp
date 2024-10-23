#include "Lexer.h"
#include <iostream>
#include <cctype>

Lexer::Lexer(const std::string &input) : input(input), pos(0), currentChar(input[pos]) {}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;
    while (currentChar != '\0')
    {
        if (isspace(currentChar))
        {
            skipWhitespace();
        }
        else if (isdigit(currentChar))
        {
            tokens.push_back(Token{TokenType::NUMBER, number()});
        }
        else if (currentChar == '+')
        {
            tokens.push_back(Token{TokenType::PLUS, "+"});
            advance();
        }
        else if (currentChar == '-')
        {
            tokens.push_back(Token{TokenType::MINUS, "-"});
            advance();
        }
        else if (currentChar == '*')
        {
            tokens.push_back(Token{TokenType::MULTIPLY, "*"});
            advance();
        }
        else if (currentChar == '/')
        {
            tokens.push_back(Token{TokenType::DIVIDE, "/"});
            advance();
        }
        else if (currentChar == '(')
        {
            tokens.push_back(Token{TokenType::LPAREN, "("});
            advance();
        }
        else if (currentChar == ')')
        {
            tokens.push_back(Token{TokenType::RPAREN, ")"});
            advance();
        }
        else
        {
            std::cerr << "Unknown character: " << currentChar << std::endl;
            exit(1);
        }
    }
    tokens.push_back(Token{TokenType::END_OF_FILE, ""});
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
