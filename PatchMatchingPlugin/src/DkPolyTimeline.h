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

	enum class TimeLinePointType {
		Control,
		Line,
	};

	struct DkTimeLinePoint {
		QWidget* Widget;
		TimeLinePointType Type;
		QPointF Pos;
	};

	class DkInterpolatedSyncedPolyon : public QObject
	{
		Q_OBJECT
	public: 
		DkInterpolatedSyncedPolyon(QSharedPointer<DkSyncedPolygon> poly);
		virtual ~DkInterpolatedSyncedPolyon();
		
		void setStep(double step);
		auto getStep();
		auto size();
		const auto& points() const;

	public slots:
		void recalcuate();
		
		//void addPoint();

	signals:
		void changed();
		
	private:
		QSharedPointer<DkSyncedPolygon> mPolygon;
		QVector<std::pair<TimeLinePointType, QPointF>> mPoints;
		double mStep;
	};

	//class DkSingleTimeline : public QObject
	//{
	//	Q_OBJECT
	//public:
	//	DkSingleTimeline(int row, DkPolyTimeline* parent = 0);
	//	virtual ~DkSingleTimeline();
	//	void setPolygon(QSharedPointer<DkSyncedPolygon> poly);
	//	void clear();
	//	void setImage(QSharedPointer<nmc::DkImageContainerT> img);

	//public slots:
	//	void setTransform(QTransform transform);
	//	void refresh();
	//	void addPoint(QSharedPointer<DkControlPoint> point);
	//	void redraw();

	//private:
	//	void addElement(QSharedPointer<DkControlPoint> point);

	//	
	//	QTransform mTransform;
	//	DkPolyTimeline* mParent;
	//		//<! polygon stores the saved state
	//	int mLayoutRow;
	//};

	class DkTrackedTransform : public QObject
	{
		Q_OBJECT
	public:
		DkTrackedTransform(QTransform transform = QTransform());
		virtual ~DkTrackedTransform();
		QTransform get();

	signals:
		void changed(QTransform newvalue);
	
	public slots:
		void set(QTransform value);

	private:
		QTransform mTransform;
	};

	class DkPolyTimeline : public QLabel
	{
		Q_OBJECT

	public:
		DkPolyTimeline(QSharedPointer<DkSyncedPolygon> poly, QWidget* parent = 0);
		virtual ~DkPolyTimeline();
		QSharedPointer<DkTrackedTransform> addPolygon();
		void reset();

	public slots:
		void setImage(QSharedPointer<nmc::DkImageContainerT> img);
		void updateTransform(QSharedPointer<DkTrackedTransform> sender);
		void refresh();
		void removeTransform(QSharedPointer<DkTrackedTransform> sender);

	private:
		void updateEmptyElements();
		void setGridElement(QWidget* widget, int row, int column);
		void clearGridElement(QWidget* widget);

		QGridLayout* mLayout;
		QScrollArea* mScrollArea;

		QVector<QSharedPointer<DkTrackedTransform>> mList;   //list of transforms (renderer)
		QVector<QVector<QLabel*>> mElements;					//label elements for rendering

		// could be put in something else
		QSharedPointer<DkInterpolatedSyncedPolyon> mPolygon;	//synced polygon wrapped to interpolate
		QPixmap mImage;											//image to do stuff
	};

}

