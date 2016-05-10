#pragma once
#include <qdockwidget>
#include <QVBoxLayout>
#include <vector>
#include <memory>
#include <QScrollArea>
#include "DkSyncedPolygon.h"

namespace nmp {

	class DkPolyTimeline;

	class DkSingleTimeline : public QWidget
	{
		Q_OBJECT
	public:
		DkSingleTimeline(QWidget* parent = 0);
		virtual ~DkSingleTimeline();
		void setPolygon(std::shared_ptr<DkSyncedPolygon> poly);

	public slots:
		void setTransform(QTransform transform);
		void update();

	private:
		void clear();
		void addElement();

		//std::vector<std::unique_ptr<QImage>> mElements;

		QHBoxLayout* mLayout;
		QTransform mTransform;
		DkPolyTimeline* mParent;
		std::shared_ptr<DkSyncedPolygon> mPoly;		//<! polygon stores the saved state
	};

	class DkPolyTimeline : public QWidget
	{
		Q_OBJECT
	public:
		DkPolyTimeline(QWidget* parent = 0);
		virtual ~DkPolyTimeline();
		DkSingleTimeline* addPolygon();

	private:
		
		QVBoxLayout* mLayout;
		QWidget* mWidget;
		QScrollArea* mScrollArea;
	};

}

