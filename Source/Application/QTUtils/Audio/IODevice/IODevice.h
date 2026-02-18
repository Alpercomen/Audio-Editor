#pragma once
#include <cstdint>
#include <vector>

#include <spdlog/spdlog.h>

#include <QIODevice>

namespace Audio
{
	class IODevice : public QIODevice
	{
		Q_OBJECT
	public:
		IODevice(QObject* parent = nullptr) : QIODevice(parent) {}

		static inline qint16 floatToI16(float x)
		{
			// clamp [-1, 1]
			if (x > 1.0f) x = 1.0f;
			if (x < -1.0f) x = -1.0f;
			// map to int16
			return (qint16)std::lrintf(x * 32767.0f);
		}

		void setBuffer(const float* data, std::int64_t frames, int channels)
		{
			pData = data;
			mFrames = frames;
			mChannels = channels;
			mFramePos = 0;
		}

		void seekToFrame(std::int64_t frame)
		{
			frame = std::clamp<std::int64_t>(frame, 0, std::max<std::int64_t>(0, mFrames - 1));
			mFramePos = frame;
		}

		qint64 bytesAvailable() const override
		{
			if (!pData || mFrames <= 0 || mChannels <= 0) 
				return QIODevice::bytesAvailable();

			const std::int64_t framesRemaining = mFrames - mFramePos;
			const qint64 bytesRemaining = (qint64)framesRemaining * (qint64)mChannels * (qint64)sizeof(qint16);
			return bytesRemaining + QIODevice::bytesAvailable();
		}

		bool atEnd() const override
		{
			if (!pData || mFrames <= 0 || mChannels <= 0) 
				return true;

			if (mFramePos >= mFrames)
				return true;
			else
				return false;
		}

		std::int64_t getCurrentFrame() const { return mFramePos; }
		std::int64_t getTotalFrames() const { return mFrames; }

		bool isSequential() const override { return false; }

	protected:
		qint64 readData(char* out, qint64 maxBytes) override
		{

			if (!pData || mFrames <= 0 || mChannels <= 0) return 0;

			const std::int64_t framesRemaining = mFrames - mFramePos;
			if (framesRemaining <= 0) return 0;

			// We're outputting int16 samples:
			const qint64 bytesPerFrame = (qint64)mChannels * (qint64)sizeof(qint16);

			// Only write whole frames
			const std::int64_t framesToWrite =
				std::min<std::int64_t>(framesRemaining, (std::int64_t)(maxBytes / bytesPerFrame));

			const qint64 bytesToWrite = (qint64)framesToWrite * bytesPerFrame;

			auto* dst = reinterpret_cast<qint16*>(out);
			const float* src = pData + (size_t)(mFramePos * mChannels);

			const std::int64_t samplesToWrite = framesToWrite * (std::int64_t)mChannels;
			for (std::int64_t i = 0; i < samplesToWrite; ++i)
				dst[i] = floatToI16(src[i]);

			mFramePos += framesToWrite;
			return bytesToWrite;
		}

		qint64 writeData(const char* data, qint64 len) override { return -1; }

	private:
		std::int64_t mFrames = 0;
		std::int64_t mFramePos = 0;

		int mChannels = 0;
		const float* pData = nullptr;
	};
}
