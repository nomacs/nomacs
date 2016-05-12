#include "DkPolyTimeline.h"
#include "DkBaseWidgets.h"
#include <QLabel>
#include "AspectRatioPixmapLabel.h"
#include <QDebug>
#include <QPainter>
namespace nmp {

	DkPolyTimeline::DkPolyTimeline(QWidget* parent)
		: QLabel(parent),
			mScrollArea(new QScrollArea(this))
	{
		QWidget* c = new QWidget(this);
		mLayout = new QGridLayout(c);
		mLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);


		nmc::DkResizableScrollArea* mScrollArea = new nmc::DkResizableScrollArea(this);
		mScrollArea->setWidgetResizable(true);
		mScrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

		mScrollArea->setWidget(c);

		QVBoxLayout* l = new QVBoxLayout(this);
		l->setSpacing(0);
		l->setContentsMargins(0,0,0,0);
		l->addWidget(mScrollArea);
	}


	DkPolyTimeline::~DkPolyTimeline()
	{
	}

	DkSingleTimeline* DkPolyTimeline::addPolygon()
	{
		DkSingleTimeline* tl = new DkSingleTimeline(mList.size(), this);
		mList.push_back(tl);
		return tl;
	}

	void DkPolyTimeline::reset()
	{
		auto item = mLayout->itemAt(0);
		while (item = mLayout->itemAt(0)) {
			if (item->widget()) {
				delete item->widget();
			}
		}
	}

	void DkPolyTimeline::setGridElement(QWidget * widget, int row, int column)
	{
		mLayout->addWidget(widget, row, column);
	}

	void DkPolyTimeline::clearGridElement(QWidget * widget)
	{
		mLayout->removeWidget(widget);
	}


	DkSingleTimeline::DkSingleTimeline(int row, DkPolyTimeline* parent)
		: QObject(parent),
			mParent(parent),
			mLayoutRow(row)
	{
	}

	DkSingleTimeline::~DkSingleTimeline()
	{
	}

	void DkSingleTimeline::setPolygon(QSharedPointer<DkSyncedPolygon> poly)
	{
		mPoly = poly;
		refresh();
	}

	void DkSingleTimeline::clear()
	{
		for (auto elem : mElements) {
			mParent->clearGridElement(elem.first);
			delete elem.first;
		}
		mElements.clear();
	}

	void DkSingleTimeline::setImage(QSharedPointer<nmc::DkImageContainerT> img)
	{
		mImage = QPixmap::fromImage(img->image());
	}

	void DkSingleTimeline::addElement(QSharedPointer<DkControlPoint> point)
	{
		auto elem = std::make_shared<QImage>();
		//AspectRatioPixmapLabel* label = new AspectRatioPixmapLabel(mParent);
	
		auto label = new QLabel();
		label->setStyleSheet("QLabel{background-color: #eee; }");
		label->setMaximumSize(QSize(200, 200));
		label->setMinimumWidth(100);

		label->setScaledContents(true);
		
		label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

		mElements.push_back(std::make_pair(label, point));
		mParent->setGridElement(label, mLayoutRow, mElements.size());
	}



	void DkSingleTimeline::setTransform(QTransform transform)
	{
		mTransform = transform;
		redraw();
	}

	void DkSingleTimeline::refresh()
	{
		qDebug() << "Refresh timeline ";
		if (!mPoly) {
			return;
		}

		clear();

		for (auto p : mPoly->points()) {
			addElement(p);
		}
		//mParent->update();
		redraw();
	}

	void DkSingleTimeline::addPoint(QSharedPointer<DkControlPoint> point)
	{
		addElement(point);
		redraw();
	}

	void DkSingleTimeline::redraw()
	{
		for (auto pair : mElements) {
			QPixmap pm(40, 40);  // lets draw here
			QPainter paint(&pm);  // our painter

			QRect src(pm.rect());
			src.moveCenter(pair.second->getPos().toPoint());
			src = mTransform.mapRect(src);

			paint.drawPixmap(pm.rect(), mImage, src);

			pair.first->setPixmap(pm);
			pair.first->update();
		}
	}

	DkTimelineLabel::DkTimelineLabel(QWidget * parent)
		:QLabel(parent)
	{
	}
	DkTimelineLabel::~DkTimelineLabel()
	{
	}
}