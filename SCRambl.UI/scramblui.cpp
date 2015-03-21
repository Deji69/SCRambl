#include "stdafx.h"
#include "scramblui.h"

SCRamblUI * gMainWindow;

SCRamblUI * SCRamblUI::mainWindow() {
	return gMainWindow;
}
void SCRamblUI::setMainWindow(SCRamblUI * wind) {
	gMainWindow = wind;
}

void SCRamblUI::setupEditor()
{
	QFont font;
	font.setFamily("Courier");
	font.setStyleHint(QFont::Monospace);
	font.setFixedPitch(true);
	font.setPointSize(10);
	QFontMetrics metrics(font);

	editor = ui.plainTextEdit;
	editor->setFont(font);
	
	editor->setTabStopWidth(4 * metrics.width(' '));
}

void SCRamblUI::setupActions()
{
	// Toolbar File Buttons
	connect(ui.actionNew, SIGNAL(triggered()), this, SLOT(newFile()));
	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));

	// Tools Menu
	connect(ui.action_Options, SIGNAL(triggered()), this, SLOT(showOptions()));
}

void SCRamblUI::newFile()
{
	editor->setPlainText("");
}

void SCRamblUI::openFile(const QString& path)
{
	QString fileName = path;

	if (fileName.isNull())
		fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", "SCR Files (*.sc)");
	if (!fileName.isEmpty()) {
		QFile file(fileName);
		if (file.open(QFile::ReadOnly | QFile::Text))
			editor->setPlainText(file.readAll());
	}
}

void SCRamblUI::showOptions()
{
	// Save current settings, just in case
	GlobalConfig().Save();

	// Request new settings, load old ones on cancel
	auto r = options.exec();
	if (r == QDialog::Rejected) GlobalConfig().Load();
	else GlobalConfig().Save();
	options.updateConfigDisplay();
}

SCRamblUI::SCRamblUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setMainWindow(this);
	setupEditor();
	setupActions();
	GlobalConfig().Load();
}

SCRamblUI::~SCRamblUI()
{
	options.close();
}
