#ifndef COMPILATION_H
#define COMPILATION_H
#include "CodeAnalysis/Diagnostic.h"
#include "CodeAnalysis/SyntaxTree.h"
#include <any>
#include <unordered_map>

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
    Compilation(SyntaxTree *syntaxTree) : SyntaxTree(syntaxTree) {}

    EvaluationResult Evaluate(std::unordered_map<VariableSymbol, std::any> &variables);

private:
    SyntaxTree *SyntaxTree;
};

#endif