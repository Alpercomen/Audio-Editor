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

		auto* grid = new QGridLayout();
		grid->setContentsMargins(0, 0, 0, 0);
		grid->setSpacing(0);

		pTimeline = new TimelineView(this);
		mVScroll = new QScrollBar(Qt::Vertical, this);
		mHScroll = new QScrollBar(Qt::Horizontal, this);

		pTrackHeader = new TrackHeaderView(this);

		grid->addWidget(pTrackHeader, 0, 0);
		grid->addWidget(pTimeline, 0, 1);
		grid->addWidget(mVScroll, 0, 2);
		grid->addWidget(mHScroll, 1, 1);

		grid->setColumnStretch(0, 0);
		grid->setColumnStretch(1, 1);
		grid->setColumnStretch(2, 0);

		auto* corner = new QWidget(this);
		corner->setFixedSize(16, 16);
		grid->addWidget(corner, 1, 2);

		root->addLayout(grid, 1);
		root->addWidget(pHint, 0);

		pHint = new QLabel(this);
		pHint->setText("No audio loaded. Use File -> Open or drag & drop an audio file here.");
		pHint->setAlignment(Qt::AlignCenter);
		root->addWidget(pHint, 0);

		mPlayheadTimer.setInterval(16);

		connect(&mPlayheadTimer, &QTimer::timeout, this, [this]()
			{
				if (pTimeline)
					pTimeline->setPlayheadFrame(mPlayback.getCurrentFrame());
			});

		connect(pTimeline, &TimelineView::seekRequested, this, [this](Int64 f)
			{
				mPlayback.seekToFrame(f);
			});

		connect(mHScroll, &QScrollBar::valueChanged, this, [this](int v)
			{
				if (pTimeline)
					pTimeline->setViewStartFrame((Int64)v);
			});

		connect(mVScroll, &QScrollBar::valueChanged, this, [this](int v)
			{
				if (pTimeline)
					pTimeline->setVerticalScrollPx(v);

				if (pTrackHeader) 
					pTrackHeader->setVerticalScrollPx(v);
			});

		connect(pTimeline, &TimelineView::viewChanged, this, [this]
			{
				syncScrollbarsFromView();
			});
	}

	void EditorWidget::setDocument(Audio::AudioDocument doc, QString)
	{
		if (!doc.isValid())
			return;

		if (!mProject)
		{
			mProject = MakeShared<Audio::Project>();
			mProject->sampleRate = doc.sampleRate;
			mProject->channels = std::max(1, doc.channels);
			mProject->tracks.clear();
		}

		auto src = MakeShared<Audio::AudioSource>();
		src->sampleRate = doc.sampleRate;
		src->channels = doc.channels;
		src->interleaved = doc.interleaved;

		mProject->tracks.push_back(Audio::Track{ "Track " + std::to_string(mProject->tracks.size() + 1) });
		auto& track = mProject->tracks[mProject->tracks.size() - 1];

		static Uint64 gClipId = 1;

		Audio::Clip clip;
		clip.id = gClipId++;
		clip.source = src;
		clip.startFrameOnTimeline = pTimeline ? pTimeline->playheadFrame() : mProject->lengthFrames;
		clip.sourceInFrame = 0;
		clip.sourceOutFrame = src->frames();

		track.clips.push_back(std::move(clip));
		mProject->recomputeLength();

		pTimeline->setProject(mProject);
		pTrackHeader->setProject(mProject);
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
		if (urls.isEmpty())
			return;

		const QString path = urls.first().toLocalFile();
		if (path.isEmpty())
			return;

		String err;
		auto doc = Audio::AudioDocument::LoadFromFile(path.toStdString(), err);
		if (!doc.isValid())
		{
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

	void EditorWidget::syncScrollbarsFromView()
	{
		if (!pTimeline)
			return;

		const auto maxH = pTimeline->maxStartFrame();
		mHScroll->setRange(0, (int)maxH);
		mHScroll->setPageStep(std::max(1, (int)(pTimeline->viewEndFrame() - pTimeline->viewStartFrame())));
		mHScroll->setValue((int)pTimeline->viewStartFrame());

		const int maxV = pTimeline->maxVerticalScrollPx();
		mVScroll->setRange(0, maxV);
		mVScroll->setPageStep(std::max(1, pTimeline->height()));
		mVScroll->setValue(pTimeline->verticalScrollPx());
	}
}