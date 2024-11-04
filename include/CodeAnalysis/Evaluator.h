#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "CodeAnalysis/Binder.h"
#include <any>
#include <unordered_map>

class Evaluator
{
public:
    BoundBlockStatement *_root;
    std::unordered_map<VariableSymbol, std::any> &_variables;
    Evaluator(BoundBlockStatement *root, std::unordered_map<VariableSymbol, std::any> &variables) : _root(root), _variables(variables) {}
    std::any Evaluate();

    std::any EvaluateExpression(BoundExpression *node);

private:
    void EvaluateExpressionStatement(BoundExpressionStatement *node);
    void EvaluateVariableDeclaration(BoundVariableDeclaration *node);

    std::any EvaluateLiteralExpression(BoundLiteralExpression *node);
    std::any EvaluateVariableExpression(BoundVariableExpression *node);
    std::any EvaluateAssignmentExpression(BoundAssignmentExpression *node);
    std::any EvaluateUnaryExpression(BoundUnaryExpression *node);
    std::any EvaluateBinaryExpression(BoundBinaryExpression *node);
    std::any EvaluateCallExpression(BoundCallExpression *node);
    std::any EvaluateConversionExpression(BoundConversionExpression *node);
    std::any _lastValue = "";
};

#endif
