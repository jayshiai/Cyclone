#include "CodeAnalysis/Binder.h"
#include "CodeAnalysis/Evaluator.h"
#include "CodeAnalysis/SyntaxTree.h"
#include "CodeAnalysis/Diagnostic.h"
#include "CodeAnalysis/Compilation.h"
#include "CodeAnalysis/Lowerer.h"
#include <unordered_map>
#include <atomic>

std::string cConvertBoundNodeKind(BoundNodeKind kind)
{
    switch (kind)
    {
    case BoundNodeKind::LiteralExpression:
        return "LiteralExpression";
    case BoundNodeKind::UnaryExpression:
        return "UnaryExpression";
    case BoundNodeKind::BinaryExpression:
        return "BinaryExpression";
    case BoundNodeKind::ParenthesizedExpression:
        return "ParenthesizedExpression";
    case BoundNodeKind::VariableExpression:
        return "VariableExpression";
    case BoundNodeKind::AssignmentExpression:
        return "AssignmentExpression";
    case BoundNodeKind::ExpressionStatement:
        return "ExpressionStatement";
    case BoundNodeKind::VariableDeclaration:
        return "VariableDeclaration";
    case BoundNodeKind::BlockStatement:
        return "BlockStatement";
    default:
        return "Unknown";
    }
}
EvaluationResult Compilation::Evaluate(std::unordered_map<VariableSymbol, std::any> &variables)
{
    std::vector<Diagnostic> diagnostics = syntaxTree->Diagnostics;
    diagnostics.insert(diagnostics.end(), GlobalScope()->Diagnostics.begin(), GlobalScope()->Diagnostics.end());

    if (diagnostics.size() > 0)
    {
        return EvaluationResult(diagnostics, std::any());
    }
    // Evaluator evaluator = Evaluator(GlobalScope()->Statement, variables);
    BoundBlockStatement *statement = GetStatement();
    Evaluator evaluator = Evaluator(statement, variables);

    std::any value = evaluator.Evaluate();

    return EvaluationResult(diagnostics, value);
}

BoundGlobalScope *Compilation::GlobalScope()
{
    if (_globalScope == nullptr)
    {
        // _globalScope = new BoundGlobalScope(Binder::BindGlobalScope(Previous == nullptr ? nullptr : Previous->GlobalScope(), syntaxTree->Root));
        BoundGlobalScope *globalScope = new BoundGlobalScope(Binder::BindGlobalScope(Previous == nullptr ? nullptr : Previous->GlobalScope(), syntaxTree->Root));

        std::atomic<BoundGlobalScope *> &atomicGlobalScope = reinterpret_cast<std::atomic<BoundGlobalScope *> &>(_globalScope);

        BoundGlobalScope *expected = nullptr;
        atomicGlobalScope.compare_exchange_strong(expected, globalScope);

        _globalScope = atomicGlobalScope.load();
    }
    return _globalScope;
};

void Compilation::EmitTree(std::ostream &os)
{
    BoundBlockStatement *statement = GetStatement();
    statement->WriteTo(os);
}

BoundBlockStatement *Compilation::GetStatement()
{
    BoundStatement *statement = GlobalScope()->Statement;
    return Lowerer::Lower(statement);
}

Compilation *Compilation::ContinueWith(SyntaxTree *syntaxTree)
{
    return new Compilation(this, syntaxTree);
}