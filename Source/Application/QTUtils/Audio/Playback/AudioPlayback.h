#pragma once
#include <Application/QTUtils/Audio/IODevice/IODevice.h>
#include <Application/Audio/Document/AudioDocument.h>

#include <QObject>
#include <QAudioSink>
#include <QAudioFormat>

#include <memory>

namespace Audio
{
	class AudioPlayback : public QObject
	{
		Q_OBJECT
	public:
		explicit AudioPlayback(QObject* parent = nullptr);

		void setDocument(const Audio::AudioDocument doc);
		void play();
		void stop();
		void seekToFrame(std::int64_t frame);

		bool isPlaying() const { return pSink && pSink->state() == QAudio::ActiveState; }
		std::int64_t getCurrentFrame() const;

	signals:
		void playbackStopped();

	private:
		std::unique_ptr<Audio::AudioDocument> pDoc;
		std::unique_ptr<QAudioSink> pSink;

		IODevice mDevice;
		QAudioFormat mFormat;

		bool mPendingStart = false;
		std::int64_t mPendingStartFrame = 0;

		void startAtFrameNow(std::int64_t frame);
	};
}