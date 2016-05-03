#pragma once
#include <QWidget>
#include <QPen>

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
		intermediate
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

	signals:
		void pointAdded(QSharedPointer<DkControlPoint> point);
		void changed();

	public slots:
		void removePoint(QSharedPointer<DkControlPoint> point);

	private:
		
		QVector<QSharedPointer<DkControlPoint> > mControlPoints;
		QVector<DkPolygonRenderer*> mRenderer;
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

		void setTransform(const QTransform& transform);
		QTransform getTransform() const;
		//void setColor(const QColor& color);

	public slots:
		void setWorldMatrix(QTransform worldMatrix);
		void refresh();
		void addPoint(QSharedPointer<DkControlPoint> point);
		void update();

	private:
		DkSyncedPolygon* mPolygon;
		QWidget* mViewport;

		QTransform mWorldMatrix;
		QTransform mTransform;

		QVector<DkControlPointRepresentation*> mPoints;
		QVector<DkLineRepresentation*> mLines;
		//QColor mColor;
	};

	class DkControlPointRepresentation : public QWidget
	{
		Q_OBJECT

	public:

		enum class type
		{
			square,
			circle
		};

		DkControlPointRepresentation(QSharedPointer<DkControlPoint> point, QWidget* viewport, DkPolygonRenderer* renderer);
		void draw(QPainter *painter);
		void drawPoint(QPainter* painter, int size);
		void paintEvent(QPaintEvent *event) override;
		void move(QTransform transform);
		void mousePressEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;

	signals:
		void moved();
		void removed(QSharedPointer<DkControlPoint> point);

	private:
		QWidget* mViewport;
		QSharedPointer<DkControlPoint> mPoint;
		QPointF posGrab;
		QPointF initialPos;
		DkPolygonRenderer* mRenderer;
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