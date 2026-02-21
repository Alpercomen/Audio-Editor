#pragma once
#include <Application/Core/Project/Project.h>
#include <Application/Core/Data.h>

#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstring>

#include <QIODevice>

namespace Audio
{

    class ProjectIODevice final : public QIODevice
    {
        Q_OBJECT
    public:
        explicit ProjectIODevice(QObject* parent = nullptr) : QIODevice(parent) {}

        void setProject(std::shared_ptr<Project> p)
        {
            mProject = std::move(p);
            mPlayheadFrame = 0;
        }

        void reserveMix(size_t samples)
        {
            if (mMix.capacity() < samples)
                mMix.reserve(samples);

            if (mMix.size() < samples)
                mMix.resize(samples);
        }

        void setPlayheadMirror(std::atomic<std::int64_t>* p) { mMirror = p; }
        void seekToFrame(int64_t frame) { mPlayheadFrame = std::max<int64_t>(0, frame); }
        int64_t currentFrame() const { return mPlayheadFrame; }
        qint64 bytesAvailable() const override { return (1 << 20) + QIODevice::bytesAvailable(); }

        bool isSequential() const override { return false; }

    protected:
        qint64 readData(char* out, qint64 maxBytes) override
        {
            auto p = mProject;
            if (!p || p->tracks.empty() || p->channels <= 0 || p->sampleRate <= 0)
                return 0;

            const int outCh = p->channels;
            const qint64 bytesPerFrame = (qint64)outCh * (qint64)sizeof(qint16);
            const int64_t framesToWrite = (int64_t)(maxBytes / bytesPerFrame);

            if (framesToWrite <= 0)
                return 0;

            const size_t needed = (size_t)(framesToWrite * outCh);

            if (mMix.size() < needed)
                mMix.resize(needed);

            std::fill_n(mMix.data(), needed, 0.0f);

            const int64_t t0 = mPlayheadFrame;
            const int64_t t1 = t0 + framesToWrite;

            // Mix tracks/clips
            for (const auto& tr : p->tracks)
            {
                if (tr.muted) continue;

                for (const auto& clip : tr.clips)
                {
                    if (clip.muted || !clip.source)
                        continue;

                    const auto& src = *clip.source;

                    if (src.sampleRate != p->sampleRate)
                        continue;

                    const int64_t clipStart = clip.startFrameOnTimeline;
                    const int64_t clipLen = (clip.sourceOutFrame - clip.sourceInFrame);
                    const int64_t clipEnd = clipStart + clipLen;

                    const int64_t a = std::max<int64_t>(t0, clipStart);
                    const int64_t b = std::min<int64_t>(t1, clipEnd);

                    if (b <= a)
                        continue;

                    const int64_t outOffset = a - t0;
                    const int64_t srcOffset = clip.sourceInFrame + (a - clipStart);
                    const int64_t nFrames = b - a;

                    const float g = clip.gain * tr.gain;

                    for (int64_t f = 0; f < nFrames; ++f)
                    {
                        const int64_t outFrame = outOffset + f;
                        const int64_t srcFrame = srcOffset + f;

                        for (int ch = 0; ch < outCh; ++ch)
                        {
                            float s = 0.0f;
                            if (src.channels == 1) {
                                s = src.interleaved[(size_t)srcFrame];
                            }
                            else {
                                const int srcCh = std::min(ch, src.channels - 1);
                                s = src.interleaved[(size_t)(srcFrame * src.channels + srcCh)];
                            }

                            mMix[(size_t)(outFrame * outCh + ch)] += s * g;
                        }
                    }
                }
            }

            auto* dst = reinterpret_cast<qint16*>(out);
            const int64_t samples = framesToWrite * outCh;

            for (int64_t i = 0; i < samples; ++i)
            {
                float x = std::clamp(mMix[(size_t)i], -1.0f, 1.0f);
                dst[i] = (qint16)std::lrintf(x * 32767.0f);
            }

            mPlayheadFrame += framesToWrite;

            if (mMirror) 
                mMirror->store(mPlayheadFrame);

            return (qint64)(framesToWrite * bytesPerFrame);
        }

        qint64 writeData(const char*, qint64) override { return -1; }

    private:
        std::shared_ptr<Project> mProject;
        int64_t mPlayheadFrame = 0;
        std::vector<float> mMix;
        std::atomic<std::int64_t>* mMirror = nullptr;
    };

} // namespace Audio