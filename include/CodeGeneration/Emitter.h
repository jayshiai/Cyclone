#ifndef EMITTER_H
#define EMITTER_H

#include "CodeAnalysis/Binder.h"
#include <unordered_map>
#include <iostream>
#include <fstream>
class Emitter
{
public:
    Emitter(std::string filename, BoundProgram *program, std::unordered_map<VariableSymbol, std::any> &variables) : _program(program), _globals(variables), Filename(filename)
    {
        _locals.push(std::unordered_map<VariableSymbol, std::any>());
    }
    void Emit();

private:
    BoundProgram *_program;
    std::string Filename;
    std::ofstream codeStream;
    std::unordered_map<VariableSymbol, std::any> &_globals;
    std::stack<std::unordered_map<VariableSymbol, std::any>> _locals;

    void EmitIncludes();
    void EmitFunctions();
    void EmitStatement(BoundBlockStatement *node);
    void EmitVariableDeclaration(BoundVariableDeclaration *node);
    void EmitExpressionStatement(BoundExpressionStatement *node);
    void EmitGotoStatement(BoundGotoStatement *node);
    void EmitConditionalGotoStatement(BoundConditionalGotoStatement *node);
    void EmitLabelStatement(BoundLabelStatement *node);
    void EmitReturnStatement(BoundReturnStatement *node);

    void EmitExpression(BoundExpression *node);
    void EmitLiteralExpression(BoundLiteralExpression *node);
    void EmitConversionExpression(BoundConversionExpression *node);
    void EmitCallExpression(BoundCallExpression *node);
    void EmitArrayInitializerExpression(BoundArrayInitializerExpression *node);
    void EmitArrayAccessExpression(BoundArrayAccessExpression *node);

    void EmitType(TypeSymbol type);

    std::string EscapeString(const std::string &str);
};

#endif