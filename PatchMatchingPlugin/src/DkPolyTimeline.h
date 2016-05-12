#pragma once
#include <qdockwidget>
#include <QVBoxLayout>
#include <vector>
#include <memory>
#include <QScrollArea>
#include "DkSyncedPolygon.h"
#include <QLabel>
#include "DkImageContainer.h"

namespace nmp {

	class DkPolyTimeline;

	class DkTimelineLabel : public QLabel
	{
		Q_OBJECT
	public:
		DkTimelineLabel(QWidget* parent = 0);
		virtual ~DkTimelineLabel();
	
	};
	class DkSingleTimeline : public QObject
	{
		Q_OBJECT
	public:
		DkSingleTimeline(int row, DkPolyTimeline* parent = 0);
		virtual ~DkSingleTimeline();
		void setPolygon(QSharedPointer<DkSyncedPolygon> poly);
		void clear();
		void setImage(QSharedPointer<nmc::DkImageContainerT> img);

	public slots:
		void setTransform(QTransform transform);
		void refresh();
		void addPoint(QSharedPointer<DkControlPoint> point);
		void redraw();

	private:
		void addElement(QSharedPointer<DkControlPoint> point);

		QPixmap mImage;
		QVector<std::pair<QLabel*,QSharedPointer<DkControlPoint>>> mElements;
		QTransform mTransform;
		DkPolyTimeline* mParent;
		QSharedPointer<DkSyncedPolygon> mPoly;		//<! polygon stores the saved state
		int mLayoutRow = 0;
	};

	class DkPolyTimeline : public QLabel
	{
		Q_OBJECT

	public:
		DkPolyTimeline(QWidget* parent = 0);
		virtual ~DkPolyTimeline();
		DkSingleTimeline* addPolygon();
		void reset();
		void setGridElement(QWidget* widget, int row, int column);
		void clearGridElement(QWidget* widget);

	private:
		
		QGridLayout* mLayout;
		QScrollArea* mScrollArea;
		QVector<DkSingleTimeline*> mList;

	};

}

