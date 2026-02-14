#pragma once
#include "MainWindow.h"

#include <Application/QTUtils/Widgets/Editor/EditorWidget.h>
#include <Application/QTUtils/Widgets/Waveform/WaveformView.h>
#include <Application/Audio/Document/AudioDocument.h>

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QFileDialog>
#include <QString>
#include <QMessageBox>

namespace UI
{
	MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
	{
		pEditor = new EditorWidget(this);
		setCentralWidget(pEditor);

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

		connect(quitAct, &QAction::triggered, this, &QWidget::close);
		connect(openAct, &QAction::triggered, this, [this] {
			const QString file = QFileDialog::getOpenFileName(
				this,
				"Open Audio",
				QString(),
				"Audio Files (*.wav *.miff *.flac *.ogg);;All Files (*,*)"
			);

			if (file.isEmpty())
				return;

			std::string err;
			Audio::AudioDocument doc = Audio::AudioDocument::LoadFromFile(file.toStdString(), err);

			if (!doc.isValid())
			{
				QMessageBox::critical(this, "Failed to load", QString::fromStdString(err));
				return;
			}

			pEditor->setDocument(std::move(doc), file);
			statusBar()->showMessage(
				QString("Loaded: %1 | %2 Hz | %3 ch | %4 sec")
				.arg(file)
				.arg(doc.sampleRate)
				.arg(doc.channels)
				.arg(doc.durationSeconds(), 0, 'f', 2),
				5000
			);
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
			if (pEditor)
				pEditor->play();
			statusBar()->showMessage("Play", 1500);
			});

		// TODO Fix this
		connect(stopAct, &QAction::triggered, this, [this] {
			if (pEditor)
				pEditor->stop();
			statusBar()->showMessage("Stop", 1500);
			});
	}
}
