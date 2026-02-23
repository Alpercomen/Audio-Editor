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

		void seekToFrame(Int64 frame);
		Int64 getCurrentFrame() const;

		void playFromStart();
		bool hasResumePosition() const;

	private:
		Bool8 mHasResumePos = false;

		SharedPtr<Audio::Project> mProject;
		UniquePtr<QAudioSink> pSink;
		Atomic<Int64> mUiPlayheadFrame = 0;

		ProjectIODevice mDevice;
		QAudioFormat mFormat;

		Int64 mPausedFrame = 0;
	};
}