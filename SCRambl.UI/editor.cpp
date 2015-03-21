#include "stdafx.h"
#include <QtGui>
#include "scramblui.h"
#include "editor.h"
#include "highlighter.h"
#include "parser.h"

CodeEditor::CodeEditor(QWidget *widge) : QPlainTextEdit(widge),
	config(this), gutter(this), highlighter(this->document()), parseTimer(nullptr)
{
	editTip = new CodeEditTip(this);

	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateGutterWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateGutter(QRect, int)));
	connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateTip(QRect, int)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateHighlighter()));
	connect(this, SIGNAL(textChanged()), this, SLOT(onCodeChange()));
	//connect(this, SIGNAL(textChanged()), this, SLOT(update()));
	//connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(update()));

	updateGutterWidth(0);
	updateHighlighter();

	highlighter.SetDefaultFormat(Config().DefaultFont);

	//SCRambl::Script script;
	//script.SetCode(document()->toPlainText().toUtf8().constData());
	setMouseTracking(true);
	installEventFilter(this);
}
CodeEditor::~CodeEditor()
{
	if (parseTimer) delete parseTimer;
}

CodeEditorPreview::CodeEditorPreview(QWidget *widge, CodeEditor::Configuration * pconfig, Highlighter::Configuration * phlconfig) : QPlainTextEdit(widge),
	config(pconfig), highlighter(this->document(), phlconfig)
{
}

void CodeEditor::paintEvent(QPaintEvent * pe)
{
	lineNumberAreaPaintEvent(pe);
	//editTip->repaint();

	QPlainTextEdit::paintEvent(pe);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *)
{
}

/*void CodeEditor::mouseMoveEvent(QMouseEvent * me)
{
	auto cur = cursorForPosition(me->globalPos());
	cur.select(QTextCursor::SelectionType::WordUnderCursor);

	auto frag = cur.selection();
	auto frags = frag.toPlainText();

	if (!cur.isNull()) {
		auto str = cur.selectedText();
		static QString ol_string;
		if (!str.isEmpty() && ol_string != str) {
			ol_string = str;
		}
	}
}*/

bool CodeEditor::eventFilter(QObject * obj, QEvent * ev)
{
	auto type = ev->type();
	static QLabel * statuslabel = new QLabel();
	static bool badded = false;
	if (!badded && SCRamblUI::mainWindow()) {
		SCRamblUI::mainWindow()->statusBar()->addPermanentWidget(statuslabel);
		badded = true;
	}

	auto curpos = mapFromGlobal(QCursor::pos());
	setViewportMargins(gutterWidth() + codeMarginLeft, 0, 0, 0);
	statuslabel->setText(QString("%1 x %2").arg(curpos.x()).arg(curpos.y()));

	if (ev->type() == QEvent::ToolTip) {
		QHelpEvent* helpEvent = static_cast<QHelpEvent*>(ev);

		int gutterwidth = gutterWidth()  + codeMarginLeft;

		QTextCursor cursor = cursorForPosition(QPoint(helpEvent->pos().x() - gutterwidth, helpEvent->pos().y()));
		//cursor.select(QTextCursor::SelectionType::WordUnderCursor);
		
		QTextCursor tmpCur = cursor;
		tmpCur.select(QTextCursor::WordUnderCursor);
		QTextBlock block = document()->findBlock(tmpCur.selectionEnd());
		QTextLayout * layout = block.layout();
		int p = tmpCur.positionInBlock();
		
		auto lay = block.layout();
		if (lay->isValidCursorPosition(p)) {
			QTextLine line = lay->lineForTextPosition(p);
			if (line.isValid()) {
				if (!cursor.movePosition(QTextCursor::MoveOperation::StartOfWord)) {
					cursor.movePosition(QTextCursor::MoveOperation::StartOfLine);
				}
				QRect rect = cursorRect(cursor);
				if (!cursor.movePosition(QTextCursor::MoveOperation::EndOfWord, QTextCursor::KeepAnchor)) {
					cursor.movePosition(QTextCursor::MoveOperation::EndOfLine, QTextCursor::KeepAnchor);
				}
				QRect rect2 = cursorRect(cursor);
				rect.setCoords(rect.x() + gutterwidth, rect.y() + rect.height(), rect.x() + rect.width() + gutterwidth + (rect2.x() - rect.x()), rect.y() + rect.height() * 2);
				editTip->setTipRect(rect);
				editTip->setText(cursor.selectedText());

				QPointF point(rect.bottomLeft());
				SCRamblUI::mainWindow()->statusBar()->showMessage(QString("%1 x %2 = ").arg(point.x()).arg(point.y()) + QString("%1").arg(block.text().mid(line.textStart(), line.textLength())));
				QPoint gpos = point.toPoint();

				if (!cursor.selectedText().isEmpty())
					QToolTip::showText(gpos, cursor.selectedText());
				else
					QToolTip::hideText();

				editTip->repaint();
				return QPlainTextEdit::eventFilter(obj, ev);
			}
		}
	}
	return QPlainTextEdit::eventFilter(obj, ev);
}

void CodeEditor::onCodeChange()
{
	GetProcessor().setCode(document()->toPlainText().toUtf8().constData());
	parseCode();
	/*static bool parseNeeded = false;
	parseNeeded = true;

	if (!parseTimer) {
		parseTimer = new QTimer(this);
		connect(parseTimer, &QTimer::timeout, [this](){
			if (parseNeeded) {
				GetProcessor().setCode(document()->toPlainText().toUtf8().constData());
				parseCode();
				parseNeeded = false;
			}
		});
		parseTimer->start();
	}*/
}

void CodeEditor::onConfigUpdate()
{
	QFont font = Config().DefaultFont.font();
	QFontMetrics metrics(font);
	setFont(font);
	setTabStopWidth(4 * metrics.width(' '));
}

void CodeEditorPreview::onConfigUpdate()
{
	QFont font = config->DefaultFont.font();
	QFontMetrics metrics(font);
	setFont(font);
	setTabStopWidth(4 * metrics.width(' '));
	highlighter.rehighlight();

	document()->setDefaultFont(font);
	verticalScrollBar()->setFont(font);
	horizontalScrollBar()->setFont(font);
}

void CodeEditor::parseCode()
{
	auto& processor = GetProcessor();
	processor.parseCode();
}

int CodeEditor::gutterWidth() const
{
	int digits = 1;
	int max = qMax(100, blockCount());

	while (max >= 10) {
		max /= 10;
		++digits;
	}

	return 8 + fontMetrics().width(QLatin1Char('9')) * digits;
}

void CodeEditor::updateGutterWidth(int)
{
	//verticalScrollBar()->setSingleStep(1);
	setViewportMargins(gutterWidth() + codeMarginLeft, codeMarginTop, codeMarginRight, codeMarginBottom);
}

void CodeEditor::updateTip(const QRect& rect, int dy)
{
	if (dy) editTip->scroll(0, dy);

	if (rect.contains(viewport()->rect())) {
		setViewportMargins(gutterWidth() + codeMarginLeft, codeMarginTop, codeMarginRight, codeMarginBottom);
	}
}

void CodeEditor::updateGutter(const QRect& rect, int dy)
{
	if (dy) gutter.scroll(0, dy);
	else if (gutterBlockCountCache.first != blockCount() || gutterBlockCountCache.second != textCursor().block().lineCount())
	{
		gutter.update(0, rect.y(), gutter.width(), rect.height());
		
		gutterBlockCountCache.first = blockCount();
		gutterBlockCountCache.second = textCursor().block().lineCount();
	}

	if (rect.contains(viewport()->rect()))
		updateGutterWidth(0);
}

void CodeEditor::updateHighlighter()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;

		QColor lineColor = QColor(Qt::yellow).lighter(100);

		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FrameBorder, 1);
		selection.format.setProperty(QTextFormat::FrameBorderStyle, QTextFrameFormat::BorderStyle_Solid);
		selection.format.setProperty(QTextFormat::FrameBorderBrush, QBrush(QColor(Qt::black)));
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	setExtraSelections(extraSelections);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
	QPlainTextEdit::resizeEvent(e);
	QRect cr = contentsRect();
	gutter.setGeometry(QRect(cr.left(), cr.top(), gutterWidth() + 8, cr.height()));
	editTip->setGeometry(QRect(cr.left(), cr.top(), cr.width(), cr.height()));
	QFontMetrics fm(font());
	this->verticalScrollBar()->setSingleStep(1);
}

void CodeEditor::gutterPaintEvent(QPaintEvent *e)
{
	QPainter painter(&gutter);
	painter.fillRect(e->rect(), Qt::white);
	updateGutterWidth(0);

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int)blockBoundingRect(block).height();

	while (block.isValid() && top <= e->rect().bottom()) {
		if (block.isVisible() && bottom >= e->rect().top()) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen(QColor(0, 140, 140));
			painter.drawText(-8, top, gutter.width() + 4, fontMetrics().height(), Qt::AlignRight | Qt::AlignVCenter, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int)blockBoundingRect(block).height();
		++blockNumber;
	}

	//QPainter painter(this);
}

CodeEditorGutter::CodeEditorGutter(CodeEditor* editor) : QWidget(editor)
{
	codeEditor = editor;
}

QSize CodeEditorGutter::sizeHint() const
{
	return QSize(codeEditor->gutterWidth(), 0);
}

void CodeEditorGutter::paintEvent(QPaintEvent *e) {
	codeEditor->gutterPaintEvent(e);
}

CodeEditTip::CodeEditTip(CodeEditor* editor) : QWidget(editor),
	codeEditor(editor)
{
	setEnabled(false);
	setAttribute(Qt::WA_TransparentForMouseEvents);
}

QSize CodeEditTip::sizeHint() const
{
	return QSize(0, 0);
}

void CodeEditTip::setText(const QString& txt)
{
	tipText = txt;
}

void CodeEditTip::setTipRect(const QRect& rect)
{
	tipRect = rect;
}

void CodeEditTip::paintEvent(QPaintEvent *e) {
	QPainter painter(this);
	painter.setBrush(Qt::black);
	auto fm = painter.fontMetrics();
	tipRect.setWidth(fm.width(tipText));
	auto boxRect = tipRect;
	boxRect.adjust(0, 0, 24, 0);
	painter.fillRect(boxRect, QColor(231, 232, 236, 255));
	painter.drawText(tipRect.topLeft() + QPoint(12, 16), tipText);
}