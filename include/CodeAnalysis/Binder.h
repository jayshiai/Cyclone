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
    BlockStatement,
    IfStatement,
    WhileStatement,
    ForStatement,

    GotoStatement,
    LabelStatement,
    ConditionalGotoStatement,
};

enum class Type
{
    Boolean,
    Integer,
    Unknown
};

enum class BoundUnaryOperatorKind
{
    Identity,
    Negation,
    LogicalNegation,
    OnesComplement,
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
    NotEquals,
    Less,
    LessOrEquals,
    Greater,
    GreaterOrEquals,
    BitwiseAnd,
    BitwiseOr,
    BitwiseXor,

};

std::string convertTypetoString(Type type);
std::string convertBoundNodeKindToString(BoundNodeKind kind);

class BoundNode
{
public:
    virtual ~BoundNode() {}
    BoundNodeKind kind;
    virtual std::vector<BoundNode *> GetChildren() const { return {}; }
    void WriteTo(std::ostream &os)
    {
        PrettyPrint(os, this);
    }

    virtual std::vector<std::pair<std::string, std::string>> GetProperties() const
    {
        return {};
    }

    friend std::ostream &operator<<(std::ostream &out, BoundNode &node)
    {
        node.WriteTo(out);
        return out;
    }

private:
    static void PrettyPrint(std::ostream &os, BoundNode *node, std::string indent = "", bool isLast = true);
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

    std::string ToString() const
    {
        return Name;
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

    std::vector<BoundNode *> GetChildren() const override
    {
        return {Initializer};
    }

    std::vector<std::pair<std::string, std::string>> GetProperties() const override
    {
        return {{"Variable", Variable.ToString()}};
    }
};
class BoundExpressionStatement : public BoundStatement
{
public:
    BoundExpressionStatement(BoundExpression *expression) : Expression(expression) {};
    BoundExpression *Expression;
    BoundNodeKind kind = BoundNodeKind::ExpressionStatement;
    BoundNodeKind GetKind() const override { return kind; }

    std::vector<BoundNode *> GetChildren() const override
    {
        return {Expression};
    }

    std::vector<std::pair<std::string, std::string>> GetProperties() const override
    {
        return {{"Expression", ""}};
    }
};

class BoundBlockStatement : public BoundStatement
{
public:
    BoundBlockStatement(std::vector<BoundStatement *> statements) : Statements(statements) {};
    std::vector<BoundStatement *> Statements;
    BoundNodeKind kind = BoundNodeKind::BlockStatement;
    BoundNodeKind GetKind() const override { return kind; }

    std::vector<BoundNode *> GetChildren() const override
    {
        std::vector<BoundNode *> children;
        for (auto &statement : Statements)
        {
            children.push_back(statement);
        }
        return children;
    }

    std::vector<std::pair<std::string, std::string>> GetProperties() const override
    {
        return {{"Statements", ""}};
    }
};
class LabelSymbol
{
public:
    std::string Name;
    LabelSymbol(std::string name) : Name(name) {}
    bool operator==(const LabelSymbol &other) const
    {
        return Name == other.Name;
    }
    std::string ToString() const
    {
        return Name;
    }

    friend std::ostream &operator<<(std::ostream &os, const LabelSymbol &label)
    {
        os << "LabelSymbol(Name: " << label.Name << ")";
        return os;
    }
};
namespace std
{
    template <>
    struct hash<LabelSymbol>
    {
        std::size_t operator()(const LabelSymbol &symbol) const
        {
            return std::hash<std::string>()(symbol.Name);
        }
    };
}

class BoundLabelStatement : public BoundStatement
{
public:
    BoundLabelStatement(LabelSymbol label) : Label(label) {};
    LabelSymbol Label;
    BoundNodeKind kind = BoundNodeKind::LabelStatement;
    BoundNodeKind GetKind() const override { return kind; }

    std::vector<std::pair<std::string, std::string>> GetProperties() const override
    {
        return {{"Label", Label.ToString()}};
    }

    std::vector<BoundNode *> GetChildren() const override
    {
        return {};
    }
};

class BoundGotoStatement : public BoundStatement
{
public:
    BoundGotoStatement(LabelSymbol label) : Label(label) {};
    BoundNodeKind kind = BoundNodeKind::GotoStatement;
    LabelSymbol Label;
    BoundNodeKind GetKind() const override { return kind; }

    std::vector<std::pair<std::string, std::string>> GetProperties() const override
    {
        return {{"Label", Label.ToString()}};
    }

    std::vector<BoundNode *> GetChildren() const override
    {
        return {};
    }
};

class BoundConditionalGotoStatement : public BoundStatement
{
public:
    BoundConditionalGotoStatement(LabelSymbol label, BoundExpression *condition, bool jumpIfTrue = true) : Label(label), Condition(condition), JumpIfTrue(jumpIfTrue) {};
    LabelSymbol Label;
    BoundExpression *Condition;
    bool JumpIfTrue;
    BoundNodeKind kind = BoundNodeKind::ConditionalGotoStatement;
    BoundNodeKind GetKind() const override { return kind; }

    std::vector<std::pair<std::string, std::string>> GetProperties() const override
    {
        return {{"Label", Label.ToString()}, {"Condition", ""}};
    }

    std::vector<BoundNode *> GetChildren() const override
    {
        return {Condition};
    }
};

class BoundIfStatement : public BoundStatement
{
public:
    BoundIfStatement(BoundExpression *condition, BoundStatement *thenStatement, BoundStatement *elseStatement) : Condition(condition), ThenStatement(thenStatement), ElseStatement(elseStatement) {};
    BoundExpression *Condition;
    BoundStatement *ThenStatement;
    BoundStatement *ElseStatement;
    BoundNodeKind kind = BoundNodeKind::IfStatement;
    BoundNodeKind GetKind() const override { return kind; }

    std::vector<BoundNode *> GetChildren() const override
    {
        std::vector<BoundNode *> children = {Condition, ThenStatement};
        if (ElseStatement != nullptr)
        {
            children.push_back(ElseStatement);
        }
        return children;
    }

    std::vector<std::pair<std::string, std::string>> GetProperties() const override
    {
        return {{"Condition", ""}};
    }
};

class BoundWhileStatement : public BoundStatement
{
public:
    BoundWhileStatement(BoundExpression *condition, BoundStatement *body) : Condition(condition), Body(body) {};
    BoundExpression *Condition;
    BoundStatement *Body;
    BoundNodeKind kind = BoundNodeKind::WhileStatement;
    BoundNodeKind GetKind() const override { return kind; }

    std::vector<BoundNode *> GetChildren() const override
    {
        return {Condition, Body};
    }

    std::vector<std::pair<std::string, std::string>> GetProperties() const override
    {
        return {{"Condition", ""}};
    }
};

class BoundForStatement : public BoundStatement
{
public:
    BoundForStatement(VariableSymbol variable, BoundExpression *lowerBound, BoundExpression *upperBound, BoundStatement *body) : Variable(variable), LowerBound(lowerBound), UpperBound(upperBound), Body(body) {};
    VariableSymbol Variable;
    BoundExpression *LowerBound;
    BoundExpression *UpperBound;
    BoundStatement *Body;
    BoundNodeKind kind = BoundNodeKind::ForStatement;
    BoundNodeKind GetKind() const override { return kind; }

    std::vector<BoundNode *> GetChildren() const override
    {
        return {LowerBound, UpperBound, Body};
    }

    std::vector<std::pair<std::string, std::string>> GetProperties() const override
    {
        return {{"Variable", Variable.ToString()}};
    }
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

    std::vector<BoundNode *> GetChildren() const override
    {
        return {Operand};
    }

    std::vector<std::pair<std::string, std::string>> GetProperties() const override
    {
        return {{"Operator", ""}};
    }
};

class BoundLiteralExpression : public BoundExpression
{
public:
    BoundLiteralExpression(std::string value, Type type) : BoundExpression(type), Value(value) {};
    BoundNodeKind kind = BoundNodeKind::LiteralExpression;
    std::string Value;
    Type type;
    BoundNodeKind GetKind() const override { return BoundNodeKind::LiteralExpression; }

    std::vector<std::pair<std::string, std::string>> GetProperties() const override
    {
        return {{"Value", Value}};
    }

    std::vector<BoundNode *> GetChildren() const override
    {
        return {};
    }
};

class BoundVariableExpression : public BoundExpression
{
public:
    BoundVariableExpression(VariableSymbol variable) : BoundExpression(variable.type), Variable(variable) {};
    BoundNodeKind kind = BoundNodeKind::VariableExpression;
    VariableSymbol Variable;

    BoundNodeKind GetKind() const override { return BoundNodeKind::VariableExpression; }

    std::vector<std::pair<std::string, std::string>> GetProperties() const override
    {
        return {{"Variable", Variable.ToString()}};
    }

    std::vector<BoundNode *> GetChildren() const override
    {
        return {};
    }
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

    std::vector<std::pair<std::string, std::string>> GetProperties() const override
    {
        return {{"Variable", Variable.ToString()}};
    }

    std::vector<BoundNode *> GetChildren() const override
    {
        return {Expression};
    }
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

    std::vector<BoundNode *> GetChildren() const override
    {
        return {Left, Right};
    }

    std::vector<std::pair<std::string, std::string>> GetProperties() const override
    {
        return {{"Operator", ""}};
    }
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
    Binder(BoundScope *parent) : _scope(new BoundScope(parent)) {};

    const DiagnosticBag &GetDiagnostics() const
    {
        return _diagnostics;
    }
    static BoundGlobalScope BindGlobalScope(BoundGlobalScope *previous, CompilationUnitNode *tree);

private:
    DiagnosticBag _diagnostics;
    BoundScope *_scope;

    BoundExpression *BindExpression(SyntaxNode *node);
    BoundExpression *BindExpression(SyntaxNode *node, Type type);

    static BoundScope *CreateParentScope(BoundGlobalScope *previous);
    BoundStatement *BindStatement(StatementSyntax *node);
    BoundStatement *BindBlockStatement(BlockStatementSyntax *node);
    BoundStatement *BindVariableDeclaration(VariableDeclarationSyntax *node);
    BoundStatement *BindExpressionStatement(ExpressionStatementSyntax *node);
    BoundStatement *BindIfStatement(IfStatementSyntax *node);
    BoundStatement *BindWhileStatement(WhileStatementSyntax *node);
    BoundStatement *BindForStatement(ForStatementSyntax *node);

    BoundExpression *BindLiteralExpression(LiteralExpressionNode *node);
    BoundExpression *BindUnaryExpression(UnaryExpressionNode *node);
    BoundExpression *BindBinaryExpression(BinaryExpressionNode *node);
    BoundExpression *BindNameExpression(NameExpressionNode *node);
    BoundExpression *BindAssignmentExpression(AssignmentExpressionNode *node);
};

#endif