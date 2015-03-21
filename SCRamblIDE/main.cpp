#include "stdafx.h"
#include "SCRamblIDE.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	SCRamblIDE w;
	w.show();
	return a.exec();
}
