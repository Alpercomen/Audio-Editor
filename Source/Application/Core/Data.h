#pragma once
#include <Application/Core/Definitions/Definitions.h>
#include <Application/Core/Definitions/GlmNames.h>
#include <Application/Core/Definitions/StdNames.h>
#include <Application/Core/Definitions/Singleton.h>
#include <Application/Core/Definitions/BitwiseOperator.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

namespace Audio
{

    struct AudioSource
    {
        int sampleRate = 0;
        int channels = 0;
        Vector<Float32> interleaved;

        Int64 frames() const { return channels > 0 ? (Int64)interleaved.size() / channels : 0; }
    };

    struct Clip
    {
        Uint64 id = 0;

        std::shared_ptr<AudioSource> source;

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

        Vector<Clip> clips;
    };

} // namespace Audio