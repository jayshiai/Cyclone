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
    virtual BoundStatement *RewriteReturnStatement(BoundReturnStatement *node);

    virtual BoundExpression *RewriteExpression(BoundExpression *node);
    virtual BoundExpression *RewriteErrorExpression(BoundErrorExpression *node);
    virtual BoundExpression *RewriteUnaryExpression(BoundUnaryExpression *node);
    virtual BoundExpression *RewriteBinaryExpression(BoundBinaryExpression *node);
    virtual BoundExpression *RewriteLiteralExpression(BoundLiteralExpression *node);
    virtual BoundExpression *RewriteVariableExpression(BoundVariableExpression *node);
    virtual BoundExpression *RewriteAssignmentExpression(BoundAssignmentExpression *node);
    virtual BoundExpression *RewriteCallExpression(BoundCallExpression *node);
    virtual BoundExpression *RewriteConversionExpression(BoundConversionExpression *node);
    virtual BoundExpression *RewriteArrayInitializerExpression(BoundArrayInitializerExpression *node);
    virtual BoundExpression *RewriteArrayAccessExpression(BoundArrayAccessExpression *node);
    virtual BoundExpression *RewriteArrayAssignmentExpression(BoundArrayAssignmentExpression *node);
};

class Lowerer : public BoundTreeRewriter
{
public:
    int labelCount = 0;

    Lowerer() {}

    static BoundBlockStatement *Lower(BoundStatement *node);
    static BoundBlockStatement *LowerEmitable(BoundStatement *node);
    static BoundBlockStatement *Flatten(BoundStatement *node);

protected:
    BoundStatement *RewriteIfStatement(BoundIfStatement *node) override;
    BoundStatement *RewriteWhileStatement(BoundWhileStatement *node) override;
    BoundStatement *RewriteForStatement(BoundForStatement *node) override;

private:
    BoundLabel *GenerateLabel();
};
#endif