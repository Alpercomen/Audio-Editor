#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace Audio
{
	struct AudioDocument
	{
		int sampleRate = 0;
		int channels = 0;
		std::int64_t frames = 0;
		std::vector<float> interleaved;

		bool isValid() const 
		{
			return sampleRate > 0 && channels > 0 && frames > 0 && !interleaved.empty();
		}

		double durationSeconds() const 
		{
			return (sampleRate > 0) ? (double)frames / (double)sampleRate : 0.0;
		}

		static AudioDocument LoadFromFile(const std::string& path, std::string& outError);
	};
}
