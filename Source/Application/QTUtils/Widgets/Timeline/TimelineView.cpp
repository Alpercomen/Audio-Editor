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
	void TimelineView::setProject(SharedPtr<Audio::Project> p)
	{
		mProject = std::move(p);
		emit viewChanged();
		update();
	}

	void TimelineView::setPlayheadFrame(Int64 f)
	{
		mPlayheadFrame = std::max<Int64>(0, f);
		update();
	}

	void TimelineView::setViewStartFrame(Int64 f)
	{
		mStartFrame = std::max<Int64>(0, f);
		emit viewChanged();
		update();
	}

	void TimelineView::setVerticalScrollPx(Int32 px)
	{
		mVScrollPx = std::max(0, px);
		emit viewChanged();
		update();
	}

	Int64 TimelineView::viewEndFrame() const
	{
		const Float64 fpp = getFramesPerPixel();
		return mStartFrame + (Int64)std::llround(width() * fpp);
	}

	Int64 TimelineView::maxStartFrame() const
	{
		if (!mProject)
			return 0;

		const Float64 fpp = getFramesPerPixel();
		const Int64 visible = (Int64)std::llround(width() * fpp);

		return std::max<Int64>(0, mProject->lengthFrames - visible);
	}

	int TimelineView::maxVerticalScrollPx() const
	{
		if (!mProject)
			return 0;

		const Int32 laneH = 70;
		const Int32 totalH = (int)mProject->tracks.size() * laneH;

		return std::max(0, totalH - height());
	}

	void TimelineView::paintEvent(QPaintEvent*)
	{
		QPainter p(this);
		p.fillRect(rect(), QColor(35, 35, 35));

		const Int32 w = width();
		const Int32 h = height();

		p.save();
		p.translate(0, -mVScrollPx);

		const Int32 laneH = 70;
		p.setPen(QColor(55, 55, 55));

		const Int32 totalLanes = mProject ? (Int32)mProject->tracks.size() : std::max(1, h / laneH);
		const Int32 firstLane = std::max(0, mVScrollPx / laneH);
		const Int32 lastLane = std::min(totalLanes, firstLane + (h / laneH) + 3);

		for (int i = firstLane; i <= lastLane; ++i)
			p.drawLine(0, i * laneH, w, i * laneH);

		if (!mProject)
			return;

		const Float64 framesPerPixel = getFramesPerPixel();
		const Int64 visibleFrames = (Int64)std::max(1.0, framesPerPixel * w);
		const Int64 start = mStartFrame;
		const Int64 end = start + visibleFrames;


		for (Usize ti = 0; ti < mProject->tracks.size(); ++ti)
		{
			const int y0 = (int)ti * laneH + 10;
			const int y1 = y0 + laneH - 20;

			if (y1 < 0 || y0 > h)
				continue;

			for (Usize ci = 0; ci < mProject->tracks[ti].clips.size(); ++ci)
			{
				const auto& clip = mProject->tracks[ti].clips[ci];

				if (!clip.source)
					continue;

				const Int64 clipStart = clip.startFrameOnTimeline;
				const Int64 clipLen = (clip.sourceOutFrame - clip.sourceInFrame);
				const Int64 clipEnd = clipStart + clipLen;

				const Int64 a = std::max<Int64>(start, clipStart);
				const Int64 b = std::min<Int64>(end, clipEnd);

				if (b <= a)
					continue;

				const Int32 x0 = (Int32)std::round((a - start) / framesPerPixel);
				const Int32 x1 = (Int32)std::round((b - start) / framesPerPixel);

				QRect r(x0, y0, std::max(2, x1 - x0), y1 - y0);
				QColor base = generateClipColor(clip.id);
				QLinearGradient g(r.topLeft(), r.bottomLeft());

				g.setColorAt(0.0, base.lighter(115));
				g.setColorAt(1.0, base.darker(115));

				p.fillRect(r, g);
				p.setPen(base.darker(160));
				p.drawRect(r);

				drawWaveform(p, *clip.source, clip, r, start, end, framesPerPixel);

				p.setPen(QColor(220, 220, 220));
				p.drawText(r.adjusted(6, 0, -6, 0), Qt::AlignVCenter | Qt::AlignLeft, QString("Clip %1").arg((int)clip.id));

				bool selected = mActiveClip && (int)ti == mActiveClip->trackIndex && ci == mActiveClip->clipIndex;
				p.setPen(selected ? QColor(255, 255, 255, 180) : QColor(120, 120, 160));
				p.drawRect(r);
			}
		}

		p.restore();

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

		QPoint pt = e->pos();
		pt.ry() += mVScrollPx;

		if (e->button() == Qt::LeftButton)
		{

			mActiveClip = hitTestClip(pt);

			if (mActiveClip)
			{
				const auto& clip = mProject->tracks[(Usize)mActiveClip->trackIndex].clips[(Usize)mActiveClip->clipIndex];

				mDragging = true;
				mDragStartMouse = pt;
				mDragStartClipFrame = clip.startFrameOnTimeline;

				e->accept();
				update();
				return;
			}

			const auto frame = xToFrame(pt.x());
			mPlayheadFrame = std::max<Int64>(0, frame);
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

		const Int32 dx = e->pos().x() - mDragStartMouse.x();

		const Int64 targetFrame = xToFrame(mDragStartMouse.x() + dx);
		const Int64 delta = targetFrame - xToFrame(mDragStartMouse.x());

		auto& clip = mProject->tracks[(Usize)mActiveClip->trackIndex].clips[(Usize)mActiveClip->clipIndex];

		clip.startFrameOnTimeline = std::max<Int64>(0, mDragStartClipFrame + delta);

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

		const Bool8 shiftIsPressed = (e->modifiers() & Qt::ShiftModifier);

		QPointF pixelDelta = e->pixelDelta();
		QPoint angleDelta = e->angleDelta();

		Float64 steps = 0.0;
		if (!pixelDelta.isNull())
		{
			steps = pixelDelta.y() / 120.0;

			if (std::abs(steps) < 1e-6)
				steps = pixelDelta.x() / 120.0;
		}
		else
		{
			steps = angleDelta.y() / 120.0;
			if (std::abs(steps) < 1e-6)
				steps = angleDelta.x() / 120.0;
		}

		if (std::abs(steps) < 1e-6)
		{
			e->accept();
			return;
		}

		const Float64 oldZoom = mZoom;
		const Float64 oldFpp = getFramesPerPixel();
		const Int32 mouseX = (int)std::round(e->position().x());

		if (shiftIsPressed)
		{
			const Float64 panPixelsPerStep = 200.0;
			const Float64 panPixels = -steps * panPixelsPerStep;
			const Int64 panFrames = (Int64)std::llround(panPixels * oldFpp);

			mStartFrame = std::max<Int64>(0, mStartFrame + panFrames);

			update();
			e->accept();
			emit viewChanged();
			return;
		}

		const Int64 anchorFrame = xToFrame(mouseX);

		const Float64 factorPerStep = 1.12;
		const Float64 factor = std::pow(factorPerStep, steps);

		mZoom = std::clamp(mZoom * factor, 0.1, 500.0);
		const Float64 newFpp = getFramesPerPixel();

		Int64 newStart = (Int64)std::llround((Float64)anchorFrame - (Float64)mouseX * newFpp);
		mStartFrame = std::max<Int64>(0, newStart);

		update();
		e->accept();
		emit viewChanged();
		return;
	}

	Int64 TimelineView::xToFrame(int x) const
	{
		const Float64 framesPerPixel = getFramesPerPixel();
		return mStartFrame + (Int64)std::llround((Float64)x * framesPerPixel);
	}

	int TimelineView::frameToX(Int64 frame) const
	{
		const Float64 framesPerPixel = getFramesPerPixel();
		return (int)std::llround((Float64)(frame - mStartFrame) / framesPerPixel);
	}

	Optional<HitClip> TimelineView::hitTestClip(const QPoint& pt) const
	{
		if (!mProject)
			return std::nullopt;

		const Int32 laneH = 70;
		const Int32 trackIdx = pt.y() / laneH;

		if (trackIdx < 0 || trackIdx >= (Int32)mProject->tracks.size())
			return std::nullopt;

		const auto& track = mProject->tracks[(Usize)trackIdx];

		for (int ci = 0; ci < (Int32)track.clips.size(); ++ci)
		{
			const auto& clip = track.clips[(Usize)ci];

			if (!clip.source)
				continue;

			const Int64 clipStart = clip.startFrameOnTimeline;
			const Int64 clipLen = (clip.sourceOutFrame - clip.sourceInFrame);
			const Int64 clipEnd = clipStart + clipLen;

			const Int32 x0 = frameToX(clipStart);
			const Int32 x1 = frameToX(clipEnd);

			const Int32 y0 = trackIdx * laneH + 10;
			const Int32 y1 = y0 + laneH - 20;

			QRect r(x0, y0, std::max(2, x1 - x0), y1 - y0);

			if (r.contains(pt))
				return HitClip{ trackIdx, ci };
		}

		return std::nullopt;
	}

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
		const Int32 px1 = std::min((int)p.viewport().width(), xB);

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