#ifndef UTILS_H
#define UTILS_H
#include "CodeAnalysis/SyntaxTree.h"
#include "CodeAnalysis/Binder.h"
#include <unordered_map>

void PrintDiagnostic(Diagnostic diagnostic, SourceText Text);
void PrintVariables(const std::unordered_map<VariableSymbol, std::any> &variables);
#endif