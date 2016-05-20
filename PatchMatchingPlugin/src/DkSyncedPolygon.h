#pragma once
#include <QWidget>
#include <QPen>
#include <functional>
#include <QJsonObject>
namespace {
	template<class T>
	auto mapToViewPort(const T& pos, QTransform world) {
		return world.inverted().map(pos);
	}
}

namespace nmp {

	class DkPatchMatchingViewPort;
	class DkPolygonRenderer;
	class DkControlPointRepresentation;
	class DkLineRepresentation;

	enum class ControlPointType {
		start,			// point@start, we need to know start and end
		intermediate,	
		center			// center point which controls the whole polygon
	};

	// this class handles movement and 
	// update of one single point (part of a synced polygon)
	class DkControlPoint : public QObject
	{
		Q_OBJECT

	public:

		// ctor just with position
		DkControlPoint(const QPointF& point);
		virtual ~DkControlPoint() = default;

		// position
		void setPos(const QPointF& point);
		QPointF getPos() const;
		
		// type
		void setType(ControlPointType t);
		ControlPointType getType();

	signals:
		// this signal is emitted when the position has changed
		void moved();	

	private:
		QPointF mPoint;
		ControlPointType mType;
	};

	// collection of control point makes up our synced polygon
	// lives in its own coordinate system and is needed 
	// as a container for the control points
	class DkSyncedPolygon : public QObject
	{
		Q_OBJECT

	public:

		DkSyncedPolygon();
		virtual ~DkSyncedPolygon() = default;

		// adds a new point. if its near the line segment
		// connecting two existing points its inserted in the middle
		// otherwise @the end
		// SnapDistance controls what "near" means
		void addPoint(const QPointF& coordinates);

		// num of points
		size_t size() const;

		// getter for points, this is the main purpose of this class
		const QVector<QSharedPointer<DkControlPoint> >&  points() const;

		// calculates the bounding rect for all points 
		// if transform is provided, the bounding box of the mapped points
		// is calculated
		QRectF boundingRect(QTransform transform = QTransform{}) const;
		
		// center of the bounding box (not center of gravity)
		QPointF center() const;

		// remove all points and start from scratch
		void clear();

		// write/read json
		void write(QJsonObject& json) const;
		void read(QJsonObject &json);

		void setInactive(bool inactive);
		bool isInactive() const;

	signals:
		// point is added @ the end
		void pointAdded(QSharedPointer<DkControlPoint> point);

		// point is removed
		void pointRemoved();

		// structural changes (i.e., point inserted in the middle or removed)
		void changed();
		
		// one control point moved
		void movedPoint();

	public slots:
		// removes the given control point  from the polygon
		void removePoint(QSharedPointer<DkControlPoint> point);

	private:
		// checks if this function is near any edge of the polygon
		// if yes, moves point to this point
		auto mapToNearestLine(QPointF& point);

		QVector<QSharedPointer<DkControlPoint> > mControlPoints;	// the points
		double mSnapDistance;										// distance for mapToNearestLine
		bool mInactive;
	};

	// renders the synced polygon
	// i.e. uses a transformation matrix to map it to the real location
	// and renders all its stuff using the viewport as parent
	class DkPolygonRenderer : public QObject
	{
		Q_OBJECT

	public:
		// ctro with viewport (is crucial), synced polygon (also crucial)
		DkPolygonRenderer(QWidget* viewport, QSharedPointer<DkSyncedPolygon> polygon, QTransform worldMatrix=QTransform());
		virtual ~DkPolygonRenderer() = default;
		
		// maps a point from the syncedpolygon coordinate system to the viewport
		QPointF mapToViewport(const QPointF& pos) const;

		// above function needs the world matrix, getter for it
		QTransform getWorldMatrix() const;

		// we draw all the stuff on the viewport, we need is as parent
		QWidget* getViewport();

		// translate the polygon. sets the transformation matrix
		void translate(const qreal& dx, const qreal& dy);
		
		// setter getter for the transformation matrix
		void setTransform(const QTransform& transform);
		QTransform getTransform() const;

		// color is used to distinguish the polygons
		void setColor(const QColor& color);
		QColor getColor() const;
		
		// adds a point at these coordinates, 
		// all the coordinate mapping and stuff is done here 
		// and the point is added to the synced polygon
		void addPointMouseCoords(const QPointF& coordinates);
		
		// we need this to check the bounds
		void setImageRect(QRect rect);

		// read write json
		void write(QJsonObject& json) const;
		void read(QJsonObject& json);

		bool isInactive() const;

		QSharedPointer<DkSyncedPolygon> getPolygon();

	signals:
		// this signal is emitted whenever a transform is changed
		// which is needed for updating stuff
		void transformChanged(QTransform transform);

		// cleanup signal when the center point is removed
		void removed();

	
	public slots:
		// sets tranformation matrix to rotate
		void rotate(qreal angle, QPointF center);
		void rotateCenter(qreal angle);

		// sets the world matrix (should be called every time it changes)
		void setWorldMatrix(QTransform worldMatrix);
		
		// remove all widgets and build it up again
		// should be called when the structure changes
		void refresh();

		// just add a point at the end
		void addPoint(QSharedPointer<DkControlPoint> point);
		
		// moves the points to their respective positions
		void update();

		// cleanup -> delets all widges
		void clear();

		// maps the point to the boundries of the image (with margin)
		// by casting a line from the last point to the new one
		// and intersecting it with the image rect
		QPointF mapToImageRect(const QPointF& point);

		// just makes sure that the point stays inside the
		// image rect
		QPointF mapToImageRectSimple(const QPointF& point);
		
		// returns image rect minus margins
		QRectF getImageRect();
	private:
		

		//initialization list
		QSharedPointer<DkSyncedPolygon> mPolygon;		// polygon for points
		QWidget* mViewport;				// viewport to add widgets
		QTransform mWorldMatrix;		// world matrix for rendering/positions
		QSharedPointer<DkControlPoint> mControlCenter;	// control point @ center
		DkControlPointRepresentation* mCenter;			// widget for control point @ center
		QColor mColor;					// we would like to distinguish the different polygons	

		// using default constructor
		QTransform mTransform;			// local transform (from syncedpolygon to this)
		QVector<DkControlPointRepresentation*> mPoints;		// widgets for points
		QVector<DkLineRepresentation*> mLines;				// widgets for lines

		QRectF mImageRect;	// image rect for boundary
		double mMargin;		// margin for mImageRect
	};

	// this point represents a point in the polygon
	// i.e. actual widget mainly for drawing and mouse events(!!!)
	class DkControlPointRepresentation : public QWidget
	{
		Q_OBJECT

	public:
		// ctro with point (which is connected), viewport for parent, and renderer for other useful stuff
		DkControlPointRepresentation(QSharedPointer<DkControlPoint> point, QWidget* viewport, DkPolygonRenderer* renderer);
		virtual ~DkControlPointRepresentation() = default;

		// drawing is hard. needs three functions. 
		void draw(QPainter *painter);
		void drawPoint(QPainter* painter, int size);
		void paintEvent(QPaintEvent *event) override;
		
		// moves the point to the actual position
		// @attention: movement must be done outside this widget (called)
		// outerwise strange things happen
		void move(QTransform transform);

		// mouse events for movement and removing points
		void mousePressEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;

	signals:
		// moved a point by ..., actual movement on control point in synced polygon
		// is already done. so this is just for info
		void moved(qreal dx, qreal dy);

		// removed the point with control point... 
		void removed(QSharedPointer<DkControlPoint> point);

		// rotation around point with given angle, this 
		// must be cached by the renderer and actually done
		void rotated(qreal angle, QPointF point);
	private:

		QSharedPointer<DkControlPoint> mPoint;		// my control point
		DkPolygonRenderer* mRenderer;				
		std::function<void(QMouseEvent* event)> mMouseMove;	// mouse move function, set on mouse press
	};

	// same as above but for lines
	// simpler since it does not handle mouse events
	// just drawing
	class DkLineRepresentation : public QWidget
	{
	public:
		// ctor with two control points
		DkLineRepresentation(
			const std::pair<QSharedPointer<DkControlPoint>, QSharedPointer<DkControlPoint>>& line,
			QWidget* viewport);
		virtual ~DkLineRepresentation() = default;

		void paintEvent(QPaintEvent *event) override;

		// moves the control points and stores the result in mMapped
		void move(QTransform transform);
	
	private:
		std::pair<QSharedPointer<DkControlPoint>, QSharedPointer<DkControlPoint>> mLine;
		QPen pen;
		std::pair<QPoint, QPoint> mMapped;	// actual endpoints in right coordinate syzstem
	};
}