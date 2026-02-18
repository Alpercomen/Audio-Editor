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
            });

        mDevice.setBuffer(pDoc->interleaved.data(), pDoc->frames, pDoc->channels);

        if (!mDevice.isOpen())
            mDevice.open(QIODevice::ReadOnly | QIODevice::Unbuffered);
	}

	void AudioPlayback::play()
	{
        if (!pSink || !pDoc || !pDoc->isValid())
            return;

        if (!mDevice.isOpen())
            mDevice.open(QIODevice::ReadOnly | QIODevice::Unbuffered);

        const auto st = pSink->state();
        if (st == QAudio::ActiveState || st == QAudio::IdleState) 
            pSink->reset();

        mDevice.seekToFrame(0);
        pSink->start(&mDevice);
	}

	void AudioPlayback::stop()
	{
        if (!pSink) 
            return;

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