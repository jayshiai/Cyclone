#include "CodeAnalysis/Lowerer.h"

BoundBlockStatement *Lowerer::Lower(BoundStatement *node)
{
    Lowerer lowerer;
    BoundStatement *result = lowerer.RewriteStatement(node);
    return Flatten(static_cast<BoundBlockStatement *>(result));
}

BoundStatement *Lowerer::RewriteIfStatement(BoundIfStatement *node)
{
    if (node->ElseStatement == nullptr)
    {
        LabelSymbol *endLabel = GenerateLabel();
        BoundStatement *gotoFalse = new BoundConditionalGotoStatement(*endLabel, node->Condition, true);
        BoundStatement *endLabelStatement = new BoundLabelStatement(*endLabel);
        BoundBlockStatement *result = new BoundBlockStatement({node->ThenStatement, gotoFalse, endLabelStatement});
        return RewriteStatement(result);
    }
    else
    {

        LabelSymbol *elseLabel = GenerateLabel();
        LabelSymbol *endLabel = GenerateLabel();
        BoundStatement *gotoFalse = new BoundConditionalGotoStatement(*elseLabel, node->Condition, true);
        BoundStatement *gotoEnd = new BoundGotoStatement(*endLabel);
        BoundStatement *elseLabelStatement = new BoundLabelStatement(*elseLabel);
        BoundStatement *endLabelStatement = new BoundLabelStatement(*endLabel);
        BoundBlockStatement *result = new BoundBlockStatement({gotoFalse, node->ThenStatement, gotoEnd, elseLabelStatement, node->ElseStatement, endLabelStatement});
        return RewriteStatement(result);
    }
}

BoundStatement *Lowerer::RewriteWhileStatement(BoundWhileStatement *node)
{
    LabelSymbol *continueLabel = GenerateLabel();
    LabelSymbol *checkLabel = GenerateLabel();
    LabelSymbol *endLabel = GenerateLabel();

    BoundGotoStatement *gotoCheck = new BoundGotoStatement(*checkLabel);
    BoundLabelStatement *continueLabelStatement = new BoundLabelStatement(*continueLabel);
    BoundLabelStatement *checkLabelStatement = new BoundLabelStatement(*checkLabel);
    BoundConditionalGotoStatement *gotoTrue = new BoundConditionalGotoStatement(*continueLabel, node->Condition, false);
    BoundLabelStatement *endLabelStatement = new BoundLabelStatement(*endLabel);

    BoundBlockStatement *result = new BoundBlockStatement({gotoCheck, continueLabelStatement, node->Body, checkLabelStatement, gotoTrue, endLabelStatement});
    return RewriteStatement(result);
}

BoundStatement *Lowerer::RewriteForStatement(BoundForStatement *node)
{
    BoundVariableDeclaration *variableDeclaration = new BoundVariableDeclaration(node->Variable, node->LowerBound);
    BoundVariableExpression *variableExpression = new BoundVariableExpression(node->Variable);
    BoundBinaryOperator *lessOrEquals = BoundBinaryOperator::Bind(SyntaxKind::LESS_EQUALS, Type::Integer, Type::Integer);
    BoundBinaryExpression *condition = new BoundBinaryExpression(variableExpression, lessOrEquals, node->UpperBound);

    BoundExpressionStatement *increment = new BoundExpressionStatement(new BoundAssignmentExpression(node->Variable, new BoundBinaryExpression(variableExpression, BoundBinaryOperator::Bind(SyntaxKind::PLUS, Type::Integer, Type::Integer), new BoundLiteralExpression("1", Type::Integer))));

    BoundBlockStatement *whileBody = new BoundBlockStatement({node->Body, increment});
    BoundWhileStatement *whileStatement = new BoundWhileStatement(condition, whileBody);
    BoundBlockStatement *result = new BoundBlockStatement({variableDeclaration, whileStatement});

    return RewriteStatement(result);
}

BoundBlockStatement *Lowerer::Flatten(BoundStatement *node)
{
    std::vector<BoundStatement *> statements;
    std::stack<BoundStatement *> stack;
    stack.push(node);

    while (!stack.empty())
    {
        BoundStatement *current = stack.top();
        stack.pop();

        if (current->GetKind() == BoundNodeKind::BlockStatement)
        {
            BoundBlockStatement *block = static_cast<BoundBlockStatement *>(current);
            for (int i = block->Statements.size() - 1; i >= 0; i--)
            {
                stack.push(block->Statements[i]);
            }
        }
        else
        {
            statements.push_back(current);
        }
    }

    return new BoundBlockStatement(statements);
}

LabelSymbol *Lowerer::GenerateLabel()
{
    return new LabelSymbol("Label" + std::to_string(labelCount++));
}