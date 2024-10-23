#include "lexer.h"
#include "parser.h"
#include "SyntaxTree.h"
#include <iostream>

// Function to print the AST
// Function to print the AST
void PrintAST(SyntaxNode *node, std::string indent = "", bool isLast = true)
{
    if (!node)
        return; // Safety check for null pointer

    // Use ASCII characters instead of Unicode
    std::cout << indent << "|--" << node->getType(); // Ensure getType() is implemented properly

    // Handle specific node types for additional output
    if (auto *numNode = dynamic_cast<NumericLiteralNode *>(node))
    {
        std::cout << ": " << numNode->value;
    }
    std::cout << std::endl;

    // If there are children (only applicable for BinaryExpressionNode)
    if (auto *binNode = dynamic_cast<BinaryExpressionNode *>(node))
    {
        PrintAST(binNode->left, indent + (isLast ? "    " : "|   "), false);
        std::cout << indent + (isLast ? "    |-- Operation: " : "|   |-- Operation: ") << binNode->op << std::endl;
        PrintAST(binNode->right, indent + (isLast ? "    " : "|   "), true);
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

    std::cout << "Diagnostics:" << std::endl;
    for (auto &diagnostic : Root.diagnostics)
    {
        std::cout << diagnostic << std::endl;
    }

    return 0;
}
