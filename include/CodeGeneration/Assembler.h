#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "CodeAnalysis/Binder.h"
#include <unordered_map>
#include <iostream>
#include <fstream>

class Assembler
{
public:
    Assembler(std::string filename, BoundProgram *program, std::unordered_map<VariableSymbol, std::any> &variables) : _program(program), _globals(variables), Filename(filename)
    {
        _locals.push(std::unordered_map<VariableSymbol, std::any>());
    }
    void Assemble();

private:
    std::unordered_map<VariableSymbol, std::any> &_globals;
    std::stack<std::unordered_map<VariableSymbol, std::any>> _locals;
    std::unordered_map<VariableSymbol, std::any> _variablesMap;
    int _currentVariableOffset = 0;
    BoundProgram *_program;
    std::string Filename;
    std::ofstream codeStream;
    std::ofstream dataStream;

    void AssembleFunctions();
    void AssembleStatement(BoundBlockStatement *node);
    void AssembleVariableDeclaration(BoundVariableDeclaration *node);
    void AssembleArrayDeclaration(BoundVariableDeclaration *node);
    void AssembleExpressionStatement(BoundExpressionStatement *node);
    void AssembleGotoStatement(BoundGotoStatement *node);
    void AssembleConditionalGotoStatement(BoundConditionalGotoStatement *node);
    void AssembleLabelStatement(BoundLabelStatement *node);
    void AssembleReturnStatement(BoundReturnStatement *node);

    void AssembleExpression(BoundExpression *node);
    void AssembleLiteralExpression(BoundLiteralExpression *node);
    void AssembleVariableExpression(BoundVariableExpression *node);
    void AssembleAssignmentExpression(BoundAssignmentExpression *node);
    void AssembleConversionExpression(BoundConversionExpression *node);
    void AssembleCallExpression(BoundCallExpression *node);
    void AssembleArrayInitializerExpression(BoundArrayInitializerExpression *node);
    void AssembleArrayAccessExpression(BoundArrayAccessExpression *node);
    void AssembleArrayAssignmentExpression(BoundArrayAssignmentExpression *node);
    void AssembleUnaryExpression(BoundUnaryExpression *node);
    void AssembleBinaryExpression(BoundBinaryExpression *node);
    // void AssembleType(TypeSymbol type);

    // std::string EscapeString(const std::string &str);
};

#endif