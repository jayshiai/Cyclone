#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "CodeAnalysis/Lexer.h"
#include "CodeAnalysis/SyntaxTree.h"
class Parser
{
public:
    Parser(const std::vector<Token> &tokens);
    SyntaxTree parse();
    const DiagnosticBag &GetDiagnostics() const
    {
        return _diagnostics;
    }

private:
    DiagnosticBag _diagnostics;
    std::vector<Token> tokens;
    size_t currentTokenIndex;
    Token currentToken;
    Token peek( int offset);
    void NextToken();
    SyntaxNode *Expect(SyntaxKind kind);
    SyntaxNode *ParseAssignmentExpression();
    SyntaxNode *ParseExpression();
    SyntaxNode *ParseBinaryExpression(int precedence = 0);
    SyntaxNode *ParsePrimaryExpression();
    int GetBinaryPrecedence(SyntaxKind kind);
    int GetUnaryPrecedence(SyntaxKind kind);
};

#endif
