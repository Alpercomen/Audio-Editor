#pragma once
#include <QColor>
#include <cstdint>

namespace UI
{
    static uint32_t hash32(uint64_t x)
    {
        x += 0x9E3779B97F4A7C15ull;
        x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
        x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
        x = x ^ (x >> 31);
        return (uint32_t)x;
    }

    static QColor generateClipColor(uint64_t clipId)
    {
        const uint32_t h = hash32(clipId);

        const int hue = (int)(h % 360);

        const int sat = 140 + (int)((h >> 8) % 60);
        const int val = 130 + (int)((h >> 16) % 40);

        return QColor::fromHsv(hue, sat, val);
    }
}
