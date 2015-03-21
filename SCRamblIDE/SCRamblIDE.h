#ifndef SCRAMBLIDE_H
#define SCRAMBLIDE_H

#include <QtWidgets/QMainWindow>
#include "ui_SCRamblIDE.h"

class SCRamblIDE : public QMainWindow
{
	Q_OBJECT

public:
	SCRamblIDE(QWidget *parent = 0);
	~SCRamblIDE();

private:
	Ui::SCRamblIDEClass ui;
};

#endif // SCRAMBLIDE_H
