#ifndef SOURCE_TEXT_H
#define SOURCE_TEXT_H

#include <string>
#include <vector>
struct TextSpan
{
    int Start;
    int Length;
    int End;
    TextSpan(int start, int length)
        : Start(start), Length(length)
    {
        End = start + length;
    }

    static TextSpan FromBounds(int start, int end)
    {
        int length = end - start;
        return TextSpan(start, length);
    }

    std::string ToString() const
    {
        return std::to_string(Start) + " " + std::to_string(Length);
    }
};

class TextLine;
class SourceText
{
private:
    std::string _text;
    SourceText(const std::string &text, std::string filename = "")
        : _text(text), Filename(filename)
    {
        _lines = ParseLines(text);
    }

    std::vector<TextLine> ParseLines(const std::string &text);

    static void AddLine(std::vector<TextLine> &result, const SourceText &sourceText, int position, int lineStart, int lineBreakWidth);

    static int GetLineBreakWidth(const std::string &text, int position);

public:
    std::string Filename;
    std::vector<TextLine> _lines;
    static SourceText From(const std::string &text, std::string filename = "")
    {
        return SourceText(text, filename);
    }

    char operator[](int index) const
    {
        return _text[index];
    }

    int Length() const
    {
        return static_cast<int>(_text.length());
    }

    int GetLineIndex(int position) const;

    std::string ToString() const
    {
        return _text;
    }

    std::string ToString(int start, int length) const
    {
        return _text.substr(start, length);
    }

    std::string ToString(TextSpan span) const
    {
        return ToString(span.Start, span.Length);
    };
};

class TextLine
{
public:
    int Start;
    int Length;
    int End;
    SourceText Text;
    int LengthIncludingLineBreak;
    TextSpan Span;
    TextSpan SpanIncludingLineBreak;

    TextLine(SourceText text, int start, int length, int lengthIncludingLineBreak)
        : Text(text), Start(start), Length(length), LengthIncludingLineBreak(lengthIncludingLineBreak),
          Span(start, length), SpanIncludingLineBreak(start, lengthIncludingLineBreak)
    {
        End = start + length;
    }

    std::string ToString()
    {
        return Text.ToString(Span);
    }
};

class TextLocation
{
public:
    SourceText Text;
    TextSpan Span;
    TextLocation(SourceText text, TextSpan span)
        : Text(text), Span(span), Filename(text.Filename)
    {

        StartLine = text.GetLineIndex(span.Start);
        StartCharacter = span.Start - text._lines[StartLine].Start;
        EndLine = text.GetLineIndex(span.End);
        EndCharacter = span.End - text._lines[EndLine].Start;
    }

    std::string ToString() const
    {
        return Span.ToString();
    }

    std::string Filename;
    int StartLine;
    int StartCharacter;
    int EndLine;
    int EndCharacter;
};
#endif