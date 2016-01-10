/********************************************************************************
** Form generated from reading UI file 'scramblui.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SCRAMBLUI_H
#define UI_SCRAMBLUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "editor.h"

QT_BEGIN_NAMESPACE

class Ui_SCRamblUIClass
{
public:
    QAction *actionNew;
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionSaveAll;
    QAction *action_Options;
    QAction *actionBuild;
    QAction *actionBuildRun;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *action_Output;
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QTabWidget *gridTabWidget;
    QWidget *gridTabWidgetPage1;
    QHBoxLayout *horizontalLayout;
    CodeEditor *plainTextEdit;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuTools;
    QMenu *menuView;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QToolBar *toolBar;
    QToolBar *toolBar_2;
    QDockWidget *dockWidget;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout;
    QFrame *frame;
    QComboBox *comboBox;
    QTableWidget *tableWidget;

    void setupUi(QMainWindow *SCRamblUIClass)
    {
        if (SCRamblUIClass->objectName().isEmpty())
            SCRamblUIClass->setObjectName(QStringLiteral("SCRamblUIClass"));
        SCRamblUIClass->resize(980, 700);
        SCRamblUIClass->setDocumentMode(true);
        SCRamblUIClass->setDockNestingEnabled(false);
        SCRamblUIClass->setDockOptions(QMainWindow::AllowTabbedDocks|QMainWindow::AnimatedDocks|QMainWindow::VerticalTabs);
        actionNew = new QAction(SCRamblUIClass);
        actionNew->setObjectName(QStringLiteral("actionNew"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/SCRamblUI/new_icon"), QSize(), QIcon::Normal, QIcon::Off);
        actionNew->setIcon(icon);
        actionOpen = new QAction(SCRamblUIClass);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/SCRamblUI/open_icon"), QSize(), QIcon::Normal, QIcon::Off);
        actionOpen->setIcon(icon1);
        actionSave = new QAction(SCRamblUIClass);
        actionSave->setObjectName(QStringLiteral("actionSave"));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/SCRamblUI/save_icon"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave->setIcon(icon2);
        actionSaveAll = new QAction(SCRamblUIClass);
        actionSaveAll->setObjectName(QStringLiteral("actionSaveAll"));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/SCRamblUI/saveall_icon"), QSize(), QIcon::Normal, QIcon::Off);
        actionSaveAll->setIcon(icon3);
        action_Options = new QAction(SCRamblUIClass);
        action_Options->setObjectName(QStringLiteral("action_Options"));
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/SCRamblUI/options_icon"), QSize(), QIcon::Normal, QIcon::Off);
        action_Options->setIcon(icon4);
        actionBuild = new QAction(SCRamblUIClass);
        actionBuild->setObjectName(QStringLiteral("actionBuild"));
        QIcon icon5;
        icon5.addFile(QStringLiteral(":/SCRamblUI/build_icon"), QSize(), QIcon::Normal, QIcon::Off);
        actionBuild->setIcon(icon5);
        actionBuildRun = new QAction(SCRamblUIClass);
        actionBuildRun->setObjectName(QStringLiteral("actionBuildRun"));
        QIcon icon6;
        icon6.addFile(QStringLiteral(":/SCRamblUI/buildrun_icon"), QSize(), QIcon::Normal, QIcon::Off);
        actionBuildRun->setIcon(icon6);
        actionUndo = new QAction(SCRamblUIClass);
        actionUndo->setObjectName(QStringLiteral("actionUndo"));
        QIcon icon7;
        icon7.addFile(QStringLiteral(":/SCRamblUI/undo_icon"), QSize(), QIcon::Normal, QIcon::Off);
        actionUndo->setIcon(icon7);
        actionRedo = new QAction(SCRamblUIClass);
        actionRedo->setObjectName(QStringLiteral("actionRedo"));
        QIcon icon8;
        icon8.addFile(QStringLiteral(":/SCRamblUI/redo_icon"), QSize(), QIcon::Normal, QIcon::Off);
        actionRedo->setIcon(icon8);
        action_Output = new QAction(SCRamblUIClass);
        action_Output->setObjectName(QStringLiteral("action_Output"));
        action_Output->setCheckable(true);
        action_Output->setChecked(true);
        centralWidget = new QWidget(SCRamblUIClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridTabWidget = new QTabWidget(centralWidget);
        gridTabWidget->setObjectName(QStringLiteral("gridTabWidget"));
        gridTabWidget->setTabPosition(QTabWidget::North);
        gridTabWidget->setTabShape(QTabWidget::Rounded);
        gridTabWidget->setDocumentMode(true);
        gridTabWidget->setTabsClosable(true);
        gridTabWidget->setMovable(true);
        gridTabWidget->setTabBarAutoHide(false);
        gridTabWidgetPage1 = new QWidget();
        gridTabWidgetPage1->setObjectName(QStringLiteral("gridTabWidgetPage1"));
        horizontalLayout = new QHBoxLayout(gridTabWidgetPage1);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        plainTextEdit = new CodeEditor(gridTabWidgetPage1);
        plainTextEdit->setObjectName(QStringLiteral("plainTextEdit"));
        QFont font;
        font.setFamily(QStringLiteral("Courier New"));
        plainTextEdit->setFont(font);
        plainTextEdit->setStyleSheet(QLatin1String("*{\n"
"	\n"
"	selection-background-color: rgb(0, 255, 127);\n"
"}"));
        plainTextEdit->setFrameShadow(QFrame::Raised);
        plainTextEdit->setMidLineWidth(1);
        plainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        plainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        plainTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

        horizontalLayout->addWidget(plainTextEdit);

        gridTabWidget->addTab(gridTabWidgetPage1, QString());

        gridLayout->addWidget(gridTabWidget, 0, 0, 1, 1);

        SCRamblUIClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(SCRamblUIClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 980, 26));
        menuBar->setDefaultUp(false);
        menuBar->setNativeMenuBar(false);
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuTools = new QMenu(menuBar);
        menuTools->setObjectName(QStringLiteral("menuTools"));
        menuView = new QMenu(menuBar);
        menuView->setObjectName(QStringLiteral("menuView"));
        SCRamblUIClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(SCRamblUIClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        mainToolBar->setIconSize(QSize(28, 28));
        mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
        SCRamblUIClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(SCRamblUIClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        SCRamblUIClass->setStatusBar(statusBar);
        toolBar = new QToolBar(SCRamblUIClass);
        toolBar->setObjectName(QStringLiteral("toolBar"));
        toolBar->setIconSize(QSize(28, 28));
        SCRamblUIClass->addToolBar(Qt::TopToolBarArea, toolBar);
        toolBar_2 = new QToolBar(SCRamblUIClass);
        toolBar_2->setObjectName(QStringLiteral("toolBar_2"));
        SCRamblUIClass->addToolBar(Qt::TopToolBarArea, toolBar_2);
        dockWidget = new QDockWidget(SCRamblUIClass);
        dockWidget->setObjectName(QStringLiteral("dockWidget"));
        dockWidget->setFloating(false);
        dockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
        dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));
        dockWidgetContents->setAutoFillBackground(true);
        verticalLayout = new QVBoxLayout(dockWidgetContents);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        frame = new QFrame(dockWidgetContents);
        frame->setObjectName(QStringLiteral("frame"));
        frame->setMinimumSize(QSize(0, 24));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        comboBox = new QComboBox(frame);
        comboBox->setObjectName(QStringLiteral("comboBox"));
        comboBox->setGeometry(QRect(0, 0, 57, 22));
        comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        verticalLayout->addWidget(frame);

        tableWidget = new QTableWidget(dockWidgetContents);
        if (tableWidget->columnCount() < 2)
            tableWidget->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        tableWidget->setObjectName(QStringLiteral("tableWidget"));
        tableWidget->setAlternatingRowColors(true);

        verticalLayout->addWidget(tableWidget);

        dockWidget->setWidget(dockWidgetContents);
        SCRamblUIClass->addDockWidget(static_cast<Qt::DockWidgetArea>(8), dockWidget);
        dockWidget->raise();

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuView->menuAction());
        menuBar->addAction(menuTools->menuAction());
        menuTools->addAction(action_Options);
        menuView->addAction(action_Output);
        mainToolBar->addAction(actionNew);
        mainToolBar->addAction(actionOpen);
        mainToolBar->addAction(actionSave);
        mainToolBar->addAction(actionSaveAll);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionUndo);
        mainToolBar->addAction(actionRedo);
        toolBar->addAction(actionBuild);
        toolBar->addAction(actionBuildRun);

        retranslateUi(SCRamblUIClass);
        QObject::connect(action_Output, SIGNAL(triggered(bool)), dockWidget, SLOT(setVisible(bool)));
        QObject::connect(dockWidget, SIGNAL(visibilityChanged(bool)), action_Output, SLOT(setChecked(bool)));
        QObject::connect(actionUndo, SIGNAL(triggered()), plainTextEdit, SLOT(undo()));
        QObject::connect(actionRedo, SIGNAL(triggered()), plainTextEdit, SLOT(redo()));

        QMetaObject::connectSlotsByName(SCRamblUIClass);
    } // setupUi

    void retranslateUi(QMainWindow *SCRamblUIClass)
    {
        SCRamblUIClass->setWindowTitle(QApplication::translate("SCRamblUIClass", "SCRamblUI", 0));
        actionNew->setText(QApplication::translate("SCRamblUIClass", "&New", 0));
#ifndef QT_NO_TOOLTIP
        actionNew->setToolTip(QApplication::translate("SCRamblUIClass", "New File", 0));
#endif // QT_NO_TOOLTIP
        actionNew->setShortcut(QApplication::translate("SCRamblUIClass", "Ctrl+N", 0));
        actionOpen->setText(QApplication::translate("SCRamblUIClass", "&Open", 0));
#ifndef QT_NO_TOOLTIP
        actionOpen->setToolTip(QApplication::translate("SCRamblUIClass", "Open File", 0));
#endif // QT_NO_TOOLTIP
        actionOpen->setShortcut(QApplication::translate("SCRamblUIClass", "Ctrl+O", 0));
        actionSave->setText(QApplication::translate("SCRamblUIClass", "&Save", 0));
#ifndef QT_NO_TOOLTIP
        actionSave->setToolTip(QApplication::translate("SCRamblUIClass", "Save File", 0));
#endif // QT_NO_TOOLTIP
        actionSave->setShortcut(QApplication::translate("SCRamblUIClass", "Ctrl+S", 0));
        actionSaveAll->setText(QApplication::translate("SCRamblUIClass", "Save A&ll", 0));
#ifndef QT_NO_TOOLTIP
        actionSaveAll->setToolTip(QApplication::translate("SCRamblUIClass", "Save All", 0));
#endif // QT_NO_TOOLTIP
        actionSaveAll->setShortcut(QApplication::translate("SCRamblUIClass", "Ctrl+Shift+S", 0));
        action_Options->setText(QApplication::translate("SCRamblUIClass", "&Options", 0));
        action_Options->setShortcut(QApplication::translate("SCRamblUIClass", "F10", 0));
        actionBuild->setText(QApplication::translate("SCRamblUIClass", "&Build", 0));
#ifndef QT_NO_TOOLTIP
        actionBuild->setToolTip(QApplication::translate("SCRamblUIClass", "Build", 0));
#endif // QT_NO_TOOLTIP
        actionBuild->setShortcut(QApplication::translate("SCRamblUIClass", "F7", 0));
        actionBuildRun->setText(QApplication::translate("SCRamblUIClass", "&Build & Run", 0));
#ifndef QT_NO_TOOLTIP
        actionBuildRun->setToolTip(QApplication::translate("SCRamblUIClass", "Build & Run", 0));
#endif // QT_NO_TOOLTIP
        actionBuildRun->setShortcut(QApplication::translate("SCRamblUIClass", "F5", 0));
        actionUndo->setText(QApplication::translate("SCRamblUIClass", "&Undo", 0));
#ifndef QT_NO_TOOLTIP
        actionUndo->setToolTip(QApplication::translate("SCRamblUIClass", "Undo", 0));
#endif // QT_NO_TOOLTIP
        actionUndo->setShortcut(QApplication::translate("SCRamblUIClass", "Ctrl+Z", 0));
        actionRedo->setText(QApplication::translate("SCRamblUIClass", "&Redo", 0));
#ifndef QT_NO_TOOLTIP
        actionRedo->setToolTip(QApplication::translate("SCRamblUIClass", "Redo", 0));
#endif // QT_NO_TOOLTIP
        actionRedo->setShortcut(QApplication::translate("SCRamblUIClass", "Ctrl+Y", 0));
        action_Output->setText(QApplication::translate("SCRamblUIClass", "&Output", 0));
        action_Output->setShortcut(QApplication::translate("SCRamblUIClass", "Ctrl+Shift+O", 0));
#ifndef QT_NO_WHATSTHIS
        plainTextEdit->setWhatsThis(QApplication::translate("SCRamblUIClass", "<html><head/><body><p><br/></p></body></html>", 0));
#endif // QT_NO_WHATSTHIS
        plainTextEdit->setPlainText(QApplication::translate("SCRamblUIClass", "/*********************************************\n"
"* GInput CLEO Extension\n"
"* Tactile Cheat Activation\n"
"* written by Deji\n"
"* http://gtag.gtagaming.com\n"
"* Compile with SCRambl (v1.0.41 or later)\n"
"*********************************************/\n"
"\n"
"#pushcfg \"CaseConversion\" \"NONE\"\n"
"#register_var 0x8 VAR_INT player\n"
"#register_var 0xC VAR_INT scplayer\n"
"#define FUNC_AddToCheatString 0x438480\n"
"#define FUNC_GetModuleHandle 0x81E406\n"
"#define REF_CheatHashes 0x438513\n"
"\n"
"// Compile with this set to 1 to enable a list of buttons in the cheat string\n"
"#define SHOW_INPUT 0\n"
"#if SHOW_INPUT\n"
"	#define FUNC_strlen 0x826330\n"
"	#define MEM_CheatString 0x969110\n"
"#endif\n"
"\n"
"/*\n"
"	/* lecture */\n"
"	Scripts are run before the game fully starts to do set-ups like in the 'INIT' missions\n"
"	or continue to the beginning of a loop in a saved script) until the first WAIT\n"
"	So it's a good idea to always WAIT, unless we really need that\n"
"*/\n"
"WAIT 0\n"
"\n"
"{\n"
"	L"
                        "VAR_INT pCheatTable pTranslation pButtons hGInput GInput_GetSourceButtonInput\n"
"	GET_LABEL_POINTER (cheats) pCheatTable\n"
"	GET_LABEL_POINTER (buttons) pButtons\n"
"	GET_LABEL_POINTER (translation) pTranslation\n"
"	\n"
"	// Get the module handle for GInput.asi\n"
"	CALL_FUNCTION_RETURN (FUNC_GetModuleHandle, 1, 0, STRING:\"GInput.asi\") hGInput\n"
"\n"
"	// If not found, wait to warn the user\n"
"	IF hGInput = 0\n"
"		WHILE NOT IS_PLAYER_PLAYING (player)\n"
"		OR NOT IS_PLAYER_CONTROL_ON (player)\n"
"		OR IS_HELP_MESSAGE_BEING_DISPLAYED\n"
"			WAIT 0\n"
"		ENDWHILE\n"
"		PRINT_HELP_STRING \"Tactile Cheat Activation~n~~r~Error: GInput.asi not found!\"\n"
"		TERMINATE_THIS_CUSTOM_SCRIPT\n"
"	ENDIF\n"
"	\n"
"	// Get the only func we need and save it\n"
"	GET_DYNAMIC_LIBRARY_PROCEDURE (\"_GInput_GetSourceButtonInput@8\", hGInput) GInput_GetSourceButtonInput\n"
"	\n"
"main:\n"
"	WAIT 0\n"
"	\n"
"	IF IS_PLAYER_PLAYING (player)\n"
"		LVAR_INT counter pAddr button temp_val1 temp_val2\n"
"		\n"
"		// Check for butt"
                        "on presses\n"
"		REPEAT 14 counter\n"
"			button = counter + LEFTSHOULDER1\n"
"			\n"
"			IF NOT button == START\n"
"			AND NOT button == SELECT\n"
"				// Will return the exact input of a specific button, disregarding situational mapping\n"
"				CALL_FUNCTION_RETURN (GInput_GetSourceButtonInput, 2, 0, button, player) temp_val1\n"
"				\n"
"				// Get the address of the \"pressed buttons\" array, read the current value, write the new one\n"
"				pAddr = pButtons + counter\n"
"				READ_MEMORY (pAddr, 1, FALSE) temp_val2\n"
"				WRITE_MEMORY (pAddr, 1, temp_val1, FALSE)\n"
"				\n"
"				// If it wasn't already in the \"pressed buttons\" array, add it to the cheat string\n"
"				IF NOT temp_val1 = 0\n"
"				AND temp_val2 = 0\n"
"					// Find the appropriate ASCII character for this button so it can be hashed\n"
"					pAddr = pTranslation + counter\n"
"					READ_MEMORY (pAddr, 1, FALSE) temp_val2\n"
"					\n"
"					// Temporarily replace the cheat hashes array - the cheats will be only usable directly through this"
                        " script\n"
"					READ_MEMORY (REF_CheatHashes, 4, TRUE) temp_val1\n"
"					WRITE_MEMORY (REF_CheatHashes, 4, pCheatTable, TRUE)\n"
"					\n"
"					// Pass to the func which handles PC cheats - it'll take care of everything (R* used the same method)\n"
"					CALL_FUNCTION (FUNC_AddToCheatString, 1, 1, temp_val2)\n"
"					\n"
"					// Restore the cheat hashes so PC cheats can be entered, other scripts can also have custom cheats, etc.\n"
"					WRITE_MEMORY (REF_CheatHashes, 4, temp_val1, TRUE)\n"
"				ENDIF\n"
"			ENDIF\n"
"		ENDREPEAT\n"
"		\n"
"		#if SHOW_INPUT\n"
"			// For, like, taking a video of this script and showing how the cheats are being entered...\n"
"			LVAR_INT len pCheatText pCheatEnd\n"
"			\n"
"			// Get the cheat string length and continue if we have anything to print\n"
"			CALL_FUNCTION_RETURN (FUNC_strlen, 1, 1, MEM_CheatString) len\n"
"			IF len > 0\n"
"				GET_LABEL_POINTER (cheat_text) pCheatText\n"
"				\n"
"				// Only show the last 16 buttons\n"
"				IF len > 16\n"
"					len = 16\n"
""
                        "				ENDIF\n"
"				\n"
"				// The cheat string is in reverse, so start from the end...\n"
"				pCheatEnd = MEM_CheatString + len\n"
"				WHILE pCheatEnd > MEM_CheatString\n"
"					--pCheatEnd\n"
"					--len\n"
"					\n"
"					// [space]\n"
"					WRITE_MEMORY (pCheatText, 1, 0x20, FALSE)\n"
"					++pCheatText\n"
"					\n"
"					// Read the next/last char\n"
"					READ_MEMORY (pCheatEnd, 1, FALSE) temp_val1\n"
"					\n"
"					// Convert the cheat string char to a GXT symbol - which GInput will convert to a texture\n"
"					IF NOT temp_val1 == 0x31		// 1\n"
"						IF NOT temp_val1 == 0x32		// 2\n"
"							IF NOT temp_val1 == 0x33		// 3\n"
"								IF NOT temp_val1 == 0x34		// 4\n"
"									IF NOT temp_val1 == 0x55		// U(p)\n"
"										IF NOT temp_val1 == 0x44		// D(own)\n"
"											IF NOT temp_val1 == 0x4C		//  L(eft)\n"
"												IF NOT temp_val1 == 0x52		// R(ight)\n"
"													IF NOT temp_val1 == 0x53		// S(quare)\n"
"														IF NOT temp_val1 == 0x54		// T(riangle)\n"
"															IF NOT "
                        "temp_val1 == 0x58		// X\n"
"																IF NOT temp_val1 == 0x43		// C(ircle)\n"
"																	// it looks hideous...\n"
"																	//WRITE_MEMORY (pCheatText, 1, temp_val1, FALSE)\n"
"																	//++pCheatText\n"
"																	--pCheatText\n"
"																	GOTO showinput_continue\n"
"																ELSE\n"
"																	temp_val2 = 0x6F	// o\n"
"																ENDIF\n"
"															ELSE\n"
"																temp_val2 = 0x78	// x\n"
"															ENDIF\n"
"														ELSE\n"
"															temp_val2 = 0x74	// t\n"
"														ENDIF\n"
"													ELSE\n"
"														temp_val2 = 0x71	// q\n"
"													ENDIF\n"
"												ELSE\n"
"													temp_val2 = 0x3E	// r\n"
"												ENDIF\n"
"											ELSE\n"
"												temp_val2 = 0x3C	// l\n"
"											ENDIF\n"
"										ELSE\n"
"											temp_val2 = 0x64	// d\n"
"										ENDIF\n"
"									ELSE\n"
"										temp_val2 = 0x75	// u\n"
"									ENDIF\n"
"								ELSE\n"
"									temp_val2 = 0x76"
                        "	// v\n"
"								ENDIF\n"
"							ELSE\n"
"								temp_val2 = 0x6A	// j\n"
"							ENDIF\n"
"						ELSE\n"
"							temp_val2 = 0x6D	// m\n"
"						ENDIF\n"
"					ELSE\n"
"						temp_val2 = 0x6B	// k\n"
"					ENDIF\n"
"					\n"
"					// Concatenate the GXT symbol '~%c~' (where %c is the character we just got above, duh)\n"
"					WRITE_MEMORY (pCheatText, 1, 0x7E, FALSE)\n"
"					++pCheatText\n"
"					WRITE_MEMORY (pCheatText, 1, temp_val2, FALSE)\n"
"					++pCheatText\n"
"					WRITE_MEMORY (pCheatText, 1, 0x7E, FALSE)\n"
"					++pCheatText\n"
"					\n"
"					showinput_continue:\n"
"				ENDWHILE\n"
"				\n"
"				// \\0 at the end, standard...\n"
"				WRITE_MEMORY (pCheatText, 1, 0, FALSE)\n"
"				\n"
"				// Add the text label\n"
"				GET_LABEL_POINTER (cheat_text) pCheatText\n"
"				ADD_TEXT_LABEL (\"_TAC\", pCheatText)\n"
"			ELSE\n"
"				ADD_TEXT_LABEL (\"_TAC\", \"\")\n"
"			ENDIF\n"
"			\n"
"			// Print out the list\n"
"			USE_TEXT_COMMANDS TRUE\n"
"			SET_TEXT_SCALE (2.0, 2.0)\n"
"			SET_TEXT_SIZE 1.0\n"
""
                        "			DISPLAY_TEXT (8.0, 10.0, _TAC)\n"
"		#endif\n"
"	ENDIF\n"
"	\n"
"	GOTO main\n"
"}\n"
"\n"
"#if SHOW_INPUT\n"
"	// if we're showing input, this gives us space to fit the GXT keys\n"
"	#hex\n"
"	cheat_text:\n"
"	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00		// 32\n"
"	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00		// 64\n"
"	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00		// 96\n"
"	#endhex\n"
"#endif\n"
"\n"
"// Space to store which buttons are currently held, so we dont repeatedly add it\n"
"#hex\n"
"buttons:\n"
"00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00			// 16\n"
"#endhex\n"
"\n"
"/*\n"
"	GTASA PS2 Cheat Hashes\n"
"	Recovered from the PS2 GTASA binary.\n"
"	Cheats are encrypted by reversing a string (so the input string doesn't need reversing every time)\n"
"	and calculating a CRC hash from it. The string is formatted depending on the buttons to press:"
                        "\n"
"		X = Cross\n"
"		T = Triangle\n"
"		C = Circle\n"
"		S = Square\n"
"		U = Up\n"
"		D = Down\n"
"		L = Left\n"
"		R = Right\n"
"		1 = L1\n"
"		2 = L2\n"
"		3 = R1\n"
"		4 = R2\n"
"	e.g. for the weapon cheat (first in the hash table), hashing \"URDLURDL4143\" will produce 0x518BAD1E.\n"
"	\n"
"	GTASA was the first (and perhaps last) game in the series to use hashed cheats. This means that there\n"
"	are a huge number of possible combinations (over 200,000 confirmed) which can produce the same hash.\n"
"*/\n"
"#hex\n"
"cheats:\n"
"1EAD8B51 FB747986 05027580 A5E57B20 A49EF197 	// 0\n"
"E56ECF4D 1B17F6D8 30264DA4 021E33D2 292F88AE	// 5\n"
"5402598F C91A176A 0DE5EDB4 A666C4CB E09B9F7F 	// 10\n"
"281603F0 0292842A 0D295494 EEEEA1E6 E47DBBE5 	// 15\n"
"875C86B1 B76C60D9 1FF833B5 DDA2ECE3 817C02F8 	// 20\n"
"2B4F181B 546C6509 B194B562 FBA8B3C9 5213EE53 	// 25\n"
"E116C1BA BFB3A08F 8A93579C 88F72076 98F427FD 	// 30\n"
"70BBE456 B5525F9A 0049BA49 196A8B20 4E854486 	// 35\n"
"2013E451 94F19BE8 CF20CCC3 B78FC99B 773B"
                        "9521 	// 40\n"
"BDB08091 A965D112 4E6D2E9C F498B87E 490D5A82 	// 45\n"
"1CB73F10 B3FB6C9A 4D9A372A 57D0F8A6 33958F68 	// 50\n"
"54E44076 599435C3 0DD6D4E1 63C154D3 9B456E4E 	// 55\n"
"C60903ED 2D27C6F0 4FC21EC3 5FA8EFF3 E5310EB4 	// 60\n"
"CD041833 0C5FA944 F45180C5 07ECFFF5 AEF732CB 	// 65\n"
"2104BCF9 675C20F9 7C374CF2 B4736264 9A2CDF20 	// 70\n"
"2053E2CD FC6C817F 45E3CFA3 31830183 E2865BF3 	// 75\n"
"9EC12E53 9B1CC977 4E2B51E2 7A2D4EBC 1566F262 	// 80\n"
"D3576DF9 E11C79D5 333443EB 860376FA 11A7E1AC 	// 90\n"
"00000000 00000000 00000000 00000000 00000000	// 95\n"
"#endhex\n"
"#hex\n"
"translation:\n"
"// LEFTSHOULDER1-CIRCLE\n"
"\"1\" \"2\" \"3\" \"4\" \"U\" \"D\" \"L\" \"R\" 00 00 \"S\" \"T\" \"X\" \"C\" 00\n"
"#endhex\n"
"", 0));
        gridTabWidget->setTabText(gridTabWidget->indexOf(gridTabWidgetPage1), QString());
        menuFile->setTitle(QApplication::translate("SCRamblUIClass", "&File", 0));
        menuTools->setTitle(QApplication::translate("SCRamblUIClass", "Tools", 0));
        menuView->setTitle(QApplication::translate("SCRamblUIClass", "&View", 0));
#ifndef QT_NO_ACCESSIBILITY
        mainToolBar->setAccessibleName(QApplication::translate("SCRamblUIClass", "Hello", 0));
#endif // QT_NO_ACCESSIBILITY
#ifndef QT_NO_STATUSTIP
        statusBar->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
        toolBar->setWindowTitle(QApplication::translate("SCRamblUIClass", "toolBar", 0));
        toolBar_2->setWindowTitle(QApplication::translate("SCRamblUIClass", "toolBar_2", 0));
        dockWidget->setWindowTitle(QApplication::translate("SCRamblUIClass", "Output", 0));
        comboBox->clear();
        comboBox->insertItems(0, QStringList()
         << QApplication::translate("SCRamblUIClass", "Build", 0)
        );
        QTableWidgetItem *___qtablewidgetitem = tableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("SCRamblUIClass", "Position", 0));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("SCRamblUIClass", "Message", 0));
    } // retranslateUi

};

namespace Ui {
    class SCRamblUIClass: public Ui_SCRamblUIClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SCRAMBLUI_H
