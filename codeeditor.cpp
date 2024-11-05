#include "codeeditor.h"

#include <QPainter>
#include <QTextBlock>

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::highlightMultiLineComments);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}


int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 20 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}




void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}


void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

bool CodeEditor::hasComment(const QString &_text, const QRegularExpression &_expression)
{
    return _expression.match(_text, 0).hasMatch();
}

bool CodeEditor::isSingleLineComment(const QString &_text, const QRegularExpression &_expression)
{
    QRegularExpressionMatchIterator matchIterator = _expression.globalMatch(_text);
    int matchCount = 0;
    while (matchIterator.hasNext()) {
        matchIterator.next();
        matchCount++;
    }

    return (matchCount > 1) ? true : false;
}

void CodeEditor::highlightMultiLineComments(int newBlockCount)
{
    const QRegularExpression triSingle = QRegularExpression(QStringLiteral("'''"));
    bool inComment = false;
    int commentNumber = 0;
    for (QTextBlock _block = document()->begin(); _block != document()->end(); _block = _block.next()) {
        bool blockHasComment    = hasComment( _block.text(), triSingle );
        if ( !inComment && blockHasComment )
        {
            // START COMMENT
            if (isSingleLineComment( _block.text(), triSingle )) {
                // IS SINGLELINE COMMENT
                inComment = false;
            } else {
                // IS MULTILINE COMMENT
                inComment = true;
            }
        }
        else if ( inComment && !blockHasComment )
        {
            // COMMENT CONTINUE
            continue;
        }
        else if ( inComment && blockHasComment )
        {
            // END COMMENT
            // DO STUFF
            inComment = false;
            commentNumber++;

        }
    }
}


void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}


void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(colorCurrentBlock);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}


void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), colorLNABackground);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QPen(colorLNAColor));
            painter.setFont(QFont(fontLNA));
            painter.drawText(-10, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
