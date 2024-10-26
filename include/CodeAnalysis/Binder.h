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
    AssignmentExpression
};

enum class Type
{
    Boolean,
    Integer
};

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
};

class BoundLiteralExpression : public BoundExpression
{
public:
    BoundLiteralExpression(std::string value, Type type) : BoundExpression(type), Value(value) {};
    BoundNodeKind kind = BoundNodeKind::LiteralExpression;
    std::string Value;
    Type type;
};

class VariableSymbol
{
public:
    std::string Name;
    Type type;
    VariableSymbol(std::string name, Type type) : Name(name), type(type) {}

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
class BoundVariableExpression : public BoundExpression
{
public:
    BoundVariableExpression(VariableSymbol variable) : BoundExpression(variable.type), Variable(variable) {};
    BoundNodeKind kind = BoundNodeKind::VariableExpression;
    VariableSymbol Variable;
};

class BoundAssignmentExpression : public BoundExpression
{
public:
    BoundAssignmentExpression(VariableSymbol variable, BoundExpression *expression) : BoundExpression(expression->type), Variable(variable), Expression(expression) {};
    BoundNodeKind kind = BoundNodeKind::AssignmentExpression;
    VariableSymbol Variable;
    BoundExpression *Expression;
    Type type;
};

class BoundBinaryExpression : public BoundExpression
{
public:
    BoundBinaryExpression(BoundExpression *left, BoundBinaryOperator *op, BoundExpression *right) : BoundExpression(op->ResultType), Left(left), Op(op), Right(right) {
                                                                                                    };
    BoundNodeKind kind = BoundNodeKind::BinaryExpression;
    BoundBinaryOperator *Op;
    BoundExpression *Left;
    BoundExpression *Right;
    Type type;
};
class Binder
{
public:
    Binder(std::unordered_map<VariableSymbol, std::any> &variables) : _variables(variables) {};
    std::unordered_map<VariableSymbol, std::any> _variables;
    BoundExpression *BindExpression(SyntaxNode *node);
    const DiagnosticBag &GetDiagnostics() const
    {
        return _diagnostics;
    }

private:
    DiagnosticBag _diagnostics;
    BoundExpression *BindLiteralExpression(LiteralExpressionNode *node);
    BoundExpression *BindUnaryExpression(UnaryExpressionNode *node);
    BoundExpression *BindBinaryExpression(BinaryExpressionNode *node);
    BoundExpression *BindNameExpression(NameExpressionNode *node);
    BoundExpression *BindAssignmentExpression(AssignmentExpressionNode *node);
};

#endif