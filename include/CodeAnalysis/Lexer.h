#ifndef LEXER_H
#define LEXER_H
#include "SyntaxTree.h"
#include "CodeAnalysis/Diagnostic.h"
#include <string>
#include <vector>

class Lexer
{
public:
    Lexer(const std::string &input);
    std::vector<Token> tokenize();
    const DiagnosticBag &GetDiagnostics() const
    {
        return _diagnostics;
    }

private:
    DiagnosticBag _diagnostics;
    std::string input;
    size_t pos;
    char currentChar;
    char lookAhead;
    void advance();
    void skipWhitespace();
    std::string number();
    SyntaxKind checkKeyword(const std::string &keyword);
};

#endif
