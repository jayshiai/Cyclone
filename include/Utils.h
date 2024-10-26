#ifndef UTILS_H
#define UTILS_H
#include "CodeAnalysis/SyntaxTree.h"
#include "CodeAnalysis/Binder.h"
std::string convertSyntaxKindToString(SyntaxKind kind);
std::string convertTypetoString(Type type);
#endif