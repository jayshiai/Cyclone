#include "CodeAnalysis/Diagnostic.h"

void DiagnosticBag::AddRange(const DiagnosticBag &diagnostics)
{
    _diagnostics.insert(_diagnostics.end(), diagnostics._diagnostics.begin(), diagnostics._diagnostics.end());
}

void DiagnosticBag::ReportInvalidNumber(const TextSpan &span, const std::string &text, const std::string &type)
{
    std::ostringstream oss;
    oss << "The number " << text << " isn't a valid " << type << ".";
    Report(span, oss.str());
}

void DiagnosticBag::ReportBadCharacter(int position, char character)
{
    TextSpan span(position, 1);
    std::ostringstream oss;
    oss << "Bad character input: '" << character << "'.";
    Report(span, oss.str());
}
void DiagnosticBag::ReportUnterminatedString(TextSpan span)
{
    Report(span, "Unterminated string literal.");
}
void DiagnosticBag::ReportUnexpectedToken(const TextSpan &span, const std::string &actualKind, const std::string &expectedKind)
{
    std::ostringstream oss;
    oss << "Unexpected token <" << actualKind << ">, expected <" << expectedKind << ">.";
    Report(span, oss.str());
}

void DiagnosticBag::ReportUndefinedUnaryOperator(const TextSpan &span, const std::string &operatorText, const std::string &operandType)
{
    std::ostringstream oss;
    oss << "Unary operator '" << operatorText << "' is not defined for type " << operandType << ".";
    Report(span, oss.str());
}

void DiagnosticBag::ReportUndefinedBinaryOperator(const TextSpan &span, const std::string &operatorText, const std::string &leftType, const std::string &rightType)
{
    std::ostringstream oss;
    oss << "Binary operator '" << operatorText << "' is not defined for types " << leftType << " and " << rightType << ".";
    Report(span, oss.str());
}

void DiagnosticBag::ReportUndefinedName(const TextSpan &span, const std::string &name)
{
    std::ostringstream oss;
    oss << "Variable '" << name << "' doesn't exist.";
    Report(span, oss.str());
}

void DiagnosticBag::Report(const TextSpan &span, const std::string &message)
{
    _diagnostics.emplace_back(span, message);
}

void DiagnosticBag::ReportCannotConvert(TextSpan span, std::string fromType, std::string toType)
{
    std::ostringstream oss;
    oss << "Cannot convert type " << fromType << " to " << toType << ".";
    Report(span, oss.str());
}

void DiagnosticBag::ReportVariableAlreadyDeclared(TextSpan span, std::string name)
{
    std::ostringstream oss;
    oss << "Variable '" << name << "' is already declared.";
    Report(span, oss.str());
}

void DiagnosticBag::ReportCannotAssign(TextSpan span, std::string name)
{
    std::ostringstream oss;
    oss << "Variable '" << name << "' is read-only and cannot be assigned to.";
    Report(span, oss.str());
}

void DiagnosticBag::ReportUndefinedFunction(const TextSpan &span, const std::string &name)
{
    std::ostringstream oss;
    oss << "Function '" << name << "' doesn't exist.";
    Report(span, oss.str());
}

void DiagnosticBag::ReportWrongArgumentCount(const TextSpan &span, const std::string &name, int expectedCount, int actualCount)
{
    std::ostringstream oss;
    oss << "Function '" << name << "' requires " << expectedCount << " arguments but was given " << actualCount << ".";
    Report(span, oss.str());
}

void DiagnosticBag::ReportWrongArgumentType(const TextSpan &span, const std::string &name, const std::string &expectedType, const std::string &actualType)
{
    std::ostringstream oss;
    oss << "Parameter '" << name << "' requires a value of type " << expectedType << " but was given a value of type " << actualType << ".";
    Report(span, oss.str());
}

void DiagnosticBag::ReportExpressionMustHaveValue(const TextSpan &span)
{
    Report(span, "Expression must have a value.");
}