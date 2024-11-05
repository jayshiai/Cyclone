#include "CodeAnalysis/Parser.h"
#include "CodeAnalysis/SyntaxTree.h"
#include "CodeAnalysis/Lexer.h"
#include <iostream>
#include "Utils.h"

Parser::Parser(SourceText text) : Text(text), currentTokenIndex(0)
{
    Lexer lexer(text);
    std::vector<Token> lexedTokens = lexer.tokenize();
    tokens = lexedTokens;
    currentToken = tokens[0];
    _diagnostics.AddRange(lexer.GetDiagnostics());
}
Token Parser::peek(int offset)
{
    if (currentTokenIndex + offset >= tokens.size())
    {
        return tokens[tokens.size() - 1];
    }
    return tokens[currentTokenIndex + offset];
}

CompilationUnitNode *Parser::ParseCompilationUnit()
{
    std::vector<MemberSyntax *> members = ParseMembers();
    Token endOfFileToken = Expect(SyntaxKind::END_OF_FILE);
    return new CompilationUnitNode(members, endOfFileToken);
}
std::vector<MemberSyntax *> Parser::ParseMembers()
{
    std::vector<MemberSyntax *> members;
    while (currentToken.Kind != SyntaxKind::END_OF_FILE)
    {
        Token startToken = currentToken;
        MemberSyntax *member = ParseMember();
        members.push_back(member);

        if (currentToken == startToken)
        {
            NextToken();
        }
    }
    return members;
}

MemberSyntax *Parser::ParseMember()
{
    if (currentToken.Kind == SyntaxKind::FUNCTION_KEYWORD)
    {
        return ParseFunctionDeclaration();
    }
    return ParseGlobalStatement();
}

MemberSyntax *Parser::ParseFunctionDeclaration()
{
    Token functionKeyword = Expect(SyntaxKind::FUNCTION_KEYWORD);
    Token identifier = Expect(SyntaxKind::IDENTIFIER);
    Token openParenthesis = Expect(SyntaxKind::LPAREN);
    SeparatedSyntaxList<ParameterNode> parameters = ParseParameterList();
    Token closeParenthesis = Expect(SyntaxKind::RPAREN);
    TypeClauseNode *type = ParseOptionalTypeClause();
    BlockStatementSyntax *body = ParseBlockStatement();
    return new FunctionDeclarationSyntax(functionKeyword, identifier, openParenthesis, parameters, closeParenthesis, type, body);
}

SeparatedSyntaxList<ParameterNode> Parser::ParseParameterList()
{
    std::vector<SyntaxNode *> nodesAndSeparators;
    while (currentToken.Kind != SyntaxKind::RPAREN && currentToken.Kind != SyntaxKind::END_OF_FILE)
    {
        ParameterNode *parameter = ParseParameter();
        nodesAndSeparators.push_back(parameter);

        if (currentToken.Kind != SyntaxKind::RPAREN)
        {
            Token *comma = new Token(Expect(SyntaxKind::COMMA));
            nodesAndSeparators.push_back(comma);
        }
    }
    return SeparatedSyntaxList<ParameterNode>(nodesAndSeparators);
}

ParameterNode *Parser::ParseParameter()
{
    Token identifier = Expect(SyntaxKind::IDENTIFIER);
    TypeClauseNode *type = ParseTypeClause();
    return new ParameterNode(identifier, type);
}

MemberSyntax *Parser::ParseGlobalStatement()
{
    StatementSyntax *statement = ParseStatement();
    return new GlobalStatementSyntax(statement);
}
StatementSyntax *Parser::ParseStatement()
{
    switch (currentToken.Kind)
    {
    case SyntaxKind::OPEN_BRACE:
        return ParseBlockStatement();
    case SyntaxKind::LET_KEYWORD:
    case SyntaxKind::VAR_KEYWORD:
        return ParseVariableDeclaration();
    case SyntaxKind::IF_KEYWORD:
        return ParseIfStatement();
    case SyntaxKind::WHILE_KEYWORD:
        return ParseWhileStatement();
    case SyntaxKind::FOR_KEYWORD:
        return ParseForStatement();
    case SyntaxKind::BREAK_KEYWORD:
        return ParseBreakStatement();
    case SyntaxKind::CONTINUE_KEYWORD:
        return ParseContinueStatement();
    default:
        return ParseExpressionStatement();
    }
}

BlockStatementSyntax *Parser::ParseBlockStatement()
{
    std::vector<StatementSyntax *> statements;
    Token openBraceToken = Expect(SyntaxKind::OPEN_BRACE);

    while (currentToken.Kind != SyntaxKind::CLOSE_BRACE && currentToken.Kind != SyntaxKind::END_OF_FILE)
    {
        Token startToken = currentToken;
        StatementSyntax *statement = ParseStatement();
        statements.push_back(statement);

        if (currentToken == startToken)
        {
            NextToken();
        }
    }

    Token closeBraceToken = Expect(SyntaxKind::CLOSE_BRACE);
    return new BlockStatementSyntax(openBraceToken, statements, closeBraceToken);
}

StatementSyntax *Parser::ParseVariableDeclaration()
{
    SyntaxKind expected = currentToken.Kind == SyntaxKind::LET_KEYWORD ? SyntaxKind::LET_KEYWORD : SyntaxKind::VAR_KEYWORD;
    Token keyword = Expect(expected);
    Token identifier = Expect(SyntaxKind::IDENTIFIER);
    TypeClauseNode *typeClause = ParseOptionalTypeClause();
    Token equals = Expect(SyntaxKind::EQUALS);
    SyntaxNode *initializer = ParseExpression();
    return new VariableDeclarationSyntax(keyword, identifier, typeClause, equals, initializer);
}

TypeClauseNode *Parser::ParseOptionalTypeClause()
{
    if (currentToken.Kind != SyntaxKind::COLON)
    {
        return nullptr;
    }
    return ParseTypeClause();
}

TypeClauseNode *Parser::ParseTypeClause()
{
    Token colon = Expect(SyntaxKind::COLON);
    Token identifier = Expect(SyntaxKind::IDENTIFIER);
    return new TypeClauseNode(colon, identifier);
}

StatementSyntax *Parser::ParseIfStatement()
{
    Token ifKeyword = Expect(SyntaxKind::IF_KEYWORD);
    SyntaxNode *condition = ParseExpression();
    StatementSyntax *thenStatement = ParseStatement();
    ElseClauseSyntax *elseClause = ParseElseClause();
    return new IfStatementSyntax(ifKeyword, condition, thenStatement, elseClause);
}

ElseClauseSyntax *Parser::ParseElseClause()
{
    if (currentToken.Kind != SyntaxKind::ELSE_KEYWORD)
    {
        return nullptr;
    }
    Token elseKeyword = Expect(SyntaxKind::ELSE_KEYWORD);
    StatementSyntax *elseStatement = ParseStatement();
    return new ElseClauseSyntax(elseKeyword, elseStatement);
}

StatementSyntax *Parser::ParseWhileStatement()
{
    Token whileKeyword = Expect(SyntaxKind::WHILE_KEYWORD);
    SyntaxNode *condition = ParseExpression();
    StatementSyntax *body = ParseStatement();
    return new WhileStatementSyntax(whileKeyword, condition, body);
}

StatementSyntax *Parser::ParseForStatement()
{
    Token keyword = Expect(SyntaxKind::FOR_KEYWORD);
    Token identifier = Expect(SyntaxKind::IDENTIFIER);
    Token equals = Expect(SyntaxKind::EQUALS);
    SyntaxNode *lowerBound = ParseExpression();
    Token toKeyword = Expect(SyntaxKind::TO_KEYWORD);
    SyntaxNode *upperBound = ParseExpression();
    StatementSyntax *body = ParseStatement();
    return new ForStatementSyntax(keyword, identifier, equals, lowerBound, toKeyword, upperBound, body);
}

StatementSyntax *Parser::ParseBreakStatement()
{
    Token keyword = Expect(SyntaxKind::BREAK_KEYWORD);
    return new BreakStatementSyntax(keyword);
}

StatementSyntax *Parser::ParseContinueStatement()
{
    Token keyword = Expect(SyntaxKind::CONTINUE_KEYWORD);
    return new ContinueStatementSyntax(keyword);
}

ExpressionStatementSyntax *Parser::ParseExpressionStatement()
{
    SyntaxNode *expression = ParseExpression();
    return new ExpressionStatementSyntax(expression);
}

void Parser::NextToken()
{
    currentTokenIndex++;
    if (currentTokenIndex < tokens.size())
    {
        currentToken = tokens[currentTokenIndex];
    }
}
SyntaxNode *Parser::ParseExpression()
{
    return ParseAssignmentExpression();
}
SyntaxNode *Parser::ParseAssignmentExpression()
{
    if (peek(0).Kind == SyntaxKind::IDENTIFIER && peek(1).Kind == SyntaxKind::EQUALS)
    {
        Token identifier = currentToken;
        NextToken();
        Token equals = currentToken;
        NextToken();
        SyntaxNode *right = ParseAssignmentExpression();
        return new AssignmentExpressionNode(identifier, equals, right);
    }
    return ParseBinaryExpression();
}
SyntaxNode *Parser::ParseBinaryExpression(int parentPrecedence)
{
    SyntaxNode *left;

    int unaryPrecedence = GetUnaryPrecedence(currentToken.Kind);
    if (unaryPrecedence != 0 && unaryPrecedence >= parentPrecedence)
    {
        Token op = currentToken;
        NextToken();
        SyntaxNode *expression = ParseBinaryExpression(unaryPrecedence);
        left = new UnaryExpressionNode(expression, op);
    }
    else
    {
        left = ParsePrimaryExpression();
    }

    while (true)
    {

        int precedence = GetBinaryPrecedence(currentToken.Kind);
        if (precedence == 0 || precedence <= parentPrecedence)
        {
            break;
        }
        Token op = currentToken;
        NextToken();
        SyntaxNode *right = ParseBinaryExpression(precedence);
        left = new BinaryExpressionNode(left, right, op);
    }
    return left;
}

int Parser::GetUnaryPrecedence(SyntaxKind kind)
{
    switch (kind)
    {
    case SyntaxKind::PLUS:
    case SyntaxKind::MINUS:
    case SyntaxKind::BANG:
    case SyntaxKind::TILDE:
        return 6;
    default:
        return 0;
    }
}

int Parser::GetBinaryPrecedence(SyntaxKind kind)
{
    switch (kind)
    {
    case SyntaxKind::MULTIPLY:
    case SyntaxKind::DIVIDE:
        return 5;
    case SyntaxKind::PLUS:
    case SyntaxKind::MINUS:
        return 4;
    case SyntaxKind::EQUALS_EQUALS:
    case SyntaxKind::BANG_EQUALS:
    case SyntaxKind::LESS:
    case SyntaxKind::LESS_EQUALS:
    case SyntaxKind::GREATER:
    case SyntaxKind::GREATER_EQUALS:
        return 3;
    case SyntaxKind::AMPERSAND:
    case SyntaxKind::AMPERSAND_AMPERSAND:
        return 2;
    case SyntaxKind::PIPE:
    case SyntaxKind::PIPE_PIPE:
    case SyntaxKind::HAT:
        return 1;
    default:
        return 0;
    }
}
Token Parser::Expect(SyntaxKind kind)
{
    if (currentToken.Kind == kind)
    {
        Token token = currentToken;
        NextToken();
        return token;
    }
    else
    {
        _diagnostics.ReportUnexpectedToken(currentToken.Span, convertSyntaxKindToString(currentToken.Kind), convertSyntaxKindToString(kind));
        return Token(kind, currentToken.value, currentToken.position);
    }
}
SyntaxNode *Parser::ParsePrimaryExpression()
{

    switch (currentToken.Kind)
    {
    case SyntaxKind::LPAREN:
        return ParseParenthesizedExpression();
        break;
    case SyntaxKind::TRUE_KEYWORD:
    case SyntaxKind::FALSE_KEYWORD:
        return ParseBooleanLiteral();
        break;
    case SyntaxKind::NUMBER:
        return ParseNumberLiteral();
        break;
    case SyntaxKind::STRING:
        return ParseStringLiteral();
        break;
    case SyntaxKind::IDENTIFIER:
    default:
        return ParseNameOrCallExpression();
        break;
    }
}

SyntaxNode *Parser::ParseNameOrCallExpression()
{
    if (peek(0).Kind == SyntaxKind::IDENTIFIER && peek(1).Kind == SyntaxKind::LPAREN)
    {
        return ParseCallExpression();
    }

    return ParseNameExpression();
}

SyntaxNode *Parser::ParseCallExpression()
{
    Token identifier = Expect(SyntaxKind::IDENTIFIER);
    Token openParenthesis = Expect(SyntaxKind::LPAREN);
    SeparatedSyntaxList<SyntaxNode> arguments = ParseArguments();
    Token closeParenthesis = Expect(SyntaxKind::RPAREN);
    return new CallExpressionNode(identifier, openParenthesis, arguments, closeParenthesis);
}

SeparatedSyntaxList<SyntaxNode> Parser::ParseArguments()
{
    std::vector<SyntaxNode *> nodesAndSeparators;
    while (currentToken.Kind != SyntaxKind::RPAREN && currentToken.Kind != SyntaxKind::END_OF_FILE)
    {
        SyntaxNode *expression = ParseExpression();
        nodesAndSeparators.push_back(expression);

        if (currentToken.Kind != SyntaxKind::RPAREN)
        {
            Token *comma = new Token(Expect(SyntaxKind::COMMA));
            nodesAndSeparators.push_back(comma);
        }
    }
    return SeparatedSyntaxList<SyntaxNode>(nodesAndSeparators);
}

SyntaxNode *Parser::ParseParenthesizedExpression()
{
    NextToken();
    SyntaxNode *expression = ParseExpression();
    Expect(SyntaxKind::RPAREN);
    return new ParenthesizedExpressionNode(expression);
}

SyntaxNode *Parser::ParseBooleanLiteral()
{
    bool isTrue = currentToken.Kind == SyntaxKind::TRUE_KEYWORD;
    SyntaxNode *node = new LiteralExpressionNode(currentToken, isTrue);
    NextToken();
    return node;
}

SyntaxNode *Parser::ParseStringLiteral()
{
    Token stringToken = Expect(SyntaxKind::STRING);
    return new LiteralExpressionNode(stringToken);
}
SyntaxNode *Parser::ParseNumberLiteral()
{
    SyntaxNode *node = new LiteralExpressionNode(currentToken, std::stoi(currentToken.value));
    NextToken();
    return node;
}

SyntaxNode *Parser::ParseNameExpression()
{
    Token name = Expect(SyntaxKind::IDENTIFIER);
    return new NameExpressionNode(name);
}