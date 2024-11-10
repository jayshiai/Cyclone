#ifndef SYMBOL_H
#define SYMBOL_H
#include <string>
#include <ostream>
#include <vector>
#include "CodeAnalysis/SyntaxTree.h"
#include "CodeAnalysis/IndentedTextWriter.h"
enum class SymbolKind
{
    Variable,
    LocalVariable,
    GlobalVariable,
    Type,
    Function,
    Parameter
};

class Symbol
{
public:
    std::string Name;

    void WriteTo(std::ostream &os);
    void WriteTo(IndentedTextWriter &writer);
    std::string ToString()
    {
        std::ostringstream writer;
        WriteTo(writer);
        return writer.str();
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
    static const TypeSymbol Void;
    static const TypeSymbol Null;
    static const TypeSymbol Any;
    static const TypeSymbol ArrayInt;
    static const TypeSymbol ArrayBool;
    static const TypeSymbol ArrayString;
    static const TypeSymbol ArrayAny;
    virtual SymbolKind GetKind() const override
    {
        return SymbolKind::Type;
    }

    friend std::ostream &operator<<(std::ostream &os, const TypeSymbol &type)
    {
        os << "TypeSymbol(Name: " << type.Name << ")";
        return os;
    }
    bool IsArray()
    {
        return Name.find("array") != std::string::npos;
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
    SymbolKind Kind;
    VariableSymbol() : Symbol(""), IsReadOnly(false), Type(TypeSymbol::Error) {}
    VariableSymbol(std::string name, bool isReadOnly, TypeSymbol type) : Symbol(name), Name(name), IsReadOnly(isReadOnly), Type(type) {}
    VariableSymbol(SymbolKind kind, std::string name, bool isReadOnly, TypeSymbol type) : Symbol(name), Name(name), IsReadOnly(isReadOnly), Type(type), Kind(kind) {}
    SymbolKind GetKind() const override
    {
        return Kind;
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

class LocalVariableSymbol : public VariableSymbol
{
public:
    SymbolKind GetKind() const override
    {
        return SymbolKind::LocalVariable;
    }

    LocalVariableSymbol(std::string name, bool isReadOnly, TypeSymbol type) : VariableSymbol(SymbolKind::LocalVariable, name, isReadOnly, type) {}
};

class GlobalVariableSymbol : public VariableSymbol
{
public:
    SymbolKind GetKind() const override
    {
        return SymbolKind::GlobalVariable;
    }

    GlobalVariableSymbol(std::string name, bool isReadOnly, TypeSymbol type) : VariableSymbol(SymbolKind::GlobalVariable, name, isReadOnly, type) {}
};
class ParameterSymbol : public LocalVariableSymbol
{

public:
    SymbolKind GetKind() const override
    {
        return SymbolKind::Parameter;
    }

    ParameterSymbol(std::string name, TypeSymbol type) : LocalVariableSymbol(name, true, type) {}
};

class FunctionSymbol : public Symbol
{
public:
    SymbolKind GetKind() const override
    {
        return SymbolKind::Function;
    }

    FunctionSymbol(std::string name, std::vector<ParameterSymbol> parameters, TypeSymbol type, FunctionDeclarationSyntax *declaration = nullptr) : Symbol(name), Parameters(parameters), Type(type), Declaration(declaration) {}
    FunctionSymbol() : Symbol(""), Parameters({}), Type(TypeSymbol::Error), Declaration(nullptr) {};
    std::vector<ParameterSymbol> Parameters;
    TypeSymbol Type;
    FunctionDeclarationSyntax *Declaration = nullptr;
    friend std::ostream &operator<<(std::ostream &os, const FunctionSymbol &func)
    {
        os << "FunctionSymbol(Name: " << func.Name << ")";
        return os;
    }

    std::string ToString() const
    {
        return Name;
    }

    bool operator==(const FunctionSymbol &other) const
    {
        return Name == other.Name;
    }
};

namespace std
{
    template <>
    struct hash<FunctionSymbol>
    {
        std::size_t operator()(const FunctionSymbol &symbol) const
        {
            return std::hash<std::string>()(symbol.ToString());
        }
    };
}

class BuiltInFunctions
{
public:
    static const FunctionSymbol Print;
    static const FunctionSymbol Input;
    static const FunctionSymbol Random;

    static std::vector<FunctionSymbol> GetAll()
    {
        return {Print, Input, Random};
    }
};

class SymbolPrinter
{
public:
    static void WriteTo(const Symbol *symbol, IndentedTextWriter &writer);

private:
    static void WriteFunctionTo(const FunctionSymbol *symbol, IndentedTextWriter &writer);
    static void WriteGlobalVariableTo(const GlobalVariableSymbol *symbol, IndentedTextWriter &writer);
    static void WriteLocalVariableTo(const LocalVariableSymbol *symbol, IndentedTextWriter &writer);
    // static void WriteParameterTo(const ParameterSymbol *symbol, IndentedTextWriter &writer);
    // static void WriteTypeTo(const TypeSymbol *symbol, IndentedTextWriter &writer);
};
#endif