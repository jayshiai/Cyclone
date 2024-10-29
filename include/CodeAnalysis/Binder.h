#ifndef BINDER_H
#define BINDER_H

#include "CodeAnalysis/SyntaxTree.h"
#include "CodeAnalysis/Symbol.h"
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

std::string convertBoundNodeKindToString(BoundNodeKind kind);

class BoundNode
{
public:
    virtual ~BoundNode() {}
    BoundNodeKind kind;
    virtual std::vector<BoundNode *> GetChildren() const { return {}; }
    virtual BoundNodeKind GetKind() const = 0;
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
    TypeSymbol type;
    BoundExpression(TypeSymbol type) : type(type) {}
    virtual ~BoundExpression() {}
};
class BoundStatement : public BoundNode
{
public:
    virtual ~BoundStatement() {}
};
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

class BoundLabel
{
public:
    std::string Name;
    BoundLabel(std::string name) : Name(name) {}
    bool operator==(const BoundLabel &other) const
    {
        return Name == other.Name;
    }
    std::string ToString() const
    {
        return Name;
    }

    friend std::ostream &operator<<(std::ostream &os, const BoundLabel &label)
    {
        os << "LabelSymbol(Name: " << label.Name << ")";
        return os;
    }
};
namespace std
{
    template <>
    struct hash<BoundLabel>
    {
        std::size_t operator()(const BoundLabel &symbol) const
        {
            return std::hash<std::string>()(symbol.Name);
        }
    };
}

class BoundLabelStatement : public BoundStatement
{
public:
    BoundLabelStatement(BoundLabel label) : Label(label) {};
    BoundLabel Label;
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
    BoundGotoStatement(BoundLabel label) : Label(label) {};
    BoundNodeKind kind = BoundNodeKind::GotoStatement;
    BoundLabel Label;
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
    BoundConditionalGotoStatement(BoundLabel label, BoundExpression *condition, bool jumpIfTrue = true) : Label(label), Condition(condition), JumpIfTrue(jumpIfTrue) {};
    BoundLabel Label;
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
    TypeSymbol OperandType;
    TypeSymbol ResultType;
    static BoundUnaryOperator *Bind(SyntaxKind syntaxKind, TypeSymbol operandType);
    std::string ToString() const;

private:
    BoundUnaryOperator(SyntaxKind syntaxKind, BoundUnaryOperatorKind kind, TypeSymbol operandType, TypeSymbol resultType) : syntaxKind(syntaxKind), Kind(kind), OperandType(operandType), ResultType(resultType) {};
    BoundUnaryOperator(SyntaxKind syntaxKind, BoundUnaryOperatorKind kind, TypeSymbol operandType) : BoundUnaryOperator(syntaxKind, kind, operandType, operandType) {};
    static const std::vector<BoundUnaryOperator> operators;
};

class BoundBinaryOperator
{
public:
    SyntaxKind syntaxKind;
    BoundBinaryOperatorKind Kind;
    TypeSymbol LeftType;
    TypeSymbol RightType;
    TypeSymbol ResultType;
    static BoundBinaryOperator *Bind(SyntaxKind syntaxKind, TypeSymbol leftType, TypeSymbol rightType);
    std::string ToString() const;

private:
    BoundBinaryOperator(SyntaxKind syntaxKind, BoundBinaryOperatorKind kind, TypeSymbol leftType, TypeSymbol rightTpye, TypeSymbol resultType) : syntaxKind(syntaxKind), Kind(kind), LeftType(leftType), RightType(rightTpye), ResultType(resultType) {};
    BoundBinaryOperator(SyntaxKind syntaxKind, BoundBinaryOperatorKind kind, TypeSymbol operandType, TypeSymbol resultType) : BoundBinaryOperator(syntaxKind, kind, operandType, operandType, resultType) {};
    BoundBinaryOperator(SyntaxKind syntaxKind, BoundBinaryOperatorKind kind, TypeSymbol type) : BoundBinaryOperator(syntaxKind, kind, type, type, type) {};
    static const std::vector<BoundBinaryOperator> operators;
};

class BoundUnaryExpression : public BoundExpression
{
public:
    BoundUnaryExpression(BoundUnaryOperator *op, BoundExpression *operand) : BoundExpression(op->ResultType), Op(op), Operand(operand), type(op->ResultType) {};
    BoundNodeKind kind = BoundNodeKind::UnaryExpression;
    BoundUnaryOperator *Op;
    BoundExpression *Operand;
    TypeSymbol type;
    BoundNodeKind GetKind() const override { return BoundNodeKind::UnaryExpression; }

    std::vector<BoundNode *> GetChildren() const override
    {
        return {Operand};
    }

    std::vector<std::pair<std::string, std::string>> GetProperties() const override
    {
        return {{"Operator", Op->ToString()}};
    }
};

class BoundLiteralExpression : public BoundExpression
{
public:
    BoundLiteralExpression(std::string value, TypeSymbol type) : BoundExpression(type), Value(value), type(type) {};
    BoundNodeKind kind = BoundNodeKind::LiteralExpression;
    std::string Value;
    TypeSymbol type;
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
    BoundVariableExpression(VariableSymbol variable) : BoundExpression(variable.Type), Variable(variable) {};
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
    BoundAssignmentExpression(VariableSymbol variable, BoundExpression *expression) : BoundExpression(expression->type), Variable(variable), Expression(expression), type(expression->type) {};
    BoundNodeKind kind = BoundNodeKind::AssignmentExpression;
    VariableSymbol Variable;
    BoundExpression *Expression;
    TypeSymbol type;

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
    BoundBinaryExpression(BoundExpression *left, BoundBinaryOperator *op, BoundExpression *right) : BoundExpression(op->ResultType), Left(left), Op(op), Right(right), type(op->ResultType) {};
    BoundNodeKind kind = BoundNodeKind::BinaryExpression;
    BoundBinaryOperator *Op;
    BoundExpression *Left;
    BoundExpression *Right;
    TypeSymbol type;
    BoundNodeKind GetKind() const override { return BoundNodeKind::BinaryExpression; }

    std::vector<BoundNode *> GetChildren() const override
    {
        return {Left, Right};
    }

    std::vector<std::pair<std::string, std::string>> GetProperties() const override
    {
        return {{"Operator", Op->ToString()}};
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
    BoundExpression *BindExpression(SyntaxNode *node, TypeSymbol type);

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