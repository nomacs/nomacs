#pragma once
#include <qdockwidget>
#include <QVBoxLayout>
#include <vector>
#include <memory>
#include <QScrollArea>
class DkSingleTimeline : public QWidget
{
	Q_OBJECT
public:
	DkSingleTimeline(QWidget* parent = 0);
	virtual ~DkSingleTimeline();

public slots:
	void refresh();

private:
	void clear();
	void addElement();

	QHBoxLayout* mLayout;
	std::vector<std::unique_ptr<QImage>> mElements;
};

class DkPolyTimeline : public QWidget
{
	Q_OBJECT
public:
	DkPolyTimeline();
	virtual ~DkPolyTimeline();
	void addPolygon(/*don't know which parameters yet*/);

private:
	QVBoxLayout* mLayout;
	QWidget* mWidget;
	QScrollArea* mScrollArea;
};

