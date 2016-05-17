#pragma once
#include <QWidget>
#include <QPen>
#include <functional>

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
		start,
		intermediate,
		center
	};

	class DkControlPoint : public QObject
	{
		Q_OBJECT

	public:

		DkControlPoint(const QPointF& point);
		virtual ~DkControlPoint();
		void setPos(const QPointF& point);
		QPointF getPos() const;
		void setType(ControlPointType t);
		ControlPointType getType();

	signals:
		void moved();

	private:
		QPointF mPoint;
		ControlPointType mType;
	};

	class DkSyncedPolygon : public QObject
	{
		Q_OBJECT

	public:
		DkSyncedPolygon();
		void addPoint(const QPointF& coordinates);
		virtual ~DkSyncedPolygon();

		size_t size() const;
		const QVector<QSharedPointer<DkControlPoint> >&  points() const;
		QRectF boundingRect() const;
		QPointF center() const;
		void clear();

	signals:
		void pointAdded(QSharedPointer<DkControlPoint> point);
		void pointRemoved();
		void changed();
		void movedPoint();

	public slots:
		void removePoint(QSharedPointer<DkControlPoint> point);

	private:
	
		QVector<QSharedPointer<DkControlPoint> > mControlPoints;
	};


	class DkPolygonRenderer : public QObject
	{
		Q_OBJECT

	public:
		DkPolygonRenderer(QWidget* viewport, DkSyncedPolygon* polygon, QTransform worldMatrix=QTransform());
		virtual ~DkPolygonRenderer();
		
		QPointF mapToViewport(const QPointF& pos) const;
		QTransform getWorldMatrix() const;
		QWidget* getViewport();

		void translate(const qreal& dx, const qreal& dy);
		
		void setTransform(const QTransform& transform);
		QTransform getTransform() const;

		void setColor(const QColor& color);
		QColor getColor() const;
		
		void addPointMouseCoords(const QPointF& coordinates);
		QVector<QPointF> mapToImage(QTransform image);

	signals:
		// this signal is emitted whenever a transform is changed
		// which is needed for updating stuff
		void transformChanged(QTransform transform);
		void removed();

	
	public slots:
		void rotate(qreal angle, QPointF center);
		void rotateCenter(qreal angle);
		void setWorldMatrix(QTransform worldMatrix);
		
		void refresh();
		
		void addPoint(QSharedPointer<DkControlPoint> point);
		
		void update();
		void clear();

	private:
		//initialization list
		DkSyncedPolygon* mPolygon;
		QWidget* mViewport;
		QTransform mWorldMatrix;
		QSharedPointer<DkControlPoint> mControlCenter;
		DkControlPointRepresentation* mCenter;
		QColor mColor;

		// using default constructor
		QTransform mTransform;
		QVector<DkControlPointRepresentation*> mPoints;
		QVector<DkLineRepresentation*> mLines;
	};

	class DkControlPointRepresentation : public QWidget
	{
		Q_OBJECT

	public:

		DkControlPointRepresentation(QSharedPointer<DkControlPoint> point, QWidget* viewport, DkPolygonRenderer* renderer);
		void draw(QPainter *painter);
		void drawPoint(QPainter* painter, int size);
		void paintEvent(QPaintEvent *event) override;
		void move(QTransform transform);
		void mousePressEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;

	signals:
		void moved(qreal dx, qreal dy);
		void removed(QSharedPointer<DkControlPoint> point);
		void rotated(qreal angle, QPointF point);
	private:

		QSharedPointer<DkControlPoint> mPoint;
		DkPolygonRenderer* mRenderer;
		std::function<void(QMouseEvent* event)> mMouseMove;
	};

	class DkLineRepresentation : public QWidget
	{
	public:
		DkLineRepresentation(
			const std::pair<QSharedPointer<DkControlPoint>, QSharedPointer<DkControlPoint>>& line,
			QWidget* viewport);
		void paintEvent(QPaintEvent *event) override;
		void move(QTransform transform);
	
	private:
		std::pair<QSharedPointer<DkControlPoint>, QSharedPointer<DkControlPoint>> mLine;
		QPen pen;
		std::pair<QPoint, QPoint> mMapped;
	};
}