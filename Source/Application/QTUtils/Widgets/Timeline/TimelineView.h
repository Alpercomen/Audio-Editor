#pragma once
#include <Application/Core/Project/Project.h>
#include <Application/Core/Core.h>

#include <memory>
#include <optional>
#include <cstdint>

#include <QWidget>

namespace UI
{
	class TimelineView : public QWidget
	{
		Q_OBJECT
	public:
		explicit TimelineView(QWidget* parent = nullptr) : QWidget(parent) {}

		void setProject(SharedPtr<Audio::Project> p);
		void setPlayheadFrame(Int64 f);
		Float64 getFramesPerPixel() const { return 200.0 / std::max(0.1, mZoom); }

		void setViewStartFrame(Int64 f);
		Int64 viewStartFrame() const { return mStartFrame; }

		void setVerticalScrollPx(Int32 px);
		Int32 verticalScrollPx() const { return mVScrollPx; }

		Int64 viewEndFrame() const;
		Int64 maxStartFrame() const;
		Int32 maxVerticalScrollPx() const;

		Int64 playheadFrame() const { return mPlayheadFrame; }

	signals:
		void seekRequested(Int64 frame);

	signals:
		void viewChanged();

	protected:
		void paintEvent(QPaintEvent* e) override;

		void mousePressEvent(QMouseEvent* e) override;
		void mouseMoveEvent(QMouseEvent* e) override;
		void mouseReleaseEvent(QMouseEvent* e) override;

		void wheelEvent(QWheelEvent* e) override;

	private:
		SharedPtr<Audio::Project> mProject;
		Optional<HitClip> mActiveClip;

		Int64 mPlayheadFrame = 0;
		Int64 mStartFrame = 0;
		Int64 mDragStartClipFrame = 0;

		Float64 mZoom = 1.0;
		Int32 mVScrollPx = 0;
		Bool8 mDragging = false;

		QPoint mDragStartMouse;

		Int32 frameToX(Int64 frame) const;
		Int64 xToFrame(int x) const;
		Optional<HitClip> hitTestClip(const QPoint& pt) const;
	};

	void drawWaveform(QPainter& p, const Audio::AudioSource& src, const Audio::Clip& clip, const QRect& clipRect, Int64 viewStartFrame, Int64 viewEndFrame, Float64 framesPerPixel);
}