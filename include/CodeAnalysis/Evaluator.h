#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "CodeAnalysis/Binder.h"
#include <any>
#include <unordered_map>

class Evaluator
{
public:
    BoundStatement *_root;
    std::unordered_map<VariableSymbol, std::any> &_variables;
    Evaluator(BoundStatement *root, std::unordered_map<VariableSymbol, std::any> &variables) : _root(root), _variables(variables) {}
    std::any Evaluate()
    {
        EvaluateStatement(_root);
        return _lastValue;
    };

    std::any EvaluateExpression(BoundExpression *node);

private:
    void EvaluateStatement(BoundStatement *node);
    void EvaluateBlockStatement(BoundBlockStatement *node);
    void EvaluateExpressionStatement(BoundExpressionStatement *node);
    void EvaluateVariableDeclaration(BoundVariableDeclaration *node);

    std::any EvaluateLiteralExpression(BoundLiteralExpression *node);
    std::any EvaluateVariableExpression(BoundVariableExpression *node);
    std::any EvaluateAssignmentExpression(BoundAssignmentExpression *node);
    std::any EvaluateUnaryExpression(BoundUnaryExpression *node);
    std::any EvaluateBinaryExpression(BoundBinaryExpression *node);
    std::any _lastValue;
};

#endif
