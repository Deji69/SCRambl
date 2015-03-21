#include "stdafx.h"
#include "pugixml.hpp"
#include <QtGui>
#include <QApplication>
#include "scramblui.h"

/*void SetupConfigEditor(pugi::xml_node node)
{

}

void SetupConfigDefinitions(pugi::xml_node node)
{
	auto buildConfig = node.append_child("BuildConfig");
	auto attr = buildConfig.append_attribute("Path");
	attr = "";
}

void SetupConfig(pugi::xml_document& doc)
{
	doc.reset();
	auto root = doc.append_child("SCRamblUI");
	auto attr = root.append_attribute("Version");
	attr = "1.0";
	SetupConfigDefinitions(root.append_child("Definitions"));
	SetupConfigEditor(root.append_child("Editor"));
}*/

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	SCRamblUI ide;
	pugi::xml_document xml;
	bool has_file = false;

	if (xml.load_file("config.xml") != pugi::status_ok)
		has_file = true;
	
	ide.setWindowTitle(QObject::tr("SCRambl UI"));
	ide.showMaximized();
	
	return app.exec();
}