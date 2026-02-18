#include "WaveformView.h"

#include <QPainter>
#include <QWheelEvent>

namespace UI
{
	void WaveformView::paintEvent(QPaintEvent* e)
	{
		QPainter p(this);
		p.fillRect(rect(), QColor(40, 40, 40));

		if (!mDoc.isValid())
			return;
		
		const int w = width();
		const int h = height();
		const int midY = h / 2;

		const double visibleFramesD = (double)mDoc.frames / std::max(1.0, mZoom);
		const std::int64_t visibleFrames = (std::int64_t)std::max(1.0, visibleFramesD);

		std::int64_t start = mCenterFrame - visibleFrames / 2;
		std::int64_t end = start + visibleFrames;

		if (start < 0)
		{
			start = 0;
			end = std::min<std::int64_t>(mDoc.frames, visibleFrames);
		}

		if (end > mDoc.frames)
		{
			end = mDoc.frames;
			start = std::max<std::int64_t>(0, end - visibleFrames);
		}

		const std::int64_t range = std::max<std::int64_t>(1, end - start);
		const double framesPerPixel = (double)range / (double)std::max(1, w);

		// Draw center line
		p.setPen(QColor(70, 70, 70));
		p.drawLine(0, midY, w, midY);

		// Draw waveform (using channel 0)
		p.setPen(QColor(200, 200, 200));

		for (int x = 0; x < w; ++x)
		{
			const std::int64_t f0 = start + (std::int64_t)std::floor(x * framesPerPixel);
			const std::int64_t f1 = start + (std::int64_t)std::floor((x + 1) * framesPerPixel);

			const std::int64_t a = std::clamp<std::int64_t>(f0, 0, mDoc.frames - 1);
			const std::int64_t b = std::clamp<std::int64_t>(std::max(f1, f0 + 1), 0, mDoc.frames);

			float mn = 1.0f;
			float mx = -1.0f;

			for (std::int64_t f = a; f < b; ++f)
			{
				const std::int64_t idx = f * mDoc.channels;
				const float s = mDoc.interleaved[(size_t)idx];
				mn = std::min(mn, s);
				mx = std::max(mx, s);
			}

			// map [-1, 1] -> pixels
			const int y1 = midY - (int)std::round(mx * (midY - 10));
			const int y2 = midY - (int)std::round(mn * (midY - 10));
			p.drawLine(x, y1, x, y2);
		}

		// Simple overlay text
		p.setPen(QColor(100, 100, 100));
		p.drawText(10, 20,
			QString("%1 Hz | %2 ch | %3 s | zoom %4x")
			.arg(mDoc.sampleRate)
			.arg(mDoc.channels)
			.arg(mDoc.durationSeconds(), 0, 'f', 2)
			.arg(mZoom, 0, 'f', 2));

		// Draw playhead line (bright red)
		if (mPlayheadFrame >= 0 && mDoc.isValid())
		{
			// only draw if within visible range
			if (mPlayheadFrame >= start && mPlayheadFrame <= end)
			{
				const double t = (double)(mPlayheadFrame - start) / (double)range; // 0..1
				const int x = (int)std::round(t * (w - 1));

				QPen pen(QColor(255, 60, 60));
				pen.setWidth(2);
				p.setPen(pen);
				p.drawLine(x, 0, x, h);
			}
		}
	}

	void WaveformView::wheelEvent(QWheelEvent* e)
	{
		if (!mDoc.isValid())
			return;

		const double delta = e->angleDelta().y();
		if (delta == 0)
			return;

		const double factor = (delta > 0) ? 1.1 : 1.0 / 1.1;

		mZoom = std::clamp(mZoom * factor, 1.0, 500.0);
		update();
	}
}
