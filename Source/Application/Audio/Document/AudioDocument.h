#pragma once
#include <Application/Core/Data.h>

#include <string>
#include <vector>
#include <cstdint>

namespace Audio
{
	struct AudioDocument
	{
		Int32 sampleRate = 0;
		Int32 channels = 0;
		Int64 frames = 0;
		std::vector<float> interleaved;

		Bool8 isValid() const 
		{
			return sampleRate > 0 && channels > 0 && frames > 0 && !interleaved.empty();
		}

		Float64 durationSeconds() const 
		{
			return (sampleRate > 0) ? (Float64)frames / (Float64)sampleRate : 0.0;
		}

		static AudioDocument LoadFromFile(const String& path, String& outError);
	};
}
