#ifndef LEXER_H
#define LEXER_H
#include "SyntaxTree.h"
#include "CodeAnalysis/Diagnostic.h"
#include <string>
#include <vector>

class Lexer
{
public:
    Lexer(SourceText &text) : input(text), pos(0), currentChar(input[pos]), lookAhead(input[pos + 1]) {};
    std::vector<Token> tokenize();
    const DiagnosticBag &GetDiagnostics() const
    {
        return _diagnostics;
    }

private:
    DiagnosticBag _diagnostics;
    SourceText input;
    size_t pos;
    char currentChar;
    char lookAhead;
    void advance();
    Token GenerateStringToken();
    Token GenerateWhitespaceToken();
    Token GenerateNumberToken();
    Token GenerateIdentifierToken();
    SyntaxKind checkKeyword(const std::string &keyword);
};

#endif
