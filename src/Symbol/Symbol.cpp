#include "CodeAnalysis/Symbol.h"
#include "CodeAnalysis/IndentedTextWriter.h"
const TypeSymbol TypeSymbol::Integer("int");
const TypeSymbol TypeSymbol::Float("float");
const TypeSymbol TypeSymbol::Boolean("bool");
const TypeSymbol TypeSymbol::String("string");
const TypeSymbol TypeSymbol::Void("void");
const TypeSymbol TypeSymbol::Error("?");
const TypeSymbol TypeSymbol::Null("null");
const TypeSymbol TypeSymbol::Any("any");
const TypeSymbol TypeSymbol::ArrayInt("array<int>");
const TypeSymbol TypeSymbol::ArrayFloat("array<float>");
const TypeSymbol TypeSymbol::ArrayBool("array<bool>");
const TypeSymbol TypeSymbol::ArrayString("array<string>");
const TypeSymbol TypeSymbol::ArrayAny("array<any>");

const FunctionSymbol BuiltInFunctions::Print("print", std::vector<ParameterSymbol>{ParameterSymbol("text", TypeSymbol::String)}, TypeSymbol::Void);
const FunctionSymbol BuiltInFunctions::Input("input", std::vector<ParameterSymbol>{}, TypeSymbol::String);
const FunctionSymbol BuiltInFunctions::Random("random", std::vector<ParameterSymbol>{ParameterSymbol("max", TypeSymbol::Integer)}, TypeSymbol::Integer);
const FunctionSymbol BuiltInFunctions::ArrayLength("size", std::vector<ParameterSymbol>{ParameterSymbol("array", TypeSymbol::ArrayAny)}, TypeSymbol::Integer);
const FunctionSymbol BuiltInFunctions::StringLength("len", std::vector<ParameterSymbol>{ParameterSymbol("text", TypeSymbol::String)}, TypeSymbol::Integer);

void Symbol::WriteTo(std::ostream &os)
{
    IndentedTextWriter writer(os);
    WriteTo(writer);
}

void Symbol::WriteTo(IndentedTextWriter &writer)
{
    SymbolPrinter::WriteTo(const_cast<const Symbol *>(this), writer);
}