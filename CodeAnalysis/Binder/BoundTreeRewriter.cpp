#include "CodeAnalysis/Lowerer.h"

BoundStatement *BoundTreeRewriter::RewriteStatement(BoundStatement *node)
{
    switch (node->GetKind())
    {
    case BoundNodeKind::BlockStatement:
        return RewriteBlockStatement((BoundBlockStatement *)node);
    case BoundNodeKind::ExpressionStatement:
        return RewriteExpressionStatement((BoundExpressionStatement *)node);
    case BoundNodeKind::VariableDeclaration:
        return RewriteVariableDeclaration((BoundVariableDeclaration *)node);
    case BoundNodeKind::IfStatement:
        return RewriteIfStatement((BoundIfStatement *)node);
    case BoundNodeKind::WhileStatement:
        return RewriteWhileStatement((BoundWhileStatement *)node);
    case BoundNodeKind::ForStatement:
        return RewriteForStatement((BoundForStatement *)node);
    case BoundNodeKind::GotoStatement:
        return RewriteGotoStatement((BoundGotoStatement *)node);
    case BoundNodeKind::LabelStatement:
        return RewriteLabelStatement((BoundLabelStatement *)node);
    case BoundNodeKind::ConditionalGotoStatement:
        return RewriteConditionalGotoStatement((BoundConditionalGotoStatement *)node);
    default:
        return node;
    }
}

BoundStatement *BoundTreeRewriter::RewriteBlockStatement(BoundBlockStatement *node)
{
    std::vector<BoundStatement *> newStatements;
    bool hasChanges = false;

    for (BoundStatement *statement : node->Statements)
    {
        BoundStatement *newStatement = RewriteStatement(statement);
        newStatements.push_back(newStatement);
        if (newStatement != statement)
            hasChanges = true;
    }

    if (!hasChanges)
        return node;

    return new BoundBlockStatement(newStatements);
}

BoundStatement *BoundTreeRewriter::RewriteVariableDeclaration(BoundVariableDeclaration *node)
{
    BoundExpression *initializer = RewriteExpression(node->Initializer);
    if (initializer == node->Initializer)
        return node;

    return new BoundVariableDeclaration(node->Variable, initializer);
}

BoundStatement *BoundTreeRewriter::RewriteIfStatement(BoundIfStatement *node)
{
    BoundExpression *condition = RewriteExpression(node->Condition);
    BoundStatement *thenStatement = RewriteStatement(node->ThenStatement);
    BoundStatement *elseStatement = node->ElseStatement == nullptr ? nullptr : RewriteStatement(node->ElseStatement);

    if (condition == node->Condition && thenStatement == node->ThenStatement && elseStatement == node->ElseStatement)
        return node;

    return new BoundIfStatement(condition, thenStatement, elseStatement);
}

BoundStatement *BoundTreeRewriter::RewriteWhileStatement(BoundWhileStatement *node)
{
    BoundExpression *condition = RewriteExpression(node->Condition);
    BoundStatement *body = RewriteStatement(node->Body);

    if (condition == node->Condition && body == node->Body)
        return node;

    return new BoundWhileStatement(condition, body);
}

BoundStatement *BoundTreeRewriter::RewriteForStatement(BoundForStatement *node)
{
    BoundExpression *lowerBound = RewriteExpression(node->LowerBound);
    BoundExpression *upperBound = RewriteExpression(node->UpperBound);
    BoundStatement *body = RewriteStatement(node->Body);

    if (lowerBound == node->LowerBound && upperBound == node->UpperBound && body == node->Body)
        return node;

    return new BoundForStatement(node->Variable, lowerBound, upperBound, body);
}

BoundStatement *BoundTreeRewriter::RewriteGotoStatement(BoundGotoStatement *node)
{
    return node;
}

BoundStatement *BoundTreeRewriter::RewriteLabelStatement(BoundLabelStatement *node)
{
    return node;
}

BoundStatement *BoundTreeRewriter::RewriteConditionalGotoStatement(BoundConditionalGotoStatement *node)
{
    BoundExpression *condition = RewriteExpression(node->Condition);
    if (condition == node->Condition)
        return node;

    return new BoundConditionalGotoStatement(node->Label, condition, node->JumpIfTrue);
}

BoundStatement *BoundTreeRewriter::RewriteExpressionStatement(BoundExpressionStatement *node)
{
    BoundExpression *expression = RewriteExpression(node->Expression);
    if (expression == node->Expression)
        return node;

    return new BoundExpressionStatement(expression);
}

BoundExpression *BoundTreeRewriter::RewriteExpression(BoundExpression *node)
{
    switch (node->GetKind())
    {
    case BoundNodeKind::ErrorExpression:
        return RewriteErrorExpression((BoundErrorExpression *)node);
    case BoundNodeKind::UnaryExpression:
        return RewriteUnaryExpression((BoundUnaryExpression *)node);
    case BoundNodeKind::BinaryExpression:
        return RewriteBinaryExpression((BoundBinaryExpression *)node);
    case BoundNodeKind::LiteralExpression:
        return RewriteLiteralExpression((BoundLiteralExpression *)node);
    case BoundNodeKind::VariableExpression:
        return RewriteVariableExpression((BoundVariableExpression *)node);
    case BoundNodeKind::AssignmentExpression:
        return RewriteAssignmentExpression((BoundAssignmentExpression *)node);
    default:
        throw std::runtime_error("Unexpected node: " + convertBoundNodeKindToString(node->GetKind()));
        return node;
    }
}

BoundExpression *BoundTreeRewriter::RewriteErrorExpression(BoundErrorExpression *node)
{
    return node;
}

BoundExpression *BoundTreeRewriter::RewriteUnaryExpression(BoundUnaryExpression *node)
{
    BoundExpression *operand = RewriteExpression(node->Operand);
    if (operand == node->Operand)
        return node;

    return new BoundUnaryExpression(node->Op, operand);
}

BoundExpression *BoundTreeRewriter::RewriteBinaryExpression(BoundBinaryExpression *node)
{
    BoundExpression *left = RewriteExpression(node->Left);
    BoundExpression *right = RewriteExpression(node->Right);

    if (left == node->Left && right == node->Right)
        return node;

    return new BoundBinaryExpression(left, node->Op, right);
}

BoundExpression *BoundTreeRewriter::RewriteLiteralExpression(BoundLiteralExpression *node)
{
    return node;
}

BoundExpression *BoundTreeRewriter::RewriteVariableExpression(BoundVariableExpression *node)
{
    return node;
}

BoundExpression *BoundTreeRewriter::RewriteAssignmentExpression(BoundAssignmentExpression *node)
{
    BoundExpression *expression = RewriteExpression(node->Expression);
    if (expression == node->Expression)
        return node;

    return new BoundAssignmentExpression(node->Variable, expression);
}
