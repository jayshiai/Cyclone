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
SyntaxNode *Parser::ParseExpression(int parentPrecedence)
{
    SyntaxNode *left = ParsePrimaryExpression();
    while (true)
    {

        int precedence = GetBinaryPrecedence(currentToken.type);
        if (precedence == 0 || precedence <= parentPrecedence)
        {
            break;
        }
        std::string op = currentToken.value;
        NextToken();
        SyntaxNode *right = ParseExpression(precedence);
        left = new BinaryExpressionNode(left, right, op);
    }
    return left;
}

int Parser::GetBinaryPrecedence(TokenType type)
{
    switch (type)
    {
    case TokenType::MULTIPLY:
    case TokenType::DIVIDE:
        return 2;
    case TokenType::PLUS:
    case TokenType::MINUS:
        return 1;

    default:
        return 0;
    }
}

SyntaxNode *Parser::ParsePrimaryExpression()
{

    switch (currentToken.type)
    {
    case TokenType::LPAREN:
    {
        NextToken();
        SyntaxNode *expression = ParseExpression();
        if (currentToken.type == TokenType::RPAREN)
        {
            NextToken();
        }
        else
        {
            diagnostics.push_back("Expected ')' but got :" + currentToken.value + " at position: " + std::to_string(currentToken.position));
        }
        return new ParenthesizedExpressionNode(expression);
        break;
    }
    case TokenType::NUMBER:
    {
        SyntaxNode *node = new NumericLiteralNode(currentToken.value);
        NextToken();
        return node;
        break;
    }
    default:
        diagnostics.push_back("Unexpected token: " + currentToken.value + " at position: " + std::to_string(currentToken.position));
        break;
    }
}
