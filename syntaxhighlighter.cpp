/* syntaxhighlighter.cpp
PURPOSE:
- Highlights text based on WBS syntax
*/
#include "syntaxhighlighter.h"
#include <QRegExp>

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    // 5. Symbol Highlighting (Priority 5)
    QTextCharFormat symbolFormat;
    symbolFormat.setForeground(Qt::darkMagenta);
    QStringList symbolPatterns = {
        "\\(", "\\)", "\\{", "\\}", "\\[", "\\]",
        "\\+", "-", "\\*", "\\/", "=", "!", "%",
        "\\^", "\\|", "~", "<", ">", ",", "\\.", 
        ";", ":", "'", "\"", "#", "\\$", "&", 
        "\\?", "@", "_", "\\|", "\\\\", "→", "←", 
        "↑", "↓", "≥", "≤", "≡", "≈", "×", "·", 
        "•"
    };
    for (const QString &pattern : symbolPatterns) {
        QRegExp symbolExpression(pattern);
        int symbolIndex = symbolExpression.indexIn(text);
        while (symbolIndex >= 0) {
            int length = symbolExpression.matchedLength();
            setFormat(symbolIndex, length, symbolFormat);
            symbolIndex = symbolExpression.indexIn(text, symbolIndex + length);
        }
    }

    // 4. Number Highlighting (Priority 4)
    QTextCharFormat numberFormat;
    numberFormat.setForeground(Qt::darkYellow);
    QRegExp numberPattern("\\b(?![a-zA-Z_])[0-9]+(?:\\.[0-9]+)?\\b");  // Find numbers unless they are part of a word
    int numberIndex = numberPattern.indexIn(text);
    while (numberIndex >= 0) {
        int length = numberPattern.matchedLength();
        setFormat(numberIndex, length, numberFormat);
        numberIndex = numberPattern.indexIn(text, numberIndex + length);
    }

    // 3. Keyword Highlighting (Priority 3)
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(Qt::blue);
    QStringList keywordPatterns = { "\\bconst\\b", "\\bcreate\\b", "\\bopen\\b", "\\bfile\\b", "\\bcolorset\\b", "\\bforeach\\b", "\\busing\\b", "\\bexport\\b", "\\boutput\\b", "\\bin\\b", "\\bas\\b", "\\bdo\\b", "\\bxor\\b", "\\band\\b", "\\bor\\b", "\\bnot\\b" };
    for (const QString &pattern : keywordPatterns) {
        QRegExp expression(pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, keywordFormat);
            index = expression.indexIn(text, index + length);
        }
    }

    // 2. String Highlighting (Priority 2)
    QTextCharFormat stringFormat;
    stringFormat.setForeground(Qt::red);
    QRegExp stringPattern("\"[^\"]*\"");  // Find text inside double quotes
    int stringIndex = stringPattern.indexIn(text);
    while (stringIndex >= 0) {
        int length = stringPattern.matchedLength();
        setFormat(stringIndex, length, stringFormat);
        stringIndex = stringPattern.indexIn(text, stringIndex + length);
    }

    // 1. Comment Highlighting (Priority 1)
    QTextCharFormat commentFormat;
    commentFormat.setForeground(Qt::darkGreen);
    QRegExp commentPattern("//[^\n]*");
    int commentIndex = commentPattern.indexIn(text);
    while (commentIndex >= 0) {
        int length = commentPattern.matchedLength();
        setFormat(commentIndex, length, commentFormat);
        commentIndex = commentPattern.indexIn(text, commentIndex + length);
    }
}
