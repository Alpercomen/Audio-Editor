#include "TrackHeaderView.h"
#include <Application/Core/Core.h>

#include <QPainter>
#include <QMouseEvent>

#include <algorithm>
#include <cmath>

namespace UI
{
	void TrackHeaderView::paintEvent(QPaintEvent*)
	{
        QPainter p(this);
        p.fillRect(rect(), QColor(28, 28, 28));

        const Int32 w = width();
        const Int32 h = height();

        p.save();
        p.translate(0, -mVScrollPx);

        // lane lines
        p.setPen(QColor(55, 55, 55));
        const Int32 totalLanes = mProject ? (Int32)mProject->tracks.size() : std::max(1, h / laneH);
        const Int32 firstLane = std::max(0, mVScrollPx / laneH);
        const Int32 lastLane = std::min(totalLanes, firstLane + (h / laneH) + 3);

        for (Int32 i = firstLane; i <= lastLane; ++i)
            p.drawLine(0, i * laneH, w, i * laneH);

        if (!mProject)
        {
            p.restore();
            return;
        }

        for (Int32 ti = 0; ti < (Int32)mProject->tracks.size(); ++ti)
        {
            const Int32 y0 = ti * laneH;

            p.fillRect(QRect(0, y0, w, laneH), QColor(30, 30, 30));

            auto& tr = mProject->tracks[(size_t)ti];

            p.setPen(QColor(220, 220, 220));
            p.drawText(QRect(12, y0 + 6, w - 24, 14), Qt::AlignLeft | Qt::AlignVCenter,
                QString::fromStdString(tr.name));

            QRect rm = muteRectForRow(y0);
            p.fillRect(rm, tr.muted ? QColor(220, 80, 80) : QColor(60, 60, 60));
            p.setPen(QColor(15, 15, 15));
            p.drawRect(rm);
            p.setPen(QColor(240, 240, 240));
            p.drawText(rm, Qt::AlignCenter, "Mute");

            QRect rmo = monoRectForRow(y0);
            p.fillRect(rmo, tr.forceMono ? QColor(90, 180, 90) : QColor(60, 60, 60));
            p.setPen(QColor(15, 15, 15));
            p.drawRect(rmo);
            p.setPen(QColor(240, 240, 240));
            p.drawText(rmo, Qt::AlignCenter, "Mono");

            QRect rp = panRectForRow(y0);
            p.fillRect(rp, QColor(45, 45, 45));
            p.setPen(QColor(15, 15, 15));
            p.drawRect(rp);

            const Int32 cx = rp.left() + rp.width() / 2;
            p.setPen(QColor(120, 120, 120));
            p.drawLine(cx, rp.top(), cx, rp.bottom());

            const Float32 t = (tr.pan + 1.0f) * 0.5f;
            const Int32 kx = rp.left() + (Int32)std::round(t * rp.width());
            p.setPen(QColor(220, 220, 220));
            p.drawLine(kx, rp.top(), kx, rp.bottom());
        }

        p.restore();
	}

	void TrackHeaderView::mousePressEvent(QMouseEvent* e)
	{
        if (!mProject || e->button() != Qt::LeftButton)
            return QWidget::mousePressEvent(e);

        QPoint pt = e->pos();
        pt.ry() += mVScrollPx;

        const Int32 ti = pt.y() / laneH;
        if (ti < 0 || ti >= (Int32)mProject->tracks.size())
            return;

        auto& tr = mProject->tracks[(size_t)ti];
        const Int32 y0 = ti * laneH;

        const QRect rm = muteRectForRow(y0);
        const QRect rmo = monoRectForRow(y0);
        const QRect rp = panRectForRow(y0);

        if (rm.contains(pt))
        {
            tr.muted = !tr.muted;
            emit trackParamsChanged();
            update();
            e->accept();
            return;
        }

        if (rmo.contains(pt))
        {
            tr.forceMono = !tr.forceMono;
            emit trackParamsChanged();
            update();
            e->accept();
            return;
        }

        if (rp.contains(pt))
        {
            mPanning = true;
            mPanTrack = ti;
            tr.pan = xToPan(pt.x(), rp);
            emit trackParamsChanged();
            update();
            e->accept();
            return;
        }

        QWidget::mousePressEvent(e);
	}

	void TrackHeaderView::mouseMoveEvent(QMouseEvent* e)
	{
        if (!mProject || !mPanning || mPanTrack < 0)
            return QWidget::mouseMoveEvent(e);

        QPoint pt = e->pos();
        pt.ry() += mVScrollPx;

        auto& tr = mProject->tracks[(size_t)mPanTrack];
        const Int32 y0 = mPanTrack * laneH;
        const QRect rp = panRectForRow(y0);

        tr.pan = xToPan(pt.x(), rp);
        emit trackParamsChanged();
        update();
        e->accept();
	}

	void TrackHeaderView::mouseReleaseEvent(QMouseEvent* e)
	{
        if (e->button() == Qt::LeftButton && mPanning)
        {
            mPanning = false;
            mPanTrack = -1;
            e->accept();
            return;
        }
        QWidget::mouseReleaseEvent(e);
	}

	Float32 TrackHeaderView::xToPan(Int32 x, const QRect& r) const
	{
		const Float32 t = std::clamp((Float32)(x - r.left()) / std::max(1, r.width()), 0.0f, 1.0f);
		return t * 2.0f - 1.0f;
	}
}
