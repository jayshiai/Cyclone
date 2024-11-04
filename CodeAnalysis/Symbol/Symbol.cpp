#include "CodeAnalysis/Symbol.h"

const TypeSymbol TypeSymbol::Integer("int");
const TypeSymbol TypeSymbol::Boolean("bool");
const TypeSymbol TypeSymbol::String("string");
const TypeSymbol TypeSymbol::Void("void");
const TypeSymbol TypeSymbol::Error("?");
const TypeSymbol TypeSymbol::Null("null");

const FunctionSymbol BuiltInFunctions::Print("print", std::vector<ParameterSymbol>{ParameterSymbol("text", TypeSymbol::String)}, TypeSymbol::Void);
const FunctionSymbol BuiltInFunctions::Input("input", std::vector<ParameterSymbol>{}, TypeSymbol::String);
const FunctionSymbol BuiltInFunctions::Random("random", std::vector<ParameterSymbol>{ParameterSymbol("max", TypeSymbol::Integer)}, TypeSymbol::Integer);