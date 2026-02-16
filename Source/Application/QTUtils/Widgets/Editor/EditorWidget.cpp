#include "EditorWidget.h"

#include <Application/QTUtils/Widgets/Waveform/WaveformView.h>

#include <algorithm>
#include <cmath>

#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QPainter>
#include <QMessageBox>

namespace UI
{
	EditorWidget::EditorWidget(QWidget* parent) : QWidget(parent)
	{
		setAcceptDrops(true);

		auto* root = new QVBoxLayout(this);
		root->setContentsMargins(12, 12, 12, 12);
		root->setSpacing(10);

		pWaveView = new WaveformView(this);
		root->addWidget(pWaveView, 1);

		pHint = new QLabel(this);
		pHint->setText("No audio loaded. Use File -> Open or drag & drop an audio file here.");
		pHint->setAlignment(Qt::AlignCenter);
		root->addWidget(pHint, 0);
	}

	void EditorWidget::play()
	{
		mPlayback.play();
	}

	void EditorWidget::stop()
	{
		mPlayback.stop();
	}

	void EditorWidget::dragEnterEvent(QDragEnterEvent* e)
	{
		if (e->mimeData()->hasUrls())
			e->acceptProposedAction();
	}

	void EditorWidget::dropEvent(QDropEvent* e)
	{
		const auto& mimeData = e->mimeData();
		const auto urls = mimeData->urls();

		if (urls.isEmpty())
			return;

		const QString path = urls.first().toLocalFile();

		if (path.isEmpty())
			return;

		std::string err;
		Audio::AudioDocument doc = Audio::AudioDocument::LoadFromFile(path.toStdString(), err);

		if (!doc.isValid())
		{
			QMessageBox::critical(this, "Failed to load", QString::fromStdString(err));
			return;
		}

		// Set audio document for the wave view
		pWaveView->setDocument(doc);
		mPlayback.setDocument(std::move(doc));
		setHintText(QString("Loaded via drop:\n%1\n%2 Hz | %3 ch | %4 sec")
			.arg(path)
			.arg(doc.sampleRate)
			.arg(doc.channels)
			.arg(doc.durationSeconds(), 0, 'f', 2));
		e->acceptProposedAction();
	}


	void EditorWidget::setHintText(const QString& text)
	{
		pHint->setText(text);
	}
}