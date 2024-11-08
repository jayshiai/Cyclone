#include "CodeAnalysis/lexer.h"
#include "CodeAnalysis/parser.h"
#include "CodeAnalysis/SyntaxTree.h"
#include "CodeAnalysis/Binder.h"
#include "CodeAnalysis/Evaluator.h"
#include "CodeAnalysis/Compilation.h"
#include "CodeAnalysis/IndentedTextWriter.h"
#include "Utils.h"
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        std::cerr << "usage: mc <source-paths>" << std::endl;
        return 1;
    }

    if (argc > 2)
    {
        std::cerr << "error: only one path supported right now" << std::endl;
        return 1;
    }

    std::string path = argv[1];

    std::ifstream file(path);
    if (!file)
    {
        std::cerr << "error: could not open file " << path << std::endl;
        return 1;
    }

    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    // std::cout << text << std::endl;
    SyntaxTree syntaxTree = SyntaxTree::Parse(text);

    Compilation *compilation = new Compilation(&syntaxTree);
    std::unordered_map<VariableSymbol, std::any> variables;
    EvaluationResult result = compilation->Evaluate(variables);

    if (result.Diagnostics.empty())
    {
        if (result.Value.has_value())
        {
            // std::cout << std::any_cast<std::string>(result.Value) << std::endl;
        }
    }
    else
    {
        IndentedTextWriter writer(std::cout);
        writer.WriteDiagnostics(result.Diagnostics, syntaxTree);
    }

    return 0;
}