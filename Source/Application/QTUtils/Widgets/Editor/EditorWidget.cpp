#include "EditorWidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>

EditorWidget::EditorWidget(QWidget* parent) : QWidget(parent)
{
	setAcceptDrops(true);

	auto* root = new QVBoxLayout(this);
	root->setContentsMargins(12, 12, 12, 12);
	root->setSpacing(10);

	auto* waveform = new QFrame(this);
	waveform->setFrameShape(QFrame::StyledPanel);
	waveform->setMinimumHeight(300);

	mHint = new QLabel(this);
	mHint->setText("No audio loaded.\nUse File -> Open or drag & drop an audio file here.");
	mHint->setAlignment(Qt::AlignCenter);

	root->addWidget(waveform, 1);
	root->addWidget(mHint, 0);
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


	// TODO Fix this
	const QString path = urls.first().toLocalFile();
	setHintText("Dropped:\n" + path + "\n(loading not implemented yet)");
	e->acceptProposedAction();
}

void EditorWidget::setHintText(const QString& text)
{
	mHint->setText(text);
}