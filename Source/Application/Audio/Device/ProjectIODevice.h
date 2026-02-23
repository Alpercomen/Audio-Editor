#pragma once
#include <Application/Core/Project/Project.h>
#include <Application/Core/Core.h>

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

        void setProject(SharedPtr<Project> p)
        {
            mProject = std::move(p);
            mPlayheadFrame = 0;
        }

        void reserveMix(Usize samples)
        {
            if (mMix.capacity() < samples)
                mMix.reserve(samples);

            if (mMix.size() < samples)
                mMix.resize(samples);
        }

        void setPlayheadMirror(Atomic<Int64>* p) { mMirror = p; }
        void seekToFrame(Int64 frame) { mPlayheadFrame = std::max<Int64>(0, frame); }
        Int64 currentFrame() const { return mPlayheadFrame; }
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
            const Int64 framesToWrite = (Int64)(maxBytes / bytesPerFrame);

            if (framesToWrite <= 0)
                return 0;

            const Usize needed = (Usize)(framesToWrite * outCh);

            if (mMix.size() < needed)
                mMix.resize(needed);

            std::fill_n(mMix.data(), needed, 0.0f);

            const Int64 t0 = mPlayheadFrame;
            const Int64 t1 = t0 + framesToWrite;

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

                    const Int64 clipStart = clip.startFrameOnTimeline;
                    const Int64 clipLen = (clip.sourceOutFrame - clip.sourceInFrame);
                    const Int64 clipEnd = clipStart + clipLen;

                    const Int64 a = std::max<Int64>(t0, clipStart);
                    const Int64 b = std::min<Int64>(t1, clipEnd);

                    if (b <= a)
                        continue;

                    const Int64 outOffset = a - t0;
                    const Int64 srcOffset = clip.sourceInFrame + (a - clipStart);
                    const Int64 nFrames = b - a;

                    const float g = clip.gain * tr.gain;

                    for (Int64 f = 0; f < nFrames; ++f)
                    {
                        const Int64 outFrame = outOffset + f;
                        const Int64 srcFrame = srcOffset + f;

                        for (int ch = 0; ch < outCh; ++ch)
                        {
                            float s = 0.0f;
                            if (src.channels == 1) {
                                s = src.interleaved[(Usize)srcFrame];
                            }
                            else {
                                const int srcCh = std::min(ch, src.channels - 1);
                                s = src.interleaved[(Usize)(srcFrame * src.channels + srcCh)];
                            }

                            mMix[(Usize)(outFrame * outCh + ch)] += s * g;
                        }
                    }
                }
            }

            auto* dst = reinterpret_cast<qint16*>(out);
            const Int64 samples = framesToWrite * outCh;

            for (Int64 i = 0; i < samples; ++i)
            {
                float x = std::clamp(mMix[(Usize)i], -1.0f, 1.0f);
                dst[i] = (qint16)std::lrintf(x * 32767.0f);
            }

            mPlayheadFrame += framesToWrite;

            if (mMirror) 
                mMirror->store(mPlayheadFrame);

            return (qint64)(framesToWrite * bytesPerFrame);
        }

        qint64 writeData(const char*, qint64) override { return -1; }

    private:
        SharedPtr<Project> mProject;
        Int64 mPlayheadFrame = 0;
        Vector<Float32> mMix;
        Atomic<Int64>* mMirror = nullptr;
    };

} // namespace Audio