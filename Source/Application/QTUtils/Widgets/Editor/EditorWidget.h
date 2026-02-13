#pragma once
#include <QWidget>

class QLabel;

class EditorWidget : public QWidget
{
	Q_OBJECT
public:
	explicit EditorWidget(QWidget* parent = nullptr);

protected:
	void dragEnterEvent(QDragEnterEvent* e) override;
	void dropEvent(QDropEvent* e) override;

private:
	QLabel* mHint = nullptr;

	void setHintText(const QString& text);
};