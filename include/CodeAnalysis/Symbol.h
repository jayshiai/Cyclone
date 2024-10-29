#ifndef SYMBOL_H
#define SYMBOL_H
#include <string>
#include <ostream>

enum class SymbolKind
{
    Variable,
    Type,
    Function,
    Parameter
};

class Symbol
{
public:
    std::string Name;
    std::string ToString() const
    {
        return Name;
    }
    virtual SymbolKind GetKind() const = 0;

protected:
    Symbol(std::string name) : Name(name) {}
};

class TypeSymbol : public Symbol
{
private:
    TypeSymbol(std::string name) : Symbol(name) {}

public:
    static const TypeSymbol Integer;
    static const TypeSymbol Boolean;
    static const TypeSymbol String;
    static const TypeSymbol Error;
    virtual SymbolKind GetKind() const override
    {
        return SymbolKind::Type;
    }

    friend std::ostream &operator<<(std::ostream &os, const TypeSymbol &type)
    {
        os << "TypeSymbol(Name: " << type.Name << ")";
        return os;
    }

    bool operator==(const TypeSymbol &other) const
    {
        return Name == other.Name;
    }

    bool operator!=(const TypeSymbol &other) const
    {
        return !(*this == other);
    }

    std::string ToString() const
    {
        return Name;
    }
};

class VariableSymbol : Symbol
{
public:
    std::string Name;
    TypeSymbol Type;
    bool IsReadOnly;
    VariableSymbol() : Symbol(""), IsReadOnly(false), Type(TypeSymbol::Error) {}
    VariableSymbol(std::string name, bool isReadOnly, TypeSymbol type) : Symbol(name), Name(name), IsReadOnly(isReadOnly), Type(type) {}

    SymbolKind GetKind() const override
    {
        return SymbolKind::Variable;
    }
    friend std::ostream &operator<<(std::ostream &os, const VariableSymbol &var)
    {
        os << "VariableSymbol(Name: " << var.Name << ")";
        return os;
    }

    bool operator==(const VariableSymbol &other) const
    {
        return Name == other.Name;
    }

    std::string ToString() const
    {
        return Name;
    }
};

namespace std
{
    template <>
    struct hash<VariableSymbol>
    {
        std::size_t operator()(const VariableSymbol &symbol) const
        {
            return std::hash<std::string>()(symbol.ToString());
        }
    };
}
#endif