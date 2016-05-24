#include "DkPolyTimeline.h"
#include "DkBaseWidgets.h"
#include <QLabel>
#include "AspectRatioPixmapLabel.h"
#include <QDebug>
#include <QPainter>
#include <cassert>
namespace nmp {
	DkTrackedTransform::DkTrackedTransform(QColor color, QTransform transform)
		: mColor(color), mTransform(transform)
	{
	}

	QTransform DkTrackedTransform::get()
	{
		return mTransform;
	}

	QColor DkTrackedTransform::getColor()
	{
		return mColor;
	}

	void DkTrackedTransform::set(QTransform transform)
	{
		mTransform = transform;
		emit changed(mTransform);
	}

	auto DkInterpolatedSyncedPolyon::getStep()
	{
		return mStep;
	}
	auto DkInterpolatedSyncedPolyon::size()
	{
		return mPoints.size();
	}
	const auto& DkInterpolatedSyncedPolyon::points() const
	{
		return mPoints;
	}
	void DkInterpolatedSyncedPolyon::setStep(double step)
	{
		mStep = step;
		recalcuate();
	}

	DkInterpolatedSyncedPolyon::DkInterpolatedSyncedPolyon(QSharedPointer<DkSyncedPolygon> poly)
		: mPolygon(poly), mStep(100)
	{
		// connect signals
		connect(mPolygon.data(), &DkSyncedPolygon::pointAdded, this, &DkInterpolatedSyncedPolyon::recalcuate);
		connect(mPolygon.data(), &DkSyncedPolygon::changed, this, &DkInterpolatedSyncedPolyon::recalcuate);
		connect(mPolygon.data(), &DkSyncedPolygon::movedPoint, this, &DkInterpolatedSyncedPolyon::recalcuate);

		recalcuate();
	}

	void DkInterpolatedSyncedPolyon::recalcuate()
	{
		mPoints.clear();	// clear since we recalculate everything

		if (mPolygon->points().empty()) {	// if no control points are here just quit
			return;
		}

		// convenience function for adding points
		auto add = [this](auto type, const auto& point) {
			mPoints.append(std::make_pair(
				type,
				point)
			);
		};

		// get start and add (we checked, size() > 0)
		auto iter_begin = mPolygon->points().begin();
		auto start = (*iter_begin)->getPos();
		add(TimeLinePointType::Control, start);

		// loop through rest of the points
		for (auto next = ++iter_begin; next != mPolygon->points().end(); ++next) {
			auto end = (*next)->getPos();

			// add points on line with stepsize mStep
			QLineF line(start, end);
			auto dist = mStep;
			while (dist < line.length()) {
				auto pat = dist / line.length();
				auto res = line.pointAt(pat);
				add(TimeLinePointType::Line, line.pointAt(pat));
				dist += mStep;
			}

			// don't forget to add control point @ the end
			add(TimeLinePointType::Control, end);
			start = end;
		}

		qDebug() << "DkInterpolatedSyncedPolygon npoints:" << mPoints.size();
		emit changed();
	}

	DkPolyTimeline::DkPolyTimeline(QSharedPointer<DkSyncedPolygon> poly, QWidget* parent)
		: QLabel(parent),
		mSize(40),
		mLayout(nullptr)
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
		
		setSyncedPolygon(poly); 
	}


	QSharedPointer<DkTrackedTransform> DkPolyTimeline::addTimeline(QSharedPointer<DkPolygonRenderer> renderer)
	{
		auto transform = QSharedPointer<DkTrackedTransform>::create(renderer->getColor(), renderer->getTransform());
		
		// make sure we update on change
		connect(transform.data(), &DkTrackedTransform::changed, 
					this, [this, transform]() { updateTransform(transform); });

		//// connections for timeline
		connect(renderer.data(), &DkPolygonRenderer::transformChanged,
			transform.data(), &std::remove_pointer<decltype(transform.data())>::type::set);

		// this is our cleanup slot
		connect(renderer.data(), &DkPolygonRenderer::removed, this, [this, transform]() {
			removeTransform(transform);
		});

		mList.push_back(transform);
	
		refresh();
		return transform;
	}

	void DkPolyTimeline::setSyncedPolygon(QSharedPointer<DkSyncedPolygon> poly)
	{
		mList.clear();

		if (mPolygon) {		// disconnect old polygon
			disconnect(mPolygon.data(), &std::remove_pointer<decltype(mPolygon.data())>::type::changed,
							this, &std::remove_pointer<decltype(this)>::type::refresh);
		}

		// connect new one
		mPolygon = QSharedPointer<DkInterpolatedSyncedPolyon>::create(poly);
		connect(mPolygon.data(), &std::remove_pointer<decltype(mPolygon.data())>::type::changed,
			this, &std::remove_pointer<decltype(this)>::type::refresh);

		mPolygon->setStep(mStepSize);
	}

	void DkPolyTimeline::setImage(QSharedPointer<nmc::DkImageContainerT> img)
	{
		mImage = mImage = QPixmap::fromImage(img->image());
	}
	void DkPolyTimeline::reset()
	{
		// delete all widgets in grid
		auto item = mLayout->itemAt(0);
		while (item = mLayout->itemAt(0)) {
			if (item->widget()) {
				delete item->widget();
			}
		}

		for (auto t : mList) {
			t->disconnect();
		}
		// remove all transforms
		mList.clear();
		mElements.clear();
	}

	void DkPolyTimeline::setGridElement(QWidget * widget, int row, int column)
	{
		mLayout->addWidget(widget, row, column);
	}

	void DkPolyTimeline::clearGridElement(QWidget * widget)
	{
		mLayout->removeWidget(widget);
	}

	void DkPolyTimeline::updateTransform(QSharedPointer<DkTrackedTransform> sender)
	{
		updateEmptyElements();

		auto row = mList.indexOf(sender);
		auto iter = mElements[row].begin();

		for (const auto& p : mPolygon->points()) {
			
			
			QPixmap pm(mSize, mSize);  // lets draw here
			pm.fill(Qt::red);
			QPainter paint(&pm);  // our painter

			QRect src(pm.rect());
			src.moveCenter(p.second.toPoint());


			src = mList[row]->get().mapRect(src);
			paint.drawPixmap(pm.rect(), mImage, src);
			
			if (p.first == TimeLinePointType::Control) {
				auto color = mList[row]->getColor();
				color.setAlpha(60);
				paint.setBrush(QBrush(color));
				paint.setPen(QPen());
				paint.drawRect(QRect(-1,-1,mSize+1, mSize+1));
			}


			if (iter != mElements[row].end()) {
				if (!*iter) {
					qDebug() << " Something wrong with this iterator ";
				}
				else {
					(*iter)->setPixmap(pm);
				}
				//break;
				++iter;
			} else {
				qDebug() << " Iterator == END ";
			}
			
		}
		update();
	}

	void DkPolyTimeline::refresh()
	{
		updateEmptyElements();

		for (auto t : mList) {
			updateTransform(t);
		}
	}

	void DkPolyTimeline::removeTransform(QSharedPointer<DkTrackedTransform> sender)
	{
		mList.removeAll(sender);

		refresh();
	}

	void DkPolyTimeline::setStepSize(int stepsize)
	{
		mStepSize = stepsize;
		mPolygon->setStep(stepsize);
	}

	void DkPolyTimeline::updateEmptyElements()
	{
		auto rows = mList.size();	//number of rows we need
		auto cols = mPolygon->size();

		// check if nothing has changed
		if (rows == mElements.size()) {
			auto ok = true;
			for (auto r : mElements) {
				ok &= r.size() == cols;
			}
			if (ok) {
				return;
			}
		}

		// function for creating new label
		auto label = [this]() {
			auto label = new QLabel();
			label->setStyleSheet("QLabel{background-color: #eee; }");
			label->setMaximumSize(QSize(200, 200));
			label->setMinimumWidth(100);

			label->setScaledContents(true);

			label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
			
			return label;
		};

		for (auto r = 0; r < std::max(rows, mElements.size()); ++r) {	// loop over rows

			if (r >= mElements.size()) {		// row missing
				mElements.push_back(QVector<QLabel*>());	// add row
			}

			for (auto c = 0; c < std::max(cols, mElements[r].size()); ++c) {	// loop over columns

				if (c >= mElements[r].size()) {		// column missing
					auto l = label();
					mElements[r].push_back(l);
					setGridElement(l, r, c);
				}

				if (r >= rows || c >= cols) {	// delete unneeded elements
					clearGridElement(mElements[r][c]);
					delete mElements[r][c];
					mElements[r][c] = nullptr;
				}
			}

			if (mElements[r].size() > cols) {	// resize vector/remove unneded columns
				mElements[r].remove(cols, mElements[r].size() - cols);
			}
		}

		if (mElements.size() > rows) {  // remove unneeded rows
			mElements.remove(rows, mElements.size() - rows);
		}

		update();  // should update 
	}

	DkTimelineLabel::DkTimelineLabel(QWidget * parent)
		:QLabel(parent)
	{
	}
}