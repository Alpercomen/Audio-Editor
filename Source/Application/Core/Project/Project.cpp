#include <Application/Core/Project/Project.h>

namespace Audio
{
	void Project::recomputeLength()
	{
        int64_t end = 0;
        for (const auto& tr : tracks) {
            for (const auto& c : tr.clips) {
                if (!c.source) continue;
                const int64_t clipLen = (c.sourceOutFrame - c.sourceInFrame);
                end = std::max(end, c.startFrameOnTimeline + clipLen);
            }
        }
        lengthFrames = end;
	}
}