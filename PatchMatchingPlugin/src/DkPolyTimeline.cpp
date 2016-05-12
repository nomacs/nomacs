#include "DkPolyTimeline.h"
#include <QLabel>
#include "AspectRatioPixmapLabel.h"
#include <QDebug>
namespace nmp {

	DkPolyTimeline::DkPolyTimeline(QWidget* parent)
		: QLabel(parent),
		//mLayout(new QVBoxLayout(this)),
			mScrollArea(new QScrollArea(this))
	{
		setStyleSheet("background-color: #fff");
		//mWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
		//mWidget->setMinimumSize(100, 100);
		//mWidget->setLayout(mLayout);
		
		
		//mScrollArea->setWidgetResizable(true);
		//mScrollArea->setWidget(mWidget);
		//mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		//mScrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		//mScrollArea->setWidgetResizable(true);
		//mScrollArea->setAlignment(Qt::AlignLeft | Qt::AlignTop);
		

		//QWidget* content = new QWidget(this);
		//content->setStyleSheet("background-color: #fff");

		
		mLayout = new QGridLayout(this);
		mLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);


		//auto dummy = new QVBoxLayout(mWidget);
		//dummy->addWidget(content);

		
		////dummy->addWidget(mScrollArea);
		//dummy->addWidget(mWidget);
		////mScrollArea->setStyleSheet("background-color: #70ff0000");
		//dummy->setMargin(0);
		//setLayout(dummy); // dummy layout
		//mLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
		//
		mScrollArea->setVisible(false);
		//update();
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

	void DkPolyTimeline::resizeEvent(QResizeEvent * resize)
	{
		qDebug() << "Timeline size = " << size();
		
		//auto cnt = mList.size();
		//for (auto t : mList) {
		//	t->setFixedHeight(floor(resize->size().height() / static_cast<double>(cnt)));
		//}
		QWidget::resizeEvent(resize);
		//mScrollArea->setFixedSize(parent()->size);
		qDebug() << "Scroll area size = " << mScrollArea->size();
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
		//auto item = mLayout->itemAt(0);
		//while (item = mLayout->itemAt(0)) {
		//	if (item->widget()) {
		//		delete item->widget();
		//	}
		//}
		mElements.clear();
	}

	void DkSingleTimeline::updateSize(QResizeEvent * resize)
	{
		//qDebug() << "size = " << resize->size().height();
		//QSize s(resize->size().height(), resize->size().height());
		//for (auto e : mElements) {
		//	//e->setFixedWidth(height());
		//	
		//	auto image = e->pixmap();
		//	e->setPixmap(image->scaled(s, Qt::KeepAspectRatio, Qt::FastTransformation));
		//	//e->setSize
		//}
	}

	void DkSingleTimeline::addElement()
	{
		auto elem = std::make_shared<QImage>();
		//AspectRatioPixmapLabel* label = new AspectRatioPixmapLabel(this);
		auto label = new QLabel(mParent);
		QPixmap pm(30, 30);
		pm.fill(QColor(0, 0, 0, 150));
		label->setPixmap(pm);
		label->setMargin(0);
		label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
		label->setMinimumSize(1, 1);
		
		mElements.push_back(label);
		qDebug() << "has height for width " << label->hasHeightForWidth();

		mParent->setGridElement(label, mLayoutRow, mElements.size());
		
	}

	void DkSingleTimeline::setTransform(QTransform transform)
	{
		qDebug() << "Timeline transform changed";
		mTransform = transform;
	}

	void DkSingleTimeline::refresh()
	{
		if (!mPoly) {
			return;
		}

		qDebug() << "Timeline update";
		clear();

		for (auto p : mPoly->points()) {
			addElement();
		}

		//updateSize();
	}
	DkTimelineLabel::DkTimelineLabel(QWidget * parent)
		:QLabel(parent)
	{
	}
	DkTimelineLabel::~DkTimelineLabel()
	{
	}

	int DkTimelineLabel::heightForWidth(int w) const
	{
		return w;
	}
}