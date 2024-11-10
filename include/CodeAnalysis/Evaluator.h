#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "CodeAnalysis/Binder.h"
#include <any>
#include <unordered_map>
#include <stack>

class Evaluator
{
public:
    Evaluator(BoundProgram *program, std::unordered_map<VariableSymbol, std::any> &variables) : _program(program), _globals(variables)
    {
        _locals.push(std::unordered_map<VariableSymbol, std::any>());
    }
    std::any Evaluate()
    {
        return EvaluateStatement(_program->statement);
    };

    std::any EvaluateExpression(BoundExpression *node);

private:
    BoundProgram *_program;
    std::unordered_map<VariableSymbol, std::any> &_globals;
    std::stack<std::unordered_map<VariableSymbol, std::any>> _locals;

    void EvaluateExpressionStatement(BoundExpressionStatement *node);
    void EvaluateVariableDeclaration(BoundVariableDeclaration *node);
    void Assign(VariableSymbol variable, std::any value);
    void AssignArray(VariableSymbol variable, std::any value, int index);
    std::any EvaluateStatement(BoundBlockStatement *node);

    std::any EvaluateLiteralExpression(BoundLiteralExpression *node);
    std::any EvaluateVariableExpression(BoundVariableExpression *node);
    std::any EvaluateAssignmentExpression(BoundAssignmentExpression *node);
    std::any EvaluateUnaryExpression(BoundUnaryExpression *node);
    std::any EvaluateBinaryExpression(BoundBinaryExpression *node);
    std::any EvaluateCallExpression(BoundCallExpression *node);
    std::any EvaluateConversionExpression(BoundConversionExpression *node);
    std::any EvaluateArrayInitializerExpression(BoundArrayInitializerExpression *node);
    std::any EvaluateArrayAccessExpression(BoundArrayAccessExpression *node);
    std::any EvaluateArrayAssignmentExpression(BoundArrayAssignmentExpression *node);
    std::any _lastValue = "";
};

#endif
