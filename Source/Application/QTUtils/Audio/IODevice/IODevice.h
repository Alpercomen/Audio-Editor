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

		void setBuffer(const float* data, std::int64_t frames, int channels)
		{
			pData = data;
			mFrames = frames;
			mChannels = channels;
			mFramePos = 0;
		}

		void seekToFrame(std::int64_t frame)
		{
			frame = std::max<std::int64_t>(0, std::min(frame, mFrames));
			mFramePos = frame;
		}

		qint64 bytesAvailable() const override
		{
			if (!pData || mFrames <= 0 || mChannels <= 0) return QIODevice::bytesAvailable();
			const std::int64_t framesRemaining = mFrames - mFramePos;
			const qint64 bytesRemaining = (qint64)framesRemaining * (qint64)mChannels * (qint64)sizeof(float);
			return bytesRemaining + QIODevice::bytesAvailable();
		}

		bool atEnd() const override
		{
			if (!pData || mFrames <= 0 || mChannels <= 0) return true;
			return mFramePos >= mFrames;
		}

		std::int64_t getCurrentFrame() const { return mFramePos; }
		std::int64_t getTotalFrames() const { return mFrames; }

		bool isSequential() const override { return true; }
		void close() override { QIODevice::close(); }

	protected:
		qint64 readData(char* out, qint64 maxBytes) override
		{
			static int dbg = 0;
			if (dbg < 10) {
				dbg++;
				spdlog::info("readData maxBytes={} frames={} ch={} pos={} data={}",
					(long long)maxBytes,
					(long long)mFrames,
					mChannels,
					(long long)mFramePos,
					(void*)pData);
			}

			if (!pData || mFrames <= 0 || mChannels <= 0)
				return 0;

			const std::int64_t framesRemaining = mFrames - mFramePos;

			if (framesRemaining <= 0)
				return 0;

			const qint64 bytesPerFrame = (qint64)mChannels * (qint64)sizeof(float);
			const qint64 framesToWrite = std::min<std::int64_t>(framesRemaining, (std::int64_t)(maxBytes / bytesPerFrame));
			const qint64 bytesToWrite = framesToWrite * bytesPerFrame;

			const float* src = pData + (size_t)(mFramePos * mChannels);
			std::memcpy(out, src, (size_t)bytesToWrite);

			mFramePos += (std::int64_t)framesToWrite;

			if (dbg < 10) 
				spdlog::info("readData returning {} bytes", (long long)bytesToWrite);

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
