#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include "CodeAnalysis/Diagnostic.h"
#include "CodeAnalysis/SourceText.h"

#include <vector>
#include <string>
#include <any>

enum class SyntaxKind
{
    NUMBER,
    STRING,

    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    LPAREN, // (
    RPAREN, // )
    LBRACKET,
    RBRACKET,
    BANG,
    EQUALS,
    LESS,
    LESS_EQUALS,
    GREATER,
    GREATER_EQUALS,
    EQUALS_EQUALS,
    BANG_EQUALS,
    AMPERSAND,
    TILDE,
    HAT,
    PIPE,

    AMPERSAND_AMPERSAND,
    PIPE_PIPE,
    IDENTIFIER,
    WHITESPACE,
    END_OF_FILE,
    BAD_TOKEN,
    OPEN_BRACE,
    CLOSE_BRACE,
    COMMA,
    COLON,

    TRUE_KEYWORD,
    FALSE_KEYWORD,
    LET_KEYWORD,
    VAR_KEYWORD,
    ELSE_KEYWORD,
    IF_KEYWORD,
    WHILE_KEYWORD,
    FOR_KEYWORD,
    TO_KEYWORD,
    FUNCTION_KEYWORD,
    BREAK_KEYWORD,
    CONTINUE_KEYWORD,
    RETURN_KEYWORD,

    LiteralExpression,
    UnaryExpression,
    BinaryExpression,
    ParenthesizedExpression,
    NameExpression,
    AssignmentExpression,
    CallExpression,

    CompilationUnit,
    FunctionDeclaration,
    GlobalStatement,
    Parameter,
    TypeClause,
    ElseClause,

    ExpressionStatement,
    VariableDeclaration,
    BlockStatement,
    IfStatement,
    WhileStatement,
    ForStatement,
    BreakStatement,
    ContinueStatement,
    ReturnStatement

};

std::string convertSyntaxKindToString(SyntaxKind kind);
class CompilationUnitNode;

class Token;

class SyntaxTree
{
public:
    SourceText Text;
    std::vector<Diagnostic> Diagnostics;
    CompilationUnitNode *Root;
    static SyntaxTree Parse(std::string text);
    static SyntaxTree Parse(SourceText text);
    static void Parse(SyntaxTree *syntaxTree, CompilationUnitNode *&root, std::vector<Diagnostic> &diagnostics);
    static std::vector<Token> ParseTokens(std::string text);
    static std::vector<Token> ParseTokens(std::string text, std::vector<Diagnostic> &diagnostics);
    static std::vector<Token> ParseTokens(SourceText text);
    static std::vector<Token> ParseTokens(SourceText text, std::vector<Diagnostic> &diagnostics);
    static SyntaxTree Load(std::string filename);

private:
    using ParseHandler = void (*)(SyntaxTree *syntaxTree, CompilationUnitNode *&root, std::vector<Diagnostic> &diagnostics);
    SyntaxTree(SourceText text, ParseHandler handler);
};

class SyntaxNode
{
public:
    virtual ~SyntaxNode() = default;
    SyntaxTree *syntaxTree;
    SyntaxKind Kind;
    TextLocation Location;
    virtual std::vector<SyntaxNode *> GetChildren() const { return {}; }
    virtual TextSpan Span() const
    {
        const auto &children = GetChildren();
        if (children.empty())
        {
            return TextSpan(0, 0);
        }

        int start = children.front()->Span().Start;
        int end = children.back()->Span().End;

        return TextSpan::FromBounds(start, end);
    }

    void WriteTo(std::ostream &os)
    {
        PrettyPrint(os, this);
    }
    // Token GetLastToken()
    // {
    //     auto children = GetChildren();
    //     if (children.empty())
    //     {
    //         return Token();
    //     }
    //     return children.back()->GetLastToken();
    // }
    std::string ToString()
    {
        std::ostringstream writer;
        WriteTo(writer);
        return writer.str();
    }
    SyntaxNode(SyntaxTree *_syntaxTree, SyntaxKind kind) : syntaxTree(_syntaxTree), Kind(kind), Location(_syntaxTree->Text, Span())
    {
    }

private:
    static void PrettyPrint(std::ostream &os, SyntaxNode *node, std::string indent = "", bool isLast = true);
};

class Token : public SyntaxNode
{
public:
    std::string value;
    size_t position;
    TextSpan Span;

    Token(SyntaxTree *syntaxTree, SyntaxKind kind, std::string value, size_t position) : SyntaxNode(syntaxTree, kind), value(value), position(position), Span(TextSpan(position, value.empty() ? 0 : value.size())) {}
    Token(SyntaxTree *syntaxTree) : SyntaxNode(syntaxTree, SyntaxKind::BAD_TOKEN), Span(TextSpan(0, 0)) {};
    Token(SyntaxTree *syntaxtree, const Token &other)
        : SyntaxNode(syntaxtree, other.Kind), value(other.value), position(other.position), Span(other.Span) {}

    bool operator==(const Token &other) const
    {
        return Kind == other.Kind && value == other.value && position == other.position;
    }

    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {};
    }
};

// Abstract class representing a statement syntax
class StatementSyntax : public SyntaxNode
{
public:
    StatementSyntax(SyntaxTree *syntaxTree, SyntaxKind kind) : SyntaxNode(syntaxTree, kind) {}
    virtual ~StatementSyntax() = 0; // Pure virtual destructor to make it abstract
};

// Definition for pure virtual destructor
inline StatementSyntax::~StatementSyntax() {}

class SeparatedSyntaxListBase
{
public:
    virtual ~SeparatedSyntaxListBase() = default;
    virtual std::vector<SyntaxNode *> GetWithSeparators() const = 0;
};

template <typename T>
class SeparatedSyntaxList : public SeparatedSyntaxListBase
{
    static_assert(std::is_base_of<SyntaxNode, T>::value, "T must be a derived class of SyntaxNode.");

private:
    std::vector<SyntaxNode *> nodesAndSeparators;

public:
    SeparatedSyntaxList(std::vector<SyntaxNode *> nodesAndSeparators)
        : nodesAndSeparators(std::move(nodesAndSeparators)) {}

    int Count() const
    {
        return (nodesAndSeparators.size() + 1) / 2;
    }

    T *operator[](int index) const
    {
        return static_cast<T *>(nodesAndSeparators[index * 2]);
    }

    Token *GetSeparator(int index) const
    {
        if (index == Count() - 1)
            return nullptr;
        return static_cast<Token *>(nodesAndSeparators[index * 2 + 1]);
    }

    std::vector<SyntaxNode *> GetWithSeparators() const override
    {
        return nodesAndSeparators;
    }

    // Iterator to allow ranged-for loop support
    class Iterator
    {
    private:
        const SeparatedSyntaxList &list;
        int index;

    public:
        Iterator(const SeparatedSyntaxList &list, int index) : list(list), index(index) {}

        T *operator*() const
        {
            return list[index];
        }

        Iterator &operator++()
        {
            ++index;
            return *this;
        }

        bool operator!=(const Iterator &other) const
        {
            return index != other.index;
        }
    };

    Iterator begin() const
    {
        return Iterator(*this, 0);
    }

    Iterator end() const
    {
        return Iterator(*this, Count());
    }
};

class TypeClauseNode : public SyntaxNode
{
public:
    Token ColonToken;
    Token IdentifierToken;
    TypeClauseNode(SyntaxTree *syntaxTree, Token ColonToken, Token IdentifierToken)
        : SyntaxNode(syntaxTree, SyntaxKind::TypeClause), ColonToken(ColonToken), IdentifierToken(IdentifierToken) {}

    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&ColonToken)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&IdentifierToken))};
    }

    TextSpan Span() const override
    {
        return TextSpan(ColonToken.Span.Start, IdentifierToken.Span.End);
    }
};

class VariableDeclarationSyntax : public StatementSyntax
{
public:
    VariableDeclarationSyntax(SyntaxTree *syntaxTree, Token keyword, Token identifier, TypeClauseNode *typeClause, Token equalsToken, SyntaxNode *initializer)
        : StatementSyntax(syntaxTree, SyntaxKind::VariableDeclaration), Keyword(keyword), Identifier(identifier), TypeClause(typeClause), EqualsToken(equalsToken), Initializer(initializer) {}
    Token Identifier;
    Token EqualsToken;
    Token Keyword;
    TypeClauseNode *TypeClause;
    SyntaxNode *Initializer;

    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&Keyword)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&Identifier)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&EqualsToken)), Initializer};
    }
};

class ExpressionStatementSyntax : public StatementSyntax
{
public:
    ExpressionStatementSyntax(SyntaxTree *syntaxTree, SyntaxNode *expression)
        : StatementSyntax(syntaxTree, SyntaxKind::ExpressionStatement), Expression(expression) {}
    SyntaxNode *Expression;
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {Expression};
    }
};
class BlockStatementSyntax : public StatementSyntax
{
public:
    BlockStatementSyntax(SyntaxTree *syntaxTree, Token openBraceToken, std::vector<StatementSyntax *> statements, Token closeBraceToken)
        : StatementSyntax(syntaxTree, SyntaxKind::BlockStatement), OpenBraceToken(openBraceToken), Statements(statements), CloseBraceToken(closeBraceToken) {}
    Token OpenBraceToken;
    std::vector<StatementSyntax *> Statements;
    Token CloseBraceToken;
    std::vector<SyntaxNode *> GetChildren() const override
    {
        std::vector<SyntaxNode *> children;
        children.push_back(const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&OpenBraceToken)));
        for (auto statement : Statements)
        {
            children.push_back(statement);
        }
        children.push_back(const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&CloseBraceToken)));
        return children;
    }
};

class ForStatementSyntax : public StatementSyntax
{
public:
    ForStatementSyntax(SyntaxTree *syntaxTree, Token keyword, Token identifier, Token equalsToken, SyntaxNode *lowerBound, Token toKeyword, SyntaxNode *upperBound, StatementSyntax *body)
        : StatementSyntax(syntaxTree, SyntaxKind::ForStatement), Keyword(keyword), Identifier(identifier), EqualsToken(equalsToken), LowerBound(lowerBound), ToKeyword(toKeyword), UpperBound(upperBound), Body(body) {}
    Token Keyword;
    Token Identifier;
    Token EqualsToken;
    SyntaxNode *LowerBound;
    Token ToKeyword;
    SyntaxNode *UpperBound;
    StatementSyntax *Body;

    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&Keyword)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&Identifier)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&EqualsToken)), LowerBound, const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&ToKeyword)), UpperBound, Body};
    }
};
class BreakStatementSyntax : public StatementSyntax
{
public:
    Token Keyword;
    BreakStatementSyntax(SyntaxTree *syntaxTree, Token keyword)
        : StatementSyntax(syntaxTree, SyntaxKind::BreakStatement), Keyword(keyword) {}

    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&Keyword))};
    }
};

class ContinueStatementSyntax : public StatementSyntax
{

public:
    Token Keyword;
    ContinueStatementSyntax(SyntaxTree *syntaxTree, Token keyword)
        : StatementSyntax(syntaxTree, SyntaxKind::ContinueStatement), Keyword(keyword) {}

    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&Keyword))};
    }
};

class ReturnStatementSyntax : public StatementSyntax
{
public:
    Token Keyword;
    SyntaxNode *Expression;
    ReturnStatementSyntax(SyntaxTree *syntaxTree, Token keyword, SyntaxNode *expression)
        : StatementSyntax(syntaxTree, SyntaxKind::ReturnStatement), Keyword(keyword), Expression(expression) {}

    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&Keyword)), Expression};
    }
};
class ElseClauseSyntax : public SyntaxNode
{
public:
    ElseClauseSyntax(SyntaxTree *syntaxTree, Token elseKeyword, StatementSyntax *elseStatement)
        : SyntaxNode(syntaxTree, SyntaxKind::ElseClause), ElseKeyword(elseKeyword), ElseStatement(elseStatement) {}

    Token ElseKeyword;
    StatementSyntax *ElseStatement;
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&ElseKeyword)), ElseStatement};
    }
};

class IfStatementSyntax : public StatementSyntax
{
public:
    IfStatementSyntax(SyntaxTree *syntaxTree, Token ifkeyword, SyntaxNode *condition, StatementSyntax *thenStatement, ElseClauseSyntax *elseClause)
        : StatementSyntax(syntaxTree, SyntaxKind::IfStatement), IfKeyword(ifkeyword), Condition(condition), ThenStatement(thenStatement), ElseClause(elseClause) {}
    Token IfKeyword;
    SyntaxNode *Condition;
    StatementSyntax *ThenStatement;
    ElseClauseSyntax *ElseClause;
    std::vector<SyntaxNode *> GetChildren() const override
    {
        std::vector<SyntaxNode *> children = {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&IfKeyword)), Condition, ThenStatement};
        if (ElseClause != nullptr)
        {
            children.push_back(ElseClause);
        }
        return children;
    }
};

class WhileStatementSyntax : public StatementSyntax
{
public:
    WhileStatementSyntax(SyntaxTree *syntaxTree, Token whileKeyword, SyntaxNode *condition, StatementSyntax *body)
        : StatementSyntax(syntaxTree, SyntaxKind::WhileStatement), WhileKeyword(whileKeyword), Condition(condition), Body(body) {}
    Token WhileKeyword;
    SyntaxNode *Condition;
    StatementSyntax *Body;
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&WhileKeyword)), Condition, Body};
    }
};

class MemberSyntax : public SyntaxNode
{
public:
    MemberSyntax(SyntaxTree *syntaxTree, SyntaxKind kind) : SyntaxNode(syntaxTree, kind) {}
    virtual std::vector<SyntaxNode *> GetChildren() const override
    {
        return {};
    }
};
class GlobalStatementSyntax : public MemberSyntax
{
public:
    GlobalStatementSyntax(SyntaxTree *syntaxTree, StatementSyntax *statement)
        : MemberSyntax(syntaxTree, SyntaxKind::GlobalStatement), Statement(statement) {}
    StatementSyntax *Statement;
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {Statement};
    }
};

class ParameterNode : public SyntaxNode
{
public:
    Token IdentifierToken;
    TypeClauseNode *Type;

    ParameterNode(SyntaxTree *syntaxTree, Token IdentifierToken, TypeClauseNode *Type)
        : SyntaxNode(syntaxTree, SyntaxKind::Parameter), IdentifierToken(IdentifierToken), Type(Type) {}

    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&IdentifierToken)), Type};
    }
};

class FunctionDeclarationSyntax : public MemberSyntax
{
public:
    FunctionDeclarationSyntax(SyntaxTree *syntaxTree, Token functionKeyword, Token identifier, Token openParenthesisToken, SeparatedSyntaxList<ParameterNode> parameters, Token closeParenthesisToken, TypeClauseNode *type, BlockStatementSyntax *body)
        : MemberSyntax(syntaxTree, SyntaxKind::FunctionDeclaration), FunctionKeyword(functionKeyword), Identifier(identifier), OpenParenthesisToken(openParenthesisToken), Parameters(parameters), CloseParenthesisToken(closeParenthesisToken), Type(type), Body(body) {}
    Token FunctionKeyword;
    Token Identifier;
    Token OpenParenthesisToken;
    SeparatedSyntaxList<ParameterNode> Parameters;
    Token CloseParenthesisToken;
    TypeClauseNode *Type;
    BlockStatementSyntax *Body;

    std::vector<SyntaxNode *> GetChildren() const override
    {
        std::vector<SyntaxNode *> children = {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&FunctionKeyword)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&Identifier)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&OpenParenthesisToken))};
        int index = 0;
        for (auto parameter : Parameters)
        {
            children.push_back(parameter);
            Token *separator = Parameters.GetSeparator(index);
            if (separator != nullptr)
            {
                children.push_back(separator);
            }
            ++index;
        }
        children.push_back(const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&CloseParenthesisToken)));
        if (Type != nullptr)
        {
            children.push_back(Type);
        }
        children.push_back(Body);
        return children;
    }
};

class CompilationUnitNode : public SyntaxNode
{
public:
    CompilationUnitNode(SyntaxTree *syntaxTree, std::vector<MemberSyntax *> members, Token endOfFileToken)
        : SyntaxNode(syntaxTree, SyntaxKind::CompilationUnit), Members(members), EndOfFileToken(endOfFileToken) {}

    std::vector<MemberSyntax *> Members;
    Token EndOfFileToken;
    std::vector<SyntaxNode *> GetChildren() const override
    {
        std::vector<SyntaxNode *> children;
        for (auto member : Members)
        {
            children.push_back(member);
        }
        children.push_back(const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&EndOfFileToken)));
        return children;
    }
};
class LiteralExpressionNode : public SyntaxNode
{
public:
    Token LiteralToken;
    std::any Value;

    LiteralExpressionNode(SyntaxTree *syntaxTree, Token LiteralToken, std::any Value)
        : SyntaxNode(syntaxTree, SyntaxKind::LiteralExpression), LiteralToken(LiteralToken), Value(Value) {}

    LiteralExpressionNode(SyntaxTree *syntaxTree, Token LiteralToken)
        : SyntaxNode(syntaxTree, SyntaxKind::LiteralExpression), LiteralToken(LiteralToken), Value(LiteralToken.value) {}
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&LiteralToken))};
    }
};

class NameExpressionNode : public SyntaxNode
{
public:
    Token IdentifierToken;
    NameExpressionNode(SyntaxTree *syntaxTree, Token IdentifierToken)
        : SyntaxNode(syntaxTree, SyntaxKind::NameExpression), IdentifierToken(IdentifierToken) {}
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&IdentifierToken))};
    }
};

class AssignmentExpressionNode : public SyntaxNode
{
public:
    Token IdentifierToken;
    Token EqualsToken;
    SyntaxNode *Expression;
    AssignmentExpressionNode(SyntaxTree *syntaxTree, Token IdentifierToken, Token EqualsToken, SyntaxNode *Expression)
        : SyntaxNode(syntaxTree, SyntaxKind::AssignmentExpression), IdentifierToken(IdentifierToken), EqualsToken(EqualsToken), Expression(Expression) {}
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&IdentifierToken)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&EqualsToken)), Expression};
    }
};
class BinaryExpressionNode : public SyntaxNode
{
public:
    SyntaxNode *left;
    SyntaxNode *right;
    Token OperatorToken;
    BinaryExpressionNode(SyntaxTree *syntaxTree, SyntaxNode *left, SyntaxNode *right, Token OperatorToken)
        : SyntaxNode(syntaxTree, SyntaxKind::BinaryExpression), left(left), right(right), OperatorToken(OperatorToken) {}

    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {left, const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&OperatorToken)), right};
    }
};

class ParenthesizedExpressionNode : public SyntaxNode
{
public:
    SyntaxNode *expression;
    ParenthesizedExpressionNode(SyntaxTree *syntaxTree, SyntaxNode *expression)
        : SyntaxNode(syntaxTree, SyntaxKind::ParenthesizedExpression), expression(expression) {}
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {expression};
    }
};

class UnaryExpressionNode : public SyntaxNode
{
public:
    SyntaxNode *expression;
    Token OperatorToken;
    UnaryExpressionNode(SyntaxTree *syntaxTree, SyntaxNode *expression, Token OperatorToken)
        : SyntaxNode(syntaxTree, SyntaxKind::UnaryExpression), expression(expression), OperatorToken(OperatorToken) {}
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&OperatorToken)), expression};
    }
};

class CallExpressionNode : public SyntaxNode
{
public:
    Token IdentifierToken;
    Token OpenParenthesisToken;
    SeparatedSyntaxList<SyntaxNode> Arguments;
    Token CloseParenthesisToken;
    CallExpressionNode(SyntaxTree *syntaxTree, Token IdentifierToken, Token OpenParenthesisToken, SeparatedSyntaxList<SyntaxNode> Arguments, Token CloseParenthesisToken)
        : SyntaxNode(syntaxTree, SyntaxKind::CallExpression), IdentifierToken(IdentifierToken), OpenParenthesisToken(OpenParenthesisToken), Arguments(Arguments), CloseParenthesisToken(CloseParenthesisToken) {}

    std::vector<SyntaxNode *> GetChildren() const override
    {
        std::vector<SyntaxNode *> children = {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&IdentifierToken)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&OpenParenthesisToken))};
        int index = 0;
        for (auto argument : Arguments)
        {
            children.push_back(argument);
            Token *separator = Arguments.GetSeparator(index);
            if (separator != nullptr)
            {
                children.push_back(separator);
            }
            ++index;
        }
        children.push_back(const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&CloseParenthesisToken)));
        return children;
    }
};

#endif
