#include "CodeAnalysis/Binder.h"
#include "CodeAnalysis/Evaluator.h"
#include "CodeAnalysis/SyntaxTree.h"
#include "CodeAnalysis/Diagnostic.h"
#include "CodeAnalysis/Compilation.h"
#include "CodeAnalysis/Lowerer.h"
#include "CodeAnalysis/ControlFlowGraph.h"
#include "CodeAnalysis/Symbol.h"
#include <unordered_map>
#include <atomic>
#include <fstream>
#include <filesystem>
#include <algorithm>

void GenerateCFG(BoundProgram *program)
{
    std::string appDirectory = std::filesystem::current_path().string();

    std::string cfgPath = appDirectory + "/cfg.dot";
    BoundStatement *cfgStatement = nullptr;
    if (program->statement->Statements.empty() && !program->Functions.empty())
    {
        cfgStatement = program->Functions.begin()->second;
    }
    else
    {
        cfgStatement = program->statement;
    }

    ControlFlowGraph *cfg = ControlFlowGraph::Create(static_cast<BoundBlockStatement *>(cfgStatement));

    std::ofstream streamWriter(cfgPath);
    if (streamWriter.is_open())
    {
        cfg->WriteTo(streamWriter);
        streamWriter.close();
    }
    else
    {
        std::cerr << "Failed to open file for writing: " << cfgPath << std::endl;
    }
}
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

    BoundProgram *program = Binder::BindProgram(GlobalScope());
    GenerateCFG(program); // Generates Graph for Control Flow analysis
    if (program->Diagnostics.size() > 0)
    {
        diagnostics.insert(diagnostics.end(), program->Diagnostics.begin(), program->Diagnostics.end());
        return EvaluationResult(diagnostics, std::any());
    }

    Evaluator evaluator = Evaluator(program, variables);

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
    BoundProgram *program = Binder::BindProgram(GlobalScope());

    if (!program->statement->Statements.empty())
    {
        program->statement->WriteTo(os);
    }
    else
    {
        for (auto function : program->Functions)
        {
            if (std::find_if(GlobalScope()->Functions.begin(), GlobalScope()->Functions.end(),
                             [&](const FunctionSymbol &func)
                             { return func.Name == function.first.Name; }) == GlobalScope()->Functions.end())
            {
                continue;
            }
            FunctionSymbol functionSymbol = function.first;
            functionSymbol.WriteTo(os);
            function.second->WriteTo(os);
        }
    }
}

Compilation *Compilation::ContinueWith(SyntaxTree *syntaxTree)
{
    return new Compilation(this, syntaxTree);
}
