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
#include <filesystem>
#include <string>
#include <set>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;
std::vector<std::string> GetFilePaths(const std::vector<std::string> &paths)
{
    std::vector<std::string> result;

    for (const auto &path : paths)
    {
        if (fs::is_directory(path))
        {
            for (const auto &entry : fs::recursive_directory_iterator(path))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".cy")
                {
                    result.push_back(entry.path().string());
                }
            }
        }
        else
        {
            result.push_back(path);
        }
    }

    return result;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "usage: cyc <source-paths>" << std::endl;
        return 1;
    }
    std::vector<std::string> args(argv + 1, argv + argc);

    auto paths = GetFilePaths(args);
    std::vector<SyntaxTree *> syntaxTrees;
    bool hasErrors = false;

    for (const auto &path : paths)
    {

        if (!fs::exists(path))
        {
            std::cerr << "error: file '" << path << "' doesn't exist" << std::endl;
            hasErrors = true;
            continue;
        }
        SyntaxTree *syntaxTree = new SyntaxTree(SyntaxTree::Load(path));
        syntaxTrees.push_back(syntaxTree);
    }

    if (hasErrors)
        return 1;

    Compilation compilation(syntaxTrees);
    std::unordered_map<VariableSymbol, std::any> variables;
    auto result = compilation.Evaluate(variables);
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
        writer.WriteDiagnostics(result.Diagnostics);
    }

    return 0;
}