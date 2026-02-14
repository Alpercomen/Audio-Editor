#include "AudioDocument.h"
#include <sndfile.h>

namespace Audio
{
	static std::string sfErrorString(SNDFILE* f)
	{
		const char* s = sf_strerror(f);
		return s ? std::string(s) : std::string("Unknown libsndfile error.");
	}

	AudioDocument AudioDocument::LoadFromFile(const std::string& path, std::string& outError)
	{
		outError.clear();

		SF_INFO info{};
		SNDFILE* file = sf_open(path.c_str(), SFM_READ, &info);
		if (!file)
		{
			outError = sfErrorString(nullptr);
			return {};
		}

		if (info.frames <= 0 || info.channels <= 0 || info.samplerate <= 0)
		{
			outError = "Invalid audio file info.";
			sf_close(file);
			return {};
		}

		AudioDocument doc;
		doc.sampleRate = info.samplerate;
		doc.channels = info.channels;
		doc.frames = info.frames;

		const std::int64_t totalSamples = doc.frames * doc.channels;
		if (totalSamples <= 0)
		{
			outError = "Invalid sample count.";
			sf_close(file);
			return {};
		}

		doc.interleaved.resize((size_t)totalSamples);

		std::int64_t framesRead = 0;
		while (framesRead < doc.frames)
		{
			// TODO Fix the magic number
			const std::int64_t remaining = doc.frames - framesRead;
			const std::int64_t chunk = (remaining > 65536) ? 65536 : remaining;

			float* dst = doc.interleaved.data() + (size_t)(framesRead * doc.channels);
			const std::int64_t got = sf_readf_float(file, dst, chunk);

			if (got <= 0)
				break;

			framesRead += got;
		}

		sf_close(file);

		if (framesRead != doc.frames)
		{
			doc.frames += framesRead;
			doc.interleaved.resize((size_t)(doc.frames * doc.channels));
			outError = "Warning: file read was shorter than expected.";
		}

		return doc;
	}
}