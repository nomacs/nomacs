#include "DkPolygon.h"
#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
namespace nmp {
	DkPoint::DkPoint(QGraphicsItem* parent)
		: QGraphicsItem(parent), 
		  mType(type::square)
	{
		setFlag(QGraphicsItem::ItemIsMovable);
	}

	DkPoint::~DkPoint()
	{
	}

	void DkPoint::paint(QPainter* painter, const QStyleOptionGraphicsItem* style, QWidget* widget)
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
		drawPoint(painter, mSize);	// invisible rect for mouseevents...

		painter->setBrush(QColor(255, 255, 255, 100));
		drawPoint(painter, 7);

		painter->setBrush(QColor(0, 0, 0));
		drawPoint(painter, 5);

		//draw bar
		painter->setBrush(QColor(0, 0, 255, 60));
		drawPoint(painter, mSize);
	}

	QRectF DkPoint::boundingRect() const
	{
		return QRectF(-mSize / 2., -mSize/2., 20, 20);
	}

	void DkPoint::setType(type t)
	{
		mType = t;
	}

	void DkPoint::drawPoint(QPainter* painter, int size)
	{
		QRectF rect(-size/2., -size/2., size,size);
		
		switch (mType) {
		case type::square: {
			painter->drawRect(rect);
			break;
		}

		case type::circle: {
			painter->drawEllipse(rect);
			break;
		}

		default: {
			painter->drawRect(rect);
			break;
		}
		}
	}

	DkPolygon::DkPolygon(QGraphicsItem* parent)
		:QGraphicsItem(parent)
	{
		//setFlag(QGraphicsItem::ItemIsMovable);
		mPoints << new DkPoint(this);
	}


	DkPolygon::~DkPolygon()
	{
	}

	QRectF DkPolygon::boundingRect() const
	{
		return childrenBoundingRect();
	}

	void DkPolygon::paint(QPainter* painter, const QStyleOptionGraphicsItem* style, QWidget* widget)
	{
		painter->setRenderHint(QPainter::HighQualityAntialiasing);
		painter->setRenderHint(QPainter::Antialiasing);
	}


	void DkPolygon::addPoint(QGraphicsSceneMouseEvent * event)
	{
		auto point = new DkPoint(this);
		point->setPos(mapFromScene(event->scenePos()));
		mPoints << point;
	}
	DkGraphics::DkGraphics(QObject * parent)
		:mPolygon(std::make_shared<DkPolygon>())
	{
		addItem(mPolygon.get());
		setBackgroundBrush(QBrush(QColor(0, 0, 255, 40)));
	}
	void DkGraphics::mousePressEvent(QGraphicsSceneMouseEvent * event)
	{
		QGraphicsScene::mousePressEvent(event);
		if (!mouseGrabberItem()) {
			mPolygon->addPoint(event);
		}
	}
}
