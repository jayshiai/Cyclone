#ifndef LOWERER_H
#define LOWERER_H

#include "CodeAnalysis/Binder.h"
#include <stack>
class BoundTreeRewriter
{
public:
    virtual BoundStatement *RewriteStatement(BoundStatement *node);

protected:
    virtual BoundStatement *RewriteBlockStatement(BoundBlockStatement *node);
    virtual BoundStatement *RewriteVariableDeclaration(BoundVariableDeclaration *node);
    virtual BoundStatement *RewriteIfStatement(BoundIfStatement *node);
    virtual BoundStatement *RewriteWhileStatement(BoundWhileStatement *node);
    virtual BoundStatement *RewriteForStatement(BoundForStatement *node);
    virtual BoundStatement *RewriteGotoStatement(BoundGotoStatement *node);
    virtual BoundStatement *RewriteLabelStatement(BoundLabelStatement *node);
    virtual BoundStatement *RewriteConditionalGotoStatement(BoundConditionalGotoStatement *node);
    virtual BoundStatement *RewriteExpressionStatement(BoundExpressionStatement *node);
    virtual BoundExpression *RewriteExpression(BoundExpression *node);
    virtual BoundExpression *RewriteUnaryExpression(BoundUnaryExpression *node);
    virtual BoundExpression *RewriteBinaryExpression(BoundBinaryExpression *node);
    virtual BoundExpression *RewriteLiteralExpression(BoundLiteralExpression *node);
    virtual BoundExpression *RewriteVariableExpression(BoundVariableExpression *node);
    virtual BoundExpression *RewriteAssignmentExpression(BoundAssignmentExpression *node);
};

class Lowerer : public BoundTreeRewriter
{
public:
    int labelCount = 0;

    Lowerer() {}

    static BoundBlockStatement *Lower(BoundStatement *node);

protected:
    BoundStatement *RewriteIfStatement(BoundIfStatement *node) override;
    BoundStatement *RewriteWhileStatement(BoundWhileStatement *node) override;
    BoundStatement *RewriteForStatement(BoundForStatement *node) override;

private:
    static BoundBlockStatement *Flatten(BoundStatement *node);
    LabelSymbol *GenerateLabel();
};
#endif