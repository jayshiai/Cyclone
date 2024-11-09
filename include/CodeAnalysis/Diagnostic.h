#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include "CodeAnalysis/SourceText.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

class Diagnostic
{
public:
    TextLocation Location;
    std::string Message;

    Diagnostic(TextLocation location, std::string message)
        : Location(location), Message(message) {}
    std::string ToString()
    {
        return Message;
    }
};

class DiagnosticBag
{
public:
    void AddRange(const DiagnosticBag &diagnostics);

    void ReportInvalidNumber(const TextLocation &location, const std::string &text, const std::string &type);

    void ReportBadCharacter(TextLocation location, char character);
    void ReportUnterminatedString(TextLocation location);
    void ReportUnexpectedToken(const TextLocation &location, const std::string &actualKind, const std::string &expectedKind);

    void ReportUndefinedUnaryOperator(const TextLocation &location, const std::string &operatorText, const std::string &operandType);

    void ReportCannotConvert(TextLocation location, std::string fromType, std::string toType);

    void ReportVariableAlreadyDeclared(TextLocation location, std::string name);

    void ReportCannotAssign(TextLocation location, std::string name);
    void ReportUndefinedBinaryOperator(const TextLocation &location, const std::string &operatorText, const std::string &leftType, const std::string &rightType);

    void ReportParameterAlreadyDeclared(TextLocation location, std::string name);
    void ReportUndefinedName(const TextLocation &location, const std::string &name);
    void ReportUndefinedType(const TextLocation &location, const std::string &name);
    void ReportCannotConvertImplicitly(const TextLocation &location, const std::string &fromType, const std::string &toType);
    void ReportSymbolAlreadyDeclared(const TextLocation &location, const std::string &name);

    void ReportInvalidBreakOrContinue(TextLocation location, std::string text);
    void ReportAllPathsMustReturn(TextLocation location);
    void ReportInvalidReturn(TextLocation location);
    void ReportInvalidReturnExpression(TextLocation location, std::string name);
    void ReportMissingReturnExpression(TextLocation location, std::string name);

    void ReportUndefinedFunction(const TextLocation &location, const std::string &name);
    void ReportWrongArgumentCount(const TextLocation &location, const std::string &name, int expectedCount, int actualCount);
    void ReportWrongArgumentType(const TextLocation &location, const std::string &name, const std::string &expectedType, const std::string &actualType);
    void ReportExpressionMustHaveValue(const TextLocation &location);

    const std::vector<Diagnostic> &GetDiagnostics() const
    {
        return _diagnostics;
    }

private:
    std::vector<Diagnostic> _diagnostics;

    void Report(const TextLocation &location, const std::string &message);
};

#endif