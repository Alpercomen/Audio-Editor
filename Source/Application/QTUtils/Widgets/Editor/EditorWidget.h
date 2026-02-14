#pragma once
#include <Application/Audio/Document/AudioDocument.h>
#include <Application/QTUtils/Audio/Playback/AudioPlayback.h>
#include <Application/QTUtils/Widgets/Waveform/WaveformView.h>

#include <QWidget>

class QLabel;

namespace UI
{
	class EditorWidget : public QWidget
	{
		Q_OBJECT
	public:
		explicit EditorWidget(QWidget* parent = nullptr);

		void play();
		void stop();
		void setDocument(Audio::AudioDocument doc, QString sourcePath = {})
		{
			mPlayback.setDocument(doc);
			pWaveView->setDocument(doc, sourcePath);
		}

		const Audio::AudioDocument& getDocument() const 
		{
			return pWaveView->getDocument();
		}

	protected:
		void dragEnterEvent(QDragEnterEvent* e) override;
		void dropEvent(QDropEvent* e) override;

	private:
		UI::WaveformView* pWaveView = nullptr;
		Audio::AudioPlayback mPlayback;
		QLabel* pHint = nullptr;

		void setHintText(const QString& text);
	};
}