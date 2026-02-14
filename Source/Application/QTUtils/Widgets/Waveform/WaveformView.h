#pragma once
#include <Application/Audio/Document/AudioDocument.h>

#include <QWidget>

namespace UI
{

	class WaveformView : public QWidget
	{
		Q_OBJECT
	public:
		explicit WaveformView(QWidget* parent = nullptr) : QWidget(parent) {}

		void setZoom(double zoom) { mZoom = zoom; }
		void setCenterFrame(std::int64_t centerFrame) { mCenterFrame = centerFrame; }

		double getZoom() const { return mZoom; }
		std::int64_t getCenterFrame() const { return mCenterFrame; }

		void fitToView()
		{
			mZoom = 1.0;

			if (!mDoc.isValid()) 
			{
				mCenterFrame = 0;
				update();
				return;
			}

			mCenterFrame = mDoc.frames / 2;
			update();
		}

		void setDocument(Audio::AudioDocument doc, QString sourcePath = {})
		{
			mDoc = std::move(doc);
			fitToView();
		}

		const Audio::AudioDocument& getDocument() const { return mDoc; }

	protected:
		void paintEvent(QPaintEvent* e) override;
		void wheelEvent(QWheelEvent* e) override;

	private:
		Audio::AudioDocument mDoc;
		QString mPath;

		double mZoom = 1.0;
		std::int64_t mCenterFrame = 0;
	};
}
