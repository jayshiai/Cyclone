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

void Repl();

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        Repl();
    }
    else
    {
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
    }

    return 0;
}

const std::string BLUE = "\033[34m";
const std::string RESET_COLOR = "\033[0m";
const std::string GREEN = "\033[32m";
void Repl()
{

    std::string textBuilder;
    bool showTree = false;
    std::unordered_map<VariableSymbol, std::any> variables;
    bool showProgram = false;
    bool showBoundTree = false;
    Compilation *previous = nullptr;
    while (true)
    {
        if (textBuilder.size() == 0)
        {
            std::cout << GREEN << ">> " << RESET_COLOR;
        }
        else
        {
            std::cout << GREEN << "- " << RESET_COLOR;
        }
        std::string input;
        std::getline(std::cin, input);

        bool isBlank = input.empty();

        if (textBuilder.size() == 0)
        {
            if (isBlank)
            {

                continue;
            }
            else if (input == "#showTree")
            {
                showTree = !showTree;
                std::cout << showTree ? "Showing parse trees." : "Not showing parse trees";
                continue;
            }
            else if (input == "#showProgram")
            {
                showProgram = !showProgram;
                std::cout << showProgram ? "Showing program." : "Not showing program";
                continue;
            }
            else if (input == "#showBoundTree")
            {
                showBoundTree = !showBoundTree;
                std::cout << showBoundTree ? "Showing bound tree." : "Not showing bound tree";
                continue;
            }
            else if (input == "#cls")
            {
                std::cout << "\033[2J\033[H" << std::flush;
                continue;
            }
            else if (input == "#exit")
            {
                break;
            }
            else if (input == "#reset")
            {
                previous = nullptr;
                variables.clear();
                continue;
            }
        }

        textBuilder += input + "\n";
        SyntaxTree Root = SyntaxTree::Parse(textBuilder);

        if (!isBlank && Root.Diagnostics.size() > 0)
        {
            continue;
        }

        Compilation *compilation = previous == nullptr ? new Compilation({&Root}) : previous->ContinueWith(&Root);

        if (showTree)
        {

            std::cout << BLUE << "Abstract Syntax Tree" << std::endl;
            Root.Root->WriteTo(std::cout);
        }

        if (showProgram)
        {
            std::cout << BLUE << "Program" << std::endl;
            compilation->EmitTree(std::cout);
        }
        if (showBoundTree)
        {
            std::cout << BLUE << "Bound Tree" << std::endl;
            compilation->EmitBoundTree(std::cout);
        }

        EvaluationResult result = compilation->Evaluate(variables);

        if (result.Diagnostics.size() > 0)
        {

            IndentedTextWriter writer(std::cout);

            writer.WriteDiagnostics(result.Diagnostics);
            // std::sort(result.Diagnostics.begin(), result.Diagnostics.end(), [](const Diagnostic &a, const Diagnostic &b)
            //           { return a.Span.Start < b.Span.Start; });
            // for (auto &diagnostic : result.Diagnostics)
            // {
            //     PrintDiagnostic(diagnostic, Root.Text);
            // }
            std::cout << "Evaluation failed." << std::endl;
        }
        else
        {
            std::cout << std::endl;
            if (result.Value.type() == typeid(long long))
            {
                std::cout << std::any_cast<long long>(result.Value) << std::endl;
            }
            else if (result.Value.type() == typeid(float))
            {
                std::cout << std::any_cast<double>(result.Value) << std::endl;
            }
            else if (result.Value.type() == typeid(double))
            {
                std::cout << std::any_cast<double>(result.Value) << std::endl;
            }
            else if (result.Value.type() == typeid(int))
            {
                std::cout << (long long)std::any_cast<int>(result.Value) << std::endl;
            }
            else if (result.Value.type() == typeid(bool))
            {
                std::cout << std::any_cast<bool>(result.Value) << std::endl;
            }
            else if (result.Value.type() == typeid(std::string))
            {
                std::cout << std::any_cast<std::string>(result.Value) << std::endl;
            }
            else if (result.Value.type() == typeid(std::vector<std::any>))
            {
                auto values = std::any_cast<std::vector<std::any>>(result.Value);
                for (auto &value : values)
                {
                    if (value.type() == typeid(long long))
                    {
                        std::cout << std::any_cast<long long>(value) << " ";
                    }
                    if (value.type() == typeid(float))
                    {
                        std::cout << std::any_cast<double>(value) << " ";
                    }
                    else if (value.type() == typeid(double))
                    {
                        std::cout << std::any_cast<double>(value) << " ";
                    }
                    else if (value.type() == typeid(int))
                    {
                        std::cout << (long long)std::any_cast<int>(value) << " ";
                    }
                    else if (value.type() == typeid(bool))
                    {
                        std::cout << std::any_cast<bool>(value) << " ";
                    }
                    else if (value.type() == typeid(std::string))
                    {
                        std::cout << std::any_cast<std::string>(value) << " ";
                    }
                }
                std::cout << std::endl;
            }
            else
            {
                try
                {
                    std::cout << std::any_cast<std::string>(result.Value) << std::endl;
                }
                catch (const std::bad_any_cast &e)
                {
                    std::cout << std::endl;
                }
            }
            previous = compilation;
        }

        textBuilder.clear();
    }
}