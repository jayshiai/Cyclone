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
    TextSpan Span;
    std::string Message;

    Diagnostic(TextSpan span, std::string message)
        : Span(span), Message(message) {}
    std::string ToString()
    {
        return Message;
    }
};

class DiagnosticBag
{
public:
    void AddRange(const DiagnosticBag &diagnostics);

    void ReportInvalidNumber(const TextSpan &span, const std::string &text, const std::string &type);

    void ReportBadCharacter(int position, char character);
    void ReportUnterminatedString(TextSpan span);
    void ReportUnexpectedToken(const TextSpan &span, const std::string &actualKind, const std::string &expectedKind);

    void ReportUndefinedUnaryOperator(const TextSpan &span, const std::string &operatorText, const std::string &operandType);

    void ReportCannotConvert(TextSpan span, std::string fromType, std::string toType);

    void ReportVariableAlreadyDeclared(TextSpan span, std::string name);

    void ReportCannotAssign(TextSpan span, std::string name);
    void ReportUndefinedBinaryOperator(const TextSpan &span, const std::string &operatorText, const std::string &leftType, const std::string &rightType);

    void ReportParameterAlreadyDeclared(TextSpan span, std::string name);
    void ReportUndefinedName(const TextSpan &span, const std::string &name);
    void ReportUndefinedType(const TextSpan &span, const std::string &name);
    void ReportCannotConvertImplicitly(const TextSpan &span, const std::string &fromType, const std::string &toType);
    void ReportSymbolAlreadyDeclared(const TextSpan &span, const std::string &name);

    void ReportInvalidBreakOrContinue(TextSpan span, std::string text);

    void ReportInvalidReturn(TextSpan span);
    void ReportInvalidReturnExpression(TextSpan span, std::string name);
    void ReportMissingReturnExpression(TextSpan span, std::string name);

    void ReportUndefinedFunction(const TextSpan &span, const std::string &name);
    void ReportWrongArgumentCount(const TextSpan &span, const std::string &name, int expectedCount, int actualCount);
    void ReportWrongArgumentType(const TextSpan &span, const std::string &name, const std::string &expectedType, const std::string &actualType);
    void ReportExpressionMustHaveValue(const TextSpan &span);

    const std::vector<Diagnostic> &GetDiagnostics() const
    {
        return _diagnostics;
    }

private:
    std::vector<Diagnostic> _diagnostics;

    void Report(const TextSpan &span, const std::string &message);
};

#endif