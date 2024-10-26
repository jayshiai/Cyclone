#include "CodeAnalysis/Binder.h"
#include "CodeAnalysis/Evaluator.h"
#include "CodeAnalysis/SyntaxTree.h"
#include "CodeAnalysis/Diagnostic.h"
#include "CodeAnalysis/Compilation.h"
#include <unordered_map>

EvaluationResult Compilation::Evaluate(std::unordered_map<VariableSymbol, std::any> &variables)
{
    // Create a binder with the provided variables
    Binder binder(variables);
    BoundExpression *boundExpression = binder.BindExpression(SyntaxTree->root);

    // Combine diagnostics from the SyntaxTree and the binder
    DiagnosticBag diagnostics;
    diagnostics.AddRange(SyntaxTree->Diagnostics);
    diagnostics.AddRange(binder.GetDiagnostics());

    // If any diagnostics exist, return an EvaluationResult with the diagnostics and no result
    if (!diagnostics.GetDiagnostics().empty())
    {
        return EvaluationResult(diagnostics.GetDiagnostics(), nullptr);
    }

    // Evaluate the bound expression
    Evaluator evaluator(boundExpression, variables);
    std::any value = evaluator.Evaluate();
    return EvaluationResult({}, value);
}
