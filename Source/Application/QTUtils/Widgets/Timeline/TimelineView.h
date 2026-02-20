#pragma once
#include <Application/Core/Project/Project.h>
#include <Application/Core/Data.h>

#include <QWidget>
#include <memory>

namespace UI
{
	class TimelineView : public QWidget
	{
		Q_OBJECT
	public:
		explicit TimelineView(QWidget* parent = nullptr) : QWidget(parent) {}

		void setProject(std::shared_ptr<Audio::Project> p);
		void setPlayheadFrame(std::int64_t f);
		std::int64_t playheadFrame() const { return mPlayheadFrame; }

	signals:
		void seekRequested(std::int64_t frame);

	protected:
		void paintEvent(QPaintEvent* e) override;
		void mousePressEvent(QMouseEvent* e) override;

	private:
		std::shared_ptr<Audio::Project> mProject;
		std::int64_t mPlayheadFrame = 0;

		double mZoom = 1.0;
		std::int64_t mStartFrame = 0;
	};

	void drawWaveform(QPainter& p, const Audio::AudioSource& src, const Audio::Clip& clip, const QRect& r, int outChannels);
}