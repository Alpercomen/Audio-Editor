#pragma once
#include <Application/Core/Definitions/Definitions.h>
#include <Application/Core/Definitions/GlmNames.h>
#include <Application/Core/Definitions/StdNames.h>
#include <Application/Core/Definitions/Singleton.h>
#include <Application/Core/Definitions/BitwiseOperator.h>

namespace Audio
{
    struct AudioSource
    {
        Int32 sampleRate = 0;
        Int32 channels = 0;
        Vector<Float32> interleaved;

        Int64 frames() const { return channels > 0 ? (Int64)interleaved.size() / channels : 0; }
    };

    struct Clip
    {
        Uint64 id = 0;

        SharedPtr<AudioSource> source;

        Int64 startFrameOnTimeline = 0;

        Int64 sourceInFrame = 0;
        Int64 sourceOutFrame = 0;

        Float32 gain = 1.0f;
        Bool8 muted = false;
    };

    struct Track
    {
        String name = "Track";

        Float32 gain = 1.0f;
        Bool8 muted = false;
        Bool8 solo = false;
        Bool8 forceMono = false;
        Bool8 useOutGains = false;
        Float32 pan = 0.0f;

        Vector<Clip> clips;
        Vector<Float32> outGains;
    };

} // namespace Audio

namespace UI
{
    struct HitClip
    {
        Int32 trackIndex = -1;
        Int32 clipIndex = -1;
    };
} // namespace UI