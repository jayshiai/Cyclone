#include "CodeGeneration/Emitter.h"
#include "Utils.h"
void Emitter::Emit()
{
    codeStream.open("temp" + Filename + ".cpp");
    EmitIncludes();
    EmitFunctions();

    codeStream << "int main()\n";
    codeStream << "{\n";
    EmitStatement(_program->statement);
    codeStream << "return 0;\n";
    codeStream << "}\n";
    codeStream.close();
}

void Emitter::EmitIncludes()
{
    codeStream << "#include <iostream>\n";
    codeStream << "#include <string>\n";
}

void Emitter::EmitFunctions()
{
    for (auto function : _program->Functions)
    {
        EmitType(function.first.Type);
        codeStream << " " << function.first.Name << "(";
        for (int i = 0; i < function.first.Parameters.size(); i++)
        {
            EmitType(function.first.Parameters[i].Type);
            codeStream << " " << function.first.Parameters[i].Name;

            if (function.first.Parameters[i].Type.IsArray())
            {
                codeStream << "[]";
            }
            if (i < function.first.Parameters.size() - 1)
            {
                codeStream << ", ";
            }
        }
        codeStream << ")\n";
        codeStream << "{\n";
        EmitStatement(function.second);
        codeStream << "}\n";
    }
}

void Emitter::EmitStatement(BoundBlockStatement *body)
{
    codeStream << "{\n";
    for (auto s : body->Statements)
    {
        switch (s->GetKind())
        {
        case BoundNodeKind::VariableDeclaration:
            EmitVariableDeclaration((BoundVariableDeclaration *)s);
            break;
        case BoundNodeKind::ExpressionStatement:
            EmitExpressionStatement((BoundExpressionStatement *)s);
            break;
        case BoundNodeKind::GotoStatement:
            EmitGotoStatement((BoundGotoStatement *)s);
            break;
        case BoundNodeKind::ConditionalGotoStatement:
            EmitConditionalGotoStatement((BoundConditionalGotoStatement *)s);
            break;
        case BoundNodeKind::LabelStatement:
            EmitLabelStatement((BoundLabelStatement *)s);
            break;
        case BoundNodeKind::ReturnStatement:
            EmitReturnStatement((BoundReturnStatement *)s);
            break;
        case BoundNodeKind::BlockStatement:
            EmitStatement((BoundBlockStatement *)s);
            break;
        default:
            throw std::runtime_error("Unexpected node kind: " + convertBoundNodeKindToString(s->GetKind()));
        }
    }
    codeStream << "}\n";
}

void Emitter::EmitVariableDeclaration(BoundVariableDeclaration *node)
{
    EmitType(node->Variable.Type);
    codeStream << " " << node->Variable.Name;

    if (node->Variable.Type.IsArray())
    {
        codeStream << "[";
        if (node->Variable.Size != -1)
        {
            codeStream << node->Variable.Size;
        }
        codeStream << "]";
    }
    if (node->Initializer != nullptr)
    {
        if (node->Initializer->GetKind() == BoundNodeKind::CallExpression)
        {
            BoundCallExpression *call = (BoundCallExpression *)node->Initializer;

            if (call->Function == BuiltInFunctions::Input)
            {
                codeStream << ";\n";
                codeStream << "std::cin >> " << node->Variable.Name;
            }
            else
            {
                codeStream << " = ";
                EmitCallExpression(call);
            }
        }
        else
        {
            codeStream << " = ";
            EmitExpression(node->Initializer);
        }
    }
    codeStream << ";\n";
}

void Emitter::EmitExpressionStatement(BoundExpressionStatement *node)
{
    EmitExpression(node->Expression);
    codeStream << ";\n";
}

void Emitter::EmitGotoStatement(BoundGotoStatement *node)
{
    codeStream << "goto " << node->Label.ToString() << ";\n";
}

void Emitter::EmitConditionalGotoStatement(BoundConditionalGotoStatement *node)
{
    if (node->JumpIfTrue)
    {
        codeStream << "if (";
        EmitExpression(node->Condition);
        codeStream << ")\n";
        codeStream << "{\n";
        codeStream << "goto " << node->Label.ToString() << ";\n";
        codeStream << "}\n";
    }
    else
    {
        codeStream << "if (!";
        EmitExpression(node->Condition);
        codeStream << ")\n";
        codeStream << "{\n";
        codeStream << "goto " << node->Label.ToString() << ";\n";
        codeStream << "}\n";
    }
}

void Emitter::EmitLabelStatement(BoundLabelStatement *node)
{
    codeStream << node->Label.ToString() << ":\n";
}

void Emitter::EmitReturnStatement(BoundReturnStatement *node)
{
    codeStream << "return ";
    if (node->Expression != nullptr)
    {
        EmitExpression(node->Expression);
    }
    codeStream << ";\n";
}

void Emitter::EmitExpression(BoundExpression *node)
{
    switch (node->GetKind())
    {
    case BoundNodeKind::LiteralExpression:
        EmitLiteralExpression((BoundLiteralExpression *)node);
        break;
    case BoundNodeKind::VariableExpression:
        codeStream << ((BoundVariableExpression *)node)->Variable.Name;
        break;
    case BoundNodeKind::AssignmentExpression:
        codeStream << ((BoundAssignmentExpression *)node)->Variable.Name << " = ";
        EmitExpression(((BoundAssignmentExpression *)node)->Expression);
        break;
    case BoundNodeKind::UnaryExpression:
        codeStream << "(";
        codeStream << ((BoundUnaryExpression *)node)->Op->ToString();
        EmitExpression(((BoundUnaryExpression *)node)->Operand);
        codeStream << ")";
        break;
    case BoundNodeKind::BinaryExpression:
        codeStream << "(";
        EmitExpression(((BoundBinaryExpression *)node)->Left);
        codeStream << " " << ((BoundBinaryExpression *)node)->Op->ToString() << " ";
        EmitExpression(((BoundBinaryExpression *)node)->Right);
        codeStream << ")";
        break;
    case BoundNodeKind::CallExpression:
        EmitCallExpression((BoundCallExpression *)node);
        break;
    case BoundNodeKind::ConversionExpression:
        EmitConversionExpression((BoundConversionExpression *)node);
        break;
    case BoundNodeKind::ArrayInitializerExpression:
        EmitArrayInitializerExpression((BoundArrayInitializerExpression *)node);
        break;
    case BoundNodeKind::ArrayAccessExpression:
        EmitArrayAccessExpression((BoundArrayAccessExpression *)node);
        break;
    case BoundNodeKind::ArrayAssignmentExpression:
        codeStream << ((BoundArrayAssignmentExpression *)node)->Variable.Name << "[";
        EmitExpression(((BoundArrayAssignmentExpression *)node)->Index);
        codeStream << "] = ";
        EmitExpression(((BoundArrayAssignmentExpression *)node)->Expression);
        break;
    case BoundNodeKind::BlockStatement:
        EmitStatement((BoundBlockStatement *)node);
        break;
    default:
        throw std::runtime_error("Unexpected node kind: " + convertBoundNodeKindToString(node->GetKind()));
    }
}

std::string Emitter::EscapeString(const std::string &str)
{
    std::ostringstream escaped;
    for (char c : str)
    {
        switch (c)
        {
        case '\n':
            escaped << "\\n";
            break;
        case '\t':
            escaped << "\\t";
            break;
        case '\\':
            escaped << "\\\\";
            break;
        case '\"':
            escaped << "\\\"";
            break;
        default:
            escaped << c;
            break;
        }
    }
    return escaped.str();
}

void Emitter::EmitLiteralExpression(BoundLiteralExpression *node)
{
    if (node->type == TypeSymbol::String)
    {
        codeStream << "\"" << EscapeString(node->Value) << "\"";
    }
    else if (node->type == TypeSymbol::Boolean)
    {
        codeStream << (node->Value == "true" ? "true" : "false");
    }
    else
    {
        codeStream << node->Value;
    }
}

void Emitter::EmitConversionExpression(BoundConversionExpression *node)
{
    if (node->type == TypeSymbol::Integer)
    {
        codeStream << "std::stoll(";
        EmitExpression(node->Expression);
        codeStream << ")";
    }
    else if (node->type == TypeSymbol::Float)
    {
        codeStream << "std::stod(";
        EmitExpression(node->Expression);
        codeStream << ")";
    }
    else if (node->type == TypeSymbol::Boolean)
    {
        codeStream << "std::stoi(";
        EmitExpression(node->Expression);
        codeStream << ")";
    }
    else if (node->type == TypeSymbol::String)
    {
        codeStream << "std::to_string(";
        EmitExpression(node->Expression);
        codeStream << ")";
    }
    else if (node->type == TypeSymbol::ArrayAny)
    {
        EmitExpression(node->Expression);
    }
    else
    {
        std::cout << "ASDsad";
        throw std::runtime_error("Unexpected type: " + node->type.Name);
    }
}
void Emitter::EmitCallExpression(BoundCallExpression *node)
{
    if (node->Function == BuiltInFunctions::Print)
    {
        codeStream << "std::cout << ";
        EmitExpression(node->Arguments[0]);
    }
    else if (node->Function == BuiltInFunctions::ArrayLength)
    {
        codeStream << "sizeof(";
        EmitExpression(node->Arguments[0]);
        codeStream << ") / sizeof(";
        EmitExpression(node->Arguments[0]);
        codeStream << "[0])";
    }
    else if (node->Function == BuiltInFunctions::StringLength)
    {
        EmitExpression(node->Arguments[0]);
        codeStream << ".size()";
    }
    else if (node->Function == BuiltInFunctions::Random)
    {
        codeStream << "rand() % ";
        EmitExpression(node->Arguments[0]);
    }
    else if (node->Function == BuiltInFunctions::Input)
    {
        codeStream << "";
    }
    else
    {
        codeStream << node->Function.Name << "(";
        for (int i = 0; i < node->Arguments.size(); i++)
        {
            EmitExpression(node->Arguments[i]);
            if (i < node->Arguments.size() - 1)
            {
                codeStream << ", ";
            }
        }
        codeStream << ")";
    }
}

void Emitter::EmitArrayInitializerExpression(BoundArrayInitializerExpression *node)
{
    codeStream << "{";
    for (int i = 0; i < node->Elements.size(); i++)
    {
        EmitExpression(node->Elements[i]);
        if (i < node->Elements.size() - 1)
        {
            codeStream << ", ";
        }
    }
    codeStream << "}";
}

void Emitter::EmitArrayAccessExpression(BoundArrayAccessExpression *node)
{
    BoundVariableExpression *variable = (BoundVariableExpression *)node->Variable;

    codeStream << variable->Variable.Name;
    codeStream << "[";

    EmitExpression(node->Index);
    codeStream << "]";
}
void Emitter::EmitType(TypeSymbol type)
{
    if (type == TypeSymbol::Integer)
    {
        codeStream << "long long";
    }
    else if (type == TypeSymbol::Float)
    {
        codeStream << "double";
    }
    else if (type == TypeSymbol::Boolean)
    {
        codeStream << "bool";
    }
    else if (type == TypeSymbol::String)
    {
        codeStream << "std::string";
    }
    else if (type == TypeSymbol::Any)
    {
        codeStream << "std::any";
    }
    else if (type == TypeSymbol::ArrayAny)
    {
        codeStream << "std::any";
    }
    else if (type == TypeSymbol::ArrayInt)
    {
        codeStream << "long long";
    }
    else if (type == TypeSymbol::ArrayFloat)
    {
        codeStream << "double";
    }
    else if (type == TypeSymbol::ArrayBool)
    {
        codeStream << "bool";
    }
    else if (type == TypeSymbol::ArrayString)
    {
        codeStream << "std::string";
    }
    else if (type == TypeSymbol::Void)
    {
        codeStream << "void";
    }
    else
    {
        throw std::runtime_error("Unexpected type: " + type.Name);
    }
}