#include "lexer.h"
#include "parser.h"
#include "SyntaxTree.h"
#include <iostream>

// Function to print the AST
// Function to print the AST
void PrintAST(SyntaxNode *node, std::string indent = "", bool isLast = true)
{
    if (!node)
        return;

    std::cout << indent << "|--" << node->getType();

    if (auto *numNode = dynamic_cast<NumericLiteralNode *>(node))
    {
        std::cout << ": " << numNode->value;
    }
    std::cout << std::endl;

    if (auto *binNode = dynamic_cast<BinaryExpressionNode *>(node))
    {
        PrintAST(binNode->left, indent + (isLast ? "    " : "|   "), false);
        std::cout << indent + (isLast ? "    |-- Operation: " : "|   |-- Operation: ") << binNode->op << std::endl;
        PrintAST(binNode->right, indent + (isLast ? "    " : "|   "), true);
    }

    if (auto *parenNode = dynamic_cast<ParenthesizedExpressionNode *>(node))
    {
        PrintAST(parenNode->expression, indent + (isLast ? "    " : "|   "), true);
    }

    if (auto *unaryNode = dynamic_cast<UnaryExpressionNode *>(node))
    {
        std::cout << indent + (isLast ? "    |-- Operation: " : "|   |-- Operation: ") << unaryNode->op << std::endl;
        PrintAST(unaryNode->expression, indent + (isLast ? "    " : "|   "), true);
    }
}

main()
{
    std::string input;

    // Read input from user
    std::cout << "Enter an expression: ";
    std::getline(std::cin, input);

    // Create Lexer instance and tokenize the input
    Lexer lexer(input);
    std::vector<Token> tokens = lexer.tokenize();
    std::cout << "Tokens:" << std::endl;

    for (auto &token : tokens)
    {
        std::cout << "      Type: " << static_cast<int>(token.type) << ", Value: " << token.value << std::endl;
    }
    // Create Parser instance and parse the tokens
    Parser parser(tokens);
    SyntaxTree Root = parser.parse();

    // std::cout << "asd:" << Root.root->getType() << std::endl;
    // Print the AST
    std::cout << "Abstract Syntax Tree" << std::endl;
    PrintAST(Root.root);

    if (Root.diagnostics.size() != 0)
    {
        std::cout << "Diagnostics:" << std::endl;
        for (auto &diagnostic : Root.diagnostics)
        {
            std::cout << diagnostic << std::endl;
        }
    }
    return 0;
}
