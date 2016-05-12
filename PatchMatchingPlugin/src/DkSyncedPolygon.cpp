#include "DkSyncedPolygon.h"
#include <QWidget>
#include <QPainter>
#include "DkPatchMatchingPlugin.h"
#define _USE_MATH_DEFINES
#include <math.h>
namespace nmp {
	DkSyncedPolygon::DkSyncedPolygon()
	{
	}

	DkSyncedPolygon::~DkSyncedPolygon()
	{
	}

	size_t DkSyncedPolygon::size() const
	{
		return mControlPoints.size();
	}

	const QVector<QSharedPointer<DkControlPoint> >& DkSyncedPolygon::points() const
	{
		return mControlPoints;
	}

	QRectF DkSyncedPolygon::boundingRect() const
	{
		if (mControlPoints.empty()) {
			return QRectF();
		}

		auto getR = [this](const auto& p) {return QRectF(p->getPos(), QSize()); };
		auto rect = getR(mControlPoints.first());
		for (auto p : mControlPoints) {
			rect = rect.united(getR(p));
		}

		return rect;
	}

	QPointF DkSyncedPolygon::center() const
	{
		return boundingRect().center();
	}


	void DkSyncedPolygon::addPoint(const QPointF & coordinates)
	{
		auto point = QSharedPointer<DkControlPoint>::create(coordinates);
		if (mControlPoints.empty()) {
			point->setType(ControlPointType::start);
		}

		connect(point.data(), &DkControlPoint::moved, this, &DkSyncedPolygon::movedPoint);

		mControlPoints.append(point);
	
		emit pointAdded(point);
		emit changed();
	}

	void DkSyncedPolygon::removePoint(QSharedPointer<DkControlPoint> point)
	{
		mControlPoints.removeAll(point);

		if (!mControlPoints.empty()) {
			mControlPoints.first()->setType(ControlPointType::start);
		}
		emit pointRemoved();
		emit changed();
	}

	DkPolygonRenderer::DkPolygonRenderer(QWidget* viewport, DkSyncedPolygon* polygon, QTransform worldMatrix)
		: QObject(polygon),
		mPolygon(polygon),
		mViewport(viewport),
		mWorldMatrix(worldMatrix),
		mControlCenter(QSharedPointer<DkControlPoint>::create(QPointF())),
		mCenter(new DkControlPointRepresentation(mControlCenter, viewport, this)),
		mColor(0, 0, 255)
	{
		// connect synced polygon to this
		connect(polygon, &DkSyncedPolygon::pointAdded, this, &DkPolygonRenderer::addPoint);
		connect(polygon, &DkSyncedPolygon::pointRemoved, this, &DkPolygonRenderer::refresh);

		// connect center point to this
		mControlCenter->setType(ControlPointType::center);
		connect(mCenter, &DkControlPointRepresentation::moved, this, &DkPolygonRenderer::translate);
		connect(mCenter, &DkControlPointRepresentation::rotated, this, &DkPolygonRenderer::rotate);
		connect(mCenter, &DkControlPointRepresentation::removed, this, &DkPolygonRenderer::removed);
		mCenter->setVisible(false);
		
		refresh();
	}

	void DkPolygonRenderer::rotate(qreal angle, QPointF center)
	{
		QTransform t = getTransform();
		t.translate(center.x(), center.y());
		t.rotate(angle);
		t.translate(-center.x(), -center.y());
		setTransform(t);
		update();
	}

	void DkPolygonRenderer::rotateCenter(qreal angle)
	{
		return rotateCenter(angle);
	}

	void DkPolygonRenderer::translate(const qreal & dx, const qreal & dy)
	{		
		QTransform t = getTransform();
		t.translate(dx, dy);
		setTransform(t);
		update();
	}


	void DkPolygonRenderer::setTransform(const QTransform & transform)
	{
		mTransform = transform;
		emit transformChanged(transform);
		update();
	}

	DkPolygonRenderer::~DkPolygonRenderer()
	{
	}

	QTransform DkPolygonRenderer::getTransform() const
	{
		return mTransform;
	}

	void DkPolygonRenderer::setColor(const QColor & color)
	{
		mColor = color;
	}

	QColor DkPolygonRenderer::getColor() const
	{
		return mColor;
	}

	void DkPolygonRenderer::addPointMouseCoords(const QPointF & coordinates)
	{
		mPolygon->addPoint(mapToViewport(coordinates));
	}

	QVector<QPointF> DkPolygonRenderer::mapToImage(QTransform image)
	{
		QVector<QPointF> mapped;

		for (auto point : mPoints) {
			mapped.append(image.map(point->pos()));
		}
		return mapped;
	}


	void DkPolygonRenderer::refresh()
	{
		for (auto p : mPoints) {
			delete p;
		}

		for (auto l : mLines) {
			delete l;
		}

		mPoints.clear();
		mLines.clear();

		for (auto p : mPolygon->points()) {
			addPoint(p);
		}
	}


	void DkPolygonRenderer::addPoint(QSharedPointer<DkControlPoint> point)
	{
		auto prev = mPolygon->points().indexOf(point)-1;
		// add line if necessary
		if (prev >= 0) {
			auto pair = std::make_pair(mPolygon->points()[prev], point);
			auto line = new DkLineRepresentation(pair, getViewport());
			line->setVisible(true);
			mLines.append(line);
		}

		// add point
		auto rep = new DkControlPointRepresentation(point, getViewport(), this); // create new widget
		connect(rep, &DkControlPointRepresentation::moved, this, &DkPolygonRenderer::update);
		connect(point.data(), &DkControlPoint::moved, this, &DkPolygonRenderer::update);
		connect(rep, &DkControlPointRepresentation::removed, mPolygon, &DkSyncedPolygon::removePoint);
		connect(rep, &DkControlPointRepresentation::rotated, this, &DkPolygonRenderer::rotate);
		rep->setVisible(true);

		mPoints.append(rep);		

		update();
	}

	void DkPolygonRenderer::update()
	{
		auto transform = getTransform()*getWorldMatrix();
		for (auto p : mPoints) {
			p->move(transform);
		}
		for (auto l : mLines) {
			l->move(transform);
		}
		
		mControlCenter->setPos(mPolygon->center());
		mCenter->move(transform);

		// make center visible if more than 2 points
		mCenter->setVisible(mPoints.size() > 1);

		mViewport->update();
	}

	void DkPolygonRenderer::clear()
	{
		mCenter->disconnect();
		delete mCenter;
		mCenter = nullptr;

		for (auto p : mPoints) {
			p->disconnect();
			delete p;
		}
		mPoints.clear();

		for (auto l : mLines) {
			l->disconnect();
			delete l;
		}
		mLines.clear();
	}

	QPointF DkPolygonRenderer::mapToViewport(const QPointF & pos) const
	{
		return mapToViewPort(pos, getTransform()*getWorldMatrix());
	}

	QTransform DkPolygonRenderer::getWorldMatrix() const
	{
		return mWorldMatrix;
	}

	QWidget * DkPolygonRenderer::getViewport()
	{
		return mViewport;
	}

	void DkPolygonRenderer::setWorldMatrix(QTransform worldMatrix)
	{
		mWorldMatrix = worldMatrix;
		update();
	}


	void DkControlPointRepresentation::paintEvent(QPaintEvent* event)
	{
		QPainter painter(this);
		draw(&painter);
		QWidget::paintEvent(event);
	}

	void DkControlPointRepresentation::move(QTransform transform)
	{
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

		//QPen pen;
		//pen.setWidth(1);
		//pen.setColor(QColor(255, 255, 0, 100));
		painter->setRenderHint(QPainter::HighQualityAntialiasing);
		painter->setRenderHint(QPainter::Antialiasing);

		// draw the control point
		painter->setPen(penNoStroke);
		painter->setBrush(QColor(0, 0, 0, 0));
		drawPoint(painter, size().width());	// invisible rect for mouseevents...

		//painter->setBrush(QColor(255, 255, 255, 100));
		//drawPoint(painter, 11);

		painter->setBrush(QColor(0, 0, 0));
		drawPoint(painter, 7);

		auto color = mRenderer->getColor();
		color.setAlpha(60);
		//draw bar
		painter->setBrush(color);
		drawPoint(painter, size().width());
	}

	void DkControlPointRepresentation::mousePressEvent(QMouseEvent* event)
	{
		if (event->button() == Qt::LeftButton && event->modifiers() == Qt::CTRL) {
			emit removed(mPoint);
		}
		else if (event->button() == Qt::LeftButton && event->modifiers() == Qt::ShiftModifier) {
			auto posGrab = event->globalPos();
			std::shared_ptr<double> lastAngle = std::make_shared<double>(0.);

			mMouseMove = [this, posGrab, lastAngle](auto event) {
				auto newpos = event->globalPos();
				auto diff = newpos - posGrab;

				auto angle = atan2(diff.y(), diff.x())*180. / M_PI;

				if (diff.manhattanLength() > 30) {
					emit rotated(angle - *lastAngle, mPoint->getPos());
				}
				*lastAngle = angle;
			};
		}
		else if (event->button() ==  Qt::LeftButton) {

			auto posGrab = mRenderer->mapToViewport(event->globalPos());
			auto initialPos = mPoint->getPos();

			mMouseMove = [this, posGrab, initialPos](auto event) {
				auto newpos = mRenderer->mapToViewport(event->globalPos());
				mPoint->setPos(initialPos + newpos - posGrab);

				auto diff = newpos - posGrab;
				emit moved(diff.x(), diff.y());
			};
		}
	}
	
	void DkControlPointRepresentation::mouseMoveEvent(QMouseEvent* event)
	{
		if (mMouseMove) {
			mMouseMove(event);
		}
	}

	void DkControlPointRepresentation::mouseReleaseEvent(QMouseEvent* event)
	{	
		mMouseMove = nullptr;
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

		case ControlPointType::center: {
			painter->drawEllipse(rect);
			break;
		}

		case ControlPointType::start : {
			QPolygonF poly;			//draw diamond
			poly << QPointF(rect.left()+size/2., rect.top())
				<< QPointF(rect.left()+size, rect.top()+size/2.)
				<< QPointF(rect.left() + size/2., rect.top()+size)
				<< QPointF(rect.left() + 0, rect.top()+size/2.);
			painter->drawPolygon(poly);
			break;
		}

		default: {
			painter->drawRect(rect);
			break;
		}
		}
	}

	DkControlPointRepresentation::DkControlPointRepresentation(QSharedPointer<DkControlPoint> point, 
																	QWidget* viewport, DkPolygonRenderer* renderer)
		: QWidget(viewport), mPoint(point), mRenderer(renderer)
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
		emit moved();
	}

	QPointF DkControlPoint::getPos() const
	{
		return mPoint;
	}

	void DkControlPoint::setType(ControlPointType t)
	{
		mType = t;
	}

	ControlPointType DkControlPoint::getType()
	{
		return mType;
	}

	DkLineRepresentation::DkLineRepresentation(const std::pair<QSharedPointer<DkControlPoint>,
													QSharedPointer<DkControlPoint>>& line, QWidget * viewport)
		:QWidget(viewport), mLine(line)
	{
		setAttribute(Qt::WA_TransparentForMouseEvents);
		pen.setBrush(QColor(0, 0, 0));
		pen.setWidth(1);
	}

	void DkLineRepresentation::paintEvent(QPaintEvent * event)
	{
		QPainter painter(this);
		painter.setPen(pen);
		painter.setRenderHint(QPainter::HighQualityAntialiasing);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setBrush(QColor(0, 0, 0));
		painter.drawLine(mMapped.first, mMapped.second);
		//painter.drawRect(rect());

		QWidget::paintEvent(event);
	}

	void DkLineRepresentation::move(QTransform transform)
	{	
		auto first = transform.map(mLine.first->getPos()).toPoint();
		auto second = transform.map(mLine.second->getPos()).toPoint();

		auto rect = QRect(QRect(first,QSize()))
					.united(QRect(second, QSize()));

		setGeometry(rect);
		mMapped = std::make_pair(mapFromParent(first), mapFromParent(second));
	}

};
