#include "pythonhighlighter.h"

// Syntax styles that can be shared by all languages
typedef QMap<QString, QTextCharFormat> FormatMap;
//static FormatMap STYLES;
static const FormatMap STYLES = {
    { "keyword", PythonHighlighter::format("#f7d268") },
    { "operator", PythonHighlighter::format("#8c887a") },
    { "brace", PythonHighlighter::format("#9bc4f2") },
    { "defclass", PythonHighlighter::format("#bd42ed", "bold") },
    { "string", PythonHighlighter::format("#c67de3") },
    { "string2", PythonHighlighter::format("#827b67") },
    { "comment", PythonHighlighter::format("#827b67", "italic") },
    { "self", PythonHighlighter::format("black", "italic") },
    { "numbers", PythonHighlighter::format("#7fed66") }
};

// Python keywords
static const QStringList keywords = {
    "and", "assert", "break", "class", "continue", "def", "len",
    "del", "elif", "else", "except", "exec", "finally",
    "for", "from", "global", "if", "import", "in",
    "is", "str", "lambda", "not", "or", "pass", "print",
    "raise", "return", "try", "while", "yield",
    "None", "True", "False"
};

// Python operators
static const QStringList operators = {
    "=",
    // Comparison
    "==", "!=", "<", "<=", ">", ">=",
    // Arithmetic
    "\\+", "-", "\\*", "/", "//", "\\%", "\\*\\*",
    // In-place
    "\\+=", "-=", "\\*=", "/=", "\\%=",
    // Bitwise
    "\\^", "\\|", "\\&", "\\~", ">>", "<<"
};

// Python braces
static const QStringList braces = {
    QStringLiteral("\\{"), QStringLiteral("\\}"),
    QStringLiteral("\\("), QStringLiteral("\\)"),
    QStringLiteral("\\["), QStringLiteral( "\\]")
};


PythonHighlighter::PythonHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    initialize();
}

void PythonHighlighter::highlightBlock(const QString &text)
{
    highlightPythonBlock(text);
}


void PythonHighlighter::initialize()
{
    // Multi-line strings (expression, flag, style)
    // FIXME: The triple-quotes in these two lines will mess up the
    // syntax highlighting from this point onward
    _triSingle = HighlightingRule("'''", 1, STYLES["string2"]);
    _triDouble = HighlightingRule("\"\"\"", 2, STYLES["string2"]);

    // Keyword, operator, and brace rules
    for(const QString &keyword : keywords)
    {
        QString pattern = QString("\\b%1\\b").arg(keyword);
        _pythonHighlightingRules += HighlightingRule(pattern, 0, STYLES["keyword"]);
    }

    for(const QString &pattern: operators)
        _pythonHighlightingRules += HighlightingRule(pattern, 0, STYLES["operator"]);

    for(const QString &pattern: braces)
        _pythonHighlightingRules += HighlightingRule(pattern, 0, STYLES["brace"]);

    // All other rules

    // 'self'
    _pythonHighlightingRules += HighlightingRule("\\bself\\b", 0, STYLES["self"]);

    // Double-quoted string, possibly containing escape sequences
    _pythonHighlightingRules += HighlightingRule(QStringLiteral("\"[^\"\\\\]*(\\\\.[^\"\\\\]*)*\""), 0, STYLES["string"]);
    //_pythonHighlightingRules += HighlightingRule(QStringLiteral("\"[^\"\\]*(\\\\.[^\"\\]*)*\""), 0, STYLES["string"]);
    // Single-quoted string, possibly containing escape sequences
    _pythonHighlightingRules += HighlightingRule(QStringLiteral("\"[^'\\\\]*(\\\\.[^'\\\\]*)*\""), 0, STYLES["string"]);
    //_pythonHighlightingRules += HighlightingRule(QStringLiteral("\"[^'\\]*(\\\\.[^'\\]*)*\""), 0, STYLES["string"]);

    // 'def' followed by an identifier
    _pythonHighlightingRules += HighlightingRule("\\bdef\\b", 1, STYLES["defclass"]);
    // 'class' followed by an identifier
    _pythonHighlightingRules += HighlightingRule("\\bclass\\b", 1, STYLES["defclass"]);

    // From '#' until a newline
    _pythonHighlightingRules += HighlightingRule("#[^\\n]*", 0, STYLES["comment"]);

    // Numeric literals
    _pythonHighlightingRules += HighlightingRule("\\b[+-]?[0-9]+[lL]?\\b", 0, STYLES["numbers"]);
    _pythonHighlightingRules += HighlightingRule("\\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\\b", 0, STYLES["numbers"]);
    _pythonHighlightingRules += HighlightingRule("\\b[+-]?[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\\b", 0, STYLES["numbers"]);
}

void PythonHighlighter::highlightPythonBlock(const QString &text)
{
    if(text.isEmpty())
        return;

    //int index = -1;

    // Do other syntax formatting
    for (const HighlightingRule &rule : qAsConst(_pythonHighlightingRules)) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }

    setCurrentBlockState(0);

    // Do multi-line strings
    bool in_multiline = matchMultiLine(text, _triSingle.pattern, 1, _triSingle.format);
    if(!in_multiline)
        in_multiline = matchMultiLine(text, _triDouble.pattern, 2, _triDouble.format);
}

// Return a QTextCharFormat with the given attributes.
QTextCharFormat PythonHighlighter::format(const QString &colorName, const QString &style)
{
    QColor color;
    color.setNamedColor(colorName);

    QTextCharFormat format;
    format.setForeground(color);

    if(style.contains("bold"))
        format.setFontWeight(QFont::Bold);
    if(style.contains("italic"))
        format.setFontItalic(true);

    return format;
}

bool PythonHighlighter::matchMultiLine(const QString &text, const QRegularExpression &delimiter, const int inState, const QTextCharFormat &style)
{
    int start  = -1;
    int add    = -1;
    int end    = -1;
    int length =  0;

    // If inside triple-single quotes, start at 0
    QRegularExpressionMatch match = delimiter.match(text);
    if(previousBlockState() == inState)
    {
        start = 0;
        add = 0;
    }
    // Otherwise, look for the delimiter on this line
    else
    {
        start = text.indexOf(delimiter);
        // Move past this match
        add = match.capturedLength();
    }

    // As long as there's a delimiter match on this line...
    while(start >= 0)
    {
        // Look for the ending delimiter
        end = text.indexOf(delimiter, start + add);
        // Ending delimiter on this line?
        if(end >= add)
        {
            length = end - start + add + match.capturedLength();
            setCurrentBlockState(0);
        }
        // No; multi-line string
        else
        {
            setCurrentBlockState(inState);
            length = text.length() - start + add;
        }

        // Apply formatting
        setFormat(start, length, style);

        // Look for the next match
        start = text.indexOf(delimiter, start + length);
    }

    // Return True if still inside a multi-line string, False otherwise
    if(currentBlockState() == inState)
        return true;
    else
        return false;
}
