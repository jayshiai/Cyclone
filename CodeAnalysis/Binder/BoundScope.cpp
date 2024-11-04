#include "CodeAnalysis/Binder.h"

bool BoundScope::TryDeclareVariable(VariableSymbol &variable)
{

    if (_variables.find(variable.Name) != _variables.end())
    {
        return false;
    }

    _variables[variable.Name] = variable;
    return true;
};

bool BoundScope::TryLookupVariable(const std::string &name, VariableSymbol &variable) const
{
    auto it = _variables.find(name);
    if (it != _variables.end())
    {
        variable = it->second;
        return true;
    }

    if (Parent == nullptr)
    {
        return false;
    }

    return Parent->TryLookupVariable(name, variable);
};

bool BoundScope::TryDeclareFunction(FunctionSymbol &function)
{
    if (_functions.find(function.Name) != _functions.end())
    {
        return false;
    }

    _functions[function.Name] = function;
    return true;
};

bool BoundScope::TryLookupFunction(const std::string &name, FunctionSymbol &function) const
{
    auto it = _functions.find(name);
    if (it != _functions.end())
    {
        function = it->second;
        return true;
    }

    if (Parent == nullptr)
    {
        return false;
    }

    return Parent->TryLookupFunction(name, function);
};

std::vector<VariableSymbol> BoundScope::GetDeclaredVariables() const
{
    std::vector<VariableSymbol> result;
    for (const auto &pair : _variables)
    {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<FunctionSymbol> BoundScope::GetDeclaredFunctions() const
{
    std::vector<FunctionSymbol> result;
    for (const auto &pair : _functions)
    {
        result.push_back(pair.second);
    }
    return result;
}
