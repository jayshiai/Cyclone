#include "CodeAnalysis/Binder.h"

bool BoundScope::TryDeclare(VariableSymbol &variable)
{
    if (_variables.find(variable.Name) != _variables.end())
    {
        return false;
    }

    _variables[variable.Name] = variable;
    return true;
};

bool BoundScope::TryLookup(const std::string &name, VariableSymbol &variable) const
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

    return Parent->TryLookup(name, variable);
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