#include "CodeGeneration/Assembler.h"
#include "Utils.h"

void Assembler::Assemble()
{
    dataStream.open("tempData" + Filename + ".asm");
    codeStream.open("tempCode" + Filename + ".asm");

    dataStream << ".data\n";

    AssembleFunctions();

    _currentVariableOffset = 0;
    _variablesMap.clear();
    codeStream << ".global _main\n";
    codeStream << "_main:\n";
    AssembleStatement(_program->statement);
    codeStream << "mov rax, 0\n";
    codeStream << "ret\n";
    codeStream.close();
}

void Assembler::AssembleStatement(BoundBlockStatement *node)
{
    for (auto s : node->Statements)
    {
        switch (s->GetKind())
        {
        case BoundNodeKind::VariableDeclaration:
            AssembleVariableDeclaration((BoundVariableDeclaration *)s);
            break;
        case BoundNodeKind::ExpressionStatement:
            AssembleExpressionStatement((BoundExpressionStatement *)s);
            break;
        case BoundNodeKind::GotoStatement:
            AssembleGotoStatement((BoundGotoStatement *)s);
            break;
        case BoundNodeKind::ConditionalGotoStatement:
            AssembleConditionalGotoStatement((BoundConditionalGotoStatement *)s);
            break;
        case BoundNodeKind::LabelStatement:
            AssembleLabelStatement((BoundLabelStatement *)s);
            break;
        case BoundNodeKind::ReturnStatement:
            AssembleReturnStatement((BoundReturnStatement *)s);
            break;
        case BoundNodeKind::BlockStatement:
            AssembleStatement((BoundBlockStatement *)s);
            break;
        default:
            throw std::runtime_error("Unexpected node: ");
        }
    }
}

void Assembler::AssembleVariableDeclaration(BoundVariableDeclaration *node)
{

    if (node->Variable.Type.IsArray())
    {
        AssembleArrayDeclaration(node);
    }
    else
    {
        if (node->Initializer != nullptr)
        {
            AssembleExpression(node->Initializer);
        }
        else
        {
            codeStream << "mov rax, 0\n";
        }

        codeStream << "mov [rbp - " << _currentVariableOffset << "], rax\n";

        _variablesMap[node->Variable] = _currentVariableOffset;
        _currentVariableOffset += 8;
    }
}

void Assembler::AssembleArrayDeclaration(BoundVariableDeclaration *node)
{
    if (node->Initializer != nullptr)
    {
        AssembleArrayInitializerExpression((BoundArrayInitializerExpression *)node->Initializer);
    }
    else
    {
        codeStream << "times " << node->Variable.Size << " dq 0\n";
    }

    _variablesMap[node->Variable] = _currentVariableOffset;
    _currentVariableOffset += 8 * node->Variable.Size;
}

void Assembler::AssembleArrayInitializerExpression(BoundArrayInitializerExpression *node)
{
    int baseOffset = _currentVariableOffset;

    for (int i = 0; i < node->Elements.size(); i++)
    {
        AssembleExpression(node->Elements[i]);
        codeStream << "mov [rbp - " << (baseOffset + i * 8) << "], rax\n";
    }
}

void Assembler::AssembleExpressionStatement(BoundExpressionStatement *node)
{
    AssembleExpression(node->Expression);
}

void Assembler::AssembleBinaryExpression(BoundBinaryExpression *node)
{
    AssembleExpression(node->Left);
    codeStream << "push rax\n";
    AssembleExpression(node->Right);
    codeStream << "pop rcx\n";

    switch (node->Op->Kind)
    {
    case BoundBinaryOperatorKind::Addition:
        codeStream << "add rax, rcx\n";
        break;
    case BoundBinaryOperatorKind::Subtraction:
        codeStream << "sub rax, rcx\n";
        break;
    case BoundBinaryOperatorKind::Multiplication:
        codeStream << "imul rax, rcx\n";
        break;
    case BoundBinaryOperatorKind::Division:
        codeStream << "cqo\n";
        codeStream << "idiv rcx\n";
        break;
    case BoundBinaryOperatorKind::LogicalAnd:
        codeStream << "and rax, rcx\n";
        break;
    case BoundBinaryOperatorKind::LogicalOr:
        codeStream << "or rax, rcx\n";
        break;
    case BoundBinaryOperatorKind::Equals:
        codeStream << "cmp rax, rcx\n";
        codeStream << "sete al\n";
        codeStream << "movzb rax, al\n";
        break;
    case BoundBinaryOperatorKind::NotEquals:
        codeStream << "cmp rax, rcx\n";
        codeStream << "setne al\n";
        codeStream << "movzb rax, al\n";
        break;
    case BoundBinaryOperatorKind::Less:
        codeStream << "cmp rax, rcx\n";
        codeStream << "setl al\n";
        codeStream << "movzb rax, al\n";
        break;
    case BoundBinaryOperatorKind::LessOrEquals:
        codeStream << "cmp rax, rcx\n";
        codeStream << "setle al\n";
        codeStream << "movzb rax, al\n";
        break;
    case BoundBinaryOperatorKind::Greater:
        codeStream << "cmp rax, rcx\n";
        codeStream << "setg al\n";
        codeStream << "movzb rax, al\n";
        break;
    case BoundBinaryOperatorKind::GreaterOrEquals:
        codeStream << "cmp rax, rcx\n";
        codeStream << "setge al\n";
        codeStream << "movzb rax, al\n";
        break;
    default:
        throw std::runtime_error("Unexpected binary operator: ");
    }
}
void Assembler::AssembleGotoStatement(BoundGotoStatement *node)
{
    codeStream << "jmp _" << node->Label.Name << "\n";
}
void Assembler::AssembleConditionalGotoStatement(BoundConditionalGotoStatement *node)
{
    AssembleExpression(node->Condition);
    codeStream << "cmp rax, 0\n";
    codeStream << "je _" << node->Label.Name << "\n";
}

void Assembler::AssembleLabelStatement(BoundLabelStatement *node)
{
    codeStream << "_" << node->Label.Name << ":\n";
}

void Assembler::AssembleReturnStatement(BoundReturnStatement *node)
{
    AssembleExpression(node->Expression);
}

void Assembler::AssembleExpression(BoundExpression *node)
{
    switch (node->GetKind())
    {
    case BoundNodeKind::LiteralExpression:
        AssembleLiteralExpression((BoundLiteralExpression *)node);
        break;
    case BoundNodeKind::VariableExpression:
        AssembleVariableExpression((BoundVariableExpression *)node);
        break;
    case BoundNodeKind::AssignmentExpression:
        AssembleAssignmentExpression((BoundAssignmentExpression *)node);
        break;
    case BoundNodeKind::UnaryExpression:
        AssembleUnaryExpression((BoundUnaryExpression *)node);
        break;
    case BoundNodeKind::BinaryExpression:
        AssembleBinaryExpression((BoundBinaryExpression *)node);
        break;
    case BoundNodeKind::ConversionExpression:
        AssembleConversionExpression((BoundConversionExpression *)node);
        break;
    case BoundNodeKind::CallExpression:
        AssembleCallExpression((BoundCallExpression *)node);
        break;
    case BoundNodeKind::ArrayInitializerExpression:
        AssembleArrayInitializerExpression((BoundArrayInitializerExpression *)node);
        break;
    case BoundNodeKind::ArrayAccessExpression:
        AssembleArrayAccessExpression((BoundArrayAccessExpression *)node);
        break;
    case BoundNodeKind::ArrayAssignmentExpression:
        AssembleArrayAssignmentExpression((BoundArrayAssignmentExpression *)node);
        break;
    case BoundNodeKind::BlockStatement:
        AssembleStatement((BoundBlockStatement *)node);
        break;
    case BoundNodeKind::ErrorExpression:
        throw std::runtime_error("Error expression");
    default:
        throw std::runtime_error("Unexpected node: ");
    }
}

void Assembler::AssembleLiteralExpression(BoundLiteralExpression *node)
{
    if (node->type == TypeSymbol::String)
    {
        codeStream << "mov rax, " << node->Value << "\n";
    }
    else if (node->type == TypeSymbol::Boolean)
    {
        codeStream << "mov rax, " << (node->Value == "true" ? "1" : "0") << "\n";
    }
    else
    {
        codeStream << "mov rax, " << node->Value << "\n";
    }
}

void Assembler::AssembleVariableExpression(BoundVariableExpression *node)
{
    if (_variablesMap.find(node->Variable) == _variablesMap.end())
    {
        throw std::runtime_error("Variable not found: " + node->Variable.Name);
    }

    int offset = std::any_cast<int>(_variablesMap[node->Variable]);
    codeStream << "mov rax, [rbp - " << offset << "]\n";
}

void Assembler::AssembleAssignmentExpression(BoundAssignmentExpression *node)
{
    if (_variablesMap.find(node->Variable) == _variablesMap.end())
    {
        throw std::runtime_error("Variable not found: " + node->Variable.Name);
    }

    AssembleExpression(node->Expression);
    int offset = std::any_cast<int>(_variablesMap[node->Variable]);
    codeStream << "mov [rbp - " << offset << "], rax\n";
}

void Assembler::AssembleArrayAccessExpression(BoundArrayAccessExpression *node)
{
    AssembleExpression(node->Variable);
    codeStream << "mov rcx, rax\n";
    AssembleExpression(node->Index);
    codeStream << "imul rax, 8\n";
    codeStream << "add rax, rcx\n";
    codeStream << "mov rax, [rax]\n";
}

void Assembler::AssembleUnaryExpression(BoundUnaryExpression *node)
{
    AssembleExpression(node->Operand);

    switch (node->Op->Kind)
    {
    case BoundUnaryOperatorKind::Negation:
        codeStream << "neg rax\n";
        break;
    case BoundUnaryOperatorKind::LogicalNegation:
        codeStream << "cmp rax, 0\n";
        codeStream << "sete al\n";
        codeStream << "movzb rax, al\n";
        break;
    case BoundUnaryOperatorKind::Identity:
        break;
    case BoundUnaryOperatorKind::OnesComplement:
        codeStream << "not rax\n";
        break;
    default:
        throw std::runtime_error("Unexpected unary operator: ");
    }
}

void Assembler::AssembleConversionExpression(BoundConversionExpression *node)
{
    AssembleExpression(node->Expression);
}
void Assembler::AssembleCallExpression(BoundCallExpression *node)
{
    int paramCount = node->Arguments.size();

    // Pass the first six parameters in registers as per the System V convention
    if (paramCount > 0)
        AssembleExpression(node->Arguments[0]), codeStream << "mov rdi, rax\n";
    if (paramCount > 1)
        AssembleExpression(node->Arguments[1]), codeStream << "mov rsi, rax\n";
    if (paramCount > 2)
        AssembleExpression(node->Arguments[2]), codeStream << "mov rdx, rax\n";
    if (paramCount > 3)
        AssembleExpression(node->Arguments[3]), codeStream << "mov rcx, rax\n";
    if (paramCount > 4)
        AssembleExpression(node->Arguments[4]), codeStream << "mov r8, rax\n";
    if (paramCount > 5)
        AssembleExpression(node->Arguments[5]), codeStream << "mov r9, rax\n";

    // Push remaining parameters onto the stack in reverse order
    for (int i = paramCount - 1; i >= 6; i--)
    {
        AssembleExpression(node->Arguments[i]);
        codeStream << "push rax\n";
    }

    // Call the function
    codeStream << "call _" << node->Function.Name << "\n";

    // Clean up the stack for parameters beyond the sixth
    if (paramCount > 6)
        codeStream << "add rsp, " << (paramCount - 6) * 8 << "\n";
}

void Assembler::AssembleArrayAssignmentExpression(BoundArrayAssignmentExpression *node)
{
    AssembleExpression(node->Identifier);
    codeStream << "mov rcx, rax\n";
    AssembleExpression(node->Index);
    codeStream << "imul rax, 8\n";
    codeStream << "add rax, rcx\n";
    AssembleExpression(node->Expression);
    codeStream << "mov [rax], rax\n";
}
void Assembler::AssembleFunctions()
{
    for (auto f : _program->Functions)
    {
        codeStream << "_" << f.first.Name << ":\n";
        codeStream << "push rbp\n"; // Set up stack frame
        codeStream << "mov rbp, rsp\n";

        _currentVariableOffset = 0; // Offset for local variables
        _variablesMap.clear();

        // Save the first six parameters in local stack space (if there are any)
        int numParams = f.first.Parameters.size();

        if (numParams > 0)
            codeStream << "mov [rbp - " << _currentVariableOffset << "], rdi\n", _variablesMap[f.first.Parameters[0]] = _currentVariableOffset, _currentVariableOffset += 8;
        if (numParams > 1)
            codeStream << "mov [rbp - " << _currentVariableOffset << "], rsi\n", _variablesMap[f.first.Parameters[1]] = _currentVariableOffset, _currentVariableOffset += 8;
        if (numParams > 2)
            codeStream << "mov [rbp - " << _currentVariableOffset << "], rdx\n", _variablesMap[f.first.Parameters[2]] = _currentVariableOffset, _currentVariableOffset += 8;
        if (numParams > 3)
            codeStream << "mov [rbp - " << _currentVariableOffset << "], rcx\n", _variablesMap[f.first.Parameters[3]] = _currentVariableOffset, _currentVariableOffset += 8;
        if (numParams > 4)
            codeStream << "mov [rbp - " << _currentVariableOffset << "], r8\n", _variablesMap[f.first.Parameters[4]] = _currentVariableOffset, _currentVariableOffset += 8;
        if (numParams > 5)
            codeStream << "mov [rbp - " << _currentVariableOffset << "], r9\n", _variablesMap[f.first.Parameters[5]] = _currentVariableOffset, _currentVariableOffset += 8;

        // For parameters beyond the sixth, use their positions relative to `rbp`
        for (int i = 6; i < numParams; i++)
        {
            int paramOffset = 16 + (i - 6) * 8;                 // Offset of the i-th parameter (relative to rbp)
            _variablesMap[f.first.Parameters[i]] = paramOffset; // Store the offset in the map
        }

        // Assemble the function body
        AssembleStatement(f.second);

        // Restore stack and return
        codeStream << "pop rbp\n";
        codeStream << "ret\n";
    }
}
