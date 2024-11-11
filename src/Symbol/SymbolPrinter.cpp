#include "CodeAnalysis/Symbol.h"

void SymbolPrinter::WriteTo(const Symbol *symbol, IndentedTextWriter &writer)
{
    switch (symbol->GetKind())
    {
    case SymbolKind::Function:
        WriteFunctionTo((FunctionSymbol *)symbol, writer);
        break;
    case SymbolKind::GlobalVariable:
        WriteGlobalVariableTo((GlobalVariableSymbol *)symbol, writer);
        break;
    case SymbolKind::LocalVariable:
        WriteLocalVariableTo((LocalVariableSymbol *)symbol, writer);
        break;
    default:
        throw new std::runtime_error("Unexpected symbol kind ");
        break;
    }
}

void SymbolPrinter::WriteFunctionTo(const FunctionSymbol *symbol, IndentedTextWriter &writer)
{
    writer.WriteKeyword("Function ");
    writer.WriteIdentifier(symbol->Name);
    writer.WritePunctuation("(");

    for (size_t i = 0; i < symbol->Parameters.size(); ++i)
    {
        if (i > 0)
        {
            writer.WritePunctuation(", ");
        }
        writer.WriteIdentifier(symbol->Parameters[i].Name);
        writer.WritePunctuation(": ");
        writer.WriteIdentifier(symbol->Parameters[i].Type.Name);
    }

    writer.WritePunctuation("): ");
    writer.WriteLine();
}

void SymbolPrinter::WriteGlobalVariableTo(const GlobalVariableSymbol *symbol, IndentedTextWriter &writer)
{
    writer.WriteKeyword("Global ");
    writer.WriteIdentifier(symbol->Name);
    writer.WritePunctuation(": ");
    writer.WriteIdentifier(symbol->Type.Name);
}

void SymbolPrinter::WriteLocalVariableTo(const LocalVariableSymbol *symbol, IndentedTextWriter &writer)
{
    writer.WriteKeyword("Local ");
    writer.WriteIdentifier(symbol->Name);
    writer.WritePunctuation(": ");
    writer.WriteIdentifier(symbol->Type.Name);
}