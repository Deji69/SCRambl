#ifndef SCRAMBLUI_H
#define SCRAMBLUI_H

#include <QtWidgets/QMainWindow>
#include "ui_scramblui.h"
#include "scrambl_opts.h"
#include "highlighter.h"

class SCRamblUI : public QMainWindow
{
	Q_OBJECT

public:
	SCRamblUI(QWidget *parent = 0);
	~SCRamblUI();

	static SCRamblUI *	mainWindow();

public slots:
	void newFile();
	void openFile(const QString &path = QString());
	void showOptions();

private:
	void setupEditor();
	void setupActions();

	void						setMainWindow(SCRamblUI *);

	Ui::SCRamblUIClass			ui;
	QPlainTextEdit *			editor;
	SCRamblOptions				options;
};

#endif // SCRAMBLUI_H
