#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "CodeAnalysis/Lexer.h"
#include "CodeAnalysis/SyntaxTree.h"

class Parser
{
public:
    Parser(SourceText text, const std::vector<Token> &tokens, DiagnosticBag diagnostic) : Text(text), tokens(tokens), currentTokenIndex(0), currentToken(tokens[0])
    {
        _diagnostics.AddRange(diagnostic);
    };
    Parser(SourceText text, const std::vector<Token> &tokens) : Text(text), tokens(tokens), currentTokenIndex(0), currentToken(tokens[0]) {}
    Parser(SourceText text);
    CompilationUnitNode *ParseCompilationUnit();
    const DiagnosticBag &GetDiagnostics() const
    {
        return _diagnostics;
    }
    static int GetBinaryPrecedence(SyntaxKind kind);
    static int GetUnaryPrecedence(SyntaxKind kind);

private:
    SourceText Text;
    DiagnosticBag _diagnostics;
    std::vector<Token> tokens;
    size_t currentTokenIndex;
    Token currentToken;
    Token peek(int offset);
    void NextToken();
    Token Expect(SyntaxKind kind);
    StatementSyntax *ParseStatement();
    StatementSyntax *ParseIfStatement();
    ElseClauseSyntax *ParseElseClause();
    StatementSyntax *ParseWhileStatement();
    StatementSyntax *ParseForStatement();
    StatementSyntax *ParseVariableDeclaration();
    ExpressionStatementSyntax *ParseExpressionStatement();
    BlockStatementSyntax *ParseBlockStatement();
    StatementSyntax *ParseBreakStatement();
    StatementSyntax *ParseContinueStatement();
    StatementSyntax *ParseRetrunStatement();

    SyntaxNode *ParseAssignmentExpression();
    SyntaxNode *ParseExpression();
    SyntaxNode *ParseBinaryExpression(int precedence = 0);
    SyntaxNode *ParsePrimaryExpression();
    SyntaxNode *ParseParenthesizedExpression();
    SyntaxNode *ParseBooleanLiteral();
    SyntaxNode *ParseNumberLiteral();
    SyntaxNode *ParseStringLiteral();
    SyntaxNode *ParseNameOrCallExpression();
    SyntaxNode *ParseNameExpression();
    SyntaxNode *ParseCallExpression();

    std::vector<MemberSyntax *> ParseMembers();
    MemberSyntax *ParseMember();
    MemberSyntax *ParseGlobalStatement();
    MemberSyntax *ParseFunctionDeclaration();
    SeparatedSyntaxList<ParameterNode> ParseParameterList();
    ParameterNode *ParseParameter();

    TypeClauseNode *ParseOptionalTypeClause();
    TypeClauseNode *ParseTypeClause();
    SeparatedSyntaxList<SyntaxNode> ParseArguments();
};

#endif
