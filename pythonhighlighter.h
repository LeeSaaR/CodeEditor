#ifndef PYTHONHIGHLIGHTER_H
#define PYTHONHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class PythonHighlighter : public QSyntaxHighlighter
{
public:
    explicit PythonHighlighter(QTextDocument *parent = nullptr);

    // Helper
    static QTextCharFormat format(const QString &colorName, const QString &style = QString());

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
        int matchIndex = 0;

        HighlightingRule() { }
        HighlightingRule(const QRegularExpression &r, int i, const QTextCharFormat &f) : pattern(r), format(f), matchIndex(i) { }
        HighlightingRule(const QString &p, int i, const QTextCharFormat &f) : pattern(QRegularExpression(p)), format(f), matchIndex(i) { }
    };

    void initialize();
    void highlightPythonBlock(const QString &text);
    bool matchMultiLine(const QString &text, const QRegularExpression &delimiter, const int inState, const QTextCharFormat &style);

    QVector<HighlightingRule> _pythonHighlightingRules;
    HighlightingRule _triSingle, _triDouble;
};

#endif // PYTHONHIGHLIGHTER_H
