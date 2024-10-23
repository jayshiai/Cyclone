#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "Lexer.h"
#include "SyntaxTree.h"
class Parser
{
public:
    Parser(const std::vector<Token> &tokens);
    SyntaxTree parse();

private:
    std::vector<std::string> diagnostics;
    std::vector<Token> tokens;
    size_t currentTokenIndex;
    Token currentToken;
    void NextToken();
    SyntaxNode *ParseExpression();
    SyntaxNode *ParseTerm();
    SyntaxNode *ParseFactor();
};

#endif
