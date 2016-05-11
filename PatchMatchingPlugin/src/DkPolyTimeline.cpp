#include "DkPolyTimeline.h"
#include <QLabel>
#include "AspectRatioPixmapLabel.h"
#include <QDebug>
namespace nmp {

	DkPolyTimeline::DkPolyTimeline(QWidget* parent)
		: QWidget(parent),
			mLayout(new QVBoxLayout(this)),
			mWidget(new QWidget(this)),
			mScrollArea(new QScrollArea(this))
	{
		setStyleSheet("background-color: #00ff0060");
		mWidget->setStyleSheet("background-color: #ffff0060");
		mWidget->setLayout(mLayout);
		mScrollArea->setWidgetResizable(true);
		mScrollArea->setWidget(mWidget);

		auto dummy = new QVBoxLayout(this);
		dummy->addWidget(mScrollArea);
		//dummy->addWidget(mWidget);
		mScrollArea->setStyleSheet("background-color: #0000ff60");
		dummy->setMargin(0);
		setLayout(dummy); // dummy layout

		update();
	}


	DkPolyTimeline::~DkPolyTimeline()
	{
	}

	DkSingleTimeline* DkPolyTimeline::addPolygon()
	{
		auto tl = new DkSingleTimeline(this);
		mLayout->addWidget(tl);
		tl->setVisible(true);
		return tl;
	}

	void DkPolyTimeline::reset()
	{
		auto item = mLayout->itemAt(0);
		while (item = mLayout->itemAt(0)) {
			if (item->widget()) {
				delete item->widget()
					;
			}
		}
	}

	DkSingleTimeline::DkSingleTimeline(QWidget* parent)
		: QWidget(parent),
			mLayout(new QHBoxLayout(this))
	{
		setStyleSheet("background-color: #0000ff40");
		setLayout(mLayout);
		mLayout->setMargin(0);
		mLayout->setAlignment(Qt::AlignLeft);
	}

	DkSingleTimeline::~DkSingleTimeline()
	{
	}

	void DkSingleTimeline::setPolygon(QSharedPointer<DkSyncedPolygon> poly)
	{
		mPoly = poly;
		update();
		updateGeometry();
	}

	void DkSingleTimeline::clear()
	{
		auto item = mLayout->itemAt(0);
		while (item = mLayout->itemAt(0)) {
			if (item->widget()) {
				delete item->widget();
			}
		}
	}

	void DkSingleTimeline::addElement()
	{
		auto elem = std::make_shared<QImage>();
		AspectRatioPixmapLabel* label = new AspectRatioPixmapLabel(this);

		QPixmap pm(30, 30);
		pm.fill(QColor(0, 255, 0, 255));
		label->setPixmap(pm);
		label->setMargin(0);

		mLayout->addWidget(label);
	}

	void DkSingleTimeline::setTransform(QTransform transform)
	{
		qDebug() << "Timeline transform changed";
		mTransform = transform;
	}

	void DkSingleTimeline::update()
	{
		if (!mPoly) {
			return;
		}

		qDebug() << "Timeline update";
		clear();

		for (auto p : mPoly->points()) {
			addElement();
		}
	}
}