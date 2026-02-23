#pragma once
#include <Application/Audio/Document/AudioDocument.h>
#include <Application/Audio/Playback/AudioPlayback.h>
#include <Application/QTUtils/Widgets/Timeline/TimelineView.h>
#include <Application/Core/Project/Project.h>
#include <Application/Core/Data.h>

#include <QWidget>
#include <QTimer>
#include <QScrollBar>

#include <memory>

class QLabel;

namespace UI
{
	class EditorWidget : public QWidget
	{
		Q_OBJECT
	public:
		explicit EditorWidget(QWidget* parent = nullptr);

		void play();
		void stop();
		void pause();
		void resume();
		void playFromStart();

		bool isPlaying() const { return mPlayback.isPlaying(); }
		bool isPaused()  const { return mPlayback.isPaused(); }
		bool hasAudio() const { return mProject && !mProject->tracks.empty() && !mProject->tracks[0].clips.empty(); }

		void togglePlayPause();

		void setDocument(Audio::AudioDocument doc, QString sourcePath = {});

	protected:
		void dragEnterEvent(QDragEnterEvent* e) override;
		void dropEvent(QDropEvent* e) override;

	private:
		UI::TimelineView* pTimeline = nullptr;
		Audio::AudioPlayback mPlayback;
		SharedPtr<Audio::Project> mProject;

		QLabel* pHint = nullptr;
		QTimer mPlayheadTimer;

		QScrollBar* mHScroll = nullptr;
		QScrollBar* mVScroll = nullptr;

		void setHintText(const QString& text);
		void syncScrollbarsFromView();
	};
}