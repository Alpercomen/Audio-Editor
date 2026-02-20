#pragma once
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
        std::vector<float> interleaved;

        int64_t frames() const { return channels > 0 ? (int64_t)interleaved.size() / channels : 0; }
    };

    struct Clip
    {
        uint64_t id = 0;

        std::shared_ptr<AudioSource> source;

        int64_t startFrameOnTimeline = 0;

        int64_t sourceInFrame = 0;
        int64_t sourceOutFrame = 0;

        float gain = 1.0f;
        bool muted = false;
    };

    struct Track
    {
        std::string name = "Track";
        float gain = 1.0f;
        bool muted = false;
        bool solo = false;

        std::vector<Clip> clips;
    };

} // namespace Audio