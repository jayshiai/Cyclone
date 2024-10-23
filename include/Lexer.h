#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

enum class TokenType
{
    NUMBER,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    LPAREN, // (
    RPAREN, // )
    END_OF_FILE
};

struct Token
{
    TokenType type;
    std::string value;
    Token(TokenType type, std::string value) : type(type), value(value) {}
};

class Lexer
{
public:
    Lexer(const std::string &input);
    std::vector<Token> tokenize();

private:
    std::string input;
    size_t pos;
    char currentChar;
    void advance();
    void skipWhitespace();
    std::string number();
};

#endif
