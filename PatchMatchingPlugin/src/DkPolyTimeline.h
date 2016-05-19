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

	// label for rendering one timeline element
	// not used yet
	class DkTimelineLabel : public QLabel
	{
		Q_OBJECT
	public:
		DkTimelineLabel(QWidget* parent = 0);
		virtual ~DkTimelineLabel() = default;
	
	};

	// we have again different types, namely
	enum class TimeLinePointType {
		Control,	// actual control point at this position
		Line,		// position is interpolated on line
	};

	// takes a syncedpolygon and interpolates the 
	// point on the edges with a given stepsize
	class DkInterpolatedSyncedPolyon : public QObject
	{
		Q_OBJECT
	public: 
		// ctor with synced poly
		DkInterpolatedSyncedPolyon(QSharedPointer<DkSyncedPolygon> poly);
		virtual ~DkInterpolatedSyncedPolyon() = default;
		
		// setter/getter for the stepsize (in pixel)
		// can be changed on the fly
		void setStep(double step);
		auto getStep();

		// num of points
		auto size();
		// and actual points 
		const auto& points() const;

	public slots:
		void recalcuate();

	signals:
		void changed();
		
	private:
		QSharedPointer<DkSyncedPolygon> mPolygon;		// storage for control points and changes
		QVector<std::pair<TimeLinePointType, QPointF>> mPoints;		// the calcuated points
		double mStep;	
	};

	// since the timeline is just a view on the synced polygon with a transform
	// all we need is a transform which is updated on changes
	// this class provides this
	// and we have a color. because transforms like to have color too...
	class DkTrackedTransform : public QObject
	{
		Q_OBJECT
	public:
		DkTrackedTransform(QColor color, QTransform transform = QTransform());
		virtual ~DkTrackedTransform() = default;
		QTransform get();
		QColor getColor();

	signals:
		void changed(QTransform newvalue);
	
	public slots:
		void set(QTransform value);

	private:
		QTransform mTransform;
		QColor mColor;
	};

	// the label handles all the drawing stuff
	// uses a grid and a scroll area
	// image has to be set in order for this all to work!
	// transforms are used to represent different renderer 
	class DkPolyTimeline : public QLabel
	{
		Q_OBJECT

	public:
		// ctor with synced poly and parent (is dock in our case)
		DkPolyTimeline(QSharedPointer<DkSyncedPolygon> poly, QWidget* parent = 0);
		virtual ~DkPolyTimeline() = default;

		// adds a polygon (renderer), i.e. a transform 
		// returns transform to allow connecting the signals/slots to the renderer
		QSharedPointer<DkTrackedTransform> addPolygon(QColor color);
		void reset();

	public slots:
		// should be set
		void setImage(QSharedPointer<nmc::DkImageContainerT> img);
		void updateTransform(QSharedPointer<DkTrackedTransform> sender);
		
		// recalculates the needed widgets and redraws everything
		void refresh();

		// removes a row/renderer
		void removeTransform(QSharedPointer<DkTrackedTransform> sender);

	private:
		// recalculates how many widgets are needed and add or removes thems
		void updateEmptyElements();

		void setGridElement(QWidget* widget, int row, int column);
		void clearGridElement(QWidget* widget);

		// initializer list
		// could be put in something else
		QSharedPointer<DkInterpolatedSyncedPolyon> mPolygon;	//synced polygon wrapped to interpolate
		qreal mSize;		// size in pixels of the rendered patches
		QGridLayout* mLayout;

		// other members
		QVector<QSharedPointer<DkTrackedTransform>> mList;   //list of transforms (renderer)
		QVector<QVector<QLabel*>> mElements;					//label elements for rendering
		QPixmap mImage;											//image to do stuff
		
	};

}

