#include "CodeAnalysis/Binder.h"
#include "CodeAnalysis/Evaluator.h"
#include "CodeAnalysis/SyntaxTree.h"
#include "CodeAnalysis/Diagnostic.h"
#include "CodeAnalysis/Compilation.h"
#include "CodeAnalysis/Lowerer.h"
#include "CodeAnalysis/ControlFlowGraph.h"
#include "CodeAnalysis/Symbol.h"
#include "CodeGeneration/Emitter.h"
#include "CodeGeneration/Assembler.h"
#include <unordered_map>
#include <atomic>
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;
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

    std::vector<Diagnostic> diagnostics;

    for (const auto &st : syntaxTrees)
    {
        diagnostics.insert(diagnostics.end(), st->Diagnostics.begin(), st->Diagnostics.end());
    }
    if (diagnostics.size() > 0)
    {
        return EvaluationResult(diagnostics, std::any());
    }
    BoundGlobalScope *globalScope = GlobalScope();
    diagnostics.insert(diagnostics.end(), globalScope->Diagnostics.begin(), globalScope->Diagnostics.end());

    if (diagnostics.size() > 0)
    {
        return EvaluationResult(diagnostics, std::any());
    }

    BoundProgram *program = Binder::BindProgram(globalScope);

    // GenerateCFG(program); // Generates Graph for    Control Flow analysis
    if (program->Diagnostics.size() > 0)
    {
        diagnostics.insert(diagnostics.end(), program->Diagnostics.begin(), program->Diagnostics.end());
        return EvaluationResult(diagnostics, std::any());
    }

    // Assembler assembler = Assembler("jay", program, variables);

    // assembler.Assemble();

    Evaluator evaluator = Evaluator(program, variables);

    std::any value = evaluator.Evaluate();

    return EvaluationResult(diagnostics, value);
}

EvaluationResult Compilation::Compile(std::unordered_map<VariableSymbol, std::any> &variables, std::string outputFileName)
{

    std::vector<Diagnostic> diagnostics;

    for (const auto &st : syntaxTrees)
    {
        diagnostics.insert(diagnostics.end(), st->Diagnostics.begin(), st->Diagnostics.end());
    }
    if (diagnostics.size() > 0)
    {
        return EvaluationResult(diagnostics, 1);
    }
    BoundGlobalScope *globalScope = GlobalScope();
    diagnostics.insert(diagnostics.end(), globalScope->Diagnostics.begin(), globalScope->Diagnostics.end());

    if (diagnostics.size() > 0)
    {
        return EvaluationResult(diagnostics, 1);
    }

    BoundProgram *program = Binder::BindEmitableProgram(globalScope);

    // BoundBlockStatement *statement = program->statement;
    // BoundBlockStatement *flatStatement = Lowerer::Flatten(statement);
    // program->statement = flatStatement;
    // GenerateCFG(program); // Generates Graph for Control Flow analysis
    if (program->Diagnostics.size() > 0)
    {
        diagnostics.insert(diagnostics.end(), program->Diagnostics.begin(), program->Diagnostics.end());
        return EvaluationResult(diagnostics, 1);
    }

    Emitter emitter = Emitter(outputFileName, program, variables);
    emitter.Emit();

    // Evaluator evaluator = Evaluator(program, variables);

    // std::any value = evaluator.Evaluate();
    // At the end of the program, compile the temp.cpp file
    std::string command = "g++  temp" + outputFileName + ".cpp -o " + outputFileName;

    int ret = system(command.c_str());
    if (ret != 0)
    {
        std::cerr << "Compilation failed with error code " << ret << std::endl;
        return EvaluationResult(diagnostics, 1);
    }

    std::cout << "Compilation successful. Executable created as: " + outputFileName << std::endl;
    fs::remove("temp" + outputFileName + ".cpp");
    return EvaluationResult(diagnostics, 0);
}

BoundGlobalScope *Compilation::GlobalScope()
{
    if (_globalScope == nullptr)
    {
        BoundGlobalScope *previous = nullptr;
        if (Previous != nullptr)
        {
            previous = Previous->GlobalScope();
        }
        auto trees = syntaxTrees;
        _globalScope = Binder::BindGlobalScope(previous, trees);
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
void Compilation::EmitBoundTree(std::ostream &os)
{
    BoundProgram *program = Binder::BindProgram(GlobalScope());

    if (!program->statement->Statements.empty())
    {
        program->statement->PrintTo(os);
    }
}
Compilation *Compilation::ContinueWith(SyntaxTree *syntaxTree)
{
    return new Compilation(this, {syntaxTree});
}
