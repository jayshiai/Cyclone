#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "CodeAnalysis/Binder.h"
#include <any>
#include <unordered_map>

class Evaluator
{
public:
    BoundExpression *_root;
    std::unordered_map<VariableSymbol, std::any> &_variables;
    Evaluator(BoundExpression *root, std::unordered_map<VariableSymbol, std::any> &variables) : _root(root), _variables(variables) {}
    std::any Evaluate()
    {
        return EvaluateExpression(_root);
    };

    std::any EvaluateExpression(BoundExpression *node);
};

#endif
