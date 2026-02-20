#include <Application/QTUtils/Widgets/Editor/EditorWidget.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QUrl>

namespace UI
{
	EditorWidget::EditorWidget(QWidget* parent) : QWidget(parent)
	{
		setAcceptDrops(true);

		auto* root = new QVBoxLayout(this);
		root->setContentsMargins(12, 12, 12, 12);
		root->setSpacing(10);

		pTimeline = new TimelineView(this);
		root->addWidget(pTimeline, 1);

		pHint = new QLabel(this);
		pHint->setText("No audio loaded. Use File -> Open or drag & drop an audio file here.");
		pHint->setAlignment(Qt::AlignCenter);
		root->addWidget(pHint, 0);

		// playhead update
		mPlayheadTimer.setInterval(16);
		connect(&mPlayheadTimer, &QTimer::timeout, this, [this]()
			{
				if (!pTimeline) return;
				pTimeline->setPlayheadFrame(mPlayback.getCurrentFrame());
			});

		// click-to-seek
		connect(pTimeline, &TimelineView::seekRequested, this, [this](std::int64_t f)
			{
				mPlayback.seekToFrame(f);
			});
	}

	void EditorWidget::setDocument(Audio::AudioDocument doc, QString)
	{
		if (!doc.isValid())
			return;

		if (!mProject)
		{
			mProject = std::make_shared<Audio::Project>();
			mProject->sampleRate = doc.sampleRate;
			mProject->channels = std::max(1, doc.channels);
			mProject->tracks.clear();
		}

		auto src = std::make_shared<Audio::AudioSource>();
		src->sampleRate = doc.sampleRate;
		src->channels = doc.channels;
		src->interleaved = doc.interleaved;

		mProject->tracks.push_back(Audio::Track{ "Track " + std::to_string(mProject->tracks.size() + 1) });
		auto& track = mProject->tracks[mProject->tracks.size() - 1];

		Audio::Clip clip;
		clip.source = src;
		clip.startFrameOnTimeline = 0;
		clip.sourceInFrame = 0;
		clip.sourceOutFrame = src->frames();

		track.clips.push_back(std::move(clip));
		mProject->tracks[0].clips.push_back(std::move(clip));
		mProject->recomputeLength();

		pTimeline->setProject(mProject);
		pTimeline->setPlayheadFrame(0);

		mPlayback.setProject(mProject);
	}

	void EditorWidget::play()
	{
		mPlayback.play();
		mPlayheadTimer.start();
	}

	void EditorWidget::stop()
	{
		mPlayback.stop();
		mPlayheadTimer.stop();
	}

	void EditorWidget::pause()
	{
		mPlayback.pause();
		mPlayheadTimer.stop();
	}

	void EditorWidget::resume()
	{
		mPlayback.resume();
		mPlayheadTimer.start();
	}

	void EditorWidget::playFromStart()
	{
		mPlayback.playFromStart();
		mPlayheadTimer.start();
	}

	void EditorWidget::togglePlayPause()
	{
		if (mPlayback.isPlaying())
		{
			mPlayback.pause();
			return;
		}

		if (mPlayback.isPaused() || mPlayback.hasResumePosition())
		{
			mPlayback.resume();
			mPlayheadTimer.start();
			return;
		}

		mPlayback.playFromStart();
		mPlayheadTimer.start();
	}

	void EditorWidget::dragEnterEvent(QDragEnterEvent* e)
	{
		if (e->mimeData()->hasUrls())
			e->acceptProposedAction();
	}

	void EditorWidget::dropEvent(QDropEvent* e)
	{
		const auto urls = e->mimeData()->urls();
		if (urls.isEmpty()) return;

		const QString path = urls.first().toLocalFile();
		if (path.isEmpty()) return;

		std::string err;
		auto doc = Audio::AudioDocument::LoadFromFile(path.toStdString(), err);
		if (!doc.isValid()) {
			QMessageBox::critical(this, "Failed to load", QString::fromStdString(err));
			return;
		}

		setHintText(QString("Loaded:\n%1\n%2 Hz | %3 ch | %4 sec")
			.arg(path)
			.arg(doc.sampleRate)
			.arg(doc.channels)
			.arg(doc.durationSeconds(), 0, 'f', 2));

		setDocument(std::move(doc));
		e->acceptProposedAction();
	}

	void EditorWidget::setHintText(const QString& text)
	{
		pHint->setText(text);
	}
}