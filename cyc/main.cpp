#include "CodeAnalysis/Lexer.h"
#include "CodeAnalysis/Parser.h"
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
#include <array>

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

bool isGppInstalled()
{
    std::array<char, 128> buffer;
    std::string result = "";
    const char *command;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    command = "where g++"; // Use 'where' on Windows
#else
    command = "which g++"; // Use 'which' on Linux/macOS
#endif

    FILE *pipe = popen(command, "r");
    if (!pipe)
    {
        return false;
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        result += buffer.data();
    }

    // If result is empty, g++ is not installed
    return !result.empty();
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "usage: cyc <source-paths> -o <output-filename>" << std::endl;
        return 1;
    }
    std::vector<std::string> args(argv + 1, argv + argc);

    std::string outputFileName;

    std::vector<std::string> sourcePaths;

    for (size_t i = 0; i < args.size(); ++i)
    {
        if (args[i] == "-o" && i + 1 < args.size())
        {
            outputFileName = args[i + 1];
            i++; // Skip the next argument, since it is the output file name
        }
        else
        {
            sourcePaths.push_back(args[i]);
        }
    }

    if (outputFileName.empty())
    {
        std::cerr << "Error: Output filename must be specified with the '-o' flag." << std::endl;
        return 1;
    }

    if (!isGppInstalled())
    {
        std::cerr << "Error: g++ is not installed on the system." << std::endl;
        std::cerr << "Please install g++ before running Cyclone Compiler." << std::endl;
        std::cerr << "If you don't want to install g++, you can use the online compiler at https://cy.3dubs.in or use the Interpreter." << std::endl;
        return 1;
    }
    auto paths = GetFilePaths(sourcePaths);
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
    auto result = compilation.Compile(variables, outputFileName);
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