#include "DkSyncedPolygon.h"
#include <QWidget>
#include <QPainter>

namespace nmp {
	DkSyncedPolygon::DkSyncedPolygon(DkPatchMatchingViewPort* viewport, QTransform* worldMatrix)
		:QObject(viewport), mViewport(viewport)
	{
		addRenderer();
		auto r = addRenderer();
		r->setTransform(QTransform().translate(100, 100));
	}

	DkSyncedPolygon::~DkSyncedPolygon()
	{
	}

	QPointF DkSyncedPolygon::mapToViewport(const QPointF & pos) const
	{
		return mWorldMatrix->inverted().map(pos);
	}

	QTransform* DkSyncedPolygon::worldMatrix() const
	{
		return mWorldMatrix;
	}

	QWidget * DkSyncedPolygon::viewport()
	{
		return mViewport;
	}

	DkPolygonWidget* DkSyncedPolygon::addRenderer()
	{
		auto renderer = new DkPolygonWidget(this);
		mRenderer.append(renderer);
		connect(this, &DkSyncedPolygon::pointAdded, renderer, &DkPolygonWidget::addPoint);
		return renderer;
	}

	QVector<QSharedPointer<DkControlPoint> >& DkSyncedPolygon::points()
	{
		return mControlPoints;
	}

	void DkSyncedPolygon::setWorldMatrix(QTransform * worldMatrix)
	{
		mWorldMatrix = worldMatrix;
	}


	void DkSyncedPolygon::addPoint(const QPointF & coordinates)
	{
		auto point = QSharedPointer<DkControlPoint>(new DkControlPoint(coordinates));
		mControlPoints.append(point);
		emit pointAdded(point);
	}

	DkPolygonWidget::DkPolygonWidget(DkPatchMatchingViewPort* viewport, DkSyncedPolygon* polygon)
		:QObject(polygon), mPolygon(polygon)
	{
	}

	void DkPolygonWidget::setTransform(const QTransform & transform)
	{
		mTransform = transform;
		auto world = mPolygon->worldMatrix();

		if (world) {
			mCombinedTransform = mTransform*(*world);
		}
		else {
			mCombinedTransform = mTransform;
		}
	}

	DkPolygonWidget::~DkPolygonWidget()
	{
	}

	QTransform DkPolygonWidget::transform()
	{
		return mTransform;
	}

	void DkPolygonWidget::addPoint(QSharedPointer<DkControlPoint> point)
	{
		auto rep = new DkControlPointRepresentation(point, &mCombinedTransform, mViewport)); // create new widget
		rep->setVisible(true);
		mPoints.append(rep);
	}

	void DkControlPointRepresentation::paintEvent(QPaintEvent* event)
	{
		movePoint();
		QPainter painter(this);
		draw(&painter);
		QWidget::paintEvent(event);
	}

	void DkControlPointRepresentation::movePoint()
	{
		auto transform = QTransform();
		if (mTransform) {
			transform = *mTransform;
		}
		auto center = transform.map(mPoint->getPos());	// transform central position
		auto g = static_cast<QRectF>(geometry());	// get geometry
		g.moveCenter(center);	// recenter
		setGeometry(g.toRect());	// save
	}


	void DkControlPointRepresentation::draw(QPainter* painter)
	{
		QPen penNoStroke;
		penNoStroke.setWidth(0);
		penNoStroke.setColor(QColor(0, 0, 0, 0));

		QPen pen;
		pen.setWidth(1);
		pen.setColor(QColor(255, 255, 0, 100));
		painter->setRenderHint(QPainter::HighQualityAntialiasing);
		painter->setRenderHint(QPainter::Antialiasing);

		// draw the control point
		painter->setPen(penNoStroke);
		painter->setBrush(QColor(0, 0, 0, 0));
		drawPoint(painter, size().width());	// invisible rect for mouseevents...

		painter->setBrush(QColor(255, 255, 255, 100));
		drawPoint(painter, 7);

		painter->setBrush(QColor(0, 0, 0));
		drawPoint(painter, 5);

		//draw bar
		painter->setBrush(QColor(0, 0, 255, 60));
		drawPoint(painter, size().width());
	}

	void DkControlPointRepresentation::drawPoint(QPainter* painter, int size)
	{
		QRectF rect(QPointF(), QSize(size, size));
		rect.moveCenter(QRectF(QPointF(), this->size()).center());


		switch (mPoint->getType()) {
		case ControlPointType::intermediate: {
			painter->drawRect(rect);
			break;
		}

		case ControlPointType::start: {
			painter->drawEllipse(rect);
			break;
		}

		default: {
			painter->drawRect(rect);
			break;
		}
		}
	}

	void DkControlPointRepresentation::updatePoint()
	{
		update();
	}

	DkControlPointRepresentation::DkControlPointRepresentation(QSharedPointer<DkControlPoint> point, QTransform* transform, QWidget* parent)
		: QWidget(parent), mTransform(transform), mPoint(point)
	{
		setGeometry(QRect(-10, -10, 20, 20));
	}

	DkControlPoint::DkControlPoint(const QPointF& point)
		: mPoint(point)
	{
	}

	DkControlPoint::~DkControlPoint()
	{
	}

	void DkControlPoint::setPos(const QPointF & point)
	{
		mPoint = point;
		emit changed();
	}

	QPointF DkControlPoint::getPos() const
	{
		return mPoint;
	}

	void DkControlPoint::setType(ControlPointType t)
	{
		mType = t;
		emit changed();
	}

	ControlPointType DkControlPoint::getType()
	{
		return mType;
	}

};
