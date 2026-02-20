#pragma once
#include <Application/Core/Data.h>

namespace Audio
{
    class Project {
    public:
        Project() = default;
        ~Project() = default;
        void recomputeLength();

        int sampleRate = 48000;
        int channels = 2;

        std::vector<Track> tracks;
        int64_t lengthFrames = 0;
    };
}