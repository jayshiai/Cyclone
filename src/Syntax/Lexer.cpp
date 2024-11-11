#include "CodeAnalysis/Lexer.h"
#include "CodeAnalysis/SyntaxTree.h"
#include <iostream>
#include <cctype>
#include <stdexcept>
#include "CodeAnalysis/Diagnostic.h"
#include "Utils.h"

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;
    while (currentChar != '\0')
    {
        switch (currentChar)
        {
        case '+':
            tokens.push_back(Token{_syntaxTree, SyntaxKind::PLUS, "+", pos});
            advance();
            break;

        case '-':
            tokens.push_back(Token{_syntaxTree, SyntaxKind::MINUS, "-", pos});
            advance();
            break;

        case '*':
            tokens.push_back(Token{_syntaxTree, SyntaxKind::MULTIPLY, "*", pos});
            advance();
            break;

        case '/':
            if (lookAhead == '/')
            {
                GenerateSingleLineComment();
            }
            else if (lookAhead == '*')
            {
                GenerateMultiLineComment();
            }
            else
            {
                tokens.push_back(Token{_syntaxTree, SyntaxKind::DIVIDE, "/", pos});
                advance();
            }
            break;

        case '(':
            tokens.push_back(Token{_syntaxTree, SyntaxKind::LPAREN, "(", pos});
            advance();
            break;

        case ')':
            tokens.push_back(Token{_syntaxTree, SyntaxKind::RPAREN, ")", pos});
            advance();
            break;
        case '{':
            tokens.push_back(Token{_syntaxTree, SyntaxKind::OPEN_BRACE, "{", pos});
            advance();
            break;
        case '}':
            tokens.push_back(Token{_syntaxTree, SyntaxKind::CLOSE_BRACE, "}", pos});
            advance();
            break;
        case '[':
            tokens.push_back(Token{_syntaxTree, SyntaxKind::OPEN_BRACKET, "[", pos});
            advance();
            break;
        case ']':
            tokens.push_back(Token{_syntaxTree, SyntaxKind::CLOSE_BRACKET, "]", pos});
            advance();
            break;
        case ':':
            tokens.push_back(Token{_syntaxTree, SyntaxKind::COLON, ":", pos});
            advance();
            break;
        case ',':
            tokens.push_back(Token{_syntaxTree, SyntaxKind::COMMA, ",", pos});
            advance();
            break;
        case '~':
            tokens.push_back(Token{_syntaxTree, SyntaxKind::TILDE, "~", pos});
            advance();
            break;
        case '^':
            tokens.push_back(Token{_syntaxTree, SyntaxKind::HAT, "^", pos});
            advance();
            break;
        case '&':
            if (lookAhead == '&')
            {
                tokens.push_back(Token{_syntaxTree, SyntaxKind::AMPERSAND_AMPERSAND, "&&", pos});
                advance();
                advance();
            }
            else
            {
                tokens.push_back(Token{_syntaxTree, SyntaxKind::AMPERSAND, "&", pos});
                advance();
            }
            break;
        case '|':
            if (lookAhead == '|')
            {
                tokens.push_back(Token{_syntaxTree, SyntaxKind::PIPE_PIPE, "||", pos});
                advance();
                advance();
            }
            else
            {
                tokens.push_back(Token{_syntaxTree, SyntaxKind::PIPE, "|", pos});
                advance();
            }
            break;
        case '=':
            if (lookAhead == '=')
            {
                tokens.push_back(Token{_syntaxTree, SyntaxKind::EQUALS_EQUALS, "==", pos});
                advance();
                advance();
            }
            else
            {
                tokens.push_back(Token{_syntaxTree, SyntaxKind::EQUALS, "=", pos});
                advance();
            }
            break;
        case '!':
            if (lookAhead == '=')
            {
                tokens.push_back(Token{_syntaxTree, SyntaxKind::BANG_EQUALS, "!=", pos});
                advance();
                advance();
            }
            else
            {
                tokens.push_back(Token{_syntaxTree, SyntaxKind::BANG, "!", pos});
                advance();
            }
            break;
        case '<':
            if (lookAhead == '=')
            {
                tokens.push_back(Token{_syntaxTree, SyntaxKind::LESS_EQUALS, "<=", pos});
                advance();
                advance();
            }
            else
            {
                tokens.push_back(Token{_syntaxTree, SyntaxKind::LESS, "<", pos});
                advance();
            }
            break;
        case '>':
            if (lookAhead == '=')
            {
                tokens.push_back(Token{_syntaxTree, SyntaxKind::GREATER_EQUALS, ">=", pos});
                advance();
                advance();
            }
            else
            {
                tokens.push_back(Token{_syntaxTree, SyntaxKind::GREATER, ">", pos});
                advance();
            }
            break;
        case '"':
            tokens.push_back(GenerateStringToken());
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            tokens.push_back(GenerateNumberToken());
            break;
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            GenerateWhitespaceToken();
            break;
        default:

            // if (isspace(currentChar))
            // {
            //     skipWhitespace();
            // }

            if (isalpha(currentChar))
            {
                tokens.push_back(GenerateIdentifierToken());
            }
            else if (isspace(currentChar))
            {
                GenerateWhitespaceToken();
            }
            else
            {
                TextSpan span(pos, 1);
                TextLocation location(_syntaxTree->Text, span);
                _diagnostics.ReportBadCharacter(location, currentChar);
                // tokens.push_back(Token{_syntaxTree, SyntaxKind::BAD_TOKEN, std::string(1, currentChar), pos});
                advance();
            }

            break;
        }
    }
    tokens.push_back(Token{_syntaxTree, SyntaxKind::END_OF_FILE, "", pos});
    // std::cout << "TOKENS: " << std::endl;
    // for (auto token : tokens)
    // {
    //     std::cout << "  " << convertSyntaxKindToString(token.Kind) << " " << token.value << std::endl;
    // }
    return tokens;
}

void Lexer::advance()
{
    pos++;
    if (pos < input.Length())
    {
        currentChar = input[pos];
        lookAhead = input[pos + 1];
    }
    else
    {
        currentChar = '\0';
    }
}

Token Lexer::GenerateStringToken()
{
    size_t start = pos;

    std::string result;
    bool done = false;
    advance();
    // while (currentChar != '"')
    // {
    //     if (currentChar == '\0')
    //     {
    //         _diagnostics.ReportUnterminatedString(TextSpan(start, pos - start));
    //         return Token{_syntaxTree, SyntaxKind::BAD_TOKEN, input.ToString(start, pos - start), start};
    //     }
    //     advance();
    // }

    while (!done)
    {
        switch (currentChar)
        {
        case '\0':
        case '\r':
        case '\n':
        {
            TextSpan span(start, 1);
            TextLocation location(_syntaxTree->Text, span);
            _diagnostics.ReportUnterminatedString(location);
            return Token{_syntaxTree, SyntaxKind::BAD_TOKEN, input.ToString(start, pos - start), start};
        }
        case '"':
            done = true;
            break;
        case '\\':
            advance();
            switch (currentChar)
            {
            case 'n':
                result += '\n';
                break;
            case 't':
                result += '\t';
                break;
            case 'r':
                result += '\r';
                break;
            case '\\':
                result += '\\';
                break;
            case '"':
                result += '"';
                break;
            default:
            {
                TextSpan span(pos, 1);
                TextLocation location(_syntaxTree->Text, span);
                _diagnostics.ReportBadCharacter(location, currentChar);
                break;
            }
            }
            advance();
            break;
        default:
            result += currentChar;
            advance();
            break;
        }
    }
    advance();
    return Token{_syntaxTree, SyntaxKind::STRING, result, start};
}

Token Lexer::GenerateWhitespaceToken()
{
    while (isspace(currentChar))
    {
        advance();
    }
    return Token{_syntaxTree, SyntaxKind::WHITESPACE, " ", pos};
}
Token Lexer::GenerateNumberToken()
{
    size_t start = pos;
    std::string result;
    while (pos < input.Length() && isdigit(currentChar))
    {
        result += currentChar;
        advance();
    }

    int length = pos - start;
    int value;

    if (isalpha(currentChar))
    {
        while (isalnum(currentChar))
        {
            result += currentChar;
            advance();
        }

        TextSpan span(start, pos - start);
        TextLocation location(_syntaxTree->Text, span);
        _diagnostics.ReportInvalidNumber(location, result, "int");
        return Token{_syntaxTree, SyntaxKind::BAD_TOKEN, result, start};
    }
    return Token{_syntaxTree, SyntaxKind::NUMBER, result, pos};
}

Token Lexer::GenerateSingleLineComment()
{
    size_t start = pos;
    while (currentChar != '\n' && currentChar != '\0' && currentChar != '\r')
    {
        advance();
    }
    return Token{_syntaxTree, SyntaxKind::SingleLineComment, input.ToString(start, pos - start), start};
}

Token Lexer::GenerateMultiLineComment()
{
    size_t start = pos;
    advance();
    advance();

    while (currentChar != '*' && lookAhead != '/')
    {
        if (currentChar == '\0' || pos >= input.Length())
        {
            TextSpan span(start, pos - start);
            TextLocation location(_syntaxTree->Text, span);
            _diagnostics.ReportUnterminatedComment(location);
            return Token{_syntaxTree, SyntaxKind::BAD_TOKEN, input.ToString(start, pos - start), start};
        }
        advance();
    }
    advance();
    advance();
    return Token{_syntaxTree, SyntaxKind::MultiLineComment, input.ToString(start, pos - start), start};
}
Token Lexer::GenerateIdentifierToken()
{
    size_t start = pos;
    while (isalnum(currentChar))
    {
        advance();
    }
    int length = pos - start;
    std::string text = input.ToString(start, length);
    SyntaxKind kind = checkKeyword(text);
    return Token{_syntaxTree, kind, text, start};
}

SyntaxKind Lexer::checkKeyword(const std::string &keyword)
{
    if (keyword == "true")
        return SyntaxKind::TRUE_KEYWORD;
    else if (keyword == "false")
        return SyntaxKind::FALSE_KEYWORD;
    else if (keyword == "const")
        return SyntaxKind::CONST_KEYWORD;
    else if (keyword == "var")
        return SyntaxKind::VAR_KEYWORD;
    else if (keyword == "if")
        return SyntaxKind::IF_KEYWORD;
    else if (keyword == "else")
        return SyntaxKind::ELSE_KEYWORD;
    else if (keyword == "while")
        return SyntaxKind::WHILE_KEYWORD;
    else if (keyword == "for")
        return SyntaxKind::FOR_KEYWORD;
    else if (keyword == "to")
        return SyntaxKind::TO_KEYWORD;
    else if (keyword == "function")
        return SyntaxKind::FUNCTION_KEYWORD;
    else if (keyword == "break")
        return SyntaxKind::BREAK_KEYWORD;
    else if (keyword == "continue")
        return SyntaxKind::CONTINUE_KEYWORD;
    else if (keyword == "return")
        return SyntaxKind::RETURN_KEYWORD;
    return SyntaxKind::IDENTIFIER;
}