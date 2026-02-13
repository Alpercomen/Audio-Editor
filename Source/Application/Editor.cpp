#pragma once
#include <QApplication>
#include <Application/QTUtils/Main/MainWindow.h>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	MainWindow w;
	w.resize(1200, 700);
	w.show();
	return app.exec();
}