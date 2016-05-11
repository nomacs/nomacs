#include "DkPolyTimeline.h"
#include <QLabel>
#include "AspectRatioPixmapLabel.h"

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

	DkSingleTimeline::DkSingleTimeline(QWidget* parent)
		: QWidget(parent),
			mLayout(new QHBoxLayout(this))
	{
		setStyleSheet("background-color: #0000ff40");
		setLayout(mLayout);
		mLayout->setMargin(0);
		//update();
	}

	DkSingleTimeline::~DkSingleTimeline()
	{
	}

	void DkSingleTimeline::setPolygon(QSharedPointer<DkSyncedPolygon> poly)
	{
		mPoly = poly;
		update();
	}

	void DkSingleTimeline::clear()
	{
		for (auto c : mLayout->children()) {
			delete c;
		}
	}

	void DkSingleTimeline::addElement()
	{
		auto elem = std::make_shared<QImage>();
		AspectRatioPixmapLabel* label = new AspectRatioPixmapLabel(this);

		QPixmap pm(30, 30);
		pm.fill(QColor(255, 0, 0, 255));
		label->setPixmap(pm);
		label->setMargin(0);

		mLayout->addWidget(label);
	}

	void DkSingleTimeline::setTransform(QTransform transform)
	{
		mTransform = transform;
	}

	void DkSingleTimeline::update()
	{
		clear();

		addElement();
		addElement();
		addElement();
		addElement();
		addElement();
		addElement();
		addElement();
	}
}