#ifndef CONTROL_FLOW_GRAPH_H
#define CONTROL_FLOW_GRAPH_H
#include "CodeAnalysis/Binder.h"
#include <vector>
#include <string>
#include <ostream>
#include <unordered_map>

class ControlFlowGraph
{
public:
    class BasicBlock;
    class BasicBlockBranch;
    class BasicBlockBuilder;
    class GraphBuilder;

    BasicBlock *Start;
    BasicBlock *End;
    std::vector<BasicBlock *> Blocks;
    std::vector<BasicBlockBranch *> Branches;

    static ControlFlowGraph *Create(BoundBlockStatement *body);
    static bool AllPathsReturn(BoundBlockStatement *body);
    void WriteTo(std::ostream &writer) const;

private:
    ControlFlowGraph(BasicBlock *start, BasicBlock *end,
                     std::vector<BasicBlock *> blocks,
                     std::vector<BasicBlockBranch *> branches)
        : Start(start), End(end), Blocks(blocks), Branches(branches) {};

public:
    class BasicBlock
    {
    public:
        BasicBlock() {};
        BasicBlock(bool isStart)
        {
            IsStart = isStart;
            IsEnd = !isStart;
        };

        bool IsStart;
        bool IsEnd;

        std::vector<BoundStatement *> Statements;
        std::vector<BasicBlockBranch *> Incoming;
        std::vector<BasicBlockBranch *> Outgoing;

        std::string ToString() const
        {
            if (IsStart)
            {
                return "<Start>";
            }

            if (IsEnd)
            {
                return "<End>";
            }

            for (auto statement : Statements)
            {
                statement->WriteTo(std::cout);
                return "BasicBlock";
            }

            return "BasicBlock";
        };
    };

    class BasicBlockBranch
    {
    public:
        BasicBlockBranch(BasicBlock *from, BasicBlock *to, BoundExpression *condition) : From(from), To(to), Condition(condition) {};

        BasicBlock *From;
        BasicBlock *To;
        BoundExpression *Condition;
        std::string ToString() const
        {
            if (Condition == nullptr)
            {
                return "";
            }
            Condition->WriteTo(std::cout);

            return "BasicBlockBranch";
        };
    };

    class BasicBlockBuilder
    {
    public:
        std::vector<BasicBlock *> Build(BoundBlockStatement *block);

    private:
        std::vector<BoundStatement *> _statements;
        std::vector<BasicBlock *> _blocks;

        void StartBlock()
        {
            EndBlock();
        }

        void EndBlock()
        {
            if (!_statements.empty())
            {

                BasicBlock *block = new BasicBlock();
                block->Statements = _statements;
                _blocks.push_back(block);
                _statements.clear();
            }
        }
    };

    class GraphBuilder
    {
    public:
        ControlFlowGraph *Build(std::vector<BasicBlock *> &blocks);

    private:
        void Connect(BasicBlock *from, BasicBlock *to, BoundExpression *condition = nullptr);
        void RemoveBlock(std::vector<BasicBlock *> &blocks, BasicBlock *block);
        BoundExpression *Negate(BoundExpression *condition);

        std::unordered_map<BoundStatement *, BasicBlock *> _blockFromStatement;
        std::unordered_map<BoundLabel, BasicBlock *> _blockFromLabel;
        std::vector<BasicBlockBranch *> _branches;
        BasicBlock *_start = new BasicBlock(true);
        BasicBlock *_end = new BasicBlock(false);
    };
};

#endif