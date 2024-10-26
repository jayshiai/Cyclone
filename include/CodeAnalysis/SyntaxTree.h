#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include "CodeAnalysis/Diagnostic.h"
#include <vector>
#include <string>
#include <any>
enum class SyntaxKind
{
    NUMBER,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    LPAREN, // (
    RPAREN, // )
    TRUE,
    FALSE,
    BANG,
    EQUALS,
    EQUALS_EQUALS,
    BANG_EQUALS,
    AMPERSAND_AMPERSAND,
    PIPE_PIPE,
    IDENTIFIER,
    WHITESPACE,
    END_OF_FILE,
    BAD_TOKEN,

    LiteralExpression,
    UnaryExpression,
    BinaryExpression,
    ParenthesizedExpression,
    NameExpression,
    AssignmentExpression,
};

class SyntaxNode
{
public:
    virtual ~SyntaxNode() = default;
    SyntaxKind Kind;
    virtual std::vector<SyntaxNode *> GetChildren() const { return {}; }

protected:
    SyntaxNode(SyntaxKind kind) : Kind(kind) {}
};

class Token : public SyntaxNode
{
public:
    std::string value;
    size_t position;
    TextSpan Span;
    Token(SyntaxKind kind, std::string value, size_t position) : SyntaxNode(kind), value(value), position(position), Span(TextSpan(position, value.size())) {}
    Token();
};

class LiteralExpressionNode : public SyntaxNode
{
public:
    Token LiteralToken;
    std::any Value;
    LiteralExpressionNode(Token LiteralToken)
        : SyntaxNode(SyntaxKind::LiteralExpression), LiteralToken(LiteralToken), Value(LiteralToken.value) {}
    LiteralExpressionNode(Token LiteralToken, std::any Value)
        : SyntaxNode(SyntaxKind::LiteralExpression), LiteralToken(LiteralToken), Value(Value) {}
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&LiteralToken))};
    }
};

class NameExpressionNode : public SyntaxNode
{
public:
    Token IdentifierToken;
    NameExpressionNode(Token IdentifierToken)
        : SyntaxNode(SyntaxKind::NameExpression), IdentifierToken(IdentifierToken) {}
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&IdentifierToken))};
    }
};

class AssignmentExpressionNode : public SyntaxNode
{
public:
    Token IdentifierToken;
    Token EqualsToken;
    SyntaxNode *Expression;
    AssignmentExpressionNode(Token IdentifierToken, Token EqualsToken, SyntaxNode *Expression)
        : SyntaxNode(SyntaxKind::AssignmentExpression), IdentifierToken(IdentifierToken), EqualsToken(EqualsToken), Expression(Expression) {}
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&IdentifierToken)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&EqualsToken)), Expression};
    }
};
class BinaryExpressionNode : public SyntaxNode
{
public:
    SyntaxNode *left;
    SyntaxNode *right;
    Token OperatorToken;
    BinaryExpressionNode(SyntaxNode *left, SyntaxNode *right, Token OperatorToken)
        : SyntaxNode(SyntaxKind::BinaryExpression), left(left), right(right), OperatorToken(OperatorToken) {}

    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {left, const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&OperatorToken)), right};
    }
};

class ParenthesizedExpressionNode : public SyntaxNode
{
public:
    SyntaxNode *expression;
    ParenthesizedExpressionNode(SyntaxNode *expression)
        : SyntaxNode(SyntaxKind::ParenthesizedExpression), expression(expression) {}
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {expression};
    }
};

class UnaryExpressionNode : public SyntaxNode
{
public:
    SyntaxNode *expression;
    Token OperatorToken;
    UnaryExpressionNode(SyntaxNode *expression, Token OperatorToken)
        : SyntaxNode(SyntaxKind::UnaryExpression), expression(expression), OperatorToken(OperatorToken) {}
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&OperatorToken)), expression};
    }
};

class SyntaxTree
{
public:
    SyntaxTree(SyntaxNode *root, DiagnosticBag _diagnostics) : root(root), Diagnostics(_diagnostics) {};
    SyntaxNode *root;
    DiagnosticBag Diagnostics;
};

#endif
