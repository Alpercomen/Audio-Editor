#pragma once
#include "MainWindow.h"

#include <Application/QTUtils/Widgets/Editor/EditorWidget.h>
#include <Application/Audio/Document/AudioDocument.h>

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include <QStyle>
#include <QToolButton>

namespace UI
{
	MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
	{
		pEditor = new EditorWidget(this);
		setCentralWidget(pEditor);

		buildMenus();
		buildToolbar();

		statusBar()->showMessage("Ready");
		setWindowTitle("Sona");
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
		connect(openAct, &QAction::triggered, this, [this]
			{
				const QString file = QFileDialog::getOpenFileName(
					this,
					"Open Audio",
					QString(),
					"Audio Files (*.wav *.miff *.flac *.ogg);;All Files (*,*)"
				);

				if (file.isEmpty())
					return;

				String err;
				Audio::AudioDocument doc = Audio::AudioDocument::LoadFromFile(file.toStdString(), err);

				if (!doc.isValid())
				{
					QMessageBox::critical(this, "Failed to load", QString::fromStdString(err));
					return;
				}

				statusBar()->showMessage(
					QString("Loaded: %1 | %2 Hz | %3 ch | %4 sec")
					.arg(file)
					.arg(doc.sampleRate)
					.arg(doc.channels)
					.arg(doc.durationSeconds(), 0, 'f', 2),
					5000
				);

				pEditor->stop();
				pEditor->setDocument(std::move(doc), file);
			});
	}

	void MainWindow::buildToolbar()
	{
		auto* toolBar = addToolBar("Transport");
		toolBar->setMovable(false);

		const QIcon fromStartIcon = style()->standardIcon(QStyle::SP_MediaSkipBackward);
		const QIcon playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
		const QIcon pauseIcon = style()->standardIcon(QStyle::SP_MediaPause);
		const QIcon stopIcon = style()->standardIcon(QStyle::SP_MediaStop);

		auto* playFromStartAct = toolBar->addAction(fromStartIcon, "Play from start");
		auto* playPauseAct = toolBar->addAction(playIcon, "Play/Pause");
		playPauseAct->setCheckable(true);
		auto* stopAct = toolBar->addAction(stopIcon, "Stop");

		toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

		connect(playFromStartAct, &QAction::triggered, this, [this, playPauseAct, pauseIcon](Bool8)
			{
				if (!pEditor)
					return;

				pEditor->playFromStart();

				playPauseAct->setIcon(pauseIcon);
				playPauseAct->setChecked(true);
			});

		connect(playPauseAct, &QAction::triggered, this, [this, playPauseAct, playIcon, pauseIcon](Bool8)
			{
				if (!pEditor)
					return;

				pEditor->togglePlayPause();

				if (pEditor->isPlaying())
				{
					playPauseAct->setIcon(pauseIcon);
					playPauseAct->setChecked(true);
				}
				else
				{
					playPauseAct->setIcon(playIcon);
					playPauseAct->setChecked(false);
				}
			});

		connect(stopAct, &QAction::triggered, this, [this, playPauseAct, playIcon]()
			{
				if (!pEditor)
					return;
				pEditor->stop();

				playPauseAct->setIcon(playIcon);
				playPauseAct->setChecked(false);
			});
	}
}
