#pragma once
#include <Application/Core/Core.h>

#include <QWidget>
#include <QPainter>

namespace UI
{
	static void drawWaveform(QPainter& p, const Audio::AudioSource& src, const Audio::Clip& clip, const QRect& clipRect, Int64 viewStartFrame, Int64 viewEndFrame, Float64 framesPerPixel)
	{
		if (clipRect.width() <= 2 || clipRect.height() <= 4)
			return;

		if (src.channels <= 0)
			return;

		const Int64 clipStart = clip.startFrameOnTimeline;
		const Int64 clipLen = (clip.sourceOutFrame - clip.sourceInFrame);

		if (clipLen <= 0)
			return;

		const Int64 clipEnd = clipStart + clipLen;

		const Int64 visA = std::max(viewStartFrame, clipStart);
		const Int64 visB = std::min(viewEndFrame, clipEnd);

		if (visB <= visA)
			return;

		const Int64 srcA = clip.sourceInFrame + (visA - clipStart);
		const Int64 srcB = clip.sourceInFrame + (visB - clipStart);

		const Int32 xA = (int)std::round((visA - viewStartFrame) / framesPerPixel);
		const Int32 xB = (int)std::round((visB - viewStartFrame) / framesPerPixel);

		const Int32 px0 = std::max(0, xA);
		const Int32 px1 = std::min((Int32)p.viewport().width(), xB);

		if (px1 <= px0)
			return;

		const int visibleW = std::max(1, xB - xA);
		const Float64 srcFramesPerPixel = (Float64)(srcB - srcA) / (Float64)visibleW;

		const int midY = clipRect.center().y();
		const int halfH = std::max(1, clipRect.height() / 2 - 2);

		p.setPen(QColor(255, 255, 255, 180));

		for (int px = px0; px < px1; ++px)
		{
			const int localX = px - xA;

			const Int64 f0 = srcA + (Int64)std::floor(localX * srcFramesPerPixel);
			const Int64 f1 = srcA + (Int64)std::floor((localX + 1) * srcFramesPerPixel);

			const Int64 a = std::clamp<Int64>(f0, srcA, srcB - 1);
			const Int64 b = std::clamp<Int64>(std::max<Int64>(f1, f0 + 1), srcA, srcB);

			float mn = 1.0f, mx = -1.0f;

			for (Int64 f = a; f < b; ++f)
			{
				const Int64 idx = f * src.channels;
				const Float32 s = src.interleaved[(Usize)idx];
				mn = std::min(mn, s);
				mx = std::max(mx, s);
			}

			const Int32 y1 = midY - (Int32)std::round(mx * halfH);
			const Int32 y2 = midY - (Int32)std::round(mn * halfH);

			p.drawLine(px, y1, px, y2);
		}
	}
}