#ifndef BINDER_H
#define BINDER_H

#include "CodeAnalysis/SyntaxTree.h"
#include <unordered_map>
#include <any>
enum class BoundNodeKind
{
    LiteralExpression,
    UnaryExpression,
    BinaryExpression,
    ParenthesizedExpression,
    VariableExpression,
    AssignmentExpression,

    ExpressionStatement,
    VariableDeclaration,
    BlockStatement
};

enum class Type
{
    Boolean,
    Integer,
    Unknown
};

std::string convertTypetoString(Type type);

class BoundNode
{
public:
    virtual ~BoundNode() {}
    BoundNodeKind kind;
};

class BoundExpression : public BoundNode
{
public:
    Type type;
    BoundExpression(Type type) : type(type) {}
    virtual ~BoundExpression() {}
    // Add virtual method to get the kind dynamically
    virtual BoundNodeKind GetKind() const = 0;
};
class BoundStatement : public BoundNode
{
public:
    virtual ~BoundStatement() {}
    virtual BoundNodeKind GetKind() const = 0;
};
class VariableSymbol
{
public:
    std::string Name;
    Type type;
    bool IsReadOnly;
    VariableSymbol() : Name(""), IsReadOnly(false), type(Type::Unknown) {}
    VariableSymbol(std::string name, bool isReadOnly, Type type) : Name(name), IsReadOnly(isReadOnly), type(type) {}

    friend std::ostream &operator<<(std::ostream &os, const VariableSymbol &var)
    {
        os << "VariableSymbol(Name: " << var.Name << ")";
        return os;
    }

    bool operator==(const VariableSymbol &other) const
    {
        return Name == other.Name;
    }
};

namespace std
{
    template <>
    struct hash<VariableSymbol>
    {
        std::size_t operator()(const VariableSymbol &symbol) const
        {
            return std::hash<std::string>()(symbol.Name);
        }
    };
}
class BoundVariableDeclaration : public BoundStatement
{
public:
    BoundVariableDeclaration(VariableSymbol variable, BoundExpression *initializer) : Variable(variable), Initializer(initializer) {};
    VariableSymbol Variable;
    BoundExpression *Initializer;
    BoundNodeKind kind = BoundNodeKind::VariableDeclaration;
    BoundNodeKind GetKind() const override { return kind; }
};
class BoundExpressionStatement : public BoundStatement
{
public:
    BoundExpressionStatement(BoundExpression *expression) : Expression(expression) {};
    BoundExpression *Expression;
    BoundNodeKind kind = BoundNodeKind::ExpressionStatement;
    BoundNodeKind GetKind() const override { return kind; }
};

class BoundBlockStatement : public BoundStatement
{
public:
    BoundBlockStatement(std::vector<BoundStatement *> statements) : Statements(statements) {};
    std::vector<BoundStatement *> Statements;
    BoundNodeKind kind = BoundNodeKind::BlockStatement;
    BoundNodeKind GetKind() const override { return kind; }
};
enum class BoundUnaryOperatorKind
{
    Identity,
    Negation,
    LogicalNegation
};

enum class BoundBinaryOperatorKind
{
    Addition,
    Subtraction,
    Multiplication,
    Division,
    LogicalAnd,
    LogicalOr,
    Equals,
    NotEquals
};

class BoundUnaryOperator
{
public:
    SyntaxKind syntaxKind;
    BoundUnaryOperatorKind Kind;
    Type OperandType;
    Type ResultType;
    static BoundUnaryOperator *Bind(SyntaxKind syntaxKind, Type operandType);

private:
    BoundUnaryOperator(SyntaxKind syntaxKind, BoundUnaryOperatorKind kind, Type operandType, Type resultType) : syntaxKind(syntaxKind), Kind(kind), OperandType(operandType), ResultType(resultType) {};
    BoundUnaryOperator(SyntaxKind syntaxKind, BoundUnaryOperatorKind kind, Type operandType) : BoundUnaryOperator(syntaxKind, kind, operandType, operandType) {};
    static const std::vector<BoundUnaryOperator> operators;
};

class BoundBinaryOperator
{
public:
    SyntaxKind syntaxKind;
    BoundBinaryOperatorKind Kind;
    Type LeftType;
    Type RightType;
    Type ResultType;
    static BoundBinaryOperator *Bind(SyntaxKind syntaxKind, Type leftType, Type rightType);

private:
    BoundBinaryOperator(SyntaxKind syntaxKind, BoundBinaryOperatorKind kind, Type leftType, Type rightTpye, Type resultType) : syntaxKind(syntaxKind), Kind(kind), LeftType(leftType), RightType(rightTpye), ResultType(resultType) {};
    BoundBinaryOperator(SyntaxKind syntaxKind, BoundBinaryOperatorKind kind, Type operandType, Type resultType) : BoundBinaryOperator(syntaxKind, kind, operandType, operandType, resultType) {};
    BoundBinaryOperator(SyntaxKind syntaxKind, BoundBinaryOperatorKind kind, Type type) : BoundBinaryOperator(syntaxKind, kind, type, type, type) {};
    static const std::vector<BoundBinaryOperator> operators;
};

class BoundUnaryExpression : public BoundExpression
{
public:
    BoundUnaryExpression(BoundUnaryOperator *op, BoundExpression *operand) : BoundExpression(op->ResultType), Op(op), Operand(operand) {
                                                                             };
    BoundNodeKind kind = BoundNodeKind::UnaryExpression;
    BoundUnaryOperator *Op;
    BoundExpression *Operand;
    Type type;
    BoundNodeKind GetKind() const override { return BoundNodeKind::UnaryExpression; }
};

class BoundLiteralExpression : public BoundExpression
{
public:
    BoundLiteralExpression(std::string value, Type type) : BoundExpression(type), Value(value) {};
    BoundNodeKind kind = BoundNodeKind::LiteralExpression;
    std::string Value;
    Type type;
    BoundNodeKind GetKind() const override { return BoundNodeKind::LiteralExpression; }
};

class BoundVariableExpression : public BoundExpression
{
public:
    BoundVariableExpression(VariableSymbol variable) : BoundExpression(variable.type), Variable(variable) {};
    BoundNodeKind kind = BoundNodeKind::VariableExpression;
    VariableSymbol Variable;

    BoundNodeKind GetKind() const override { return BoundNodeKind::VariableExpression; }
};

class BoundAssignmentExpression : public BoundExpression
{
public:
    BoundAssignmentExpression(VariableSymbol variable, BoundExpression *expression) : BoundExpression(expression->type), Variable(variable), Expression(expression) {};
    BoundNodeKind kind = BoundNodeKind::AssignmentExpression;
    VariableSymbol Variable;
    BoundExpression *Expression;
    Type type;

    BoundNodeKind GetKind() const override { return BoundNodeKind::AssignmentExpression; }
};

class BoundBinaryExpression : public BoundExpression
{
public:
    BoundBinaryExpression(BoundExpression *left, BoundBinaryOperator *op, BoundExpression *right) : BoundExpression(op->ResultType), Left(left), Op(op), Right(right) {};
    BoundNodeKind kind = BoundNodeKind::BinaryExpression;
    BoundBinaryOperator *Op;
    BoundExpression *Left;
    BoundExpression *Right;
    Type type;
    BoundNodeKind GetKind() const override { return BoundNodeKind::BinaryExpression; }
};

class BoundScope
{
public:
    BoundScope(BoundScope *parent) : Parent(parent) {}
    BoundScope *Parent;

    bool TryDeclare(VariableSymbol &variable);
    bool TryLookup(const std::string &name, VariableSymbol &variable) const;
    std::vector<VariableSymbol> GetDeclaredVariables() const;

private:
    std::unordered_map<std::string, VariableSymbol> _variables;
};

class BoundGlobalScope
{
public:
    BoundGlobalScope(BoundGlobalScope *previous, std::vector<Diagnostic> diagnostics, std::vector<VariableSymbol> variables, BoundStatement *statement) : Previous(previous), Diagnostics(diagnostics), Variables(variables), Statement(statement) {}
    BoundGlobalScope *Previous;
    std::vector<Diagnostic> Diagnostics;
    std::vector<VariableSymbol> Variables;
    BoundStatement *Statement;
};
class Binder
{
public:
    Binder(BoundScope parent) : _scope(BoundScope(parent)) {};

    BoundExpression *BindExpression(SyntaxNode *node);
    const DiagnosticBag &GetDiagnostics() const
    {
        return _diagnostics;
    }
    static BoundGlobalScope BindGlobalScope(BoundGlobalScope *previous, CompilationUnitNode *tree);

private:
    DiagnosticBag _diagnostics;
    BoundScope _scope;

    static BoundScope CreateParentScope(BoundGlobalScope *previous);
    BoundStatement *BindStatement(StatementSyntax *node);
    BoundStatement *BindBlockStatement(BlockStatementSyntax *node);
    BoundStatement *BindVariableDeclaration(VariableDeclarationSyntax *node);
    BoundStatement *BindExpressionStatement(ExpressionStatementSyntax *node);
    BoundExpression *BindLiteralExpression(LiteralExpressionNode *node);
    BoundExpression *BindUnaryExpression(UnaryExpressionNode *node);
    BoundExpression *BindBinaryExpression(BinaryExpressionNode *node);
    BoundExpression *BindNameExpression(NameExpressionNode *node);
    BoundExpression *BindAssignmentExpression(AssignmentExpressionNode *node);
};

#endif