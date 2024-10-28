#ifndef COMPILATION_H
#define COMPILATION_H
#include "CodeAnalysis/Diagnostic.h"
#include "CodeAnalysis/SyntaxTree.h"
#include "CodeAnalysis/Binder.h"
#include <any>
#include <unordered_map>
#include <atomic>
class EvaluationResult
{
public:
    EvaluationResult(std::vector<Diagnostic> diagnostics, std::any value) : Diagnostics(diagnostics), Value(value) {}
    std::vector<Diagnostic> Diagnostics;
    std::any Value;
};

class Compilation
{
public:
    Compilation(Compilation *previous, SyntaxTree *syntaxTree) : Previous(previous), syntaxTree(syntaxTree) {}
    Compilation(SyntaxTree *syntaxTree) : Compilation(nullptr, syntaxTree) {}
    Compilation *Previous;

    BoundGlobalScope *GlobalScope();
    EvaluationResult Evaluate(std::unordered_map<VariableSymbol, std::any> &variables);
    Compilation *ContinueWith(SyntaxTree *syntaxTree);

private:
    BoundGlobalScope *_globalScope = nullptr;
    SyntaxTree *syntaxTree;
};

#endif