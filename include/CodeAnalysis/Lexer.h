#ifndef LEXER_H
#define LEXER_H
#include "SyntaxTree.h"
#include "CodeAnalysis/Diagnostic.h"
#include <string>
#include <vector>

class Lexer
{
public:
    Lexer(SyntaxTree *syntaxTree) : _syntaxTree(syntaxTree), input(syntaxTree->Text), pos(0), currentChar(input[0]), lookAhead(input[1]) {};
    std::vector<Token> tokenize();
    const DiagnosticBag &GetDiagnostics() const
    {
        return _diagnostics;
    }

private:
    DiagnosticBag _diagnostics;
    SourceText input;
    SyntaxTree *_syntaxTree;
    size_t pos;
    char currentChar;
    char lookAhead;
    void advance();
    Token GenerateStringToken();
    Token GenerateWhitespaceToken();
    Token GenerateNumberToken();
    Token GenerateIdentifierToken();
    Token GenerateSingleLineComment();
    Token GenerateMultiLineComment();
    SyntaxKind checkKeyword(const std::string &keyword);
};

#endif
