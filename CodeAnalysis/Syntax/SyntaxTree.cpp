#include "CodeAnalysis/Lexer.h"
#include "CodeAnalysis/Parser.h"
#include "CodeAnalysis/SyntaxTree.h"

SyntaxTree SyntaxTree::Parse(std::string text)
{
    SourceText sourceText = SourceText::From(text);
    return Parse(sourceText);
}

SyntaxTree SyntaxTree::Parse(SourceText text)
{
    return SyntaxTree(text);
}

SyntaxTree::SyntaxTree(SourceText text) : Text(text)
{
    Parser parser(text);
    CompilationUnitNode *root = parser.ParseCompilationUnit();
    Diagnostics = parser.GetDiagnostics().GetDiagnostics();
    Root = root;
}

void SyntaxNode::PrettyPrint(std::ostream &os, SyntaxNode *node, std::string indent, bool isLast)
{
    const std::string RESET_COLOR = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string GRAY = "\033[90m";

    os << indent << GRAY << "|--" << BLUE;

    if (Token *token = dynamic_cast<Token *>(node))
    {
        os << GREEN << convertSyntaxKindToString(node->Kind) << " " << token->value;
        }

    else if (StatementSyntax *stmt = dynamic_cast<StatementSyntax *>(node))
    {
        os << YELLOW << convertSyntaxKindToString(stmt->Kind);
    }

    else
    {
        os << convertSyntaxKindToString(node->Kind);
    }

    os << RESET_COLOR << std::endl;
    indent += isLast ? "   " : "|   ";

    auto children = node->GetChildren();
    for (size_t i = 0; i < children.size(); i++)
    {
        PrettyPrint(os, children[i], indent, i == children.size() - 1);
    }
}