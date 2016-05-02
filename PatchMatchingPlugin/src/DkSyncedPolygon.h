#pragma once
#include <QWidget>

namespace nmp {

	class DkPatchMatchingViewPort;
	class DkPolygonWidget;
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
		DkSyncedPolygon(DkPatchMatchingViewPort* viewport, QTransform* worldMatrix);
		void addPoint(const QPointF& coordinates);
		virtual ~DkSyncedPolygon();
		QPointF mapToViewport(const QPointF& pos) const;
		QTransform* worldMatrix() const;
		DkPolygonWidget* addRenderer();
		QVector<QSharedPointer<DkControlPoint> >& points();
		void setWorldMatrix(QTransform* worldMatrix);

	signals:
		void pointAdded(QSharedPointer<DkControlPoint> point);
		void changed();

		public slots:
		void worldMatrixChanged(QTransform matrix);

	protected:
		QVector<QSharedPointer<DkControlPoint> > mControlPoints;

		QVector<DkPolygonWidget*> mRenderer;

		DkPatchMatchingViewPort* mViewport;
	};


	class DkPolygonWidget : public QObject
	{
		Q_OBJECT

	public:
		DkPolygonWidget(DkPatchMatchingViewPort* viewport, DkSyncedPolygon* polygon);
		void setTransform(const QTransform& transform);
		virtual ~DkPolygonWidget();
		QTransform transform();

		public slots:
		void addPoint(QSharedPointer<DkControlPoint> point);

	private:
		DkSyncedPolygon* mPolygon;
		QTransform mTransform;

		QTransform mCombinedTransform;
		QVector<DkControlPointRepresentation*> mPoints;
		DkPatchMatchingViewPort* mViewport;
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

		DkControlPointRepresentation(QSharedPointer<DkControlPoint> point, QTransform* transform, QWidget* parent);
		void draw(QPainter *painter);
		void drawPoint(QPainter* painter, int size);
		void paintEvent(QPaintEvent *event) override;
		void movePoint();

	signals:
		void moved();

		public slots:
		void updatePoint();

	protected:

		QTransform* mTransform;
		QSharedPointer<DkControlPoint> mPoint;
		DkPatchMatchingViewPort* mViewport;
	};
}