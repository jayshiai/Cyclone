#include "CodeAnalysis/Binder.h"
#include "CodeAnalysis/Parser.h"
void BoundNodePrinter::WriteTo(const BoundNode *node, std::ostream &os)
{

    if (IndentedTextWriter *iw = dynamic_cast<IndentedTextWriter *>(&os))
    {
        WriteTo(node, *iw);
    }
    else
    {
        IndentedTextWriter niw(os);
        WriteTo(node, niw);
    }
}
void BoundNodePrinter::WriteTo(const BoundNode *node, IndentedTextWriter &writer)
{
    switch (node->GetKind())
    {
    case BoundNodeKind::BlockStatement:
        WriteBlockStatement((BoundBlockStatement *)node, writer);
        break;
    case BoundNodeKind::VariableDeclaration:
        WriteVariableDeclaration((BoundVariableDeclaration *)node, writer);
        break;
    case BoundNodeKind::IfStatement:
        WriteIfStatement((BoundIfStatement *)node, writer);
        break;
    case BoundNodeKind::WhileStatement:
        WriteWhileStatement((BoundWhileStatement *)node, writer);
        break;
    case BoundNodeKind::ForStatement:
        WriteForStatement((BoundForStatement *)node, writer);
        break;
    case BoundNodeKind::LabelStatement:
        WriteLabelStatement((BoundLabelStatement *)node, writer);
        break;
    case BoundNodeKind::GotoStatement:
        WriteGotoStatement((BoundGotoStatement *)node, writer);
        break;
    case BoundNodeKind::ConditionalGotoStatement:
        WriteConditionalGotoStatement((BoundConditionalGotoStatement *)node, writer);
        break;
    case BoundNodeKind::ReturnStatement:
        WriteReturnStatement((BoundReturnStatement *)node, writer);
        break;
    case BoundNodeKind::ExpressionStatement:
        WriteExpressionStatement((BoundExpressionStatement *)node, writer);
        break;
    case BoundNodeKind::ErrorExpression:
        WriteErrorExpression((BoundErrorExpression *)node, writer);
        break;
    case BoundNodeKind::LiteralExpression:
        WriteLiteralExpression((BoundLiteralExpression *)node, writer);
        break;
    case BoundNodeKind::VariableExpression:
        WriteVariableExpression((BoundVariableExpression *)node, writer);
        break;
    case BoundNodeKind::AssignmentExpression:
        WriteAssignmentExpression((BoundAssignmentExpression *)node, writer);
        break;
    case BoundNodeKind::UnaryExpression:
        WriteUnaryExpression((BoundUnaryExpression *)node, writer);
        break;
    case BoundNodeKind::BinaryExpression:
        WriteBinaryExpression((BoundBinaryExpression *)node, writer);
        break;
    case BoundNodeKind::CallExpression:
        WriteCallExpression((BoundCallExpression *)node, writer);
        break;
    case BoundNodeKind::ConversionExpression:
        WriteConversionExpression((BoundConversionExpression *)node, writer);
        break;
    case BoundNodeKind::ArrayInitializerExpression:
        WriteArrayInitializerExpression((BoundArrayInitializerExpression *)node, writer);
        break;
    case BoundNodeKind::ArrayAccessExpression:
        WriteArrayAccessExpression((BoundArrayAccessExpression *)node, writer);
        break;
    default:
        throw std::runtime_error("Unexpected node: " + convertBoundNodeKindToString(node->GetKind()));
    }
}

void BoundNodePrinter::WriteNestedStatement(const BoundStatement *node, IndentedTextWriter &writer)
{
    if (node->GetKind() != BoundNodeKind::BlockStatement)
    {
        writer.Indent();
        WriteTo(node, writer);
        writer.Unindent();
    }
    else
    {
        WriteTo(node, writer);
    }
}

void BoundNodePrinter::WriteNestedExpression(const BoundExpression *node, int parentPrecedence, IndentedTextWriter &writer)
{
    if (node->GetKind() == BoundNodeKind::UnaryExpression)
    {
        WriteNestedExpression(node, parentPrecedence, Parser::GetUnaryPrecedence(((BoundUnaryExpression *)node)->Op->syntaxKind), writer);
    }
    else if (node->GetKind() == BoundNodeKind::BinaryExpression)
    {
        WriteNestedExpression(node, parentPrecedence, Parser::GetBinaryPrecedence(((BoundBinaryExpression *)node)->Op->syntaxKind), writer);
    }
    else
    {
        WriteTo(node, writer);
    }
}

void BoundNodePrinter::WriteNestedExpression(const BoundExpression *node, int parentPrecedence, int currentPrecedence, IndentedTextWriter &writer)
{
    if (parentPrecedence >= currentPrecedence)
    {
        writer.WritePunctuation("(");
        WriteTo(node, writer);
        writer.WritePunctuation(")");
    }
    else
    {
        WriteTo(node, writer);
    }
}

void BoundNodePrinter::WriteBlockStatement(const BoundBlockStatement *node, IndentedTextWriter &writer)
{
    writer.WritePunctuation("{");
    writer.WriteLine();
    writer.Indent();
    for (const auto &statement : node->Statements)
    {
        WriteNestedStatement(statement, writer);
    }
    writer.Unindent();
    writer.WriteLine();
    writer.WritePunctuation("}");
}

void BoundNodePrinter::WriteVariableDeclaration(const BoundVariableDeclaration *node, IndentedTextWriter &writer)
{
    writer.WriteKeyword(node->Variable.IsReadOnly ? "let" : "var");
    writer.Write(" ");
    writer.WriteIdentifier(node->Variable.ToString());
    if (node->Initializer != nullptr)
    {
        writer.Write(" = ");
        node->Initializer->WriteTo(writer);
    }
    writer.WriteLine();
}

void BoundNodePrinter::WriteIfStatement(const BoundIfStatement *node, IndentedTextWriter &writer)
{
    writer.WriteKeyword("if");
    writer.Write(" (");
    node->Condition->WriteTo(writer);
    writer.WritePunctuation(")");
    writer.WriteLine();
    WriteNestedStatement(node->ThenStatement, writer);
    if (node->ElseStatement != nullptr)
    {
        writer.WriteKeyword("else");
        writer.WriteLine();
        WriteNestedStatement(node->ElseStatement, writer);
    }
}

void BoundNodePrinter::WriteWhileStatement(const BoundWhileStatement *node, IndentedTextWriter &writer)
{
    writer.WriteKeyword("while");
    writer.Write(" (");
    node->Condition->WriteTo(writer);
    writer.WritePunctuation(")");
    writer.WriteLine();
    WriteNestedStatement(node->Body, writer);
}

void BoundNodePrinter::WriteForStatement(const BoundForStatement *node, IndentedTextWriter &writer)
{
    writer.WriteKeyword("for");
    writer.Write(" (");
    writer.WriteIdentifier(node->Variable.ToString());
    writer.WritePunctuation(" = ");
    node->LowerBound->WriteTo(writer);
    writer.WritePunctuation(" to ");
    node->UpperBound->WriteTo(writer);
    writer.WritePunctuation(")");
    writer.WriteLine();
    WriteNestedStatement(node->Body, writer);
}

void BoundNodePrinter::WriteLabelStatement(const BoundLabelStatement *node, IndentedTextWriter &writer)
{
    bool unindent = writer.GetIndentLevel() > 0;

    if (unindent)
    {
        writer.Unindent();
    }

    writer.WritePunctuation(node->Label.ToString());
    writer.WritePunctuation(":");
    writer.WriteLine();

    if (unindent)
    {
        writer.Indent();
    }
}

void BoundNodePrinter::WriteGotoStatement(const BoundGotoStatement *node, IndentedTextWriter &writer)
{
    writer.WriteKeyword("goto");
    writer.Write(" ");
    writer.WriteIdentifier(node->Label.ToString());
    writer.WriteLine();
}

void BoundNodePrinter::WriteReturnStatement(const BoundReturnStatement *node, IndentedTextWriter &writer)
{
    writer.WriteKeyword("return");
    if (node->Expression != nullptr)
    {
        writer.Write(" ");
        node->Expression->WriteTo(writer);
    }
    writer.WriteLine();
}

void BoundNodePrinter::WriteConditionalGotoStatement(const BoundConditionalGotoStatement *node, IndentedTextWriter &writer)
{
    writer.WriteKeyword("goto");
    writer.Write(" ");
    writer.WriteIdentifier(node->Label.ToString());
    writer.Write(node->JumpIfTrue ? " if " : " unless ");
    writer.Write(" (");
    node->Condition->WriteTo(writer);
    writer.WritePunctuation(")");
    writer.WriteLine();
}

void BoundNodePrinter::WriteExpressionStatement(const BoundExpressionStatement *node, IndentedTextWriter &writer)
{
    node->Expression->WriteTo(writer);
    writer.WriteLine();
}

void BoundNodePrinter::WriteErrorExpression(const BoundErrorExpression *node, IndentedTextWriter &writer)
{
    writer.WriteKeyword("?");
}

void BoundNodePrinter::WriteLiteralExpression(const BoundLiteralExpression *node, IndentedTextWriter &writer)
{
    std::string value = node->Value;

    if (node->type == TypeSymbol::Boolean)
    {
        writer.WriteKeyword(value);
    }
    else if (node->type == TypeSymbol::Integer)
    {
        writer.WriteNumber(value);
    }
    else if (node->type == TypeSymbol::String)
    {
        writer.WriteString(value);
    }
    else if (node->type == TypeSymbol::Any)
    {
        writer.WriteIdentifier(value);
    }
    else
    {
        throw std::runtime_error("Unexpected type: " + node->type.ToString());
    }
}

void BoundNodePrinter::WriteVariableExpression(const BoundVariableExpression *node, IndentedTextWriter &writer)
{
    writer.WriteIdentifier(node->Variable.ToString());
}

void BoundNodePrinter::WriteAssignmentExpression(const BoundAssignmentExpression *node, IndentedTextWriter &writer)
{
    writer.WriteIdentifier(node->Variable.ToString());
    writer.WritePunctuation(" = ");
    node->Expression->WriteTo(writer);
}

void BoundNodePrinter::WriteUnaryExpression(const BoundUnaryExpression *node, IndentedTextWriter &writer)
{
    writer.WritePunctuation(node->Op->ToString());
    WriteNestedExpression(node->Operand, Parser::GetUnaryPrecedence(node->Op->syntaxKind), writer);
}

void BoundNodePrinter::WriteBinaryExpression(const BoundBinaryExpression *node, IndentedTextWriter &writer)
{
    WriteNestedExpression(node->Left, Parser::GetBinaryPrecedence(node->Op->syntaxKind), writer);
    writer.Write(" ");
    writer.WritePunctuation(node->Op->ToString());
    writer.Write(" ");
    WriteNestedExpression(node->Right, Parser::GetBinaryPrecedence(node->Op->syntaxKind), writer);
}

void BoundNodePrinter::WriteCallExpression(const BoundCallExpression *node, IndentedTextWriter &writer)
{
    writer.WriteIdentifier(node->Function.ToString());
    writer.WritePunctuation("(");
    bool first = true;
    for (const auto &argument : node->Arguments)
    {
        if (!first)
        {
            writer.WritePunctuation(", ");
        }
        argument->WriteTo(writer);
        first = false;
    }
    writer.WritePunctuation(")");
}

void BoundNodePrinter::WriteConversionExpression(const BoundConversionExpression *node, IndentedTextWriter &writer)
{
    writer.WritePunctuation("(");
    writer.WriteIdentifier(node->type.ToString());
    writer.WritePunctuation(") ");
    node->Expression->WriteTo(writer);
}

void BoundNodePrinter::WriteArrayInitializerExpression(const BoundArrayInitializerExpression *node, IndentedTextWriter &writer)
{
    writer.WritePunctuation("[");
    bool first = true;
    for (const auto &expression : node->Elements)
    {
        if (!first)
        {
            writer.WritePunctuation(", ");
        }
        expression->WriteTo(writer);
        first = false;
    }
    writer.WritePunctuation("]");
}

void BoundNodePrinter::WriteArrayAccessExpression(const BoundArrayAccessExpression *node, IndentedTextWriter &writer)
{
    node->Variable->WriteTo(writer);
    writer.WritePunctuation("[");
    node->Index->WriteTo(writer);
    writer.WritePunctuation("]");
}