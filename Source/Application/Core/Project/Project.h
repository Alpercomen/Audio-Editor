#pragma once
#include <Application/Core/Core.h>

namespace Audio
{
    class Project
    {
    public:
        Project() = default;
        ~Project() = default;
        void recomputeLength();

        Int32 sampleRate = 48000;
        Int32 channels = 2;

        Vector<Track> tracks;
        Int64 lengthFrames = 0;
    };
}