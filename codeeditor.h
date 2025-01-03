#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QRegularExpression>

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
QT_END_NAMESPACE

class LineNumberArea;

// CODEEDITOR

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    bool hasComment(const QString &_text, const QRegularExpression &_expression);
    bool isSingleLineComment(const QString &_text, const QRegularExpression &_expression);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightMultiLineComments(int newBlockCount);

private:
    QWidget *lineNumberArea;
    QColor colorCurrentBlock  = QColor("#1a2838");
    QColor colorLNABackground = QColor(37, 37, 37);
    QColor colorLNAColor      = QColor(102, 102, 102);
    QFont fontLNA             = QFont("Ubuntu Mono", 18);

};

// LINE NUMBER AREA
class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor)
    {}

    QSize sizeHint() const override
    {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *codeEditor;
};


#endif // CODEEDITOR_H
