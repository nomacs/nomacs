/*******************************************************************************************************
 DkControlWidget.cpp
 Created on:	31.08.2015
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2013 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2013 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2013 Florian Kleber <florian@nomacs.org>

 This file is part of nomacs.

 nomacs is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 nomacs is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 *******************************************************************************************************/

#include "DkControlWidget.h"

#include "DkViewPort.h"
#include "DkWidgets.h"
#include "DkThumbsWidgets.h"
#include "DkMetaDataWidgets.h"
#include "DkMetaData.h"
#include "DkSettings.h"
#include "DkPluginInterface.h"
#include "DkToolbars.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QStackedLayout>
#include <QSharedPointer>
#include <QGridLayout>
#include <QTransform>
#include <QMouseEvent>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

// DkControlWidget --------------------------------------------------------------------
DkControlWidget::DkControlWidget(DkViewPort *parent, Qt::WindowFlags flags) : QWidget(parent, flags) {

	mViewport = parent;
	setObjectName("DkControlWidget");
	qDebug() << metaObject()->className();

	// cropping
	mCropWidget = new DkCropWidget(QRectF(), this);

	//// thumbnails, metadata
	//thumbPool = new DkThumbPool(QFileInfo(), this);
	mFilePreview = new DkFilePreview(this, flags);
	mMetaDataInfo = new DkMetaDataHUD(this);
	mZoomWidget = new DkZoomWidget(this);
	mPlayer = new DkPlayer(this);
	addActions(mPlayer->getActions().toList());

	mFolderScroll = new DkFolderScrollBar(this);

	// file info - overview
	mFileInfoLabel = new DkFileInfoLabel(this);
	mRatingLabel = new DkRatingLabelBg(2, this, flags);
	addActions(mRatingLabel->getActions().toList());		// register actions
	mCommentWidget = new DkCommentWidget(this);

	// delayed info
	mDelayedInfo = new DkDelayedMessage(QString(), 0, this); // TODO: make a nice constructor
	mDelayedSpinner = new DkDelayedInfo(0, this);

	// info labels
	mSpinnerLabel = new DkAnimationLabel(":/nomacs/img/loading.gif", this);
	mCenterLabel = new DkLabelBg(this, "");
	mBottomLabel = new DkLabelBg(this, "");
	mBottomLeftLabel = new DkLabelBg(this, "");

	// wheel label
	QPixmap wp = QPixmap(":/nomacs/img/thumbs-move.png");
	mWheelButton = new QLabel(this);
	mWheelButton->setAttribute(Qt::WA_TransparentForMouseEvents);
	mWheelButton->setPixmap(wp);
	mWheelButton->adjustSize();
	mWheelButton->hide();

	// image histogram
	mHistogram = new DkHistogram(this);
	
	init();
	connectWidgets();

	// add mousetracking to all widgets which allows the mViewport for status updates
	QObjectList widgets = children();
	for (int idx = 0; idx < widgets.size(); idx++) {
		if (QWidget* w = qobject_cast<QWidget*>(widgets.at(idx))) {
			w->setMouseTracking(true);
		}
	}
}

void DkControlWidget::init() {

	// debug: show invisible widgets
	setFocusPolicy(Qt::StrongFocus);
	setFocus(Qt::TabFocusReason);
	setMouseTracking(true);

	// connect widgets with their settings
	mFilePreview->setDisplaySettings(&DkSettings::app.showFilePreview);
	mMetaDataInfo->setDisplaySettings(&DkSettings::app.showMetaData);
	mFileInfoLabel->setDisplaySettings(&DkSettings::app.showFileInfoLabel);
	mPlayer->setDisplaySettings(&DkSettings::app.showPlayer);
	mHistogram->setDisplaySettings(&DkSettings::app.showHistogram);
	mCommentWidget->setDisplaySettings(&DkSettings::app.showComment);
	mZoomWidget->setDisplaySettings(&DkSettings::app.showOverview);
	mFolderScroll->setDisplaySettings(&DkSettings::app.showScroller);

	// some adjustments
	mBottomLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	mBottomLeftLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	mRatingLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	mCenterLabel->setAlignment(Qt::AlignCenter);
	mZoomWidget->setContentsMargins(10, 10, 0, 0);
	mCropWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	mSpinnerLabel->halfSize();
	mCommentWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	// dummy - needed for three equal columns @markus: do not delete!
	QWidget* dw = new QWidget(this);
	dw->setMouseTracking(true);
	QBoxLayout* dLayout = new QBoxLayout(QBoxLayout::LeftToRight, dw);
	dLayout->setContentsMargins(0,0,0,0);
	dLayout->addWidget(mBottomLabel);
	dLayout->addStretch();

	// zoom widget
	QWidget* bw = new QWidget(this);
	bw->setMouseTracking(true);
	//bw->setMinimumHeight(40);
	//bw->setMaximumHeight(80);
	QVBoxLayout* zLayout = new QVBoxLayout(bw);
	zLayout->setAlignment(Qt::AlignBottom);
	zLayout->setContentsMargins(0,0,0,20);
	zLayout->setSpacing(0);
	zLayout->addWidget(mBottomLabel);
	zLayout->addWidget(mBottomLeftLabel);
	zLayout->addWidget(mCommentWidget);

	//// comment widget
	//QWidget* cw = new QWidget();
	//cw->setContentsMargins(0,20,0,20);
	//cw->setMouseTracking(true);
	//cw->setMinimumHeight(40);
	//cw->setMaximumHeight(80);
	//QBoxLayout* coLayout = new QBoxLayout(QBoxLayout::RightToLeft, cw);
	//coLayout->setAlignment(Qt::AlignLeft);
	//coLayout->setContentsMargins(0,0,0,0);
	//coLayout->addWidget(commentWidget);

	// left column widget
	QWidget* leftWidget = new QWidget(this);
	leftWidget->setMouseTracking(true);
	QBoxLayout* ulLayout = new QBoxLayout(QBoxLayout::TopToBottom, leftWidget);
	ulLayout->setContentsMargins(0,0,0,0);
	ulLayout->setSpacing(0);
	ulLayout->addWidget(mZoomWidget);
	ulLayout->addStretch();
	ulLayout->addWidget(bw);
	ulLayout->addWidget(dw);
	//ulLayout->addWidget(cw);

	// center column
	QWidget* cW = new QWidget(this);
	cW->setMouseTracking(true);
	QBoxLayout* cwLayout = new QBoxLayout(QBoxLayout::LeftToRight, cW);
	cwLayout->setContentsMargins(0,0,0,0);
	cwLayout->addStretch();
	cwLayout->addWidget(mCenterLabel);
	cwLayout->addWidget(mSpinnerLabel);
	cwLayout->addStretch();

	// center player horizontally
	QWidget* cP = new QWidget(this);
	cP->setMouseTracking(true);
	QBoxLayout* cpLayout = new QBoxLayout(QBoxLayout::LeftToRight, cP);
	cpLayout->setContentsMargins(0,0,0,0);
	cpLayout->addWidget(mPlayer);

	// center column
	QWidget* center = new QWidget(this);
	center->setMouseTracking(true);
	QBoxLayout* cLayout = new QBoxLayout(QBoxLayout::TopToBottom, center);
	cLayout->setContentsMargins(0,0,0,0);
	cLayout->addStretch();
	cLayout->addWidget(cW);
	cLayout->addStretch();
	cLayout->addWidget(cP);
	
	// rating widget
	QWidget* rw = new QWidget(this);
	rw->setMouseTracking(true);
	rw->setMinimumSize(0,0);
	QBoxLayout* rLayout = new QBoxLayout(QBoxLayout::RightToLeft, rw);
	rLayout->setContentsMargins(0,0,0,17);
	rLayout->addWidget(mRatingLabel);
	rLayout->addStretch();

	// file info
	QWidget* fw = new QWidget(this);
	fw->setContentsMargins(0,0,0,30);
	fw->setMouseTracking(true);
	fw->setMinimumSize(0,0);
	QBoxLayout* rwLayout = new QBoxLayout(QBoxLayout::RightToLeft, fw);
	rwLayout->setContentsMargins(0,0,0,0);
	rwLayout->addWidget(mFileInfoLabel);
	rwLayout->addStretch();

	// right column
	QWidget* hw = new QWidget(this);
	hw->setContentsMargins(0,10,10,0);
	hw->setMouseTracking(true);
	QBoxLayout* hwLayout = new QBoxLayout(QBoxLayout::RightToLeft, hw);
	hwLayout->setContentsMargins(0,0,0,0);
	hwLayout->addWidget(mHistogram);
	hwLayout->addStretch();

	// right column
	QWidget* rightWidget = new QWidget(this);
	rightWidget->setMouseTracking(true);
	QBoxLayout* lrLayout = new QBoxLayout(QBoxLayout::TopToBottom, rightWidget);
	lrLayout->setContentsMargins(0,0,0,0);
	lrLayout->addWidget(hw);
	lrLayout->addStretch();
	lrLayout->addWidget(fw);
	lrLayout->addWidget(rw);
	
	// init main widgets
	mWidgets.resize(widget_end);
	mWidgets[hud_widget] = new QWidget(this);
	mWidgets[crop_widget] = mCropWidget;

	// global controller layout
	mHudLayout = new QGridLayout(mWidgets[hud_widget]);
	mHudLayout->setContentsMargins(0,0,0,0);
	mHudLayout->setSpacing(0);

	//hudLayout->addWidget(thumbWidget, 0, 0);

	// add elements
	changeThumbNailPosition(mFilePreview->getWindowPosition());
	changeMetaDataPosition(mMetaDataInfo->getWindowPosition());
	//hudLayout->addWidget(filePreview, top_thumbs, left_thumbs, 1, hor_pos_end);
	mHudLayout->addWidget(leftWidget, ver_center, left, 1, 1);
	mHudLayout->addWidget(center, ver_center, hor_center, 1, 1);
	mHudLayout->addWidget(rightWidget, ver_center, right, 1, 1);
	mHudLayout->addWidget(mFolderScroll, top_scroll, left_thumbs, 1, hor_pos_end);

	//// we need to put everything into extra widgets (which are exclusive) in order to handle the mouse events correctly
	//QHBoxLayout* editLayout = new QHBoxLayout(widgets[crop_widget]);
	//editLayout->setContentsMargins(0,0,0,0);
	//editLayout->addWidget(cropWidget);

	mLayout = new QStackedLayout(this);
	mLayout->setContentsMargins(0,0,0,0);
	
	for (int idx = 0; idx < mWidgets.size(); idx++)
		mLayout->addWidget(mWidgets[idx]);

	//// TODO: remove...
	//centerLabel->setText("ich bin richtig...", -1);
	//bottomLeftLabel->setText("topLeft label...", -1);
	//spinnerLabel->show();
	
	show();
	//thumbWidget->setVisible(true);
	qDebug() << "controller initialized...";
}

void DkControlWidget::connectWidgets() {

	if (!mViewport)
		return;

	// thumbs widget
	connect(mFilePreview, SIGNAL(loadFileSignal(const QString&)), mViewport, SLOT(loadFile(const QString&)));
	connect(mFilePreview, SIGNAL(changeFileSignal(int)), mViewport, SLOT(loadFileFast(int)));
	connect(mFilePreview, SIGNAL(positionChangeSignal(int)), this, SLOT(changeThumbNailPosition(int)));

	// metadata widget
	connect(mMetaDataInfo, SIGNAL(positionChangeSignal(int)), this, SLOT(changeMetaDataPosition(int)));

	// overview
	connect(mZoomWidget->getOverview(), SIGNAL(moveViewSignal(const QPointF&)), mViewport, SLOT(moveView(const QPointF&)));
	connect(mZoomWidget->getOverview(), SIGNAL(sendTransformSignal()), mViewport, SLOT(tcpSynchronize()));

	// zoom widget
	connect(mZoomWidget, SIGNAL(zoomSignal(float)), mViewport, SLOT(zoomTo(float)));
	connect(mViewport, SIGNAL(zoomSignal(float)), mZoomWidget, SLOT(updateZoom(float)));

	// waiting
	connect(mDelayedInfo, SIGNAL(infoSignal(const QString&, int)), this, SLOT(setInfo(const QString&, int)));
	connect(mDelayedSpinner, SIGNAL(infoSignal(int)), this, SLOT(setSpinner(int)));
	
	// rating
	connect(mFileInfoLabel->getRatingLabel(), SIGNAL(newRatingSignal(int)), this, SLOT(updateRating(int)));
	connect(mRatingLabel, SIGNAL(newRatingSignal(int)), this, SLOT(updateRating(int)));
	//connect(ratingLabel, SIGNAL(newRatingSignal(int)), metaDataInfo, SLOT(setRating(int)));

	// playing
	connect(mPlayer, SIGNAL(previousSignal()), mViewport, SLOT(loadPrevFileFast()));
	connect(mPlayer, SIGNAL(nextSignal()), mViewport, SLOT(loadNextFileFast()));

	// cropping
	connect(mCropWidget, SIGNAL(enterPressedSignal(DkRotatingRect, const QColor&)), mViewport, SLOT(cropImage(DkRotatingRect, const QColor&)));
	connect(mCropWidget, SIGNAL(cancelSignal()), this, SLOT(hideCrop()));

	// comment widget
	connect(mCommentWidget, SIGNAL(showInfoSignal(const QString&)), this, SLOT(setInfo(const QString&)));

	// mViewport
	connect(mViewport, SIGNAL(infoSignal(const QString&)), this, SLOT(setInfo(const QString&)));
}

void DkControlWidget::update() {

	mZoomWidget->update();

	QWidget::update();
}

void DkControlWidget::showWidgetsSettings() {

	if (mViewport->getImage().isNull()) {
		showPreview(false);
		showScroller(false);
		showMetaData(false);
		showFileInfo(false);
		showPlayer(false);
		showOverview(false);
		showHistogram(false);
		showCommentWidget(false);
		return;
	}

	//qDebug() << "current app mode: " << DkSettings::app.currentAppMode;

	showOverview(mZoomWidget->getCurrentDisplaySetting());
	showPreview(mFilePreview->getCurrentDisplaySetting());
	showMetaData(mMetaDataInfo->getCurrentDisplaySetting());
	showFileInfo(mFileInfoLabel->getCurrentDisplaySetting());
	showPlayer(mPlayer->getCurrentDisplaySetting());
	showHistogram(mHistogram->getCurrentDisplaySetting());
	showCommentWidget(mCommentWidget->getCurrentDisplaySetting());
	showScroller(mFolderScroll->getCurrentDisplaySetting());
}

void DkControlWidget::showPreview(bool visible) {

	if (!mFilePreview)
		return;

	if (visible && !mFilePreview->isVisible())
		mFilePreview->show();
	else if (!visible && mFilePreview->isVisible())
		mFilePreview->hide(!mViewport->getImage().isNull());	// do not save settings if we have no image in the mViewport
}

void DkControlWidget::showScroller(bool visible) {

	if (!mFolderScroll)
		return;

	if (visible && !mFolderScroll->isVisible())
		mFolderScroll->show();
	else if (!visible && mFolderScroll->isVisible())
		mFolderScroll->hide(!mViewport->getImage().isNull());	// do not save settings if we have no image in the mViewport
}

void DkControlWidget::showMetaData(bool visible) {

	if (!mMetaDataInfo)
		return;

	if (visible && !mMetaDataInfo->isVisible()) {
		mMetaDataInfo->show();
		qDebug() << "mShowing metadata...";
	}
	else if (!visible && mMetaDataInfo->isVisible())
		mMetaDataInfo->hide(!mViewport->getImage().isNull());	// do not save settings if we have no image in the mViewport
}

void DkControlWidget::showFileInfo(bool visible) {

	if (!mFileInfoLabel)
		return;

	if (visible && !mFileInfoLabel->isVisible()) {
		mFileInfoLabel->show();
		mRatingLabel->block(mFileInfoLabel->isVisible());
	}
	else if (!visible && mFileInfoLabel->isVisible()) {
		mFileInfoLabel->hide(!mViewport->getImage().isNull());	// do not save settings if we have no image in the mViewport
		mRatingLabel->block(false);
	}
}

void DkControlWidget::showPlayer(bool visible) {

	if (!mPlayer)
		return;

	if (visible)
		mPlayer->show();
	else
		mPlayer->hide(!mViewport->getImage().isNull());	// do not save settings if we have no image in the mViewport
}

void DkControlWidget::showOverview(bool visible) {

	if (!mZoomWidget)
		return;

	if (visible && !mZoomWidget->isVisible()) {		
		mZoomWidget->show();
	}
	else if (!visible && mZoomWidget->isVisible()) {
		mZoomWidget->hide(!mViewport->getImage().isNull());	// do not save settings if we have no image in the mViewport
	}

}

void DkControlWidget::hideCrop(bool hide /* = true */) {

	showCrop(!hide);
}

void DkControlWidget::showCrop(bool visible) {

	mViewport->applyPluginChanges();

	if (visible) {
		mCropWidget->reset();
		switchWidget(mWidgets[crop_widget]);
		connect(mCropWidget->getToolbar(), SIGNAL(colorSignal(const QBrush&)), mViewport, SLOT(setBackgroundBrush(const QBrush&)));
	}
	else
		switchWidget();

}

void DkControlWidget::showHistogram(bool visible) {
	
	if (!mHistogram)
		return;

	if (visible && !mHistogram->isVisible()) {
		mHistogram->show();
		if(!mViewport->getImage().isNull()) mHistogram->drawHistogram(mViewport->getImage());
		else  mHistogram->clearHistogram();
	}
	else if (!visible && mHistogram->isVisible()) {
		mHistogram->hide(!mViewport->getImage().isNull());	// do not save settings if we have no image in the mViewport
	}
}

void DkControlWidget::showCommentWidget(bool visible) {

	if (!mCommentWidget)
		return;

	if (visible && !mCommentWidget->isVisible()) {
		mCommentWidget->show();
	}
	else if (!visible && mCommentWidget->isVisible()) {
		mCommentWidget->hide(!mViewport->getImage().isNull());	// do not save settings if we have no image in the mViewport
	}
}

void DkControlWidget::switchWidget(QWidget* widget) {

	if (mLayout->currentWidget() == widget || !widget && mLayout->currentWidget() == mWidgets[hud_widget])
		return;

	if (widget)
		mLayout->setCurrentWidget(widget);
	else
		mLayout->setCurrentWidget(mWidgets[hud_widget]);

	if (mLayout->currentWidget())
		qDebug() << "changed to widget: " << mLayout->currentWidget();

	// ok, this is really nasty... however, the fileInfo layout is destroyed otherwise
	if (mLayout->currentIndex() == hud_widget && mFileInfoLabel->isVisible()) {
		mFileInfoLabel->setVisible(false);
		showFileInfo(true);
	}

}

void DkControlWidget::setPluginWidget(DkViewPortInterface* pluginWidget, bool removeWidget) {

	DkPluginViewPort* pluginViewport = pluginWidget->getViewPort();

	if (!pluginViewport) return;

	if (!removeWidget) {
		pluginViewport->setWorldMatrix(mViewport->getWorldMatrixPtr());
		pluginViewport->setImgMatrix(mViewport->getImageMatrixPtr());
	}

	mViewport->setPaintWidget(dynamic_cast<QWidget*>(pluginViewport), removeWidget);
	
	if (removeWidget) {
		pluginWidget->deleteViewPort();
	}
}

void DkControlWidget::setFileInfo(QSharedPointer<DkImageContainerT> imgC) {

	if (!imgC)
		return;

	mImgC = imgC;

	QSharedPointer<DkMetaDataT> metaData = imgC->getMetaData();

	QString dateString = metaData->getExifValue("DateTimeOriginal");
	mFileInfoLabel->updateInfo(imgC->filePath(), "", dateString, metaData->getRating());
	mFileInfoLabel->setEdited(imgC->isEdited());
	mCommentWidget->setMetaData(metaData);
	updateRating(metaData->getRating());
}

void DkControlWidget::setInfo(const QString& msg, int time, int location) {

	if (location == center_label && mCenterLabel)
		mCenterLabel->setText(msg, time);
	else if (location == bottom_left_label && mBottomLabel)
		mBottomLabel->setText(msg, time);
	else if (location == top_left_label && mBottomLeftLabel)
		mBottomLeftLabel->setText(msg, time);

	update();
}

void DkControlWidget::setInfoDelayed(const QString& msg, bool start, int delayTime) {

	if (!mCenterLabel)
		return;

	if (start)
		mDelayedInfo->setInfo(msg, delayTime);
	else
		mDelayedInfo->stop();

}

void DkControlWidget::setSpinner(int time) {

	if (mSpinnerLabel)
		mSpinnerLabel->showTimed(time);
}

void DkControlWidget::setSpinnerDelayed(bool start, int time) {

	if (!mSpinnerLabel) 
		return;

	if (start)
		mDelayedSpinner->setInfo(time);
	else
		mDelayedSpinner->stop();
}

void DkControlWidget::changeMetaDataPosition(int pos) {

	if (pos == DkWidget::pos_west) {
		mHudLayout->addWidget(mMetaDataInfo, top_metadata, left_metadata, bottom_metadata-top_metadata, 1);	
	}
	else if (pos == DkWidget::pos_east) {
		mHudLayout->addWidget(mMetaDataInfo, top_metadata, right_metadata, bottom_metadata-top_metadata, 1);	
	}
	else if (pos == DkWidget::pos_north) {
		mHudLayout->addWidget(mMetaDataInfo, top_metadata, left_metadata, 1, hor_pos_end-2);	
	}
	else if (pos == DkWidget::pos_south) {
		mHudLayout->addWidget(mMetaDataInfo, bottom_metadata, left_metadata, 1, hor_pos_end-2);	
	}
}

void DkControlWidget::changeThumbNailPosition(int pos) {

	if (pos == DkWidget::pos_west) {
		mHudLayout->addWidget(mFilePreview, top_thumbs, left_thumbs, ver_pos_end, 1);	
	}
	else if (pos == DkWidget::pos_east) {
		mHudLayout->addWidget(mFilePreview, top_thumbs, right_thumbs, ver_pos_end, 1);	
	}
	else if (pos == DkWidget::pos_north) {
		mHudLayout->addWidget(mFilePreview, top_thumbs, left_thumbs, 1, hor_pos_end);	
	}
	else if (pos == DkWidget::pos_south) {
		mHudLayout->addWidget(mFilePreview, bottom_thumbs, left_thumbs, 1, hor_pos_end);	
	}
	else 
		mFilePreview->hide();

}

void DkControlWidget::stopLabels() {

	mCenterLabel->stop();
	mBottomLabel->stop();
	//topLeftLabel->stop();
	mSpinnerLabel->stop();

	//showCrop(false);
	//showThumbView(false);
	//showRecentFiles(false);
	switchWidget();
}

void DkControlWidget::settingsChanged() {

	if (mFileInfoLabel && mFileInfoLabel->isVisible()) {
		showFileInfo(false);	// just a hack but all states are preserved this way
		showFileInfo(true);
	}

}

void DkControlWidget::updateRating(int rating) {

	if (!mImgC)
		return;

	mRatingLabel->setRating(rating);

	if (mFileInfoLabel)
		mFileInfoLabel->updateRating(rating);

	QSharedPointer<DkMetaDataT> metaDataInfo = mImgC->getMetaData();
	metaDataInfo->setRating(rating);
}

void DkControlWidget::imageLoaded(bool) {

	showWidgetsSettings();
}

void DkControlWidget::setFullScreen(bool fullscreen) {

	showWidgetsSettings();

	if (fullscreen &&!mPlayer->isVisible())
		mPlayer->show(3000);		
}

DkMetaDataHUD * DkControlWidget::getMetaDataWidget() const {
	return mMetaDataInfo;
}

DkCommentWidget * DkControlWidget::getCommentWidget() const {
	return mCommentWidget;
}

DkOverview * DkControlWidget::getOverview() const {
	return mZoomWidget->getOverview();
}

DkZoomWidget * DkControlWidget::getZoomWidget() const {
	return mZoomWidget;
}

DkPlayer * DkControlWidget::getPlayer() const {
	return mPlayer;
}

DkFileInfoLabel * DkControlWidget::getFileInfoLabel() const {
	return mFileInfoLabel;
}

DkHistogram * DkControlWidget::getHistogram() const {
	return mHistogram;
}

DkCropWidget * DkControlWidget::getCropWidget() const {
	return mCropWidget;
}

DkFilePreview * DkControlWidget::getFilePreview() const {
	return mFilePreview;
}

DkFolderScrollBar * DkControlWidget::getScroller() const {
	return mFolderScroll;
}

// DkControlWidget - Events --------------------------------------------------------------------
void DkControlWidget::mousePressEvent(QMouseEvent *event) {

	qDebug() << "has mouse tracking: " << hasMouseTracking();

	mEnterPos = event->pos();

	if (mFilePreview && mFilePreview->isVisible() && event->buttons() == Qt::MiddleButton) {

		QTimer* mImgTimer = mFilePreview->getMoveImageTimer();
		mImgTimer->start(1);

		// show icon
		mWheelButton->move(event->pos().x()-16, event->pos().y()-16);
		mWheelButton->show();
	}

	QWidget::mousePressEvent(event);
}

void DkControlWidget::mouseReleaseEvent(QMouseEvent *event) {

	if (mFilePreview && mFilePreview->isVisible()) {
		mFilePreview->setCurrentDx(0);
		QTimer* mImgTimer = mFilePreview->getMoveImageTimer();
		mImgTimer->stop();
		mWheelButton->hide();
	}

	QWidget::mouseReleaseEvent(event);
}

void DkControlWidget::mouseMoveEvent(QMouseEvent *event) {

	// scroll thumbs preview
	if (mFilePreview && mFilePreview->isVisible() && event->buttons() == Qt::MiddleButton) {
		
		float dx = (float)std::fabs(mEnterPos.x() - event->pos().x())*0.015f;
		dx = std::exp(dx);
		if (mEnterPos.x() - event->pos().x() < 0)
			dx = -dx;

		mFilePreview->setCurrentDx(dx);	// update dx
	}

	QWidget::mouseMoveEvent(event);
}



void DkControlWidget::keyPressEvent(QKeyEvent *event) {
	
	QWidget::keyPressEvent(event);
}

void DkControlWidget::keyReleaseEvent(QKeyEvent *event) {

	QWidget::keyReleaseEvent(event);
}

}
