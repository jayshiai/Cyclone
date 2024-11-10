#include "CodeAnalysis/Diagnostic.h"

void DiagnosticBag::AddRange(const DiagnosticBag &diagnostics)
{
    _diagnostics.insert(_diagnostics.end(), diagnostics._diagnostics.begin(), diagnostics._diagnostics.end());
}

void DiagnosticBag::ReportInvalidNumber(const TextLocation &location, const std::string &text, const std::string &type)
{
    std::ostringstream oss;
    oss << "The number " << text << " isn't a valid " << type << ".";
    Report(location, oss.str());
}

void DiagnosticBag::ReportBadCharacter(TextLocation location, char character)
{
    std::ostringstream oss;
    oss << "Bad character input: '" << character << "'.";
    Report(location, oss.str());
}
void DiagnosticBag::ReportUnterminatedString(TextLocation location)
{
    Report(location, "Unterminated string literal.");
}
void DiagnosticBag::ReportUnexpectedToken(const TextLocation &location, const std::string &actualKind, const std::string &expectedKind)
{
    std::ostringstream oss;
    oss << "Unexpected token <" << actualKind << ">, expected <" << expectedKind << ">.";
    Report(location, oss.str());
}

void DiagnosticBag::ReportUndefinedUnaryOperator(const TextLocation &location, const std::string &operatorText, const std::string &operandType)
{
    std::ostringstream oss;
    oss << "Unary operator '" << operatorText << "' is not defined for type " << operandType << ".";
    Report(location, oss.str());
}

void DiagnosticBag::ReportUndefinedBinaryOperator(const TextLocation &location, const std::string &operatorText, const std::string &leftType, const std::string &rightType)
{
    std::ostringstream oss;
    oss << "Binary operator '" << operatorText << "' is not defined for types " << leftType << " and " << rightType << ".";
    Report(location, oss.str());
}

void DiagnosticBag::ReportParameterAlreadyDeclared(TextLocation location, std::string name)
{
    std::ostringstream oss;
    oss << "Parameter '" << name << "' is already declared.";
    Report(location, oss.str());
}

void DiagnosticBag::ReportUndefinedName(const TextLocation &location, const std::string &name)
{
    std::ostringstream oss;
    oss << "Variable '" << name << "' doesn't exist.";
    Report(location, oss.str());
}

void DiagnosticBag::ReportUndefinedType(const TextLocation &location, const std::string &name)
{
    std::ostringstream oss;
    oss << "Type '" << name << "' doesn't exist.";
    Report(location, oss.str());
}

void DiagnosticBag::Report(const TextLocation &location, const std::string &message)
{
    _diagnostics.emplace_back(location, message);
}

void DiagnosticBag::ReportCannotConvert(TextLocation location, std::string fromType, std::string toType)
{
    std::ostringstream oss;
    oss << "Cannot convert type " << fromType << " to " << toType << ".";
    Report(location, oss.str());
}

void DiagnosticBag::ReportCannotConvertImplicitly(const TextLocation &location, const std::string &fromType, const std::string &toType)
{
    std::ostringstream oss;
    oss << "Cannot convert type " << fromType << " to " << toType << " implicitly.";
    Report(location, oss.str());
}

void DiagnosticBag::ReportSymbolAlreadyDeclared(const TextLocation &location, const std::string &name)
{
    std::ostringstream oss;
    oss << name << "' is already declared.";
    Report(location, oss.str());
}

void DiagnosticBag::ReportVariableAlreadyDeclared(TextLocation location, std::string name)
{
    std::ostringstream oss;
    oss << "Variable '" << name << "' is already declared.";
    Report(location, oss.str());
}

void DiagnosticBag::ReportCannotAssign(TextLocation location, std::string name)
{
    std::ostringstream oss;
    oss << "Variable '" << name << "' is read-only and cannot be assigned to.";
    Report(location, oss.str());
}

void DiagnosticBag::ReportUndefinedFunction(const TextLocation &location, const std::string &name)
{
    std::ostringstream oss;
    oss << "Function '" << name << "' doesn't exist.";
    Report(location, oss.str());
}

void DiagnosticBag::ReportWrongArgumentCount(const TextLocation &location, const std::string &name, int expectedCount, int actualCount)
{
    std::ostringstream oss;
    oss << "Function '" << name << "' requires " << expectedCount << " arguments but was given " << actualCount << ".";
    Report(location, oss.str());
}

void DiagnosticBag::ReportWrongArgumentType(const TextLocation &location, const std::string &name, const std::string &expectedType, const std::string &actualType)
{
    std::ostringstream oss;
    oss << "Parameter '" << name << "' requires a value of type " << expectedType << " but was given a value of type " << actualType << ".";
    Report(location, oss.str());
}

void DiagnosticBag::ReportExpressionMustHaveValue(const TextLocation &location)
{
    Report(location, "Expression must have a value.");
}

void DiagnosticBag::ReportInvalidBreakOrContinue(TextLocation location, std::string text)
{
    std::ostringstream oss;
    oss << "The keyword '" << text << "' can only be used inside of loops.";
    Report(location, oss.str());
}

void DiagnosticBag::ReportInvalidReturn(TextLocation location)
{
    Report(location, "The 'return' keyword can only be used inside of functions.");
}

void DiagnosticBag::ReportInvalidReturnExpression(TextLocation location, std::string name)
{
    std::ostringstream oss;
    oss << "Since the function'" << name << "' does not return a value the 'return' keyword cannot be followed by an expression.";
    Report(location, oss.str());
}

void DiagnosticBag::ReportMissingReturnExpression(TextLocation location, std::string name)
{
    std::ostringstream oss;
    oss << "An expression of type '" << name << "' expected.";
    Report(location, oss.str());
}

void DiagnosticBag::ReportAllPathsMustReturn(TextLocation location)
{
    Report(location, "Not all code paths return a value.");
}

void DiagnosticBag::ReportTypeMismatch(const TextLocation &location, const std::string &expectedType, const std::string &actualType)
{
    std::ostringstream oss;
    oss << "Expected type '" << expectedType << "' but received type '" << actualType << "'.";
    Report(location, oss.str());
}

void DiagnosticBag::ReportInvalidArrayIndex(const TextLocation &location)
{
    Report(location, "Invalid array index.");
}

void DiagnosticBag::ReportInvalidArrayAccess(const TextLocation &location)
{
    Report(location, "Invalid array access.");
}