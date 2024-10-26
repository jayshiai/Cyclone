#include "CodeAnalysis/lexer.h"
#include "CodeAnalysis/parser.h"
#include "CodeAnalysis/SyntaxTree.h"
#include "CodeAnalysis/Binder.h"
#include "CodeAnalysis/Evaluator.h"
#include "CodeAnalysis/Compilation.h"
#include "Utils.h"
#include <iostream>
#include <any>

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
    std::string input;

    std::cout << "Enter an expression: ";
    std::getline(std::cin, input);

    Lexer lexer(input);
    std::vector<Token> tokens = lexer.tokenize();
    std::cout << "Tokens:" << std::endl;

    for (auto &token : tokens)
    {
        std::cout << "      Type: " << convertSyntaxKindToString(token.Kind) << ", Value: " << token.value << std::endl;
    }

    Parser parser(tokens);
    SyntaxTree Root = parser.parse();
    Compilation compilation(&Root);
    std::unordered_map<std::string, std::any> variables;
    EvaluationResult result = compilation.Evaluate(variables);

    std::cout << "Abstract Syntax Tree" << std::endl;
    PrintAST(Root.root);

    if (result.Diagnostics.size() > 0)
    {
        for (auto &diagnostic : result.Diagnostics)
        {
            std::cout << diagnostic.ToString() << std::endl;
        }
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

    return 0;
}
