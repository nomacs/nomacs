#pragma once
#include <QWidget>

namespace nmp {

	class DkPatchMatchingViewPort;
	class DkPolygonRenderer;
	class DkControlPointRepresentation;

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
		void changed();

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

	signals:
		void pointAdded(QSharedPointer<DkControlPoint> point);
		void changed();

	private:
		const QVector<QSharedPointer<DkControlPoint> >&  points() const;
		QVector<QSharedPointer<DkControlPoint> > mControlPoints;

		QVector<DkPolygonRenderer*> mRenderer;
	};


	class DkPolygonRenderer : public QObject
	{
		Q_OBJECT

	public:
		DkPolygonRenderer(QWidget* viewport, DkSyncedPolygon* polygon);
		virtual ~DkPolygonRenderer();
		
		QPointF mapToViewport(const QPointF& pos) const;
		QTransform getWorldMatrix() const;
		QWidget* getViewport();

		void setTransform(const QTransform& transform);
		QTransform getTransform() const;

	public slots:
		void setWorldMatrix(QTransform worldMatrix);
		void addPoint(QSharedPointer<DkControlPoint> point);
		void update();

	private:
		DkSyncedPolygon* mPolygon;
		QWidget* mViewport;

		QTransform mWorldMatrix;
		QTransform mTransform;

		QVector<DkControlPointRepresentation*> mPoints;
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

		DkControlPointRepresentation(QSharedPointer<DkControlPoint> point, QWidget* viewport);
		void draw(QPainter *painter);
		void drawPoint(QPainter* painter, int size);
		void paintEvent(QPaintEvent *event) override;
		void move(QTransform transform);

	signals:
		void moved();

	private:
		QSharedPointer<DkControlPoint> mPoint;
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
	};
}