#include "Parser.h"
#include "SyntaxTree.h"
#include <iostream>

Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens), currentTokenIndex(0), currentToken(tokens[currentTokenIndex]) {}

SyntaxTree Parser::parse()
{
    SyntaxNode *root = ParseExpression();
    return SyntaxTree(root, diagnostics);
}

void Parser::NextToken()
{
    currentTokenIndex++;
    if (currentTokenIndex < tokens.size())
    {
        currentToken = tokens[currentTokenIndex];
    }
}

SyntaxNode *Parser::ParseExpression()
{

    SyntaxNode *left = ParseTerm();

    while (currentToken.type == TokenType::PLUS || currentToken.type == TokenType::MINUS)
    {
        std::string op = currentToken.value;
        NextToken();
        SyntaxNode *right = ParseTerm();
        left = new BinaryExpressionNode(left, right, op);
    }
    return left;
}

SyntaxNode *Parser::ParseTerm()
{
    SyntaxNode *left = ParseFactor();

    while (currentToken.type == TokenType::MULTIPLY || currentToken.type == TokenType::DIVIDE)
    {
        std::string op = currentToken.value;
        NextToken();
        SyntaxNode *right = ParseFactor();

        left = new BinaryExpressionNode(left, right, op);
    }

    return left;
}

SyntaxNode *Parser::ParseFactor()
{
    if (currentToken.type == TokenType::NUMBER)
    {
        SyntaxNode *node = new NumericLiteralNode(currentToken.value);
        NextToken();
        return node;
    }
    else if (currentToken.type == TokenType::LPAREN)
    {
        NextToken();
        SyntaxNode *expression = ParseExpression();
        if (currentToken.type == TokenType::RPAREN)
        {
            NextToken();
        }
        else
        {
            diagnostics.push_back("Expected ')' but got :" + currentToken.value + " at position: " + std::to_string(currentTokenIndex));
        }
        return expression;
    }
    else
    {
        std::cerr << "Unexpected token: " << currentToken.value << std::endl;
        exit(1);
    }

    return nullptr; // Unreachable
}
