#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include <vector>
#include <string>

class SyntaxNode
{
public:
    virtual ~SyntaxNode() = default;
    virtual std::string getType() const = 0;
};

class NumericLiteralNode : public SyntaxNode
{
public:
    std::string getType() const override
    {
        return "NumericLiteral";
    }
    std::string value;
    NumericLiteralNode(std::string value) : value(value) {
                                            };
};

class BinaryExpressionNode : public SyntaxNode
{
public:
    std::string getType() const override
    {
        return "BinaryExpression";
    }
    SyntaxNode *left;
    SyntaxNode *right;
    std::string op;
    BinaryExpressionNode(SyntaxNode *left, SyntaxNode *right, std::string op) : left(left), right(right), op(op) {};

private:
};

class ParenthesizedExpressionNode : public SyntaxNode
{
public:
    std::string getType() const override
    {
        return "ParenthesizedExpression";
    }
    SyntaxNode *expression;
    ParenthesizedExpressionNode(SyntaxNode *expression) : expression(expression) {};
};

class SyntaxTree
{
public:
    SyntaxTree(SyntaxNode *root, std::vector<std::string> diagnostics) : root(root), diagnostics(diagnostics) {};
    SyntaxNode *root;
    std::vector<std::string> diagnostics;
};

#endif
