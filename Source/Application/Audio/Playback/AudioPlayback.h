#pragma once
#include <Application/Audio/Device/ProjectIODevice.h>
#include <Application/Core/Project/Project.h>
#include <Application/Core/Data.h>

#include <QObject>
#include <QAudioSink>
#include <QAudioFormat>

#include <memory>
#include <cstdint>

namespace Audio
{
	class AudioPlayback : public QObject
	{
		Q_OBJECT
	public:
		explicit AudioPlayback(QObject* parent = nullptr);

		void setProject(std::shared_ptr<Audio::Project> project);

		void play();
		void stop();
		void pause();
		void resume();

		bool isPlaying() const;
		bool isPaused() const;

		void seekToFrame(std::int64_t frame);
		std::int64_t getCurrentFrame() const;

		void playFromStart();
		bool hasResumePosition() const;

	private:
		bool mHasResumePos = false;

		std::shared_ptr<Audio::Project> mProject;
		std::unique_ptr<QAudioSink> pSink;
		std::atomic<std::int64_t> mUiPlayheadFrame = 0;

		ProjectIODevice mDevice;
		QAudioFormat mFormat;

		std::int64_t mPausedFrame = 0;
	};
}