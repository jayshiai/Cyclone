#include "CodeAnalysis/Lowerer.h"

BoundBlockStatement *Lowerer::Lower(BoundStatement *node)
{
    Lowerer lowerer;
    BoundStatement *result = lowerer.RewriteStatement(node);
    return Flatten(static_cast<BoundBlockStatement *>(result));
}
BoundBlockStatement *Lowerer::LowerEmitable(BoundStatement *node)
{
    Lowerer lowerer;
    BoundStatement *result = lowerer.RewriteStatement(node);
    return static_cast<BoundBlockStatement *>(result);
}

BoundStatement *Lowerer::RewriteIfStatement(BoundIfStatement *node)
{
    if (node->ElseStatement == nullptr)
    {
        BoundLabel *endLabel = GenerateLabel();
        BoundStatement *gotoFalse = new BoundConditionalGotoStatement(*endLabel, node->Condition, false);
        BoundStatement *endLabelStatement = new BoundLabelStatement(*endLabel);
        BoundBlockStatement *result = new BoundBlockStatement({gotoFalse, node->ThenStatement, endLabelStatement});
        return RewriteStatement(result);
    }
    else
    {

        BoundLabel *elseLabel = GenerateLabel();
        BoundLabel *endLabel = GenerateLabel();
        BoundStatement *gotoFalse = new BoundConditionalGotoStatement(*elseLabel, node->Condition, false);
        BoundStatement *gotoEnd = new BoundGotoStatement(*endLabel);
        BoundStatement *elseLabelStatement = new BoundLabelStatement(*elseLabel);
        BoundStatement *endLabelStatement = new BoundLabelStatement(*endLabel);
        BoundBlockStatement *result = new BoundBlockStatement({gotoFalse, node->ThenStatement, gotoEnd, elseLabelStatement, node->ElseStatement, endLabelStatement});
        return RewriteStatement(result);
    }
}

BoundStatement *Lowerer::RewriteWhileStatement(BoundWhileStatement *node)
{
    BoundLabel *bodyLabel = GenerateLabel();

    BoundGotoStatement *gotoContinue = new BoundGotoStatement(*node->ContinueLabel);
    BoundLabelStatement *bodyLabelStatement = new BoundLabelStatement(*bodyLabel);

    BoundLabelStatement *continueLabelStatement = new BoundLabelStatement(*node->ContinueLabel);

    BoundConditionalGotoStatement *gotoTrue = new BoundConditionalGotoStatement(*bodyLabel, node->Condition);
    BoundLabelStatement *breakLabelStatement = new BoundLabelStatement(*node->BreakLabel);

    BoundBlockStatement *result = new BoundBlockStatement({gotoContinue,
                                                           bodyLabelStatement,
                                                           node->Body,
                                                           continueLabelStatement,
                                                           gotoTrue,
                                                           breakLabelStatement});
    return RewriteStatement(result);
}

BoundStatement *Lowerer::RewriteForStatement(BoundForStatement *node)
{
    BoundVariableDeclaration *variableDeclaration = new BoundVariableDeclaration(node->Variable, node->LowerBound);
    BoundVariableExpression *variableExpression = new BoundVariableExpression(node->Variable);

    LocalVariableSymbol *upperBoundSymbol = new LocalVariableSymbol("upperBound", true, TypeSymbol::Integer);
    BoundVariableDeclaration *upperBoundDeclaration = new BoundVariableDeclaration(*upperBoundSymbol, node->UpperBound);

    BoundBinaryOperator *lessOrEquals = BoundBinaryOperator::Bind(SyntaxKind::LESS_EQUALS, TypeSymbol::Integer, TypeSymbol::Integer);
    BoundBinaryExpression *condition = new BoundBinaryExpression(variableExpression, lessOrEquals, new BoundVariableExpression(*upperBoundSymbol));

    BoundLabelStatement *continueLabelStatement = new BoundLabelStatement(*node->ContinueLabel);
    BoundExpressionStatement *increment = new BoundExpressionStatement(new BoundAssignmentExpression(node->Variable, new BoundBinaryExpression(variableExpression, BoundBinaryOperator::Bind(SyntaxKind::PLUS, TypeSymbol::Integer, TypeSymbol::Integer), new BoundLiteralExpression("1", TypeSymbol::Integer))));

    BoundBlockStatement *whileBody = new BoundBlockStatement({node->Body, continueLabelStatement, increment});
    BoundWhileStatement *whileStatement = new BoundWhileStatement(condition, whileBody, node->BreakLabel, GenerateLabel());
    BoundBlockStatement *result = new BoundBlockStatement({variableDeclaration,
                                                           upperBoundDeclaration,
                                                           whileStatement});

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

BoundLabel *Lowerer::GenerateLabel()
{
    return new BoundLabel("Label" + std::to_string(++labelCount));
}