#include "AudioPlayback.h"

#include <QMediaDevices>

#include <spdlog/spdlog.h>

namespace Audio
{
	AudioPlayback::AudioPlayback(QObject* parent) : QObject(parent)
	{
	}

	void AudioPlayback::setDocument(const AudioDocument doc)
	{
        // Hard stop old sink (flush buffers)
        if (pSink) pSink->reset();

        pDoc = std::make_unique<AudioDocument>(doc);
        if (!pDoc || !pDoc->isValid())
            return;

        mFormat = QAudioFormat();
        mFormat.setSampleRate(pDoc->sampleRate);
        mFormat.setChannelCount(pDoc->channels);
        mFormat.setSampleFormat(QAudioFormat::Int16);

        QAudioDevice dev = QMediaDevices::defaultAudioOutput();
        if (dev.isNull()) 
        {
            const auto outs = QMediaDevices::audioOutputs();

            if (!outs.isEmpty()) 
                dev = outs.first();
        }

        spdlog::info("Audio device: {}", dev.description().toStdString());

        if (!dev.isFormatSupported(mFormat)) 
        {
            spdlog::error("Requested format NOT supported: {} Hz, {} ch, Int16", mFormat.sampleRate(), mFormat.channelCount());

            auto pref = dev.preferredFormat();
            spdlog::error("Preferred format: {} Hz, {} ch, sampleFormat={}", pref.sampleRate(), pref.channelCount(), (int)pref.sampleFormat());

            return;
        }

        pSink = std::make_unique<QAudioSink>(dev, mFormat);

        connect(pSink.get(), &QAudioSink::stateChanged, this, [this](QAudio::State st) {
            spdlog::info("QAudioSink stateChanged -> {}", (int)st);

            if (st == QAudio::StoppedState) 
            {
                emit playbackStopped();

                if (mPendingStart) 
                {
                    const auto frame = mPendingStartFrame;
                    mPendingStart = false;
                    startAtFrameNow(frame);
                }
            }
            });

        mDevice.setBuffer(pDoc->interleaved.data(), pDoc->frames, pDoc->channels);

        if (!mDevice.isOpen())
            mDevice.open(QIODevice::ReadOnly);
	}

    void AudioPlayback::startAtFrameNow(std::int64_t frame)
    {
        if (!pSink || !pDoc || !pDoc->isValid())
            return;

        frame = std::clamp<std::int64_t>(frame, 0, std::max<std::int64_t>(0, pDoc->frames - 1));

        pSink->reset();

        if (!mDevice.isOpen())
            mDevice.open(QIODevice::ReadOnly);

        mDevice.seekToFrame(frame);
        pSink->start(&mDevice);
    }

	void AudioPlayback::play()
	{
        if (!pSink || !pDoc || !pDoc->isValid())
            return;

        const auto st = pSink->state();
        spdlog::info("State before play: {}", (int)st);

        // If sink is Active OR Idle, it is considered "already started".
        // Queue restart and reset to reach StoppedState cleanly.
        if (st == QAudio::ActiveState || st == QAudio::IdleState) 
        {
            mPendingStart = true;
            mPendingStartFrame = 0;
            pSink->reset();
            return;
        }

        startAtFrameNow(0);
	}

	void AudioPlayback::stop()
	{
        if (!pSink) return;

        mPendingStart = false;

        pSink->reset();
	}

	void AudioPlayback::seekToFrame(std::int64_t frame)
	{
        if (!pSink || !pDoc || !pDoc->isValid())
            return;

        frame = std::clamp<std::int64_t>(frame, 0, std::max<std::int64_t>(0, pDoc->frames - 1));

        const auto st = pSink->state();

        if (st == QAudio::ActiveState || st == QAudio::IdleState) 
        {
            mPendingStart = true;
            mPendingStartFrame = frame;
            pSink->reset();
            return;
        }

        mDevice.seekToFrame(frame);
	}

	std::int64_t AudioPlayback::getCurrentFrame() const
	{
		return mDevice.getCurrentFrame();
	}
}