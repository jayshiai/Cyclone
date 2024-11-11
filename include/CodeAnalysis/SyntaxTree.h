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
    OPEN_BRACKET,
    CLOSE_BRACKET,
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
    SingleLineComment,
    MultiLineComment,

    TRUE_KEYWORD,
    FALSE_KEYWORD,
    CONST_KEYWORD,
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
    ArrayInitializer,
    ArrayAccessExpression,
    ArrayAssignmentExpression,

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
    virtual std::vector<SyntaxNode *> GetChildren() const
    {
        return {};
    }
    virtual TextSpan Span() const
    {
        std::vector<SyntaxNode *> children = GetChildren();
        if (children.empty())
        {
            return TextSpan(0, 0);
        }

        int start = children.front()->Span().Start;
        int end = children.back()->Span().End;

        return TextSpan::FromBounds(start, end);
    }

    void InitializeLocation()
    {
        Location = TextLocation(syntaxTree->Text, Span());
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
    SyntaxNode(SyntaxTree *_syntaxTree, SyntaxKind kind) : syntaxTree(_syntaxTree), Kind(kind)
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
    TextSpan span;

    Token(SyntaxTree *syntaxTree, SyntaxKind kind, std::string value, size_t position) : SyntaxNode(syntaxTree, kind), value(value), position(position), span(TextSpan(position, value.empty() ? 0 : value.size()))
    {
        InitializeLocation();
    }
    Token(SyntaxTree *syntaxTree) : SyntaxNode(syntaxTree, SyntaxKind::BAD_TOKEN), span(TextSpan(0, 0))
    {
        InitializeLocation();
    };
    Token(SyntaxTree *syntaxtree, const Token &other)
        : SyntaxNode(syntaxtree, other.Kind), value(other.value), position(other.position), span(other.span)
    {
        InitializeLocation();
    }

    TextSpan Span() const override
    {
        return span;
    }

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

    bool IsArray;
    TypeClauseNode *ElementType;
    SyntaxNode *Size;
    TypeClauseNode(SyntaxTree *syntaxTree, Token ColonToken, Token IdentifierToken, bool isArray = false, SyntaxNode *size = nullptr)
        : SyntaxNode(syntaxTree, SyntaxKind::TypeClause), ColonToken(ColonToken), IdentifierToken(IdentifierToken), IsArray(isArray), Size(size)
    {
        InitializeLocation();
        if (isArray)
        {
            ElementType = new TypeClauseNode(syntaxTree, ColonToken, IdentifierToken);
        }
    }

    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&ColonToken)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&IdentifierToken))};
    }

    TextSpan Span() const override
    {
        return TextSpan(ColonToken.Span().Start, IdentifierToken.Span().End);
    }
};

class VariableDeclarationSyntax : public StatementSyntax
{
public:
    VariableDeclarationSyntax(SyntaxTree *syntaxTree, Token keyword, Token identifier, TypeClauseNode *typeClause, Token equalsToken, SyntaxNode *initializer)
        : StatementSyntax(syntaxTree, SyntaxKind::VariableDeclaration), Keyword(keyword), Identifier(identifier), TypeClause(typeClause), EqualsToken(equalsToken), Initializer(initializer)
    {
        InitializeLocation();
    }
    Token Identifier;
    Token EqualsToken;
    Token Keyword;
    TypeClauseNode *TypeClause;
    SyntaxNode *Initializer;

    TextSpan Span() const override
    {
        if (Initializer != nullptr)
        {
            return TextSpan(Keyword.Span().Start, Initializer->Span().End);
        }
        return TextSpan(Keyword.Span().Start, TypeClause->Span().End);
    }

    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&Keyword)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&Identifier)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&EqualsToken)), Initializer};
    }
};

class ArrayInitializerSyntax : public SyntaxNode
{
public:
    Token OpenBraceToken;
    SeparatedSyntaxList<SyntaxNode> Elements;
    Token CloseBraceToken;
    ArrayInitializerSyntax(SyntaxTree *syntaxTree, Token openBraceToken, SeparatedSyntaxList<SyntaxNode> initializers, Token closeBraceToken)
        : SyntaxNode(syntaxTree, SyntaxKind::ArrayInitializer), OpenBraceToken(openBraceToken), Elements(initializers), CloseBraceToken(closeBraceToken)
    {
        InitializeLocation();
    }

    TextSpan Span() const override
    {
        return TextSpan(OpenBraceToken.Span().Start, CloseBraceToken.Span().End);
    }
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&OpenBraceToken)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&CloseBraceToken))};
    }
};

class ArrayAccessExpressionSyntax : public SyntaxNode
{
public:
    SyntaxNode *Identifier;
    Token OpenBracketToken;
    SyntaxNode *Index;
    Token CloseBracketToken;

    ArrayAccessExpressionSyntax(SyntaxTree *syntaxTree, SyntaxNode *identifier, Token openBracketToken, SyntaxNode *index, Token closeBracketToken)
        : SyntaxNode(syntaxTree, SyntaxKind::ArrayAccessExpression), Identifier(identifier), OpenBracketToken(openBracketToken), Index(index), CloseBracketToken(closeBracketToken)
    {
        InitializeLocation();
    }

    TextSpan Span() const override
    {
        return TextSpan(Identifier->Span().Start, CloseBracketToken.Span().End);
    }

    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&Identifier)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&OpenBracketToken)), Index, const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&CloseBracketToken))};
    }
};

class ArrayAssignmentExpressionSyntax : public SyntaxNode
{
public:
    SyntaxNode *Identifier;
    Token OpenBracketToken;
    SyntaxNode *Index;
    Token CloseBracketToken;
    Token EqualsToken;
    SyntaxNode *Expression;

    ArrayAssignmentExpressionSyntax(SyntaxTree *syntaxTree, SyntaxNode *identifier, Token openBracketToken, SyntaxNode *index, Token closeBracketToken, Token equalsToken, SyntaxNode *right)
        : SyntaxNode(syntaxTree, SyntaxKind::ArrayAssignmentExpression), Identifier(identifier), OpenBracketToken(openBracketToken), Index(index), CloseBracketToken(closeBracketToken), EqualsToken(equalsToken), Expression(right)
    {
        InitializeLocation();
    }

    TextSpan Span() const override
    {
        return TextSpan(Identifier->Span().Start, Expression->Span().End);
    }

    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&Identifier)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&OpenBracketToken)), Index, const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&CloseBracketToken)), const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&EqualsToken)), Expression};
    }
};
class ExpressionStatementSyntax : public StatementSyntax
{
public:
    ExpressionStatementSyntax(SyntaxTree *syntaxTree, SyntaxNode *expression)
        : StatementSyntax(syntaxTree, SyntaxKind::ExpressionStatement), Expression(expression)
    {
        InitializeLocation();
    }
    SyntaxNode *Expression;

    TextSpan Span() const override
    {
        return Expression->Span();
    }
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {Expression};
    }
};
class BlockStatementSyntax : public StatementSyntax
{
public:
    BlockStatementSyntax(SyntaxTree *syntaxTree, Token openBraceToken, std::vector<StatementSyntax *> statements, Token closeBraceToken)
        : StatementSyntax(syntaxTree, SyntaxKind::BlockStatement), OpenBraceToken(openBraceToken), Statements(statements), CloseBraceToken(closeBraceToken)
    {
        InitializeLocation();
    }
    Token OpenBraceToken;
    std::vector<StatementSyntax *> Statements;
    Token CloseBraceToken;

    TextSpan Span() const override
    {
        return TextSpan(OpenBraceToken.Span().Start, CloseBraceToken.Span().End);
    }
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
        : StatementSyntax(syntaxTree, SyntaxKind::ForStatement), Keyword(keyword), Identifier(identifier), EqualsToken(equalsToken), LowerBound(lowerBound), ToKeyword(toKeyword), UpperBound(upperBound), Body(body)
    {
        InitializeLocation();
    }
    Token Keyword;
    Token Identifier;
    Token EqualsToken;
    SyntaxNode *LowerBound;
    Token ToKeyword;
    SyntaxNode *UpperBound;
    StatementSyntax *Body;

    TextSpan Span() const override
    {
        return TextSpan(Keyword.Span().Start, Body->Span().End);
    }

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
        : StatementSyntax(syntaxTree, SyntaxKind::BreakStatement), Keyword(keyword)
    {
        InitializeLocation();
    }

    TextSpan Span() const override
    {
        return Keyword.Span();
    }
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
        : StatementSyntax(syntaxTree, SyntaxKind::ContinueStatement), Keyword(keyword)
    {
        InitializeLocation();
    }

    TextSpan Span() const override
    {
        return Keyword.Span();
    }

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
        : StatementSyntax(syntaxTree, SyntaxKind::ReturnStatement), Keyword(keyword), Expression(expression)
    {
        InitializeLocation();
    }

    TextSpan Span() const override
    {
        if (Expression != nullptr)
        {
            return TextSpan(Keyword.Span().Start, Expression->Span().End);
        }
        return Keyword.Span();
    }

    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&Keyword)), Expression};
    }
};
class ElseClauseSyntax : public SyntaxNode
{
public:
    ElseClauseSyntax(SyntaxTree *syntaxTree, Token elseKeyword, StatementSyntax *elseStatement)
        : SyntaxNode(syntaxTree, SyntaxKind::ElseClause), ElseKeyword(elseKeyword), ElseStatement(elseStatement)
    {
        InitializeLocation();
    }

    TextSpan Span() const override
    {
        return TextSpan(ElseKeyword.Span().Start, ElseStatement->Span().End);
    }

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
        : StatementSyntax(syntaxTree, SyntaxKind::IfStatement), IfKeyword(ifkeyword), Condition(condition), ThenStatement(thenStatement), ElseClause(elseClause)
    {
        InitializeLocation();
    }
    Token IfKeyword;
    SyntaxNode *Condition;
    StatementSyntax *ThenStatement;
    ElseClauseSyntax *ElseClause;

    TextSpan Span() const override
    {
        if (ElseClause != nullptr)
        {
            return TextSpan(IfKeyword.Span().Start, ElseClause->Span().End);
        }
        return TextSpan(IfKeyword.Span().Start, ThenStatement->Span().End);
    }
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
        : StatementSyntax(syntaxTree, SyntaxKind::WhileStatement), WhileKeyword(whileKeyword), Condition(condition), Body(body)
    {
        InitializeLocation();
    }
    Token WhileKeyword;
    SyntaxNode *Condition;
    StatementSyntax *Body;

    TextSpan Span() const override
    {
        return TextSpan(WhileKeyword.Span().Start, Body->Span().End);
    }
    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&WhileKeyword)), Condition, Body};
    }
};

class MemberSyntax : public SyntaxNode
{
public:
    MemberSyntax(SyntaxTree *syntaxTree, SyntaxKind kind) : SyntaxNode(syntaxTree, kind)
    {
        InitializeLocation();
    }

    virtual std::vector<SyntaxNode *> GetChildren() const override
    {
        return {};
    }
};
class GlobalStatementSyntax : public MemberSyntax
{
public:
    GlobalStatementSyntax(SyntaxTree *syntaxTree, StatementSyntax *statement)
        : MemberSyntax(syntaxTree, SyntaxKind::GlobalStatement), Statement(statement)
    {
        InitializeLocation();
    }

    TextSpan Span() const override
    {
        return Statement->Span();
    }
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
        : SyntaxNode(syntaxTree, SyntaxKind::Parameter), IdentifierToken(IdentifierToken), Type(Type)
    {
        InitializeLocation();
    }

    TextSpan Span() const override
    {
        return TextSpan(IdentifierToken.Span().Start, Type->Span().End);
    }

    std::vector<SyntaxNode *> GetChildren() const override
    {
        return {const_cast<SyntaxNode *>(reinterpret_cast<const SyntaxNode *>(&IdentifierToken)), Type};
    }
};

class FunctionDeclarationSyntax : public MemberSyntax
{
public:
    FunctionDeclarationSyntax(SyntaxTree *syntaxTree, Token functionKeyword, Token identifier, Token openParenthesisToken, SeparatedSyntaxList<ParameterNode> parameters, Token closeParenthesisToken, TypeClauseNode *type, BlockStatementSyntax *body)
        : MemberSyntax(syntaxTree, SyntaxKind::FunctionDeclaration), FunctionKeyword(functionKeyword), Identifier(identifier), OpenParenthesisToken(openParenthesisToken), Parameters(parameters), CloseParenthesisToken(closeParenthesisToken), Type(type), Body(body)
    {
        InitializeLocation();
    }
    Token FunctionKeyword;
    Token Identifier;
    Token OpenParenthesisToken;
    SeparatedSyntaxList<ParameterNode> Parameters;
    Token CloseParenthesisToken;
    TypeClauseNode *Type;
    BlockStatementSyntax *Body;

    TextSpan Span() const override
    {
        return TextSpan(FunctionKeyword.Span().Start, Body->Span().End);
    }
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
        : SyntaxNode(syntaxTree, SyntaxKind::CompilationUnit), Members(members), EndOfFileToken(endOfFileToken)
    {
        InitializeLocation();
    }

    std::vector<MemberSyntax *> Members;
    Token EndOfFileToken;

    TextSpan Span() const override
    {
        if (Members.empty())
        {
            return EndOfFileToken.Span();
        }
        return TextSpan(Members.front()->Span().Start, EndOfFileToken.Span().End);
    }
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
        : SyntaxNode(syntaxTree, SyntaxKind::LiteralExpression), LiteralToken(LiteralToken), Value(Value)
    {
        InitializeLocation();
    }

    LiteralExpressionNode(SyntaxTree *syntaxTree, Token LiteralToken)
        : SyntaxNode(syntaxTree, SyntaxKind::LiteralExpression), LiteralToken(LiteralToken), Value(LiteralToken.value)
    {
        InitializeLocation();
    }
    TextSpan Span() const override
    {
        return LiteralToken.Span();
    }
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
        : SyntaxNode(syntaxTree, SyntaxKind::NameExpression), IdentifierToken(IdentifierToken)
    {
        InitializeLocation();
    }

    TextSpan Span() const override
    {
        return IdentifierToken.Span();
    }
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
        : SyntaxNode(syntaxTree, SyntaxKind::AssignmentExpression), IdentifierToken(IdentifierToken), EqualsToken(EqualsToken), Expression(Expression)
    {
        InitializeLocation();
    }

    TextSpan Span() const override
    {
        return TextSpan(IdentifierToken.Span().Start, Expression->Span().End);
    }
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
        : SyntaxNode(syntaxTree, SyntaxKind::BinaryExpression), left(left), right(right), OperatorToken(OperatorToken)
    {
        InitializeLocation();
    }

    TextSpan Span() const override
    {
        return TextSpan(left->Span().Start, right->Span().End);
    }

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
        : SyntaxNode(syntaxTree, SyntaxKind::ParenthesizedExpression), expression(expression)
    {
        InitializeLocation();
    }

    TextSpan Span() const override
    {
        return TextSpan(expression->Span().Start, expression->Span().End);
    }
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
        : SyntaxNode(syntaxTree, SyntaxKind::UnaryExpression), expression(expression), OperatorToken(OperatorToken)
    {
        InitializeLocation();
    }

    TextSpan Span() const override
    {
        return TextSpan(OperatorToken.Span().Start, expression->Span().End);
    }
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
        : SyntaxNode(syntaxTree, SyntaxKind::CallExpression), IdentifierToken(IdentifierToken), OpenParenthesisToken(OpenParenthesisToken), Arguments(Arguments), CloseParenthesisToken(CloseParenthesisToken)
    {
        InitializeLocation();
    }

    TextSpan Span() const override
    {
        return TextSpan(IdentifierToken.Span().Start, CloseParenthesisToken.Span().End);
    }
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
