/*******************************************************************************************************
 DkViewPort.cpp
 Created on:	05.05.2011
 
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

#include "DkNoMacs.h"	// currently needed for some enums
#include "DkThumbsWidgets.h"
#include "DkMetaData.h"
#include "DkMetaDataWidgets.h"
#include "DkNetwork.h"
#include "DkImageContainer.h"
#include "DkViewPort.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QClipboard>
#include <QShortcut>
#include <QMovie>
#include <QMimeData>
#include <QAction>
#include <qmath.h>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

// DkControlWidget --------------------------------------------------------------------
DkControlWidget::DkControlWidget(DkViewPort *parent, Qt::WindowFlags flags) : QWidget(parent, flags) {

	viewport = parent;
	setObjectName("DkControlWidget");
	qDebug() << this->metaObject()->className();

	// cropping
	cropWidget = new DkCropWidget(QRectF(), this);

	//// thumbnails, metadata
	//thumbPool = new DkThumbPool(QFileInfo(), this);
	filePreview = new DkFilePreview(this, flags);
	metaDataInfo = new DkMetaDataHUD(this);
	zoomWidget = new DkZoomWidget(this);
	player = new DkPlayer(this);
	addActions(player->getActions().toList());

	folderScroll = new DkFolderScrollBar(this);

	// file info - overview
	fileInfoLabel = new DkFileInfoLabel(this);
	ratingLabel = new DkRatingLabelBg(2, this, flags);
	addActions(ratingLabel->getActions().toList());		// register actions
	commentWidget = new DkCommentWidget(this);

	// delayed info
	delayedInfo = new DkDelayedMessage(QString(), 0, this); // TODO: make a nice constructor
	delayedSpinner = new DkDelayedInfo(0, this);

	// info labels
	spinnerLabel = new DkAnimationLabel(":/nomacs/img/loading.gif", this);
	centerLabel = new DkLabelBg(this, "");
	bottomLabel = new DkLabelBg(this, "");
	bottomLeftLabel = new DkLabelBg(this, "");

	// wheel label
	QPixmap wp = QPixmap(":/nomacs/img/thumbs-move.png");
	wheelButton = new QLabel(this);
	wheelButton->setAttribute(Qt::WA_TransparentForMouseEvents);
	wheelButton->setPixmap(wp);
	wheelButton->adjustSize();
	wheelButton->hide();

	// image histogram
	histogram = new DkHistogram(this);
	
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
	filePreview->setDisplaySettings(&DkSettings::app.showFilePreview);
	metaDataInfo->setDisplaySettings(&DkSettings::app.showMetaData);
	fileInfoLabel->setDisplaySettings(&DkSettings::app.showFileInfoLabel);
	player->setDisplaySettings(&DkSettings::app.showPlayer);
	histogram->setDisplaySettings(&DkSettings::app.showHistogram);
	commentWidget->setDisplaySettings(&DkSettings::app.showComment);
	zoomWidget->setDisplaySettings(&DkSettings::app.showOverview);
	folderScroll->setDisplaySettings(&DkSettings::app.showScroller);

	// some adjustments
	bottomLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	bottomLeftLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	ratingLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	centerLabel->setAlignment(Qt::AlignCenter);
	zoomWidget->setContentsMargins(10, 10, 0, 0);
	cropWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	spinnerLabel->halfSize();
	commentWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	// dummy - needed for three equal columns @markus: do not delete!
	QWidget* dw = new QWidget(this);
	dw->setMouseTracking(true);
	QBoxLayout* dLayout = new QBoxLayout(QBoxLayout::LeftToRight, dw);
	dLayout->setContentsMargins(0,0,0,0);
	dLayout->addWidget(bottomLabel);
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
	zLayout->addWidget(bottomLabel);
	zLayout->addWidget(bottomLeftLabel);
	zLayout->addWidget(commentWidget);

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
	ulLayout->addWidget(zoomWidget);
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
	cwLayout->addWidget(centerLabel);
	cwLayout->addWidget(spinnerLabel);
	cwLayout->addStretch();

	// center player horizontally
	QWidget* cP = new QWidget(this);
	cP->setMouseTracking(true);
	QBoxLayout* cpLayout = new QBoxLayout(QBoxLayout::LeftToRight, cP);
	cpLayout->setContentsMargins(0,0,0,0);
	cpLayout->addWidget(player);

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
	rLayout->addWidget(ratingLabel);
	rLayout->addStretch();

	// file info
	QWidget* fw = new QWidget(this);
	fw->setContentsMargins(0,0,0,30);
	fw->setMouseTracking(true);
	fw->setMinimumSize(0,0);
	QBoxLayout* rwLayout = new QBoxLayout(QBoxLayout::RightToLeft, fw);
	rwLayout->setContentsMargins(0,0,0,0);
	rwLayout->addWidget(fileInfoLabel);
	rwLayout->addStretch();

	// right column
	QWidget* hw = new QWidget(this);
	hw->setContentsMargins(0,10,10,0);
	hw->setMouseTracking(true);
	QBoxLayout* hwLayout = new QBoxLayout(QBoxLayout::RightToLeft, hw);
	hwLayout->setContentsMargins(0,0,0,0);
	hwLayout->addWidget(histogram);
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
	widgets.resize(widget_end);
	widgets[hud_widget] = new QWidget(this);
	widgets[crop_widget] = cropWidget;
	lastActiveWidget = widgets[hud_widget];

	// global controller layout
	hudLayout = new QGridLayout(widgets[hud_widget]);
	hudLayout->setContentsMargins(0,0,0,0);
	hudLayout->setSpacing(0);

	//hudLayout->addWidget(thumbWidget, 0, 0);

	// add elements
	changeThumbNailPosition(filePreview->getWindowPosition());
	changeMetaDataPosition(metaDataInfo->getWindowPosition());
	//hudLayout->addWidget(filePreview, top_thumbs, left_thumbs, 1, hor_pos_end);
	hudLayout->addWidget(leftWidget, ver_center, left, 1, 1);
	hudLayout->addWidget(center, ver_center, hor_center, 1, 1);
	hudLayout->addWidget(rightWidget, ver_center, right, 1, 1);
	hudLayout->addWidget(folderScroll, top_scroll, left_thumbs, 1, hor_pos_end);

	//// we need to put everything into extra widgets (which are exclusive) in order to handle the mouse events correctly
	//QHBoxLayout* editLayout = new QHBoxLayout(widgets[crop_widget]);
	//editLayout->setContentsMargins(0,0,0,0);
	//editLayout->addWidget(cropWidget);

	layout = new QStackedLayout(this);
	layout->setContentsMargins(0,0,0,0);
	
	for (int idx = 0; idx < widgets.size(); idx++)
		layout->addWidget(widgets[idx]);

	//// TODO: remove...
	//centerLabel->setText("ich bin richtig...", -1);
	//bottomLeftLabel->setText("topLeft label...", -1);
	//spinnerLabel->show();
	
	show();
	//thumbWidget->setVisible(true);
	qDebug() << "controller initialized...";
}

void DkControlWidget::connectWidgets() {

	if (!viewport)
		return;

	// thumbs widget
	connect(filePreview, SIGNAL(loadFileSignal(const QString&)), viewport, SLOT(loadFile(const QString&)));
	connect(filePreview, SIGNAL(changeFileSignal(int)), viewport, SLOT(loadFileFast(int)));
	connect(filePreview, SIGNAL(positionChangeSignal(int)), this, SLOT(changeThumbNailPosition(int)));

	// metadata widget
	connect(metaDataInfo, SIGNAL(positionChangeSignal(int)), this, SLOT(changeMetaDataPosition(int)));

	// overview
	connect(zoomWidget->getOverview(), SIGNAL(moveViewSignal(QPointF)), viewport, SLOT(moveView(QPointF)));
	connect(zoomWidget->getOverview(), SIGNAL(sendTransformSignal()), viewport, SLOT(tcpSynchronize()));

	// zoom widget
	connect(zoomWidget, SIGNAL(zoomSignal(float)), viewport, SLOT(zoomTo(float)));
	connect(viewport, SIGNAL(zoomSignal(float)), zoomWidget, SLOT(updateZoom(float)));

	// waiting
	connect(delayedInfo, SIGNAL(infoSignal(QString, int)), this, SLOT(setInfo(QString, int)));
	connect(delayedSpinner, SIGNAL(infoSignal(int)), this, SLOT(setSpinner(int)));
	
	// rating
	connect(fileInfoLabel->getRatingLabel(), SIGNAL(newRatingSignal(int)), this, SLOT(updateRating(int)));
	connect(ratingLabel, SIGNAL(newRatingSignal(int)), this, SLOT(updateRating(int)));
	//connect(ratingLabel, SIGNAL(newRatingSignal(int)), metaDataInfo, SLOT(setRating(int)));

	// playing
	connect(player, SIGNAL(previousSignal()), viewport, SLOT(loadPrevFileFast()));
	connect(player, SIGNAL(nextSignal()), viewport, SLOT(loadNextFileFast()));

	// cropping
	connect(cropWidget, SIGNAL(enterPressedSignal(DkRotatingRect, const QColor&)), viewport, SLOT(cropImage(DkRotatingRect, const QColor&)));
	connect(cropWidget, SIGNAL(cancelSignal()), this, SLOT(hideCrop()));

	// comment widget
	connect(commentWidget, SIGNAL(showInfoSignal(QString)), this, SLOT(setInfo(QString)));

	// mViewport
	connect(viewport, SIGNAL(infoSignal(QString)), this, SLOT(setInfo(QString)));
}

void DkControlWidget::update() {

	zoomWidget->update();

	QWidget::update();
}

void DkControlWidget::showWidgetsSettings() {

	if (viewport->getImage().isNull()) {
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

	showOverview(zoomWidget->getCurrentDisplaySetting());
	showPreview(filePreview->getCurrentDisplaySetting());
	showMetaData(metaDataInfo->getCurrentDisplaySetting());
	showFileInfo(fileInfoLabel->getCurrentDisplaySetting());
	showPlayer(player->getCurrentDisplaySetting());
	showHistogram(histogram->getCurrentDisplaySetting());
	showCommentWidget(commentWidget->getCurrentDisplaySetting());
	showScroller(folderScroll->getCurrentDisplaySetting());
}

void DkControlWidget::showPreview(bool visible) {

	if (!filePreview)
		return;

	if (visible && !filePreview->isVisible())
		filePreview->show();
	else if (!visible && filePreview->isVisible())
		filePreview->hide(!viewport->getImage().isNull());	// do not save settings if we have no image in the mViewport
}

void DkControlWidget::showScroller(bool visible) {

	if (!folderScroll)
		return;

	if (visible && !folderScroll->isVisible())
		folderScroll->show();
	else if (!visible && folderScroll->isVisible())
		folderScroll->hide(!viewport->getImage().isNull());	// do not save settings if we have no image in the mViewport
}

void DkControlWidget::showMetaData(bool visible) {

	if (!metaDataInfo)
		return;

	if (visible && !metaDataInfo->isVisible()) {
		metaDataInfo->show();
		qDebug() << "mShowing metadata...";
	}
	else if (!visible && metaDataInfo->isVisible())
		metaDataInfo->hide(!viewport->getImage().isNull());	// do not save settings if we have no image in the mViewport
}

void DkControlWidget::showFileInfo(bool visible) {

	if (!fileInfoLabel)
		return;

	if (visible && !fileInfoLabel->isVisible()) {
		fileInfoLabel->show();
		ratingLabel->block(fileInfoLabel->isVisible());
	}
	else if (!visible && fileInfoLabel->isVisible()) {
		fileInfoLabel->hide(!viewport->getImage().isNull());	// do not save settings if we have no image in the mViewport
		ratingLabel->block(false);
	}
}

void DkControlWidget::showPlayer(bool visible) {

	if (!player)
		return;

	if (visible)
		player->show();
	else
		player->hide(!viewport->getImage().isNull());	// do not save settings if we have no image in the mViewport
}

void DkControlWidget::showOverview(bool visible) {

	if (!zoomWidget)
		return;

	if (visible && !zoomWidget->isVisible()) {		
		zoomWidget->show();
	}
	else if (!visible && zoomWidget->isVisible()) {
		zoomWidget->hide(!viewport->getImage().isNull());	// do not save settings if we have no image in the mViewport
	}

}

void DkControlWidget::hideCrop(bool hide /* = true */) {

	showCrop(!hide);
}

void DkControlWidget::showCrop(bool visible) {

	viewport->applyPluginChanges();

	if (visible) {
		cropWidget->reset();
		switchWidget(widgets[crop_widget]);
		connect(cropWidget->getToolbar(), SIGNAL(colorSignal(const QBrush&)), viewport, SLOT(setBackgroundBrush(const QBrush&)));
	}
	else
		switchWidget();

}

void DkControlWidget::showHistogram(bool visible) {
	
	if (!histogram)
		return;

	if (visible && !histogram->isVisible()) {
		histogram->show();
		if(!viewport->getImage().isNull()) histogram->drawHistogram(viewport->getImage());
		else  histogram->clearHistogram();
	}
	else if (!visible && histogram->isVisible()) {
		histogram->hide(!viewport->getImage().isNull());	// do not save settings if we have no image in the mViewport
	}
}

void DkControlWidget::showCommentWidget(bool visible) {

	if (!commentWidget)
		return;

	if (visible && !commentWidget->isVisible()) {
		commentWidget->show();
	}
	else if (!visible && commentWidget->isVisible()) {
		commentWidget->hide(!viewport->getImage().isNull());	// do not save settings if we have no image in the mViewport
	}
}

void DkControlWidget::switchWidget(QWidget* widget) {

	if (layout->currentWidget() == widget || !widget && layout->currentWidget() == widgets[hud_widget])
		return;

	if (widget)
		layout->setCurrentWidget(widget);
	else
		layout->setCurrentWidget(widgets[hud_widget]);

	if (layout->currentWidget())
		qDebug() << "changed to widget: " << layout->currentWidget();

	// ok, this is really nasty... however, the fileInfo layout is destroyed otherwise
	if (layout->currentIndex() == hud_widget && fileInfoLabel->isVisible()) {
		fileInfoLabel->setVisible(false);
		showFileInfo(true);
	}

}

void DkControlWidget::setPluginWidget(DkViewPortInterface* pluginWidget, bool removeWidget) {

	DkPluginViewPort* pluginViewport = pluginWidget->getViewPort();

	if (!pluginViewport) return;

	if (!removeWidget) {
		pluginViewport->setWorldMatrix(viewport->getWorldMatrixPtr());
		pluginViewport->setImgMatrix(viewport->getImageMatrixPtr());
	}

	viewport->setPaintWidget(dynamic_cast<QWidget*>(pluginViewport), removeWidget);
	
	if (removeWidget) {
		pluginWidget->deleteViewPort();
	}
}

void DkControlWidget::setFileInfo(QSharedPointer<DkImageContainerT> imgC) {

	if (!imgC)
		return;

	this->imgC = imgC;

	QSharedPointer<DkMetaDataT> metaData = imgC->getMetaData();

	QString dateString = metaData->getExifValue("DateTimeOriginal");
	fileInfoLabel->updateInfo(imgC->filePath(), "", dateString, metaData->getRating());
	fileInfoLabel->setEdited(imgC->isEdited());
	commentWidget->setMetaData(metaData);
	updateRating(metaData->getRating());
}

void DkControlWidget::setInfo(QString msg, int time, int location) {

	if (location == center_label && centerLabel)
		centerLabel->setText(msg, time);
	else if (location == bottom_left_label && bottomLabel)
		bottomLabel->setText(msg, time);
	else if (location == top_left_label && bottomLeftLabel)
		bottomLeftLabel->setText(msg, time);

	update();
}

void DkControlWidget::setInfoDelayed(QString msg, bool start, int delayTime) {

	if (!centerLabel)
		return;

	if (start)
		delayedInfo->setInfo(msg, delayTime);
	else
		delayedInfo->stop();

}

void DkControlWidget::setSpinner(int time) {

	if (spinnerLabel)
		spinnerLabel->showTimed(time);
}

void DkControlWidget::setSpinnerDelayed(bool start, int time) {

	if (!spinnerLabel) 
		return;

	if (start)
		delayedSpinner->setInfo(time);
	else
		delayedSpinner->stop();
}

void DkControlWidget::changeMetaDataPosition(int pos) {

	if (pos == DkWidget::pos_west) {
		hudLayout->addWidget(metaDataInfo, top_metadata, left_metadata, bottom_metadata-top_metadata, 1);	
	}
	else if (pos == DkWidget::pos_east) {
		hudLayout->addWidget(metaDataInfo, top_metadata, right_metadata, bottom_metadata-top_metadata, 1);	
	}
	else if (pos == DkWidget::pos_north) {
		hudLayout->addWidget(metaDataInfo, top_metadata, left_metadata, 1, hor_pos_end-2);	
	}
	else if (pos == DkWidget::pos_south) {
		hudLayout->addWidget(metaDataInfo, bottom_metadata, left_metadata, 1, hor_pos_end-2);	
	}
}

void DkControlWidget::changeThumbNailPosition(int pos) {

	if (pos == DkWidget::pos_west) {
		hudLayout->addWidget(filePreview, top_thumbs, left_thumbs, ver_pos_end, 1);	
	}
	else if (pos == DkWidget::pos_east) {
		hudLayout->addWidget(filePreview, top_thumbs, right_thumbs, ver_pos_end, 1);	
	}
	else if (pos == DkWidget::pos_north) {
		hudLayout->addWidget(filePreview, top_thumbs, left_thumbs, 1, hor_pos_end);	
	}
	else if (pos == DkWidget::pos_south) {
		hudLayout->addWidget(filePreview, bottom_thumbs, left_thumbs, 1, hor_pos_end);	
	}
	else 
		filePreview->hide();

}

void DkControlWidget::stopLabels() {

	centerLabel->stop();
	bottomLabel->stop();
	//topLeftLabel->stop();
	spinnerLabel->stop();

	//showCrop(false);
	//showThumbView(false);
	//showRecentFiles(false);
	switchWidget();
}

void DkControlWidget::settingsChanged() {

	if (fileInfoLabel && fileInfoLabel->isVisible()) {
		showFileInfo(false);	// just a hack but all states are preserved this way
		showFileInfo(true);
	}

}

void DkControlWidget::updateRating(int rating) {

	if (!imgC)
		return;

	this->rating = rating;

	ratingLabel->setRating(rating);

	if (fileInfoLabel)
		fileInfoLabel->updateRating(rating);

	QSharedPointer<DkMetaDataT> metaDataInfo = imgC->getMetaData();
	metaDataInfo->setRating(rating);
}

void DkControlWidget::imageLoaded(bool) {

	showWidgetsSettings();
}

void DkControlWidget::setFullScreen(bool fullscreen) {

	showWidgetsSettings();

	if (fullscreen &&!player->isVisible())
		player->show(3000);		
}

// DkControlWidget - Events --------------------------------------------------------------------
void DkControlWidget::mousePressEvent(QMouseEvent *event) {

	qDebug() << "has mouse tracking: " << hasMouseTracking();

	enterPos = event->pos();

	if (filePreview && filePreview->isVisible() && event->buttons() == Qt::MiddleButton) {

		QTimer* mImgTimer = filePreview->getMoveImageTimer();
		mImgTimer->start(1);

		// show icon
		wheelButton->move(event->pos().x()-16, event->pos().y()-16);
		wheelButton->show();
	}

	QWidget::mousePressEvent(event);
}

void DkControlWidget::mouseReleaseEvent(QMouseEvent *event) {

	if (filePreview && filePreview->isVisible()) {
		filePreview->setCurrentDx(0);
		QTimer* mImgTimer = filePreview->getMoveImageTimer();
		mImgTimer->stop();
		wheelButton->hide();
	}

	QWidget::mouseReleaseEvent(event);
}

void DkControlWidget::mouseMoveEvent(QMouseEvent *event) {

	// scroll thumbs preview
	if (filePreview && filePreview->isVisible() && event->buttons() == Qt::MiddleButton) {
		
		float dx = (float)std::fabs(enterPos.x() - event->pos().x())*0.015f;
		dx = std::exp(dx);
		if (enterPos.x() - event->pos().x() < 0)
			dx = -dx;

		filePreview->setCurrentDx(dx);	// update dx
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
