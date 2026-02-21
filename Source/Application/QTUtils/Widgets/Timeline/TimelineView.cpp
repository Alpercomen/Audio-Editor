#include <Application/QTUtils/Widgets/Timeline/TimelineView.h>
#include <Application/QTUtils/Helper/Color.h>

#include <algorithm>
#include <cmath>

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QGuiApplication>

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

		const double framesPerPixel = getFramesPerPixel();
		const std::int64_t visibleFrames = (std::int64_t)std::max(1.0, framesPerPixel * w);
		const std::int64_t start = mStartFrame;
		const std::int64_t end = start + visibleFrames;

		for (size_t ti = 0; ti < mProject->tracks.size(); ++ti)
		{
			const int y0 = (int)ti * laneH + 10;
			const int y1 = y0 + laneH - 20;

			if (y1 < 0 || y0 > h)
				continue;

			for (size_t ci = 0; ci < mProject->tracks[ti].clips.size(); ++ci)
			{
				const auto& clip = mProject->tracks[ti].clips[ci];

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
				QColor base = niceClipColor(clip.id);
				QLinearGradient g(r.topLeft(), r.bottomLeft());

				g.setColorAt(0.0, base.lighter(115));
				g.setColorAt(1.0, base.darker(115));

				p.fillRect(r, g);
				p.setPen(base.darker(160));
				p.drawRect(r);

				drawWaveformVisibleSlice(p, *clip.source, clip, r, start, end, framesPerPixel);

				p.setPen(QColor(220, 220, 220));
				p.drawText(r.adjusted(6, 0, -6, 0), Qt::AlignVCenter | Qt::AlignLeft,
					QString("Clip %1").arg((int)clip.id));

				bool selected = mActiveClip && (int)ti == mActiveClip->trackIndex && ci == mActiveClip->clipIndex;
				p.setPen(selected ? QColor(255, 255, 255, 180) : QColor(120, 120, 160));
				p.drawRect(r);
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

		if (e->button() == Qt::LeftButton)
		{
			mActiveClip = hitTestClip(e->pos());
			if (mActiveClip)
			{
				const auto& clip = mProject->tracks[(size_t)mActiveClip->trackIndex]
					.clips[(size_t)mActiveClip->clipIndex];

				mDragging = true;
				mDragStartMouse = e->pos();
				mDragStartClipFrame = clip.startFrameOnTimeline;

				e->accept();
				update();
				return;
			}

			const auto frame = xToFrame(e->pos().x());
			mPlayheadFrame = std::max<std::int64_t>(0, frame);
			update();
			emit seekRequested(mPlayheadFrame);
			e->accept();
			return;
		}

		QWidget::mousePressEvent(e);
	}

	void TimelineView::mouseMoveEvent(QMouseEvent* e)
	{
		if (!mProject || !mDragging || !mActiveClip)
		{
			QWidget::mouseMoveEvent(e);
			return;
		}

		const int dx = e->pos().x() - mDragStartMouse.x();

		const std::int64_t targetFrame = xToFrame(mDragStartMouse.x() + dx);
		const std::int64_t delta = targetFrame - xToFrame(mDragStartMouse.x());

		auto& clip = mProject->tracks[(size_t)mActiveClip->trackIndex].clips[(size_t)mActiveClip->clipIndex];

		clip.startFrameOnTimeline = std::max<std::int64_t>(0, mDragStartClipFrame + delta);

		mProject->recomputeLength();
		update();

		e->accept();
	}

	void TimelineView::mouseReleaseEvent(QMouseEvent* e)
	{
		if (e->button() == Qt::LeftButton && mDragging)
		{
			mDragging = false;
			e->accept();
			return;
		}
		QWidget::mouseReleaseEvent(e);
	}

	void TimelineView::wheelEvent(QWheelEvent* e)
	{
		if (!mProject)
		{
			e->ignore();
			return;
		}

		const bool shiftIsPressed = (e->modifiers() & Qt::ShiftModifier);

		QPointF pixelDelta = e->pixelDelta();
		QPoint angleDelta = e->angleDelta();

		double steps = 0.0;
		if (!pixelDelta.isNull())
		{
			steps = pixelDelta.y() / 120.0;

			if (std::abs(steps) < 1e-6)
				steps = pixelDelta.x() / 120.0;
		}
		else
		{
			steps = angleDelta.y() / 120.0;
			if (std::abs(steps) < 1e-6) steps = angleDelta.x() / 120.0;
		}

		if (std::abs(steps) < 1e-6) { e->accept(); return; }

		const double oldZoom = mZoom;
		const double oldFpp = getFramesPerPixel();
		const int mouseX = (int)std::round(e->position().x());

		if (shiftIsPressed)
		{
			const double panPixelsPerStep = 200.0;
			const double panPixels = -steps * panPixelsPerStep;
			const std::int64_t panFrames = (std::int64_t)std::llround(panPixels * oldFpp);

			mStartFrame = std::max<std::int64_t>(0, mStartFrame + panFrames);

			update();
			e->accept();
			return;
		}
		else
		{
			const std::int64_t anchorFrame = xToFrame(mouseX);

			const double factorPerStep = 1.12;
			const double factor = std::pow(factorPerStep, steps);

			mZoom = std::clamp(mZoom * factor, 0.1, 500.0);
			const double newFpp = getFramesPerPixel();

			std::int64_t newStart = (std::int64_t)std::llround((double)anchorFrame - (double)mouseX * newFpp);
			mStartFrame = std::max<std::int64_t>(0, newStart);

			update();
			e->accept();
			return;
		}
	}

	std::int64_t TimelineView::xToFrame(int x) const
	{
		const double framesPerPixel = getFramesPerPixel();
		return mStartFrame + (std::int64_t)std::llround((double)x * framesPerPixel);
	}

	int TimelineView::frameToX(std::int64_t frame) const
	{
		const double framesPerPixel = getFramesPerPixel();
		return (int)std::llround((double)(frame - mStartFrame) / framesPerPixel);
	}

	std::optional<HitClip> TimelineView::hitTestClip(const QPoint& pt) const
	{
		if (!mProject)
			return std::nullopt;

		const int laneH = 70;
		const int trackIdx = pt.y() / laneH;

		if (trackIdx < 0 || trackIdx >= (int)mProject->tracks.size())
			return std::nullopt;

		const auto& track = mProject->tracks[(size_t)trackIdx];

		for (int ci = 0; ci < (int)track.clips.size(); ++ci)
		{
			const auto& clip = track.clips[(size_t)ci];

			if (!clip.source)
				continue;

			const std::int64_t clipStart = clip.startFrameOnTimeline;
			const std::int64_t clipLen = (clip.sourceOutFrame - clip.sourceInFrame);
			const std::int64_t clipEnd = clipStart + clipLen;

			const int x0 = frameToX(clipStart);
			const int x1 = frameToX(clipEnd);

			const int y0 = trackIdx * laneH + 10;
			const int y1 = y0 + laneH - 20;

			QRect r(x0, y0, std::max(2, x1 - x0), y1 - y0);

			if (r.contains(pt))
				return HitClip{ trackIdx, ci };
		}

		return std::nullopt;
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

	static void drawWaveformVisibleSlice(QPainter& p, const Audio::AudioSource& src, const Audio::Clip& clip, const QRect& clipRect, std::int64_t viewStartFrame, std::int64_t viewEndFrame, double framesPerPixel)
	{
		if (clipRect.width() <= 2 || clipRect.height() <= 4)
			return;

		if (src.channels <= 0)
			return;

		const std::int64_t clipStart = clip.startFrameOnTimeline;
		const std::int64_t clipLen = (clip.sourceOutFrame - clip.sourceInFrame);

		if (clipLen <= 0)
			return;

		const std::int64_t clipEnd = clipStart + clipLen;

		const std::int64_t visA = std::max(viewStartFrame, clipStart);
		const std::int64_t visB = std::min(viewEndFrame, clipEnd);

		if (visB <= visA)
			return;

		const std::int64_t srcA = clip.sourceInFrame + (visA - clipStart);
		const std::int64_t srcB = clip.sourceInFrame + (visB - clipStart);

		const int xA = (int)std::round((visA - viewStartFrame) / framesPerPixel);
		const int xB = (int)std::round((visB - viewStartFrame) / framesPerPixel);

		const int px0 = std::max(0, xA);
		const int px1 = std::min((int)p.viewport().width(), xB);

		if (px1 <= px0)
			return;

		const int visibleW = std::max(1, xB - xA);
		const double srcFramesPerPixel = (double)(srcB - srcA) / (double)visibleW;

		const int midY = clipRect.center().y();
		const int halfH = std::max(1, clipRect.height() / 2 - 2);

		p.setPen(QColor(255, 255, 255, 180));

		for (int px = px0; px < px1; ++px)
		{
			const int localX = px - xA;

			const std::int64_t f0 = srcA + (std::int64_t)std::floor(localX * srcFramesPerPixel);
			const std::int64_t f1 = srcA + (std::int64_t)std::floor((localX + 1) * srcFramesPerPixel);

			const std::int64_t a = std::clamp<std::int64_t>(f0, srcA, srcB - 1);
			const std::int64_t b = std::clamp<std::int64_t>(std::max<std::int64_t>(f1, f0 + 1), srcA, srcB);

			float mn = 1.0f, mx = -1.0f;

			for (std::int64_t f = a; f < b; ++f)
			{
				const std::int64_t idx = f * src.channels;
				const float s = src.interleaved[(size_t)idx];
				mn = std::min(mn, s);
				mx = std::max(mx, s);
			}

			const int y1 = midY - (int)std::round(mx * halfH);
			const int y2 = midY - (int)std::round(mn * halfH);

			p.drawLine(px, y1, px, y2);
		}
	}
}