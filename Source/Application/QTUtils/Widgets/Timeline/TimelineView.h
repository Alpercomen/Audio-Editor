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

		std::int64_t playheadFrame() const { return mPlayheadFrame; }

	signals:
		void seekRequested(std::int64_t frame);

	protected:
		void paintEvent(QPaintEvent* e) override;

		void mousePressEvent(QMouseEvent* e) override;
		void mouseMoveEvent(QMouseEvent* e) override;
		void mouseReleaseEvent(QMouseEvent* e) override;

		void wheelEvent(QWheelEvent* e) override;

	private:
		std::shared_ptr<Audio::Project> mProject;
		std::int64_t mPlayheadFrame = 0;

		double mZoom = 1.0;
		std::int64_t mStartFrame = 0;

		std::optional<HitClip> mActiveClip;
		bool mDragging = false;

		QPoint mDragStartMouse;
		std::int64_t mDragStartClipFrame = 0;

		std::int64_t xToFrame(int x) const;
		int frameToX(std::int64_t frame) const;
		std::optional<HitClip> hitTestClip(const QPoint& pt) const;
	};

	void drawWaveform(QPainter& p, const Audio::AudioSource& src, const Audio::Clip& clip, const QRect& r, int outChannels);
	void drawWaveformVisibleSlice(QPainter& p, const Audio::AudioSource& src, const Audio::Clip& clip, const QRect& clipRect, std::int64_t viewStartFrame, std::int64_t viewEndFrame, double framesPerPixel);
}