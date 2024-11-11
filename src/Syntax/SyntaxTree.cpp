#include "CodeAnalysis/Lexer.h"
#include "CodeAnalysis/Parser.h"
#include "CodeAnalysis/SyntaxTree.h"
#include <fstream>
SyntaxTree SyntaxTree::Parse(std::string text)
{
    SourceText sourceText = SourceText::From(text);
    return Parse(sourceText);
}

SyntaxTree SyntaxTree::Parse(SourceText text)
{

    return SyntaxTree(text, Parse);
}

void SyntaxTree::Parse(SyntaxTree *syntaxTree, CompilationUnitNode *&root, std::vector<Diagnostic> &diagnostics)
{

    Parser parser(syntaxTree);
    root = parser.ParseCompilationUnit();
    diagnostics = parser.GetDiagnostics().GetDiagnostics();
}

SyntaxTree::SyntaxTree(SourceText text, ParseHandler handler) : Text(text)
{
    // Parser parser(text);
    // CompilationUnitNode *root = parser.ParseCompilationUnit();
    handler(this, Root, Diagnostics);
    // Diagnostics = parser.GetDiagnostics().GetDiagnostics();
}

std::vector<Token> SyntaxTree::ParseTokens(std::string text)
{
    SourceText sourceText = SourceText::From(text);
    return ParseTokens(sourceText);
}

std::vector<Token> SyntaxTree::ParseTokens(std::string text, std::vector<Diagnostic> &diagnostics)
{
    SourceText sourceText = SourceText::From(text);
    return ParseTokens(sourceText, diagnostics);
}

SyntaxTree SyntaxTree::Load(std::string filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Unable to open file: " + filename);
    }
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return SyntaxTree::Parse(text);
}
std::vector<Token> SyntaxTree::ParseTokens(SourceText text)
{
    std::vector<Diagnostic> mydiagnostics;
    return ParseTokens(text, mydiagnostics);
}

void ParseTokensHn(SyntaxTree *st, CompilationUnitNode *&root, std::vector<Diagnostic> &d)
{
    Lexer lexer(st);
    std::vector<Token> tokens = lexer.tokenize();
    if (tokens[tokens.size() - 1].Kind == SyntaxKind::END_OF_FILE)
    {
        root = new CompilationUnitNode(st, {}, tokens[tokens.size() - 1]);
    }

    d = lexer.GetDiagnostics().GetDiagnostics();
}

std::vector<Token> SyntaxTree::ParseTokens(SourceText text, std::vector<Diagnostic> &diagnostics)
{
    std::vector<Token> tokens;

    SyntaxTree *syntaxTree = new SyntaxTree(text, ParseTokensHn);
    diagnostics = syntaxTree->Diagnostics;
    return tokens;
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
        if (node != nullptr)
        {
            os << convertSyntaxKindToString(node->Kind);
        }
    }

    os << RESET_COLOR << std::endl;
    indent += isLast ? "   " : "|   ";

    auto children = node->GetChildren();
    for (size_t i = 0; i < children.size(); i++)
    {
        PrettyPrint(os, children[i], indent, i == children.size() - 1);
    }
}