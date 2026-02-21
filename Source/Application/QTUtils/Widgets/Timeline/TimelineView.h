#pragma once
#include <Application/Core/Project/Project.h>
#include <Application/Core/Data.h>

#include <memory>
#include <optional>
#include <cstdint>

#include <QWidget>

namespace UI
{
	struct HitClip
	{
		int trackIndex = -1;
		int clipIndex = -1;
	};

	class TimelineView : public QWidget
	{
		Q_OBJECT
	public:
		explicit TimelineView(QWidget* parent = nullptr) : QWidget(parent) {}

		void setProject(std::shared_ptr<Audio::Project> p);
		void setPlayheadFrame(std::int64_t f);
		double getFramesPerPixel() const { return 200.0 / std::max(0.1, mZoom); }

		void setViewStartFrame(std::int64_t f);
		std::int64_t viewStartFrame() const { return mStartFrame; }

		void setVerticalScrollPx(int px);
		int verticalScrollPx() const { return mVScrollPx; }

		std::int64_t viewEndFrame() const;
		std::int64_t maxStartFrame() const;
		int maxVerticalScrollPx() const;

		std::int64_t playheadFrame() const { return mPlayheadFrame; }

	signals:
		void seekRequested(std::int64_t frame);

	signals:
		void viewChanged();

	protected:
		void paintEvent(QPaintEvent* e) override;

		void mousePressEvent(QMouseEvent* e) override;
		void mouseMoveEvent(QMouseEvent* e) override;
		void mouseReleaseEvent(QMouseEvent* e) override;

		void wheelEvent(QWheelEvent* e) override;

	private:
		std::shared_ptr<Audio::Project> mProject;
		std::optional<HitClip> mActiveClip;

		std::int64_t mPlayheadFrame = 0;
		std::int64_t mStartFrame = 0;
		std::int64_t mDragStartClipFrame = 0;

		double mZoom = 1.0;
		int mVScrollPx = 0;
		bool mDragging = false;

		QPoint mDragStartMouse;

		int frameToX(std::int64_t frame) const;
		std::int64_t xToFrame(int x) const;
		std::optional<HitClip> hitTestClip(const QPoint& pt) const;
	};

	void drawWaveform(QPainter& p, const Audio::AudioSource& src, const Audio::Clip& clip, const QRect& clipRect, std::int64_t viewStartFrame, std::int64_t viewEndFrame, double framesPerPixel);
}