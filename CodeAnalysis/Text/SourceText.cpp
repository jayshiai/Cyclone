#include "CodeAnalysis/SourceText.h"
#include <vector>
std::vector<TextLine> SourceText::ParseLines(const std::string &text)
{
    std::vector<TextLine> result;
    int position = 0;
    int lineStart = 0;

    while (position < text.length())
    {
        int lineBreakWidth = GetLineBreakWidth(text, position);

        if (lineBreakWidth == 0)
        {
            position++;
        }
        else
        {
            AddLine(result, *this, position, lineStart, lineBreakWidth);
            position += lineBreakWidth;
            lineStart = position;
        }
    }

    if (position >= lineStart)
    {
        AddLine(result, *this, position, lineStart, 0);
    }

    return result;
}

void SourceText::AddLine(std::vector<TextLine> &result, const SourceText &sourceText, int position, int lineStart, int lineBreakWidth)
{
    int lineLength = position - lineStart;
    int lineLengthIncludingLineBreak = lineLength + lineBreakWidth;

    result.emplace_back(sourceText, lineStart, lineLength, lineLengthIncludingLineBreak);
}

int SourceText::GetLineBreakWidth(const std::string &text, int position)
{
    char c = text[position];
    char l = (position + 1 >= text.length()) ? '\0' : text[position + 1];

    if (c == '\r' && l == '\n')
    {
        return 2;
    }

    if (c == '\r' || c == '\n')
    {
        return 1;
    }

    return 0;
}

int SourceText::GetLineIndex(int position) const
{
    int lower = 0;
    int upper = static_cast<int>(_lines.size()) - 1;

    while (lower <= upper)
    {
        int index = lower + (upper - lower) / 2;
        int start = _lines[index].Start;

        if (position == start)
        {
            return index;
        }

        if (start > position)
        {
            upper = index - 1;
        }
        else
        {
            lower = index + 1;
        }
    }

    return lower - 1;
}
