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
    Compilation(Compilation *previous, std::vector<SyntaxTree *> syntaxTrees) : Previous(previous), syntaxTrees(syntaxTrees) {}
    Compilation(std::vector<SyntaxTree *> syntaxTrees) : Compilation(nullptr, syntaxTrees) {}
    Compilation *Previous;

    BoundGlobalScope *GlobalScope();
    EvaluationResult Evaluate(std::unordered_map<VariableSymbol, std::any> &variables);
    EvaluationResult Compile(std::unordered_map<VariableSymbol, std::any> &variables, std::string outputFileName = "");
    Compilation *ContinueWith(SyntaxTree *syntaxTree);

    void EmitTree(std::ostream &os);
    void EmitBoundTree(std::ostream &os);

private:
    BoundGlobalScope *_globalScope = nullptr;
    std::vector<SyntaxTree *> syntaxTrees;
};

#endif