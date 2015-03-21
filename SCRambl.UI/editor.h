#ifndef EDITOR_H
#define EDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include "highlighter.h"
#include "config.h"
#include "parser.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class CodeEditor;

class CodeEditorGutter : public QWidget
{
public:
	CodeEditorGutter(CodeEditor*);
	QSize sizeHint() const;

protected:
	void paintEvent(QPaintEvent *) override;

private:
	CodeEditor	*		codeEditor;
};

class CodeEditTip : public QWidget
{
public:
	CodeEditTip(CodeEditor*);
	QSize sizeHint() const;

	void setText(const QString&);
	void setTipRect(const QRect&);

protected:
	void paintEvent(QPaintEvent *) override;

private:
	CodeEditor	*		codeEditor;
	QRect				tipRect;
	QString				tipText;
};

class CodeEditor : public QPlainTextEdit, public Configurable
{
	Q_OBJECT

public:
	class Configuration : public ConfigBase {
	public:
		QTextCharFormat			DefaultFont;

		Configuration(Configurable *configurable, bool isTemp = false) : ConfigBase("Editor", configurable, isTemp) {
			// apply defaults
			DefaultFont.setFontFamily("Courier");
			DefaultFont.setFontPointSize(10);

			// load, save
			ConfigBase::Load();
			ConfigBase::Save();
		}
		~Configuration() {
			ConfigBase::Save();
		}

	private:
		void Load(const QSettings& settings) override
		{
			DefaultFont.setFontFamily(settings.value("Font/Family", DefaultFont.fontFamily()).toString());
			DefaultFont.setFontPointSize(settings.value("Font/Size", DefaultFont.fontPointSize()).toInt());
			DefaultFont.setFontWeight(settings.value("Font/Bold", DefaultFont.fontWeight()).toInt());
			DefaultFont.setFontItalic(settings.value("Font/Italic", DefaultFont.fontItalic()).toBool());
			DefaultFont.setFontUnderline(settings.value("Font/Underline", DefaultFont.fontUnderline()).toBool());
			DefaultFont.setFontStrikeOut(settings.value("Font/Strikeout", DefaultFont.fontStrikeOut()).toBool());
		}

		void Save(QSettings& settings) override
		{
			settings.setValue("Font/Family", DefaultFont.fontFamily());
			settings.setValue("Font/Size", DefaultFont.fontPointSize());
			settings.setValue("Font/Bold", DefaultFont.fontWeight());
			settings.setValue("Font/Italic", DefaultFont.fontItalic());
			settings.setValue("Font/Underline", DefaultFont.fontUnderline());
			settings.setValue("Font/Strikeout", DefaultFont.fontStrikeOut());
		}
	};

	CodeEditor(QWidget * = nullptr);
	~CodeEditor();

	inline const Configuration& Config() const			{ return config; }
	void gutterPaintEvent(QPaintEvent*);
	void lineNumberAreaPaintEvent(QPaintEvent*);
	int gutterWidth() const;
	void parseCode();

	void onConfigUpdate() override;

protected:
	void resizeEvent(QResizeEvent *) override;
	void paintEvent(QPaintEvent *) override;
	//void mouseMoveEvent(QMouseEvent *) override;
	bool eventFilter(QObject *, QEvent *) override;

public slots:
	void updateGutterWidth(int);
	void updateHighlighter();
	void updateGutter(const QRect &, int);
	void updateTip(const QRect &, int);
	void onCodeChange();

private:
	Configuration					config;
	CodeEditorGutter				gutter;
	CodeEditTip					*	editTip;
	Highlighter						highlighter;
	QTimer *						parseTimer;
	QPair<int, int>					gutterBlockCountCache = QPair<int, int>(-1, -1);
	int								codeMarginLeft = 15;
	int								codeMarginTop = 0;
	int								codeMarginBottom = 0;
	int								codeMarginRight = 0;
};

class CodeEditorPreview : public QPlainTextEdit, public Configurable
{
	Q_OBJECT

public:
	CodeEditorPreview(QWidget * = nullptr, CodeEditor::Configuration * = nullptr, Highlighter::Configuration * = nullptr);
	void onConfigUpdate() override;

public slots:

private:
	CodeEditor::Configuration	*	config;
	Highlighter						highlighter;

};

#endif