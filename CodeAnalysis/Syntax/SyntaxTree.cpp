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
    Lexer lexer(text);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(text, tokens, lexer.GetDiagnostics());
    return parser.parse();
}