#pragma once
#include <Application/QTUtils/Main/MainWindow.h>
#include <Application/QTUtils/Widgets/Editor/EditorWidget.h>

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
	mEditor = new EditorWidget(this);
	setCentralWidget(mEditor);

	buildMenus();
	buildToolbar();

	statusBar()->showMessage("Ready");
	setWindowTitle("Audio Editor");
}

void MainWindow::buildMenus()
{
	auto* fileMenu = menuBar()->addMenu("&File");

	auto* openAct = new QAction("Open...", this);
	openAct->setShortcut(QKeySequence::Open);
	fileMenu->addAction(openAct);

	fileMenu->addSeparator();

	auto* quitAct = new QAction("Quit", this);
	quitAct->setShortcut(QKeySequence::Quit);
	fileMenu->addAction(quitAct);

	// TODO Fix this
	connect(quitAct, &QAction::triggered, this, &QWidget::close);
	connect(openAct, &QAction::triggered, this, [this] {
		statusBar()->showMessage("Open clicked (not implemented yet)", 2000);
		});
}

void MainWindow::buildToolbar()
{
	auto* toolBar = addToolBar("Transport");
	toolBar->setMovable(false);

	auto* playAct = toolBar->addAction("Play");
	auto* stopAct = toolBar->addAction("Stop");

	// TODO Fix this
	connect(playAct, &QAction::triggered, this, [this] {
		statusBar()->showMessage("Play (not implemented)", 1500);
		});

	// TODO Fix this
	connect(stopAct, &QAction::triggered, this, [this] {
		statusBar()->showMessage("Stop (not implemented)", 1500);
		});
}