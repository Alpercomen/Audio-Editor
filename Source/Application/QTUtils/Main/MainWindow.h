#pragma once
#include <QMainWindow> // Base class

class EditorWidget;

class MainWindow : public QMainWindow 
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget* parent = nullptr);

private:
	// helpers
	EditorWidget* mEditor = nullptr;

	void buildMenus();
	void buildToolbar();
};