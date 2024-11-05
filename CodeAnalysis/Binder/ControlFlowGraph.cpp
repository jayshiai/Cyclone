#include "CodeAnalysis/ControlFlowGraph.h"

#include <sstream>
#include <unordered_map>
#include <algorithm>

void ControlFlowGraph::WriteTo(std::ostream &writer) const
{
    auto Quote = [](const std::string &text)
    {
        std::string quoted = "\"" + text + "\"";
        size_t pos = 0;
        while ((pos = quoted.find("\"", pos)) != std::string::npos)
        {
            quoted.replace(pos, 1, "\\\"");
            pos += 2;
        }
        return quoted;
    };

    writer << "digraph G {" << std::endl;

    std::unordered_map<BasicBlock *, std::string> blockIds;
    for (size_t i = 0; i < Blocks.size(); i++)
    {
        std::string id = "N" + std::to_string(i);
        blockIds[Blocks[i]] = id;
    }

    for (const auto &block : Blocks)
    {
        std::string id = blockIds[block];
        std::string label = Quote(block->ToString());
        writer << "    " << id << " [label = " << label << " shape = box]" << std::endl;
    }

    for (const auto &branch : Branches)
    {
        std::string fromId = blockIds[branch->From];
        std::string toId = blockIds[branch->To];
        std::string label = Quote(branch->ToString());
        writer << "    " << fromId << " -> " << toId << " [label = " << label << "]" << std::endl;
    }

    writer << "}" << std::endl;
}

ControlFlowGraph *ControlFlowGraph::Create(BoundBlockStatement *body)
{
    BasicBlockBuilder basicBlockBuilder;
    std::vector<BasicBlock *> blocks = basicBlockBuilder.Build(body);

    GraphBuilder graphBuilder;
    return graphBuilder.Build(blocks);
}

bool ControlFlowGraph::AllPathsReturn(BoundBlockStatement *body)
{
    ControlFlowGraph *graph = Create(body);

    for (const auto &branch : graph->End->Incoming)
    {

        const auto &lastStatement = branch->From->Statements.back();

        if (lastStatement->GetKind() != BoundNodeKind::ReturnStatement)
            return false;
    }

    return true;
}

void ControlFlowGraph::GraphBuilder::Connect(BasicBlock *from, BasicBlock *to, BoundExpression *condition)
{

    if (auto literal = dynamic_cast<BoundLiteralExpression *>(condition))
    {
        bool value = (literal->Value == "true");

        if (value)
        {
            condition = nullptr;
        }
        else
        {
            return;
        }
    }

    BasicBlockBranch *branch = new BasicBlockBranch(from, to, condition);

    from->Outgoing.push_back(branch);
    to->Incoming.push_back(branch);

    _branches.push_back(branch);
}

void ControlFlowGraph::GraphBuilder::RemoveBlock(std::vector<BasicBlock *> &blocks, BasicBlock *block)
{
    for (auto *branch : block->Incoming)
    {
        auto &fromOutgoing = branch->From->Outgoing;

        fromOutgoing.erase(std::remove(fromOutgoing.begin(), fromOutgoing.end(), branch), fromOutgoing.end());

        _branches.erase(std::remove(_branches.begin(), _branches.end(), branch), _branches.end());

        delete branch;
    }

    for (auto *branch : block->Outgoing)
    {
        auto &toIncoming = branch->To->Incoming;

        toIncoming.erase(std::remove(toIncoming.begin(), toIncoming.end(), branch), toIncoming.end());

        _branches.erase(std::remove(_branches.begin(), _branches.end(), branch), _branches.end());

        delete branch;
    }

    block->Incoming.clear();
    block->Outgoing.clear();

    blocks.erase(std::remove(blocks.begin(), blocks.end(), block), blocks.end());
}

BoundExpression *ControlFlowGraph::GraphBuilder::Negate(BoundExpression *condition)
{
    if (auto *literal = dynamic_cast<BoundLiteralExpression *>(condition))
    {
        bool value = (literal->Value == "true");
        return new BoundLiteralExpression(literal->Value, TypeSymbol::Boolean);
    }

    BoundUnaryOperator *op = BoundUnaryOperator::Bind(SyntaxKind::BANG, TypeSymbol::Boolean);
    return new BoundUnaryExpression(op, condition);
}

ControlFlowGraph *ControlFlowGraph::GraphBuilder::Build(std::vector<BasicBlock *> &blocks)
{
    if (blocks.empty())
    {
        Connect(_start, _end);
    }
    else
    {
        Connect(_start, blocks.front());
    }

    for (auto *block : blocks)
    {
        for (auto *statement : block->Statements)
        {
            _blockFromStatement[statement] = block;

            if (auto *labelStatement = dynamic_cast<BoundLabelStatement *>(statement))
            {
                _blockFromLabel[labelStatement->Label] = block;
            }
        }
    }

    for (size_t i = 0; i < blocks.size(); i++)
    {
        BasicBlock *current = blocks[i];
        BasicBlock *next = (i == blocks.size() - 1) ? _end : blocks[i + 1];

        for (size_t j = 0; j < current->Statements.size(); j++)
        {
            BoundStatement *statement = current->Statements[j];
            bool isLastStatementInBlock = (j == current->Statements.size() - 1);

            switch (statement->GetKind())
            {
            case BoundNodeKind::GotoStatement:
            {
                auto *gs = static_cast<BoundGotoStatement *>(statement);
                BasicBlock *toBlock = _blockFromLabel[gs->Label];
                Connect(current, toBlock);
                break;
            }
            case BoundNodeKind::ConditionalGotoStatement:
            {
                auto *cgs = static_cast<BoundConditionalGotoStatement *>(statement);
                BasicBlock *thenBlock = _blockFromLabel[cgs->Label];
                BasicBlock *elseBlock = next;

                BoundExpression *negatedCondition = Negate(cgs->Condition);
                BoundExpression *thenCondition = cgs->JumpIfTrue ? cgs->Condition : negatedCondition;
                BoundExpression *elseCondition = cgs->JumpIfTrue ? negatedCondition : cgs->Condition;

                Connect(current, thenBlock, thenCondition);
                Connect(current, elseBlock, elseCondition);
                break;
            }
            case BoundNodeKind::ReturnStatement:
            {
                Connect(current, _end);
                break;
            }
            case BoundNodeKind::VariableDeclaration:
            case BoundNodeKind::LabelStatement:
            case BoundNodeKind::ExpressionStatement:
            {
                if (isLastStatementInBlock)
                {
                    Connect(current, next);
                }
                break;
            }
            default:
                throw std::runtime_error("Unexpected statement kind.");
            }
        }
    }

    while (true)
    {
        bool modified = false;

        for (auto it = blocks.begin(); it != blocks.end();)
        {
            BasicBlock *block = *it;

            if (block->Incoming.empty())
            {
                RemoveBlock(blocks, block);
                it = blocks.erase(it);
                modified = true;
            }
            else
            {
                ++it;
            }
        }

        if (!modified)
        {
            break;
        }
    }

    blocks.insert(blocks.begin(), _start);
    blocks.push_back(_end);

    return new ControlFlowGraph(_start, _end, blocks, _branches);
}

std::vector<ControlFlowGraph::BasicBlock *> ControlFlowGraph::BasicBlockBuilder::Build(BoundBlockStatement *block)
{
    for (auto *statement : block->Statements)
    {
        switch (statement->GetKind())
        {
        case BoundNodeKind::LabelStatement:
            StartBlock();
            _statements.push_back(statement);
            break;

        case BoundNodeKind::GotoStatement:
        case BoundNodeKind::ConditionalGotoStatement:
        case BoundNodeKind::ReturnStatement:
            _statements.push_back(statement);
            StartBlock();
            break;

        case BoundNodeKind::VariableDeclaration:
        case BoundNodeKind::ExpressionStatement:
            _statements.push_back(statement);
            break;

        default:
            throw std::runtime_error("Unexpected statement kind.");
        }
    }

    EndBlock();

    return _blocks;
}