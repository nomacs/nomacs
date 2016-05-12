#include "DkPolyTimeline.h"
#include "DkBaseWidgets.h"
#include <QLabel>
#include "AspectRatioPixmapLabel.h"
#include <QDebug>
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
		clear();
	}

	void DkSingleTimeline::setPolygon(QSharedPointer<DkSyncedPolygon> poly)
	{
		mPoly = poly;
		refresh();
	}

	void DkSingleTimeline::clear()
	{
		for (auto elem : mElements) {
			mParent->clearGridElement(elem);
			delete elem;
		}
		mElements.clear();
	}

	void DkSingleTimeline::addElement()
	{
		auto elem = std::make_shared<QImage>();
		//AspectRatioPixmapLabel* label = new AspectRatioPixmapLabel(mParent);
		auto label = new QLabel();
		label->setStyleSheet("QLabel{background-color: #eee; }");
		label->setMaximumSize(QSize(200, 200));
		label->setMinimumWidth(100);

		
		QPixmap pm(":/nomacs/img/save.svg");
		label->setPixmap(pm);
		label->setScaledContents(true);
		label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
		
		mElements.push_back(label);
		qDebug() << "has height for width " << label->hasHeightForWidth();

		mParent->setGridElement(label, mLayoutRow, mElements.size());
		
	}

	void DkSingleTimeline::setTransform(QTransform transform)
	{
		mTransform = transform;
	}

	void DkSingleTimeline::refresh()
	{
		if (!mPoly) {
			return;
		}

		clear();

		for (auto p : mPoly->points()) {
			addElement();
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