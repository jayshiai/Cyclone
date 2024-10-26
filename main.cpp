#include "CodeAnalysis/lexer.h"
#include "CodeAnalysis/parser.h"
#include "CodeAnalysis/SyntaxTree.h"
#include "CodeAnalysis/Binder.h"
#include "CodeAnalysis/Evaluator.h"
#include "CodeAnalysis/Compilation.h"
#include "Utils.h"
#include <iostream>
#include <any>

const std::string BLUE = "\033[34m";
const std::string RESET_COLOR = "\033[0m";
void PrintAST(SyntaxNode *node, std::string indent = "", bool isLast = true)
{
    if (!node)
        return;

    std::cout << indent << "|--" << convertSyntaxKindToString(node->Kind);

    if (Token *token = dynamic_cast<Token *>(node))
    {
        std::cout << " " << token->value;
    }
    std::cout << std::endl;
    indent += isLast ? "   " : "|   ";

    auto children = node->GetChildren();
    for (size_t i = 0; i < children.size(); i++)
    {
        PrintAST(children[i], indent, i == children.size() - 1);
    }
}

main()
{

    std::string textBuilder;
    std::cout << "Enter an expression: ";
    bool showTree = false;
    while (true)
    {
        if (textBuilder.size() == 0)
        {
            std::cout << ">>";
        }
        else
        {
            std::cout << "|";
        }
        std::string input;
        std::getline(std::cin, input);

        bool isBlank = input.empty();
        if (textBuilder.size() == 0)
        {
            if (isBlank)
            {
                break;
            }
            else if (input == "#showTree")
            {
                showTree = !showTree;
                std::cout << showTree ? "Showing parse trees." : "Not showing parse trees";
                continue;
            }
            else if (input == "#cls")
            {
                std::cout << "\033[2J\033[H" << std::flush;
                continue;
            }
        }

        textBuilder += input + "\n";
        SyntaxTree Root = SyntaxTree::Parse(textBuilder);

        if (!isBlank && Root.Diagnostics.GetDiagnostics().size() > 0)
        {
            // std::vector<Diagnostic> diagnostics = Root.Diagnostics.GetDiagnostics();
            // for (auto &diagnostic : diagnostics)
            // {
            //     PrintDiagnostic(diagnostic, Root.Text);
            // }
            // return 1;
            continue;
        }

        Compilation compilation(&Root);
        std::unordered_map<VariableSymbol, std::any> variables;
        EvaluationResult result = compilation.Evaluate(variables);

        if (showTree)
        {

            std::cout << BLUE << "Abstract Syntax Tree" << std::endl;
            PrintAST(Root.root);
            std::cout << RESET_COLOR << std::endl;
        }

        if (result.Diagnostics.size() > 0)
        {
            for (auto &diagnostic : result.Diagnostics)
            {
                PrintDiagnostic(diagnostic, Root.Text);
            }
            std::cout << "Evaluation failed." << std::endl;
        }
        else
        {
            if (result.Value.type() == typeid(int))
            {
                std::cout << "Result: " << std::any_cast<int>(result.Value) << std::endl;
            }
            else if (result.Value.type() == typeid(bool))
            {
                std::cout << "Result: " << std::any_cast<bool>(result.Value) << std::endl;
            }
            else
            {
                std::cout << "Unexpected type in result." << std::endl;
            }
        }

        textBuilder.clear();
    }

    return 0;
}
