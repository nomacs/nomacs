#include "DkPolyTimeline.h"
#include <QLabel>
#include "AspectRatioPixmapLabel.h"

DkPolyTimeline::DkPolyTimeline()
	:	mLayout(new QVBoxLayout(this)), 
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

void DkPolyTimeline::addPolygon()
{
	auto tl = new DkSingleTimeline(this);
	mLayout->addWidget(tl);
	tl->setVisible(true);
}

DkSingleTimeline::DkSingleTimeline(QWidget* parent)
	: QWidget(parent), mLayout(new QHBoxLayout(this))
{
	setStyleSheet("background-color: #0000ff40");
	setLayout(mLayout);
	mLayout->setMargin(0);
	refresh();
}

DkSingleTimeline::~DkSingleTimeline()
{
}

void DkSingleTimeline::clear()
{
	for (auto c : mLayout->children()) {
		delete c;
	}
	mElements.clear();
}

void DkSingleTimeline::addElement()
{
	auto elem = std::make_shared<QImage>();
	AspectRatioPixmapLabel* label = new AspectRatioPixmapLabel(this);

	QPixmap pm(30,30);
	pm.fill(QColor(255, 0, 0, 255));
	label->setPixmap(pm);
	label->setMargin(0);
	//label->setFixedSize(pm.size());

	mLayout->addWidget(label);
}

void DkSingleTimeline::refresh()
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