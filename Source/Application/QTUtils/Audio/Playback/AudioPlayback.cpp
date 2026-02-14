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
		stop();
		pDoc = std::make_shared<AudioDocument>(doc);

		if (!pDoc || !pDoc->isValid())
			return;

		mFormat = QAudioFormat();
		mFormat.setSampleRate(pDoc->sampleRate);
		mFormat.setChannelCount(pDoc->channels);
		mFormat.setSampleFormat(QAudioFormat::Float);

		QAudioDevice dev = QMediaDevices::defaultAudioOutput();
		QString desc = dev.description();

		spdlog::info("Audio device {} is set", desc.toStdString());

		if (dev.isNull()) 
		{
			const auto outs = QMediaDevices::audioOutputs();
			if (!outs.isEmpty()) dev = outs.first();
		}

		pSink = std::make_shared<QAudioSink>(dev, mFormat);

		connect(pSink.get(), &QAudioSink::stateChanged, this, [this](QAudio::State st) {
			spdlog::info("QAudioSink stateChanged -> {}", (int)st);
			});

		mDevice.setBuffer(pDoc->interleaved.data(), pDoc->frames, pDoc->channels);
		mDevice.open(QIODevice::ReadOnly);
	}

	void AudioPlayback::play()
	{
		QAudioDevice dev = QMediaDevices::defaultAudioOutput();
		QString desc = dev.description();

		spdlog::info("Device: {}", desc.toStdString());
		spdlog::info("Format: {} Hz, {} ch, sampleFormat={}",
			pSink->format().sampleRate(),
			pSink->format().channelCount(),
			(int)pSink->format().sampleFormat());

		spdlog::info("Sink state after start: {}", (int)pSink->state());

		if (!pDoc || !pDoc->isValid())
			return;

		if (!mDevice.isOpen())
			mDevice.open(QIODevice::ReadOnly);

		spdlog::info("IODevice open? {}", mDevice.isOpen());
		spdlog::info("IODevice readable? {}", mDevice.isReadable());

		pSink->start(&mDevice);

	}

	void AudioPlayback::stop()
	{
		if (pSink)
			pSink->stop();

		if (mDevice.isOpen())
			mDevice.close();
	}

	void AudioPlayback::seekToFrame(std::int64_t frame)
	{
		if (!pDoc || !pDoc->isValid())
			return;

		mDevice.seekToFrame(frame);

		if (pSink && pSink->state() == QAudio::ActiveState)
		{
			pSink->stop();
			pSink->start(&mDevice);
		}
	}

	std::int64_t AudioPlayback::getCurrentFrame() const
	{
		return mDevice.getCurrentFrame();
	}
}