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