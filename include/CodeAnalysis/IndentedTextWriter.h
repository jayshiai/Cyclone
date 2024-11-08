#ifndef INDENTED_TEXT_WRITER_H
#define INDENTED_TEXT_WRITER_H

#include "CodeAnalysis/Diagnostic.h"
#include "CodeAnalysis/SourceText.h"
#include <algorithm>
#include <iostream>
#include <string>

class IndentedTextWriter
{
public:
    IndentedTextWriter(std::ostream &output) : os(output), indentLevel(0) {}

    const std::string RESET_COLOR = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string GRAY = "\033[90m";
    void Write(const std::string &text)
    {
        if (isNewLine)
        {
            for (int i = 0; i < indentLevel; i++)
            {
                os << "    ";
            }
            isNewLine = false;
        }
        os << text;
    }

    void WriteLine()
    {
        os << std::endl;
        isNewLine = true;
    }

    bool isConsleOutput()
    {
        return &os == &std::cout;
    }
    void Indent()
    {
        ++indentLevel;
    }

    int GetIndentLevel() const
    {
        return indentLevel;
    }

    void Unindent()
    {
        if (indentLevel > 0)
        {
            --indentLevel;
        }
    }

    void SetForegroundColor(std::string color)
    {
        if (isConsleOutput())
        {
            os << color;
        }
    }

    void ResetColor()
    {
        if (isConsleOutput())
        {
            os << RESET_COLOR;
        }
    }

    void WriteKeyword(std::string text)
    {
        SetForegroundColor(CYAN);
        Write(text);
        ResetColor();
    }

    void WriteIdentifier(std::string text)
    {
        SetForegroundColor(YELLOW);
        Write(text);
        ResetColor();
    }

    void WriteNumber(std::string text)
    {
        SetForegroundColor(GREEN);
        Write(text);
        ResetColor();
    }

    void WriteString(std::string text)
    {
        SetForegroundColor(MAGENTA);
        Write(text);
        ResetColor();
    }
    void WritePunctuation(std::string text)
    {
        SetForegroundColor(GRAY);
        Write(text);
        ResetColor();
    }

    void WriteDiagnostics(std::vector<Diagnostic> diagnostics, SyntaxTree syntaxtree)
    {

        std::sort(diagnostics.begin(), diagnostics.end(), [](const Diagnostic &a, const Diagnostic &b)
                  { return a.Span.Start < b.Span.Start; });
        for (auto diagnostic : diagnostics)
        {
            auto lineIndex = syntaxtree.Text.GetLineIndex(diagnostic.Span.Start);
            auto line = syntaxtree.Text._lines[lineIndex];
            auto lineNumber = lineIndex + 1;
            auto character = diagnostic.Span.Start - line.Start + 1;

            WriteLine();
            SetForegroundColor(RED);
            Write("(" + std::to_string(lineNumber) + "," + std::to_string(character) + "): ");
            Write(diagnostic.Message);
            WriteLine();
            ResetColor();

            auto prefixSpan = TextSpan::FromBounds(line.Start, diagnostic.Span.Start);
            auto suffixSpan = TextSpan::FromBounds(diagnostic.Span.End, line.End);

            auto prefix = syntaxtree.Text.ToString(prefixSpan);
            auto error = syntaxtree.Text.ToString(diagnostic.Span);
            auto suffix = syntaxtree.Text.ToString(suffixSpan);

            Write("    ");
            Write(prefix);

            SetForegroundColor(RED);
            Write(error);
            ResetColor();

            Write(suffix);

            WriteLine();
        }

        WriteLine();
    }

    // Overload the stream insertion operator
    friend std::ostream &operator<<(std::ostream &os, const IndentedTextWriter &writer)
    {
        return os; // Just return the output stream
    }

private:
    std::ostream &os;
    int indentLevel;
    bool isNewLine;
};

#endif