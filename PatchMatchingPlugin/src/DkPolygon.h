#pragma once
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <memory>

namespace nmp {
	class DkPolygon;

	class DkGraphics: public QGraphicsScene
	{
	public:
		DkGraphics(QObject *parent = nullptr);
		virtual ~DkGraphics() {}
		void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

	private:
		std::shared_ptr<DkPolygon> mPolygon;
	};

	class DkPoint : public QGraphicsItem
	{
	public:
		enum class type
		{
			square,
			circle
		};

		explicit DkPoint(QGraphicsItem* parent = nullptr);
		virtual ~DkPoint();
		void paint(QPainter* painter, const QStyleOptionGraphicsItem* style, QWidget* widget) override;
		QRectF boundingRect() const override;
		void setType(type t);
		
	private:
		void drawPoint(QPainter* painter, int size);
		type mType;
		int mSize = 20;
	};

	class DkPolygon : public QGraphicsItem
	{
	public:
		DkPolygon(QGraphicsItem* parent = nullptr);
		virtual ~DkPolygon();
		QRectF boundingRect() const override;
		void paint(QPainter* painter, const QStyleOptionGraphicsItem* style, QWidget* widget) override;
		void addPoint(QGraphicsSceneMouseEvent* event);

	protected:
		QVector<DkPoint*> mPoints;
	};

}