#pragma once
#include <Application/Core/Data.h>

#include <QColor>
#include <cstdint>

namespace UI
{
    static Uint32 hash32(Uint64 x)
    {
        x += 0x9E3779B97F4A7C15ull;
        x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
        x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
        x = x ^ (x >> 31);
        return (Uint32)x;
    }

    static QColor generateClipColor(Uint64 clipId)
    {
        const Uint32 h = hash32(clipId);

        const Int32 hue = (int)(h % 360);

        const Int32 sat = 140 + (Int32)((h >> 8) % 60);
        const Int32 val = 130 + (Int32)((h >> 16) % 40);

        return QColor::fromHsv(hue, sat, val);
    }
}
