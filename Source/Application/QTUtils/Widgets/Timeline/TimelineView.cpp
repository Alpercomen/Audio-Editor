#include <Application/QTUtils/Widgets/Timeline/TimelineView.h>

#include <QPainter>
#include <QMouseEvent>
#include <algorithm>
#include <cmath>

namespace UI
{
	void TimelineView::setProject(std::shared_ptr<Audio::Project> p)
	{
		mProject = std::move(p);
		update();
	}

	void TimelineView::setPlayheadFrame(std::int64_t f)
	{
		mPlayheadFrame = std::max<std::int64_t>(0, f);
		update();
	}

	void TimelineView::paintEvent(QPaintEvent*)
	{
		QPainter p(this);
		p.fillRect(rect(), QColor(35, 35, 35));

		const int w = width();
		const int h = height();

		const int laneH = 70;
		const int lanes = std::max(1, h / laneH);

		p.setPen(QColor(55, 55, 55));
		for (int i = 0; i <= lanes; ++i)
			p.drawLine(0, i * laneH, w, i * laneH);

		if (!mProject)
			return;

		const double framesPerPixel = 200.0 / std::max(0.1, mZoom);
		const std::int64_t visibleFrames = (std::int64_t)std::max(1.0, framesPerPixel * w);
		const std::int64_t start = mStartFrame;
		const std::int64_t end = start + visibleFrames;

		for (size_t ti = 0; ti < mProject->tracks.size(); ++ti)
		{
			const int y0 = (int)ti * laneH + 10;
			const int y1 = y0 + laneH - 20;

			if (y1 < 0 || y0 > h)
				continue;

			for (const auto& clip : mProject->tracks[ti].clips)
			{
				if (!clip.source)
					continue;

				const std::int64_t clipStart = clip.startFrameOnTimeline;
				const std::int64_t clipLen = (clip.sourceOutFrame - clip.sourceInFrame);
				const std::int64_t clipEnd = clipStart + clipLen;

				const std::int64_t a = std::max<std::int64_t>(start, clipStart);
				const std::int64_t b = std::min<std::int64_t>(end, clipEnd);

				if (b <= a)
					continue;

				const int x0 = (int)std::round((a - start) / framesPerPixel);
				const int x1 = (int)std::round((b - start) / framesPerPixel);

				QRect r(x0, y0, std::max(2, x1 - x0), y1 - y0);
				p.fillRect(r, QColor(70, 70, 95));
				p.setPen(QColor(120, 120, 160));
				p.drawRect(r);

				drawWaveform(p, *clip.source, clip, r, mProject->channels);

				p.setPen(QColor(220, 220, 220));
				p.drawText(r.adjusted(6, 0, -6, 0), Qt::AlignVCenter | Qt::AlignLeft,
					QString("Clip %1").arg((int)ti));
			}
		}

		if (mPlayheadFrame >= start && mPlayheadFrame <= end)
		{
			const int x = (int)std::round((mPlayheadFrame - start) / framesPerPixel);
			QPen pen(QColor(255, 60, 60));
			pen.setWidth(2);
			p.setPen(pen);
			p.drawLine(x, 0, x, h);
		}
	}

	void TimelineView::mousePressEvent(QMouseEvent* e)
	{
		if (!mProject)
			return;

		const double framesPerPixel = 200.0 / std::max(0.1, mZoom);
		const std::int64_t frame = mStartFrame + (std::int64_t)std::llround(e->position().x() * framesPerPixel);
		mPlayheadFrame = frame;
		update();
		emit seekRequested(std::max<std::int64_t>(0, frame));
	}

	static void drawWaveform(QPainter& p, const Audio::AudioSource& src, const Audio::Clip& clip, const QRect& r, int outChannels)
	{
		if (r.width() <= 2 || r.height() <= 4)
			return;

		const int midY = r.center().y();
		const int halfH = std::max(1, r.height() / 2 - 2);

		const int64_t clipLen = clip.sourceOutFrame - clip.sourceInFrame;
		if (clipLen <= 0)
			return;

		const double framesPerPixel = (double)clipLen / (double)r.width();

		p.setPen(QColor(210, 210, 210));

		for (int x = 0; x < r.width(); ++x)
		{
			const int64_t f0 = clip.sourceInFrame + (int64_t)std::floor(x * framesPerPixel);
			const int64_t f1 = clip.sourceInFrame + (int64_t)std::floor((x + 1) * framesPerPixel);

			const int64_t a = std::clamp<int64_t>(f0, clip.sourceInFrame, clip.sourceOutFrame - 1);
			const int64_t b = std::clamp<int64_t>(std::max<int64_t>(f1, f0 + 1), clip.sourceInFrame, clip.sourceOutFrame);

			float mn = 1.0f;
			float mx = -1.0f;

			for (int64_t f = a; f < b; ++f)
			{
				const int64_t idx = f * src.channels;
				const float s = src.interleaved[(size_t)idx];
				mn = std::min(mn, s);
				mx = std::max(mx, s);
			}

			const int y1 = midY - (int)std::round(mx * halfH);
			const int y2 = midY - (int)std::round(mn * halfH);

			const int px = r.left() + x;
			p.drawLine(px, y1, px, y2);
		}
	}
}