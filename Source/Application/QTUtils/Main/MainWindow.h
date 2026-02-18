#pragma once
#include <QMainWindow> // Base class

namespace UI
{
	class EditorWidget;
	class MainWindow : public QMainWindow 
	{
		Q_OBJECT
	public:
		explicit MainWindow(QWidget* parent = nullptr);

	private:
		EditorWidget* pEditor = nullptr;

		void buildMenus();
		void buildToolbar();
	};
}