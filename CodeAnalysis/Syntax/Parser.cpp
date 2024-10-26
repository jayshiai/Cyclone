#include "CodeAnalysis/Parser.h"
#include "CodeAnalysis/SyntaxTree.h"
#include <iostream>
#include "Utils.h"
Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens), currentTokenIndex(0), currentToken(tokens[currentTokenIndex]) {}

Token Parser::peek(int offset)
{
    if (currentTokenIndex + offset >= tokens.size())
    {
        return tokens[tokens.size() - 1];
    }
    return tokens[currentTokenIndex + offset];
}
SyntaxTree Parser::parse()
{
    SyntaxNode *root = ParseExpression();
    return SyntaxTree(root, _diagnostics);
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
    return ParseAssignmentExpression();
}
SyntaxNode *Parser::ParseAssignmentExpression()
{
    if (peek(0).Kind == SyntaxKind::IDENTIFIER && peek(1).Kind == SyntaxKind::EQUALS)
    {
        Token identifier = currentToken;
        NextToken();
        Token equals = currentToken;
        NextToken();
        SyntaxNode *right = ParseAssignmentExpression();
        return new AssignmentExpressionNode(identifier, equals, right);
    }
    return ParseBinaryExpression();
}
SyntaxNode *Parser::ParseBinaryExpression(int parentPrecedence)
{
    SyntaxNode *left;

    int unaryPrecedence = GetUnaryPrecedence(currentToken.Kind);
    if (unaryPrecedence != 0 && unaryPrecedence >= parentPrecedence)
    {
        Token op = currentToken;
        NextToken();
        SyntaxNode *expression = ParseBinaryExpression(unaryPrecedence);
        left = new UnaryExpressionNode(expression, op);
    }
    else
    {
        left = ParsePrimaryExpression();
    }

    while (true)
    {

        int precedence = GetBinaryPrecedence(currentToken.Kind);
        if (precedence == 0 || precedence <= parentPrecedence)
        {
            break;
        }
        Token op = currentToken;
        NextToken();
        SyntaxNode *right = ParseBinaryExpression(precedence);
        left = new BinaryExpressionNode(left, right, op);
    }
    return left;
}

int Parser::GetUnaryPrecedence(SyntaxKind kind)
{
    switch (kind)
    {
    case SyntaxKind::PLUS:
    case SyntaxKind::MINUS:
    case SyntaxKind::BANG:
        return 6;
    default:
        return 0;
    }
}

int Parser::GetBinaryPrecedence(SyntaxKind kind)
{
    switch (kind)
    {
    case SyntaxKind::MULTIPLY:
    case SyntaxKind::DIVIDE:
        return 5;
    case SyntaxKind::PLUS:
    case SyntaxKind::MINUS:
        return 4;
    case SyntaxKind::EQUALS_EQUALS:
    case SyntaxKind::BANG_EQUALS:
        return 3;
    case SyntaxKind::AMPERSAND_AMPERSAND:
        return 2;
    case SyntaxKind::PIPE_PIPE:
        return 1;
    default:
        return 0;
    }
}
SyntaxNode *Parser::Expect(SyntaxKind kind)
{
    if (currentToken.Kind == kind)
    {
        SyntaxNode *node = new Token(currentToken);
        NextToken();
        return node;
    }
    else
    {
        _diagnostics.ReportUnexpectedToken(currentToken.Span, convertSyntaxKindToString(currentToken.Kind), convertSyntaxKindToString(kind));
        return nullptr;
    }
}
SyntaxNode *Parser::ParsePrimaryExpression()
{

    switch (currentToken.Kind)
    {
    case SyntaxKind::LPAREN:
    {
        NextToken();
        SyntaxNode *expression = ParseExpression();
        Expect(SyntaxKind::RPAREN);
        return new ParenthesizedExpressionNode(expression);
        break;
    }
    case SyntaxKind::TRUE:
    case SyntaxKind::FALSE:
    {
        SyntaxNode *node = new LiteralExpressionNode(currentToken);
        NextToken();
        return node;
        break;
    }
    case SyntaxKind::NUMBER:
    {
        SyntaxNode *node = new LiteralExpressionNode(currentToken);
        NextToken();
        return node;
        break;
    }
    case SyntaxKind::IDENTIFIER:
    {
        SyntaxNode *node = new NameExpressionNode(currentToken);
        NextToken();
        return node;
        break;
    }
    default:
        _diagnostics.ReportUnexpectedToken(currentToken.Span, convertSyntaxKindToString(currentToken.Kind), "Primary Expression");
        return nullptr;
        break;
    }
}
