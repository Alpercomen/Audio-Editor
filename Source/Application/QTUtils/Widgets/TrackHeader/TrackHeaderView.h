#pragma once
#include <Application/Core/Project/Project.h>

#include <QWidget>

#include <memory>

namespace UI
{
    class TrackHeaderView : public QWidget
    {
        Q_OBJECT
    public:
        explicit TrackHeaderView(QWidget* parent = nullptr) : QWidget(parent)
        {
            setFixedWidth(220);
            setMouseTracking(true);
        }

        void setProject(SharedPtr<Audio::Project> p)
        {
            mProject = std::move(p);
            update();
        }

        void setVerticalScrollPx(Int32 px)
        {
            mVScrollPx = std::max(0, px);
            update();
        }

    signals:
        void trackParamsChanged();

    protected:
        void paintEvent(QPaintEvent*) override;
        void mousePressEvent(QMouseEvent* e) override;
        void mouseMoveEvent(QMouseEvent* e) override;
        void mouseReleaseEvent(QMouseEvent* e) override;

    private:
        SharedPtr<Audio::Project> mProject;
        Int32 mVScrollPx = 0;

        Bool8 mPanning = false;
        Int32 mPanTrack = -1;

        static constexpr Int32 laneH = 70;

        QRect getMuteRect(Int32 y0) const { return QRect(12, y0 + 22, 44, 24); }
        QRect getMonoRect(Int32 y0) const { return QRect(62, y0 + 22, 54, 24); }
        QRect getPanRect(Int32 y0)  const { return QRect(12, y0 + 52, 170, 16); }

        Float32 xToPan(Int32 x, const QRect& r) const;
    };
}