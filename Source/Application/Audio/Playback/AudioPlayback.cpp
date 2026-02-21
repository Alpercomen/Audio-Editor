#include <Application/Audio/Playback/AudioPlayback.h>

#include <QMediaDevices>
#include <spdlog/spdlog.h>
#include <algorithm>

namespace Audio
{
	AudioPlayback::AudioPlayback(QObject* parent) : QObject(parent) {}

	void AudioPlayback::setProject(std::shared_ptr<Audio::Project> project)
	{
		if (pSink) 
			pSink->reset();

		mProject = std::move(project);

		if (!mProject) 
			return;

		mFormat = QAudioFormat();
		mFormat.setSampleRate(mProject->sampleRate);
		mFormat.setChannelCount(mProject->channels);
		mFormat.setSampleFormat(QAudioFormat::Int16);

		QAudioDevice dev = QMediaDevices::defaultAudioOutput();
		if (dev.isNull())
		{
			const auto outs = QMediaDevices::audioOutputs();
			if (!outs.isEmpty()) dev = outs.first();
		}

		spdlog::info("Audio device: {}", dev.description().toStdString());

		if (!dev.isFormatSupported(mFormat))
		{
			auto pref = dev.preferredFormat();
			spdlog::error("Format NOT supported. Requested {} Hz {} ch Int16. Preferred {} Hz {} ch fmt={}",
				mFormat.sampleRate(), mFormat.channelCount(),
				pref.sampleRate(), pref.channelCount(), (int)pref.sampleFormat());
			return;
		}

		pSink = std::make_unique<QAudioSink>(dev, mFormat);

		const int outCh = mProject->channels;
		const int bytesPerFrame = outCh * (int)sizeof(qint16);

		const int bufferBytes = pSink->bufferSize() > 0 ? pSink->bufferSize() : 65536;
		const int64_t maxFrames = std::max<int64_t>(1, bufferBytes / bytesPerFrame);

		mDevice.reserveMix((size_t)(maxFrames * outCh * 4));

		connect(pSink.get(), &QAudioSink::stateChanged, this, [](QAudio::State st) {
			spdlog::info("QAudioSink state -> {}", (int)st);
			});

		mDevice.setProject(mProject);
		mDevice.setPlayheadMirror(&mUiPlayheadFrame);

		if (!mDevice.isOpen())
			mDevice.open(QIODevice::ReadOnly | QIODevice::Unbuffered);

		mPausedFrame = 0;
	}

	void AudioPlayback::play()
	{
		if (!pSink || !mProject)
			return;

		pSink->reset();
		mDevice.seekToFrame(0);
		pSink->start(&mDevice);
		mUiPlayheadFrame.store(0);
	}

	void AudioPlayback::stop()
	{
		if (!pSink)
			return;

		mPausedFrame = getCurrentFrame();
		mHasResumePos = true;

		pSink->reset();
		mDevice.seekToFrame(mPausedFrame);
		mUiPlayheadFrame.store(mPausedFrame);
	}

	void AudioPlayback::pause()
	{
		if (!pSink)
			return;

		mPausedFrame = getCurrentFrame();
		mHasResumePos = true;

		if (pSink->state() == QAudio::ActiveState || pSink->state() == QAudio::IdleState)
			pSink->suspend();

		mUiPlayheadFrame.store(mPausedFrame);
	}

	void AudioPlayback::resume()
	{
		if (!pSink || !mProject)
			return;

		if (pSink->state() == QAudio::SuspendedState)
		{
			pSink->resume();
			return;
		}

		const auto frame = mHasResumePos ? mPausedFrame : 0;
		mUiPlayheadFrame.store(frame);

		pSink->reset();
		mDevice.seekToFrame(frame);
		pSink->start(&mDevice);
	}

	bool AudioPlayback::isPlaying() const
	{
		return pSink && pSink->state() == QAudio::ActiveState;
	}

	bool AudioPlayback::isPaused() const
	{
		return pSink && pSink->state() == QAudio::SuspendedState;
	}

	void AudioPlayback::seekToFrame(std::int64_t frame)
	{
		if (!pSink || !mProject)
			return;

		frame = std::max<std::int64_t>(0, frame);

		mDevice.seekToFrame(frame);
		mUiPlayheadFrame.store(frame);
	}

	std::int64_t AudioPlayback::getCurrentFrame() const
	{
		return mUiPlayheadFrame.load();
	}

	void AudioPlayback::playFromStart()
	{
		if (!pSink || !mProject)
			return;

		mPausedFrame = 0;
		mHasResumePos = false;
		mUiPlayheadFrame.store(0);

		pSink->reset();
		mDevice.seekToFrame(0);
		pSink->start(&mDevice);
	}

	bool AudioPlayback::hasResumePosition() const
	{
		return mHasResumePos;
	}
}