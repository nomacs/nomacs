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

#include "DkViewPort.h"

#include "DkNoMacs.h"	// currently needed for some enums
#include "DkThumbsWidgets.h"
#include "DkMetaDataWidgets.h"
#include "DkNetwork.h"

namespace nmc {

// DkControlWidget --------------------------------------------------------------------
DkControlWidget::DkControlWidget(DkViewPort *parent, Qt::WindowFlags flags) : QWidget(parent, flags) {

	viewport = parent;
	setObjectName("DkControlWidget");
	qDebug() << this->metaObject()->className();

	// cropping
	cropWidget = new DkCropWidget(QRectF(), this);
	recentFilesWidget = new DkRecentFilesWidget(this);

	//// thumbnails, metadata
	//thumbPool = new DkThumbPool(QFileInfo(), this);
	filePreview = new DkFilePreview(this, flags);
	folderScroll = new DkFolderScrollBar(this);
	metaDataInfo = new DkMetaDataInfo(this);
	overviewWindow = new DkOverview(this);
	player = new DkPlayer(this);
	addActions(player->getActions().toList());

	// file info - overview
	fileInfoLabel = new DkFileInfoLabel(this);
	ratingLabel = new DkRatingLabelBg(2, this, flags);
	addActions(ratingLabel->getActions().toList());		// register actions
	commentWidget = new DkCommentWidget(this);

	// delayed info
	delayedInfo = new DkDelayedMessage();
	delayedSpinner = new DkDelayedInfo(0);

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

	// add mousetracking to all widgets which allows the viewport for status updates
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
	folderScroll->setDisplaySettings(&DkSettings::app.showScroller);
	metaDataInfo->setDisplaySettings(&DkSettings::app.showMetaData);
	fileInfoLabel->setDisplaySettings(&DkSettings::app.showFileInfoLabel);
	player->setDisplaySettings(&DkSettings::app.showPlayer);
	histogram->setDisplaySettings(&DkSettings::app.showHistogram);
	commentWidget->setDisplaySettings(&DkSettings::app.showComment);

	// some adjustments
	bottomLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	bottomLeftLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	ratingLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	centerLabel->setAlignment(Qt::AlignCenter);
	overviewWindow->setContentsMargins(10, 10, 0, 0);
	//cropWidget->setMaximumSize(16777215, 16777215);		// max widget size, why is it a 24 bit int??
	cropWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	recentFilesWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	//thumbScrollWidget->setMaximumSize(16777215, 16777215);		// max widget size, why is it a 24 bit int??
	spinnerLabel->halfSize();
	commentWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	// dummy - needed for three equal columns @markus: do not delete!
	QWidget* dw = new QWidget();
	dw->setMouseTracking(true);
	QBoxLayout* dLayout = new QBoxLayout(QBoxLayout::LeftToRight, dw);
	dLayout->setContentsMargins(0,0,0,0);
	dLayout->addWidget(bottomLabel);
	dLayout->addStretch();

	// zoom widget
	QWidget* bw = new QWidget();
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
	QWidget* leftWidget = new QWidget();
	leftWidget->setMouseTracking(true);
	QBoxLayout* ulLayout = new QBoxLayout(QBoxLayout::TopToBottom, leftWidget);
	ulLayout->setContentsMargins(0,0,0,0);
	ulLayout->setSpacing(0);
	ulLayout->addWidget(overviewWindow);
	ulLayout->addStretch();
	ulLayout->addWidget(bw);
	ulLayout->addWidget(dw);
	//ulLayout->addWidget(cw);

	// center column
	QWidget* cW = new QWidget();
	cW->setMouseTracking(true);
	QBoxLayout* cwLayout = new QBoxLayout(QBoxLayout::LeftToRight, cW);
	cwLayout->setContentsMargins(0,0,0,0);
	cwLayout->addStretch();
	cwLayout->addWidget(centerLabel);
	cwLayout->addWidget(spinnerLabel);
	cwLayout->addStretch();

	// center player horizontally
	QWidget* cP = new QWidget();
	cP->setMouseTracking(true);
	QBoxLayout* cpLayout = new QBoxLayout(QBoxLayout::LeftToRight, cP);
	cpLayout->setContentsMargins(0,0,0,0);
	cpLayout->addWidget(player);

	// center column
	QWidget* center = new QWidget();
	center->setMouseTracking(true);
	QBoxLayout* cLayout = new QBoxLayout(QBoxLayout::TopToBottom, center);
	cLayout->setContentsMargins(0,0,0,0);
	cLayout->addStretch();
	cLayout->addWidget(cW);
	cLayout->addStretch();
	cLayout->addWidget(cP);
	
	// rating widget
	QWidget* rw = new QWidget();
	rw->setMouseTracking(true);
	rw->setMinimumSize(0,0);
	QBoxLayout* rLayout = new QBoxLayout(QBoxLayout::RightToLeft, rw);
	rLayout->setContentsMargins(0,0,0,17);
	rLayout->addWidget(ratingLabel);
	rLayout->addStretch();

	// file info
	QWidget* fw = new QWidget();
	fw->setContentsMargins(0,0,0,30);
	fw->setMouseTracking(true);
	fw->setMinimumSize(0,0);
	QBoxLayout* rwLayout = new QBoxLayout(QBoxLayout::RightToLeft, fw);
	rwLayout->setContentsMargins(0,0,0,0);
	rwLayout->addWidget(fileInfoLabel);
	rwLayout->addStretch();

	// right column
	QWidget* hw = new QWidget();
	hw->setContentsMargins(0,10,10,0);
	hw->setMouseTracking(true);
	QBoxLayout* hwLayout = new QBoxLayout(QBoxLayout::RightToLeft, hw);
	hwLayout->setContentsMargins(0,0,0,0);
	hwLayout->addWidget(histogram);
	hwLayout->addStretch();

	// right column
	QWidget* rightWidget = new QWidget();
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
	widgets[recent_files_widget] = recentFilesWidget;
	lastActiveWidget = widgets[hud_widget];

	// global controller layout
	hudLayout = new QGridLayout(widgets[hud_widget]);
	hudLayout->setContentsMargins(0,0,0,0);
	hudLayout->setSpacing(0);

	//hudLayout->addWidget(thumbWidget, 0, 0);

	// add elements
	changeThumbNailPosition(filePreview->getWindowPosition());
	//hudLayout->addWidget(filePreview, top_thumbs, left_thumbs, 1, hor_pos_end);
	hudLayout->addWidget(folderScroll, top_scroll, left_thumbs, 1, hor_pos_end);
	hudLayout->addWidget(metaDataInfo, bottom, left, 1, hor_pos_end);
	hudLayout->addWidget(leftWidget, ver_center, left, 1, 1);
	hudLayout->addWidget(center, ver_center, hor_center, 1, 1);
	hudLayout->addWidget(rightWidget, ver_center, right, 1, 1);
		
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

	DkImageLoader* loader = viewport->getImageLoader();

	if (loader) {
		qDebug() << "loader slots connected";

		qRegisterMetaType<QVector<QSharedPointer<DkImageContainerT> > >( "QVector<QSharedPointer<DkImageContainerT> >");

		connect(loader, SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), filePreview, SLOT(updateThumbs(QVector<QSharedPointer<DkImageContainerT> >)));
		connect(loader, SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), filePreview, SLOT(setFileInfo(QSharedPointer<DkImageContainerT>)));
		connect(loader, SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), metaDataInfo, SLOT(setImageInfo(QSharedPointer<DkImageContainerT>)));
		connect(loader, SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), this, SLOT(setFileInfo(QSharedPointer<DkImageContainerT>)));

		connect(loader, SIGNAL(showInfoSignal(QString, int, int)), this, SLOT(setInfo(QString, int, int)));
		connect(loader, SIGNAL(updateInfoSignalDelayed(QString, bool, int)), this, SLOT(setInfoDelayed(QString, bool, int)));
		connect(loader, SIGNAL(updateSpinnerSignalDelayed(bool, int)), this, SLOT(setSpinnerDelayed(bool, int)));

		connect(loader, SIGNAL(setPlayer(bool)), player, SLOT(play(bool)));

		connect(loader, SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), folderScroll, SLOT(updateDir(QVector<QSharedPointer<DkImageContainerT> >)));
		connect(loader, SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), folderScroll, SLOT(updateFile(QSharedPointer<DkImageContainerT>)));

	}

	// thumbs widget
	connect(filePreview, SIGNAL(loadFileSignal(QFileInfo)), viewport, SLOT(loadFile(QFileInfo)));
	connect(filePreview, SIGNAL(changeFileSignal(int)), viewport, SLOT(loadFileFast(int)));
	connect(filePreview, SIGNAL(positionChangeSignal(int)), this, SLOT(changeThumbNailPosition(int)));

	// recent files widget
	connect(recentFilesWidget, SIGNAL(loadFileSignal(QFileInfo)), viewport, SLOT(loadFile(QFileInfo)));

	// file scroller
	connect(folderScroll, SIGNAL(changeFileSignal(int)), viewport, SLOT(loadFileFast(int)));
	
	// overview
	connect(overviewWindow, SIGNAL(moveViewSignal(QPointF)), viewport, SLOT(moveView(QPointF)));
	connect(overviewWindow, SIGNAL(sendTransformSignal()), viewport, SLOT(tcpSynchronize()));

	// waiting
	connect(delayedInfo, SIGNAL(infoSignal(QString, int)), this, SLOT(setInfo(QString, int)));
	connect(delayedSpinner, SIGNAL(infoSignal(int)), this, SLOT(setSpinner(int)));
	
	// rating
	connect(fileInfoLabel->getRatingLabel(), SIGNAL(newRatingSignal(int)), this, SLOT(updateRating(int)));
	connect(ratingLabel, SIGNAL(newRatingSignal(int)), this, SLOT(updateRating(int)));
	connect(ratingLabel, SIGNAL(newRatingSignal(int)), metaDataInfo, SLOT(setRating(int)));

	// playing
	connect(player, SIGNAL(previousSignal()), viewport, SLOT(loadPrevFileFast()));
	connect(player, SIGNAL(nextSignal()), viewport, SLOT(loadNextFileFast()));

	// cropping
	connect(cropWidget, SIGNAL(enterPressedSignal(DkRotatingRect, const QColor&)), viewport, SLOT(cropImage(DkRotatingRect, const QColor&)));
	connect(cropWidget, SIGNAL(cancelSignal()), this, SLOT(hideCrop()));

	// comment widget
	connect(commentWidget, SIGNAL(showInfoSignal(QString)), this, SLOT(setInfo(QString)));

	// viewport
	connect(viewport, SIGNAL(infoSignal(QString)), this, SLOT(setInfo(QString)));
}

void DkControlWidget::update() {

	overviewWindow->update();

	QWidget::update();
}

void DkControlWidget::showWidgetsSettings() {

	if (viewport->getImage().isNull()) {
		showPreview(false);
		showScroller(false);
		showMetaData(false);
		showFileInfo(false);
		showPlayer(false);
		overviewWindow->hide();
		showHistogram(false);
		showCommentWidget(false);
		return;
	}

	//qDebug() << "current app mode: " << DkSettings::app.currentAppMode;

	showPreview(filePreview->getCurrentDisplaySetting());
	showScroller(folderScroll->getCurrentDisplaySetting());
	showMetaData(metaDataInfo->getCurrentDisplaySetting());
	showFileInfo(fileInfoLabel->getCurrentDisplaySetting());
	showPlayer(player->getCurrentDisplaySetting());
	showHistogram(histogram->getCurrentDisplaySetting());
	showCommentWidget(commentWidget->getCurrentDisplaySetting());
}

void DkControlWidget::showPreview(bool visible) {

	if (!filePreview)
		return;

	if (visible && !filePreview->isVisible())
		filePreview->show();
	else if (!visible && filePreview->isVisible())
		filePreview->hide();
}

void DkControlWidget::showScroller(bool visible) {

	if (!folderScroll)
		return;

	if (visible && !folderScroll->isVisible())
		folderScroll->show();
	else if (!visible && folderScroll->isVisible())
		folderScroll->hide();
}

void DkControlWidget::showMetaData(bool visible) {

	if (!metaDataInfo)
		return;

	if (visible && !metaDataInfo->isVisible()) {
		metaDataInfo->show();
		qDebug() << "showing metadata...";
	}
	else if (!visible && metaDataInfo->isVisible())
		metaDataInfo->hide();
}

void DkControlWidget::showFileInfo(bool visible) {

	if (!fileInfoLabel)
		return;

	if (visible && !fileInfoLabel->isVisible()) {
		fileInfoLabel->show();
		ratingLabel->block(fileInfoLabel->isVisible());
	}
	else if (!visible && fileInfoLabel->isVisible()) {
		fileInfoLabel->hide();
		ratingLabel->block(false);
	}
}

void DkControlWidget::showPlayer(bool visible) {

	if (!player)
		return;

	if (visible)
		player->show();
	else
		player->hide();
}

void DkControlWidget::showOverview(bool visible) {

	if (!overviewWindow)
		return;

	// viewport decides whether to show overview or not
	DkSettings::app.showOverview.setBit(DkSettings::app.currentAppMode, visible);

	if (visible && !overviewWindow->isVisible()) {		
		viewport->update();
	}
	else if (!visible && overviewWindow->isVisible()) {
		overviewWindow->hide();
	}

}

void DkControlWidget::showRecentFiles(bool visible) {

	if (visible) {
		recentFilesWidget->setCustomStyle(!viewport->getImage().isNull());
		switchWidget(widgets[recent_files_widget]);
	}
	else {
		switchWidget();
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
		histogram->hide();
	}
}

void DkControlWidget::showCommentWidget(bool visible) {

	if (!commentWidget)
		return;

	if (visible && !commentWidget->isVisible()) {
		commentWidget->show();
	}
	else if (!visible && commentWidget->isVisible()) {
		commentWidget->hide();
	}
}

void DkControlWidget::switchWidget(QWidget* widget) {

	if (layout->currentWidget() == widget)
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
	fileInfoLabel->updateInfo(imgC->file(), "", dateString, metaData->getRating());
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

void DkControlWidget::changeThumbNailPosition(int pos) {

	if (pos == DkFilePreview::cm_pos_west) {
		hudLayout->addWidget(filePreview, top_thumbs, left_thumbs, ver_pos_end, 1);	
	}
	else if (pos == DkFilePreview::cm_pos_east) {
		hudLayout->addWidget(filePreview, top_thumbs, right_thumbs, ver_pos_end, 1);	
	}
	else if (pos == DkFilePreview::cm_pos_north) {
		hudLayout->addWidget(filePreview, top_thumbs, left_thumbs, 1, hor_pos_end);	
	}
	else if (pos == DkFilePreview::cm_pos_south) {
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

void DkControlWidget::imageLoaded(bool loaded) {

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
		
		float dx = std::fabs(enterPos.x() - event->pos().x())*0.015f;
		dx = std::exp(dx);
		if (enterPos.x() - event->pos().x() < 0)
			dx = -dx;

		filePreview->setCurrentDx(dx);	// update dx
	}

	QWidget::mouseMoveEvent(event);
}



void DkControlWidget::keyPressEvent(QKeyEvent *event) {
	
	// conflicting with ESC in fullscreen
	//int mode = DkSettings::AppSettings::currentAppMode;
	//if (event->key() == Qt::Key_Escape && 
	//	(mode == DkSettings::mode_default || mode == DkSettings::mode_frameless || mode == DkSettings::mode_contrast)) {
	//	if (filePreview->isVisible()) {
	//		filePreview->hide();
	//	}
	//	if (metaDataInfo->isVisible()) {
	//		metaDataInfo->hide();
	//	}
	//}

	QWidget::keyPressEvent(event);
}

void DkControlWidget::keyReleaseEvent(QKeyEvent *event) {

	QWidget::keyReleaseEvent(event);
}


// DkViewPort --------------------------------------------------------------------
DkViewPort::DkViewPort(QWidget *parent, Qt::WindowFlags flags) : DkBaseViewPort(parent) {

	qRegisterMetaType<QSharedPointer<DkImageContainerT> >( "QSharedPointer<DkImageContainerT>");
	qRegisterMetaType<QSharedPointer<DkImageContainerT> >( "QSharedPointer<nmc::DkImageContainerT>");

	testLoaded = false;
	thumbLoaded = false;
	visibleStatusbar = false;
	//pluginImageWasApplied = false;
	fadeOpacity = 0.0f;
	fileDownloader = 0;

	imgBg.load(":/nomacs/img/nomacs-bg.png");

	loader = 0;
	
	skipImageTimer = new QTimer(this);
	skipImageTimer->setSingleShot(true);
	connect(skipImageTimer, SIGNAL(timeout()), this, SLOT(loadFullFile()));

	repeatZoomTimer = new QTimer(this);
	repeatZoomTimer->setInterval(20);
	connect(repeatZoomTimer, SIGNAL(timeout()), this, SLOT(repeatZoom()));

	fadeTimer = new QTimer(this);
	fadeTimer->setInterval(5);
	connect(fadeTimer, SIGNAL(timeout()), this, SLOT(animateFade()));

	moveTimer = new QTimer(this);
	moveTimer->setInterval(5);
	connect(moveTimer, SIGNAL(timeout()), this, SLOT(animateMove()));

	setAcceptDrops(true);

	//no border
	setMouseTracking (true);//receive mouse event everytime
	
	loader = new DkImageLoader();

	paintLayout = new QVBoxLayout(this);
	paintLayout->setContentsMargins(0,0,0,0);

	controller = new DkControlWidget(this, flags);
	//controller->show();

	controller->getOverview()->setTransforms(&worldMatrix, &imgMatrix);
	controller->getCropWidget()->setWorldTransform(&worldMatrix);
	controller->getCropWidget()->setImageTransform(&imgMatrix);
	controller->getCropWidget()->setImageRect(&imgViewRect);

	connect(loader, SIGNAL(imageLoadedSignal(QSharedPointer<DkImageContainerT>, bool)), this, SLOT(updateImage(QSharedPointer<DkImageContainerT>, bool)), Qt::QueuedConnection);
	connect(loader, SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), this, SLOT(updateImage(QSharedPointer<DkImageContainerT>)), Qt::QueuedConnection);
	//connect(loader, SIGNAL(updateImageSignal()), this, SLOT(updateImage()), Qt::QueuedConnection);
	//connect(loader, SIGNAL(fileNotLoadedSignal(QFileInfo)), this, SLOT(fileNotLoaded(QFileInfo)));
	connect(this, SIGNAL(enableNoImageSignal(bool)), controller, SLOT(imageLoaded(bool)));
	connect(&imgStorage, SIGNAL(infoSignal(QString)), this, SIGNAL(infoSignal(QString)));
	
	createShortcuts();

	qDebug() << "viewer created...";

	//// >DIR:  [7.4.2011 diem]
	//trayIcon = new QSystemTrayIcon(this->windowIcon(), this);

	// TODO:
	// one could blur the canvas if a transparent GUI is present
	// what we would need: QGraphicsBlurEffect...
	// render all widgets to the alpha channel (bw)
	// pre-render the viewport to that image... apply blur
	// and then render the blurred image after the widget is rendered...
	// performance?!
	
}

DkViewPort::~DkViewPort() {

	release();
}

void DkViewPort::release() {

	if (loader) delete loader;
	loader = 0;
}

void DkViewPort::createShortcuts() {

	//DkBaseViewPort::createShortcuts();

	shortcuts.resize(scf_end);

	// files
	shortcuts[sc_first_file] = new QShortcut(shortcut_first_file, this);
	connect(shortcuts[sc_first_file], SIGNAL(activated()), this, SLOT(loadFirst()));
	shortcuts[sc_last_file] = new QShortcut(shortcut_last_file, this);
	connect(shortcuts[sc_last_file], SIGNAL(activated()), this, SLOT(loadLast()));

	shortcuts[sc_skip_prev] = new QShortcut(shortcut_skip_prev, this);
	shortcuts[sc_skip_prev]->setContext(Qt::WidgetWithChildrenShortcut);
	connect(shortcuts[sc_skip_prev], SIGNAL(activated()), this, SLOT(loadSkipPrev10()));
	shortcuts[sc_skip_next] = new QShortcut(shortcut_skip_next, this);
	shortcuts[sc_skip_next]->setContext(Qt::WidgetWithChildrenShortcut);
	connect(shortcuts[sc_skip_next], SIGNAL(activated()), this, SLOT(loadSkipNext10()));
	
	shortcuts[sc_first_sync] = new QShortcut(shortcut_first_file_sync, this);
	connect(shortcuts[sc_first_sync], SIGNAL(activated()), this, SLOT(loadFirst()));

	shortcuts[sc_last_sync] = new QShortcut(shortcut_last_file_sync, this);
	connect(shortcuts[sc_last_sync], SIGNAL(activated()), this, SLOT(loadLast()));

	shortcuts[sc_next_sync] = new QShortcut(shortcut_next_file_sync, this);
	connect(shortcuts[sc_next_sync], SIGNAL(activated()), this, SLOT(loadNextFileFast()));

	shortcuts[sc_prev_sync] = new QShortcut(shortcut_prev_file_sync, this);
	connect(shortcuts[sc_prev_sync], SIGNAL(activated()), this, SLOT(loadPrevFileFast()));

	shortcuts[sc_delete_silent] = new QShortcut(shortcut_delete_silent, this);
	connect(shortcuts[sc_delete_silent], SIGNAL(activated()), loader, SLOT(deleteFile()));

	for (int idx = 0; idx < shortcuts.size(); idx++) {
		// assign widget shortcuts to all of them
		shortcuts[idx]->setContext(Qt::WidgetWithChildrenShortcut);
	}


}

void DkViewPort::setPaintWidget(QWidget* widget, bool removeWidget) {

	if(!removeWidget) {
		paintLayout->addWidget(widget);
		//pluginImageWasApplied = false;
	} else {
		paintLayout->removeWidget(widget);
		//widget->deleteLater();
	}
	
	//controller->raise();
	
}

#ifdef WITH_OPENCV
void DkViewPort::setImage(cv::Mat newImg) {

	QImage imgQt = DkImage::mat2QImage(newImg);
	setImage(imgQt);
}
#endif

void DkViewPort::updateImage(QSharedPointer<DkImageContainerT> image, bool loaded) {

	// things todo if a file was not loaded...
	if (!loaded) {
		controller->getPlayer()->startTimer();
		return;
	}

	// should not happen -> the loader should send this signal
	if (!loader)
		return;

	if (loader->hasImage())
		setImage(loader->getImage());
}

void DkViewPort::loadImage(QImage newImg) {

	// delete current information
	if (loader) {
		if (!unloadImage(true))
			return;	// user canceled

		loader->setImage(newImg);
		setImage(newImg);

		// save to temp folder
		loader->saveTempFile(newImg);
	}
}

void DkViewPort::loadImage(QSharedPointer<DkImageContainerT> img) {

	if (loader) {

		if (!unloadImage(true))
			return;

		if (img->hasImage()) {
			loader->setCurrentImage(img);
			setImage(img->image());
		}
		loader->load(img);
	}

}

void DkViewPort::setImage(QImage newImg) {

	DkTimer dt;

	emit movieLoadedSignal(false);

	if (!thumbLoaded) { 
		oldImgViewRect = imgViewRect;
		oldWorldMatrix = worldMatrix;
		oldImgMatrix = imgMatrix;
	}

	//imgPyramid.clear();

	controller->getOverview()->setImage(QImage());	// clear overview

	imgStorage.setImage(newImg);
	this->imgRect = QRectF(0, 0, newImg.width(), newImg.height());

	if (loader->hasMovie() && !loader->isEdited())
		loadMovie();

	emit enableNoImageSignal(!newImg.isNull());

	//qDebug() << "new image (viewport) loaded,  size: " << newImg.size() << "channel: " << imgQt.format();
	//qDebug() << "keep zoom is always: " << (DkSettings::display.keepZoom == DkSettings::zoom_always_keep);

	if (!DkSettings::slideShow.moveSpeed && (DkSettings::display.keepZoom == DkSettings::zoom_never_keep || 
		(DkSettings::display.keepZoom == DkSettings::zoom_keep_same_size && oldImgRect != imgRect)) ||
		 oldImgRect.isEmpty())
		worldMatrix.reset();
	else {
		imgViewRect = oldImgViewRect;
		imgMatrix = oldImgMatrix;
		worldMatrix = oldWorldMatrix;
	}

	updateImageMatrix();		

	// if image is not inside, we'll align it at the top left border
	if (!viewportRect.intersects(worldMatrix.mapRect(imgViewRect))) {
		worldMatrix.translate(-worldMatrix.dx(), -worldMatrix.dy());
		centerImage();
	}

	controller->getPlayer()->startTimer();
	controller->getOverview()->setImage(newImg);	// TODO: maybe we could make use of the image pyramid here
	controller->stopLabels();

	thumbLoaded = false;
	thumbFile = QFileInfo();
	oldImgRect = imgRect;
	
	// init fading
	if (DkSettings::display.fadeSec && (controller->getPlayer()->isPlaying() || parent && parent->isFullScreen())) {
		fadeTimer->start();
		fadeTime.start();
	}
	else
		fadeOpacity = 0.0f;

	// init moving
	if (DkSettings::slideShow.moveSpeed /*&& controller->getPlayer()->isPlaying()*/ 
		&& newImg.width() > width() && newImg.height() > height()) {
		targetScale = 1.0f/imgMatrix.m11();
		// TODO: if too large - do a threshold
	}

	update();

	// draw a histogram from the image -> does nothing if the histogram is invisible
	if (controller->getHistogram()) controller->getHistogram()->drawHistogram(newImg);
	if (DkSettings::sync.syncMode == DkSettings::sync_mode_remote_display)
		tcpSendImage(true);

	emit newImageSignal(&newImg);

}

void DkViewPort::setThumbImage(QImage newImg) {
	
	if (!thumbLoaded) { 
		qDebug() << "saving image matrix...";
		oldImgViewRect = imgViewRect;
		oldWorldMatrix = worldMatrix;
		oldImgMatrix = imgMatrix;
	}

	DkTimer dt;
	//imgPyramid.clear();

	imgStorage.setImage(newImg);
	QRectF oldImgRect = imgRect;
	this->imgRect = QRectF(0, 0, newImg.width(), newImg.height());

	emit enableNoImageSignal(true);

	if (!DkSettings::display.keepZoom || imgRect != oldImgRect)
		worldMatrix.reset();							

	updateImageMatrix();
	
	controller->getOverview()->setImage(newImg);
	controller->stopLabels();

	//// TODO: this is a fast fix
	//// if this thread uses the static metadata object 
	//// nomacs crashes when images are loaded fast (2 threads try to access DkMetaData simultaneously)
	//// currently we need to read the metadata twice (not nice either)
	//DkImageLoader::imgMetaData.setFileName(loader->getFile());

	//controller->updateRating(DkImageLoader::imgMetaData.getRating());

	//if (controller->getFileInfoLabel()->isVisible()) {
	//	QString dateString = QString::fromStdString(DkImageLoader::imgMetaData.getExifValue("DateTimeOriginal"));
	//	controller->getFileInfoLabel()->updateInfo(loader->getFile(), dateString, DkImageLoader::imgMetaData.getRating());
	//}

	thumbLoaded = true;

	update();

	qDebug() << "setting the image took me: " << dt.getTotal();
}

void DkViewPort::tcpSendImage(bool silent) {

	if (!silent)
		controller->setInfo("sending image...", 3000, DkControlWidget::center_label);

	if (loader)
		emit sendImageSignal(imgStorage.getImage(), loader->fileName());
	else
		emit sendImageSignal(imgStorage.getImage(), "nomacs - Image Lounge");
}

void DkViewPort::zoom(float factor, QPointF center) {

	if (imgStorage.getImage().isNull() || blockZooming)
		return;

	//factor/=5;//-0.1 <-> 0.1
	//factor+=1;//0.9 <-> 1.1

	//limit zoom out ---
	if (worldMatrix.m11()*factor < minZoom && factor < 1)
		return;

	// reset view & block if we pass the 'image fit to screen' on zoom out
	if (worldMatrix.m11() > 1 && worldMatrix.m11()*factor < 1) {

		blockZooming = true;
		zoomTimer->start(500);
		resetView();
		return;
	}

	// reset view if we pass the 'image fit to screen' on zoom in
	if (worldMatrix.m11() < 1 && worldMatrix.m11()*factor > 1) {
		
		resetView();
		return;
	}

	//limit zoom in ---
	if (worldMatrix.m11()*imgMatrix.m11() > maxZoom && factor > 1)
		return;

	bool blackBorder = false;

	// if no center assigned: zoom in at the image center
	if (center.x() == -1 || center.y() == -1)
		center = imgViewRect.center();
	else {

		// if black border - do not zoom to the mouse coordinate
		if ((float)imgViewRect.width()*(worldMatrix.m11()*factor) < (float)width()) {
			center.setX(imgViewRect.center().x());
			blackBorder = true;
		}
		if (((float)imgViewRect.height()*worldMatrix.m11()*factor) < (float)height()) {
			center.setY(imgViewRect.center().y());
			blackBorder = true;
		}
	}

	//inverse the transform
	int a, b;
	worldMatrix.inverted().map(center.x(), center.y(), &a, &b);

	worldMatrix.translate(a-factor*a, b-factor*b);
	worldMatrix.scale(factor, factor);
	
	controlImagePosition();
	if (blackBorder && factor < 1) centerImage();	// TODO: geht auch schöner
	showZoom();
	changeCursor();

	controller->update();	// why do we need to update the controller manually?
	update();

	tcpSynchronize();
	
}

void DkViewPort::resetView() {

	worldMatrix.reset();
	showZoom();
	changeCursor();

	update();

	tcpSynchronize();
}

void DkViewPort::fullView() {

	worldMatrix.reset();
	zoom(1.0f/imgMatrix.m11());
	showZoom();
	changeCursor();
	update();
	if (this->visibleRegion().isEmpty()) qDebug() << "empty region...";
}

void DkViewPort::showZoom() {

	QString zoomStr;
	zoomStr.sprintf("%.1f%%", imgMatrix.m11()*worldMatrix.m11()*100);
	controller->setInfo(zoomStr, 3000, DkControlWidget::bottom_left_label);
}

void DkViewPort::repeatZoom() {

	qDebug() << "repeating...";
	if (DkSettings::display.invertZoom && QApplication::mouseButtons() == Qt::XButton1 ||
		!DkSettings::display.invertZoom && QApplication::mouseButtons() == Qt::XButton2)
		zoom(1.1f);
	else if (!DkSettings::display.invertZoom && QApplication::mouseButtons() == Qt::XButton1 ||
		DkSettings::display.invertZoom && QApplication::mouseButtons() == Qt::XButton2)
		zoom(0.9f);
	else
		repeatZoomTimer->stop();	// safety if we don't catch the release

}

void DkViewPort::toggleResetMatrix() {

	DkSettings::display.keepZoom = !DkSettings::display.keepZoom;
}

void DkViewPort::updateImageMatrix() {

	if (imgStorage.getImage().isNull())
		return;

	QRectF oldImgRect = imgViewRect;
	QTransform oldImgMatrix = imgMatrix;

	imgMatrix.reset();

	// if the image is smaller or zoom is active: paint the image as is
	if (!viewportRect.contains(imgRect.toRect()))
		imgMatrix = getScaledImageMatrix();
	else {
		imgMatrix.translate((float)(getMainGeometry().width()-imgStorage.getImage().width())*0.5f, (float)(getMainGeometry().height()-imgStorage.getImage().height())*0.5f);
		imgMatrix.scale(1.0f, 1.0f);
	}

	imgViewRect = imgMatrix.mapRect(imgRect);

	// update world matrix
	if (worldMatrix.m11() != 1) {

		float scaleFactor = oldImgMatrix.m11()/imgMatrix.m11();
		double dx = oldImgRect.x()/scaleFactor-imgViewRect.x();
		double dy = oldImgRect.y()/scaleFactor-imgViewRect.y();

		worldMatrix.scale(scaleFactor, scaleFactor);
		worldMatrix.translate(dx, dy);
	}
}

void DkViewPort::tcpSetTransforms(QTransform newWorldMatrix, QTransform newImgMatrix, QPointF canvasSize) {

	// ok relative transform
	if (canvasSize.isNull()) {
		moveView(QPointF(newWorldMatrix.dx(), newWorldMatrix.dy())/worldMatrix.m11());
	}
	else {
		worldMatrix = newWorldMatrix;
		imgMatrix = newImgMatrix;
		updateImageMatrix();

		QPointF imgPos = QPointF(canvasSize.x()*imgStorage.getImage().width(), canvasSize.y()*imgStorage.getImage().height());

		// go to screen coordinates
		imgPos = imgMatrix.map(imgPos);

		// go to world coordinates
		imgPos = worldMatrix.map(imgPos);

		// compute difference to current viewport center - in world coordinates
		imgPos = QPointF(width()*0.5f, height()*0.5f) - imgPos;

		// back to screen coordinates
		float s = worldMatrix.m11();
		worldMatrix.translate(imgPos.x()/s, imgPos.y()/s);
	}

	update();
}

void DkViewPort::tcpSetWindowRect(QRect rect) {
	this->setGeometry(rect);
}

void DkViewPort::tcpSynchronize(QTransform relativeMatrix) {
	
	if (!relativeMatrix.isIdentity())
		emit sendTransformSignal(relativeMatrix, QTransform(), QPointF());

	// check if we need a synchronization
	if ((qApp->keyboardModifiers() == altMod ||
		DkSettings::sync.syncMode != DkSettings::sync_mode_default || DkSettings::sync.syncActions) &&
		(hasFocus() || controller->hasFocus())) {
		QPointF size = QPointF(geometry().width()/2.0f, geometry().height()/2.0f);
		size = worldMatrix.inverted().map(size);
		size = imgMatrix.inverted().map(size);
		size = QPointF(size.x()/(float)imgStorage.getImage().width(), size.y()/(float)imgStorage.getImage().height());

		emit sendTransformSignal(worldMatrix, imgMatrix, size);
	}
}

void DkViewPort::tcpForceSynchronize() {

	int oldMode = DkSettings::sync.syncMode;
	DkSettings::sync.syncMode = DkSettings::sync_mode_remote_display;
	tcpSynchronize();
	DkSettings::sync.syncMode = oldMode;
}

void DkViewPort::tcpShowConnections(QList<DkPeer> peers) {

	QString newPeers;

	for (int idx = 0; idx < peers.size(); idx++) {
		
		DkPeer cp = peers.at(idx);

		if (cp.isSynchronized() && newPeers.isEmpty()) {
			newPeers = tr("connected with: ");
			emit newClientConnectedSignal(true, cp.isLocal());
		}
		else if (newPeers.isEmpty()) {
			newPeers = tr("disconnected with: ");
			emit newClientConnectedSignal(false, cp.isLocal());
		}

		qDebug() << "cp address..." << cp.hostAddress;

		newPeers.append("\n\t");

		if (!cp.clientName.isEmpty())
			newPeers.append(cp.clientName);
		if (!cp.clientName.isEmpty() && !cp.title.isEmpty())
			newPeers.append(": ");
		if (!cp.title.isEmpty())
			newPeers.append(cp.title);
	}

	controller->setInfo(newPeers);
	update();
}

void DkViewPort::paintEvent(QPaintEvent* event) {

	QPainter painter(viewport());

	if (imgStorage.hasImage()) {
		painter.setWorldTransform(worldMatrix);

		// don't interpolate if we are forced to, at 100% or we exceed the maximal interpolation level
		if (!forceFastRendering && // force?
			fabs(imgMatrix.m11()*worldMatrix.m11()-1.0f) > FLT_EPSILON && // @100% ?
			imgMatrix.m11()*worldMatrix.m11() <= (float)DkSettings::display.interpolateZoomLevel/100.0f) {	// > max zoom level
			painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
		}

		// TODO: if fading is active we interpolate with background instead of the other image
		draw(&painter, 1.0f-fadeOpacity);

		if (/*fadeTimer->isActive() && */!fadeBuffer.isNull()) {
			float oldOp = painter.opacity();
			painter.setOpacity(fadeOpacity);
			painter.drawImage(fadeImgViewRect, fadeBuffer, fadeBuffer.rect());
			painter.setOpacity(oldOp);
		}

		//Now disable matrixWorld for overlay display
		painter.setWorldMatrixEnabled(false);
	}
	else
		drawBackground(&painter);

	//in mode zoom/panning
	if (worldMatrix.m11() > 1 && !imageInside() && 
		DkSettings::app.showOverview.testBit(DkSettings::app.currentAppMode)) {

		if (!controller->getOverview()->isVisible())
			controller->getOverview()->show();
	}
	else if (controller->getOverview()->isVisible())
		controller->getOverview()->hide();

	painter.end();

	//qDebug() << "painting main widget...";

	// propagate
	QGraphicsView::paintEvent(event);

}

// drawing functions --------------------------------------------------------------------
void DkViewPort::drawBackground(QPainter *painter) {
	
	painter->setRenderHint(QPainter::SmoothPixmapTransform);

	// fit to viewport
	QSize s = imgBg.size();
	if (s.width() > (float)(size().width()*0.5))
		s = s*((size().width()*0.5)/s.width());

	if (s.height() > size().height()*0.6)
		s = s*((size().height()*0.6)/s.height());

	QRect bgRect(QPoint(size().width()-s.width()-size().width()*0.05, size().height()-s.height()-size().height()*0.05), s);

	painter->drawImage(bgRect, imgBg, QRect(QPoint(), imgBg.size()));
}

void DkViewPort::loadMovie() {

	if (!loader)
		return;

	if (movie) {
		movie->stop();
		delete movie;
		movie = 0;
	}

	movie = new QMovie(loader->file().absoluteFilePath());
	connect(movie, SIGNAL(frameChanged(int)), this, SLOT(update()));
	movie->start();
	emit movieLoadedSignal(true);
}

void DkViewPort::pauseMovie(bool pause) {

	if (!movie)
		return;

	movie->setPaused(pause);
}

void DkViewPort::nextMovieFrame() {

	if (!movie)
		return;

	movie->jumpToNextFrame();
	update();
}

void DkViewPort::previousMovieFrame() {

	if (!movie)
		return;

	
	int fn = movie->currentFrameNumber()-1;
	if (fn == -1)
		fn = movie->frameCount()-1;
	//qDebug() << "retrieving frame: " << fn;
	
	while(movie->currentFrameNumber() != fn)
		movie->jumpToNextFrame();

	//// the subsequent thing is not working if the movie is paused
	//bool success = movie->jumpToFrame(movie->currentFrameNumber()-1);
	update();
}

void DkViewPort::drawPolygon(QPainter *painter, QPolygon *polygon) {

	QPoint lastPoint;

	for (int idx = 0; idx < polygon->size(); idx++) {

		QPoint p = polygon->at(idx);

		if (!lastPoint.isNull())
			painter->drawLine(p, lastPoint);

		lastPoint = p;
	}

}

// event listeners --------------------------------------------------------------------
void DkViewPort::resizeEvent(QResizeEvent *event) {

	viewportRect = QRect(0, 0, width(), height());

	// >DIR: diem - bug if zoom factor is large and window becomes small
	updateImageMatrix();
	centerImage();
	changeCursor();

	controller->getOverview()->setViewPortRect(geometry());
	
	controller->resize(width(), height());
	qDebug() << "controller geometry: " << controller->geometry();

	return QGraphicsView::resizeEvent(event);
}

// mouse events --------------------------------------------------------------------
bool DkViewPort::event(QEvent *event) {

	// ok obviously QGraphicsView eats all mouse events -> so we simply redirect these to QWidget in order to get them delivered here
	if (event->type() == QEvent::MouseButtonPress || 
		event->type() == QEvent::MouseButtonDblClick || 
		event->type() == QEvent::MouseButtonRelease || 
		event->type() == QEvent::MouseMove || 
		event->type() == QEvent::Wheel || 
		event->type() == QEvent::KeyPress || 
		event->type() == QEvent::KeyRelease || 
		event->type() == QEvent::DragEnter ||
		event->type() == QEvent::Drop) {

		//qDebug() << "redirecting event...";
		// mouse events that double are now fixed, since the viewport is now overlayed by the controller
		return QWidget::event(event);
	}
	else
		return DkBaseViewPort::event(event);
	
}

void DkViewPort::dragLeaveEvent(QDragLeaveEvent *event) {

	qDebug() << "";
	event->accept();	
}

void DkViewPort::dragEnterEvent(QDragEnterEvent *event) {

	printf("drag enter event\n");

	//if (event->source() == this)
	//	return;

	if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);

		QList<QUrl> urls = event->mimeData()->urls();

		for (int idx = 0; idx < urls.size(); idx++)
			qDebug() << "url: " << urls.at(idx);

		url = url.toLocalFile();

		// TODO: check if we accept appropriately (network drives that are not mounted)
		QFileInfo file = QFileInfo(url.toString());

		// just accept image files
		if (DkImageLoader::isValid(file))
			event->acceptProposedAction();
		else if (file.isDir())
			event->acceptProposedAction();
		else if (event->mimeData()->urls().at(0).isValid() && DkImageLoader::hasValidSuffix(event->mimeData()->urls().at(0).toString()))
			event->acceptProposedAction();

	}
	if (event->mimeData()->hasImage()) {
		event->acceptProposedAction();
	}

	QGraphicsView::dragEnterEvent(event);
}

void DkViewPort::dropEvent(QDropEvent *event) {

	if (event->source() == this) {
		event->accept();
		return;
	}

	if (!loadFromMime(event->mimeData()))
		controller->setInfo(tr("Sorry, I could not drop the content."));
}

bool DkViewPort::loadFromMime(const QMimeData* mimeData) {

	if (!mimeData)
		return false;

	if (mimeData->hasUrls() && mimeData->urls().size() > 0 || mimeData->hasText()) {
		QUrl url = mimeData->hasText() ? QUrl::fromUserInput(mimeData->text()) : QUrl::fromUserInput(mimeData->urls().at(0).toString());
		qDebug() << "dropping: " << url;

		QFileInfo file = QFileInfo(url.toLocalFile());
		QList<QUrl> urls = mimeData->urls();

		// merge OpenCV vec files if multiple vec files are dropped
		if (urls.size() > 1 && file.suffix() == "vec") {

			QVector<QFileInfo> vecFiles;

			for (int idx = 0; idx < urls.size(); idx++)
				vecFiles.append(urls.at(idx).toLocalFile());

			// ask user for filename
			QFileInfo sInfo(QFileDialog::getSaveFileName(this, tr("Save File"),
				vecFiles.at(0).absolutePath(), "Cascade Training File (*.vec)"));

			DkBasicLoader loader;
			int numFiles = loader.mergeVecFiles(vecFiles, sInfo);

			if (numFiles) {
				loadFile(sInfo);
				controller->setInfo(tr("%1 vec files merged").arg(numFiles));
				return true;
			}

			return false;
		}
		else
			qDebug() << urls.size() << file.suffix() << " files dropped";

		// just accept image files
		if (DkImageLoader::isValid(file))
			loadFile(file);
		else if (url.isValid())
			downloadFile(url);
		else
			return false;

		for (int idx = 1; idx < urls.size() && idx < 20; idx++)
			emit addTabSignal(QFileInfo(urls[idx].toLocalFile()));
		return true;
	}
	else if (mimeData->hasImage()) {

		QImage dropImg = qvariant_cast<QImage>(mimeData->imageData());
		loadImage(dropImg);
		return true;
	}

	return false;
	qDebug() << "drop event...";
}

void DkViewPort::mousePressEvent(QMouseEvent *event) {

	// if zoom on wheel, the additional keys should be used for switching files
	if (DkSettings::global.zoomOnWheel) {
		if(event->buttons() == Qt::XButton1)
			loadPrevFileFast();
		else if(event->buttons() == Qt::XButton2)
			loadNextFileFast();
	} 
	else if(event->buttons() == Qt::XButton1 || event->buttons() == Qt::XButton2) {
		repeatZoom();
		repeatZoomTimer->start();
	}
	
	// ok, start panning
	if (worldMatrix.m11() > 1 && !imageInside() && event->buttons() == Qt::LeftButton) {
		setCursor(Qt::ClosedHandCursor);
		posGrab = event->pos();
	}
	
	// should be sent to QWidget?!
	DkBaseViewPort::mousePressEvent(event);
}

void DkViewPort::mouseReleaseEvent(QMouseEvent *event) {
	
	repeatZoomTimer->stop();

	if (imageInside()) {
		int sa = swipeRecognition(event->pos(), posGrab.toPoint());
		swipeAction(sa);
	}

	DkBaseViewPort::mouseReleaseEvent(event);
}

void DkViewPort::mouseMoveEvent(QMouseEvent *event) {

	//qDebug() << "mouse move (DkViewPort)";
	//changeCursor();
	currentPixelPos = event->pos();

	if (visibleStatusbar)
		getPixelInfo(event->pos());

	if (worldMatrix.m11() > 1 && event->buttons() == Qt::LeftButton) {

		QPointF cPos = event->pos();
		QPointF dxy = (cPos - posGrab);
		posGrab = cPos;
		moveView(dxy/worldMatrix.m11());

		// with shift also a hotkey for fast switching...
		if ((DkSettings::sync.syncAbsoluteTransform &&
			event->modifiers() == (altMod | Qt::ShiftModifier)) || 
			(!DkSettings::sync.syncAbsoluteTransform &&
			event->modifiers() == (altMod))) {
			
			if (dxy.x() != 0 || dxy.y() != 0) {
				QTransform relTransform;
				relTransform.translate(dxy.x(), dxy.y());
				tcpSynchronize(relTransform);
			}
		}
		tcpSynchronize();
	}

	int dist = QPoint(event->pos()-posGrab.toPoint()).manhattanLength();
	
	if (event->buttons() == Qt::LeftButton 
		&& dist > QApplication::startDragDistance()
		&& imageInside()
		&& !getImage().isNull()
		&& loader
		&& !QApplication::widgetAt(event->globalPos())) {	// is NULL if the mouse leaves the window

			qDebug() << loader->file().absoluteFilePath();

			// TODO: check if we do it correct (network locations that are not mounted)
			QUrl fileUrl = QUrl::fromLocalFile(loader->file().absoluteFilePath());

			// TODO: we cannot drag&drop files with # in them because # starts a new fragment
			QList<QUrl> urls;
			urls.append(fileUrl);

			// who deletes me?
			QMimeData* mimeData = new QMimeData;

			if (loader->file().exists() && !loader->isEdited())
				mimeData->setUrls(urls);
			else if (!getImage().isNull())
				mimeData->setImageData(getImage());

			QDrag* drag = new QDrag(this);
			drag->setMimeData(mimeData);
			Qt::DropAction dropAction = drag->exec(Qt::CopyAction);
			qDebug() << "creating drag: " << fileUrl;
	}

	// send to parent
	DkBaseViewPort::mouseMoveEvent(event);
}

void DkViewPort::wheelEvent(QWheelEvent *event) {

	qDebug() << "event orientation: " << event->orientation();
	qDebug() << "modifiers: " << event->modifiers();

	if (event->modifiers() & ctrlMod)
		qDebug() << "CTRL modifier";
	if (event->modifiers() & altMod)
		qDebug() << "ALT modifier";

	if ((!DkSettings::global.zoomOnWheel && event->modifiers() != ctrlMod) || 
		(DkSettings::global.zoomOnWheel && (event->modifiers() & ctrlMod || (event->orientation() == Qt::Horizontal && !(event->modifiers() & altMod))))) {

		if (event->delta() < 0)
			loadNextFileFast();
		else
			loadPrevFileFast();
	}
	else 
		DkBaseViewPort::wheelEvent(event);

	tcpSynchronize();

}

#if QT_VERSION < 0x050000
#ifndef QT_NO_GESTURES
int DkViewPort::swipeRecognition(QNativeGestureEvent* event) {
	

	if (posGrab.isNull()) {
		posGrab = event->position;
		return no_swipe;
	}

	return swipeRecognition(event->position, posGrab.toPoint());
}
#endif
#endif

int DkViewPort::swipeRecognition(QPoint start, QPoint end) {

	DkVector vec(start.x()-end.x(), start.y()-end.y());
	float length = vec.norm();

	if (fabs(vec.norm()) < 50) {
		qDebug() << "ignoring, too small: " << vec.norm();
		return no_swipe;
	}

	double angle = DkMath::normAngleRad(vec.angle(DkVector(0,1)), 0.0, CV_PI);
	bool horizontal = false;

	if (angle > CV_PI*0.3 && angle < CV_PI*0.6)
		horizontal = true;
	else if (angle < 0.2*CV_PI || angle > 0.8*CV_PI)
		horizontal = false;
	else
		return no_swipe;	// angles ~45° are not accepted

	QPoint startPos = QWidget::mapFromGlobal(end);

	qDebug() << "vec: " << vec.x << ", " << vec.y;

	if (horizontal) {

		if (vec.x < 0)
			return next_image;
		else
			return prev_image;

	}
	// upper part of the canvas is thumbs
	else if (!horizontal && startPos.y() < height()*0.5f) {

		// downward gesture is opening
		if (vec.y > 0)
			return open_thumbs;
		else
			return close_thumbs;
	}
	// lower part of the canvas is thumbs
	else if (!horizontal && startPos.y() > height()*0.5f) {

		// upward gesture is opening
		if (vec.y < 0)
			return open_metadata;
		else
			return close_metadata;
	}

	return no_swipe;
}

void DkViewPort::swipeAction(int swipeGesture) {

	switch (swipeGesture) {
	case next_image:
		loadNextFileFast();
		break;
	case prev_image:
		loadPrevFileFast();
		break;
	case open_thumbs:
		controller->showPreview(true);
		break;
	case close_thumbs:
		controller->showPreview(false);
		break;
	case open_metadata:
		controller->showMetaData(true);
		break;
	case close_metadata:
		controller->showMetaData(false);
		break;
	default:
		break;
	}

}

void DkViewPort::setFullScreen(bool fullScreen) {

	controller->setFullScreen(fullScreen);
	toggleLena();
}

void DkViewPort::getPixelInfo(const QPoint& pos) {

	if (imgStorage.getImage().isNull())
		return;

	QPointF imgPos = worldMatrix.inverted().map(QPointF(pos));
	imgPos = imgMatrix.inverted().map(imgPos);

	QPoint xy(qFloor(imgPos.x()), qFloor(imgPos.y()));

	if (xy.x() < 0 || xy.y() < 0 || xy.x() >= imgStorage.getImage().width() || xy.y() >= imgStorage.getImage().height())
		return;

	QColor col = imgStorage.getImage().pixel(xy);
	
	QString msg = "<font color=#555555>x: " % QString::number(xy.x()) % " y: " % QString::number(xy.y()) % "</font>"
		" | r: " % QString::number(col.red()) % " g: " % QString::number(col.green()) % " b: " % QString::number(col.blue());

	if (imgStorage.getImage().hasAlphaChannel())
		msg = msg % " a: " % QString::number(col.alpha());

	msg = msg % " | <font color=#555555>" % col.name().toUpper() % "</font>";

		emit statusInfoSignal(msg, status_pixel_info);

}

QString DkViewPort::getCurrentPixelHexValue() {

	if (imgStorage.getImage().isNull() || currentPixelPos.isNull())
		return QString();

	QPointF imgPos = worldMatrix.inverted().map(QPointF(currentPixelPos));
	imgPos = imgMatrix.inverted().map(imgPos);

	QPoint xy(qFloor(imgPos.x()), qFloor(imgPos.y()));

	if (xy.x() < 0 || xy.y() < 0 || xy.x() >= imgStorage.getImage().width() || xy.y() >= imgStorage.getImage().height())
		return QString();

	QColor col = imgStorage.getImage().pixel(xy);
	
	return col.name().toUpper().remove(0,1);
}

// Copy & Paste --------------------------------------------------------
void DkViewPort::copyPixelColorValue() {

	if (getImage().isNull())
		return;

	QMimeData* mimeData = new QMimeData;

	if (!getImage().isNull())
		mimeData->setText(getCurrentPixelHexValue());

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setMimeData(mimeData);
}

void DkViewPort::copyImage() {

	qDebug() << "copying...";

	if (getImage().isNull() || !loader)
		return;

	QUrl fileUrl = QUrl("file:///" + loader->file().absoluteFilePath());

	QList<QUrl> urls;
	urls.append(fileUrl);

	QMimeData* mimeData = new QMimeData;

	if (loader->file().exists() && !loader->isEdited())
		mimeData->setUrls(urls);
	else if (!getImage().isNull())
		mimeData->setImageData(getImage());

	mimeData->setText(loader->file().absoluteFilePath());

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setMimeData(mimeData);

	qDebug() << "copying: " << fileUrl;
}

void DkViewPort::copyImageBuffer() {

	qDebug() << "copying...";

	if (getImage().isNull())
		return;

	QMimeData* mimeData = new QMimeData;

	if (!getImage().isNull())
		mimeData->setImageData(getImage());

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setMimeData(mimeData);
}

void DkViewPort::pasteImage() {

	qDebug() << "pasting...";

	QClipboard* clipboard = QApplication::clipboard();

	if (!loadFromMime(clipboard->mimeData()))
		controller->setInfo("Clipboard has no image...");

}


void DkViewPort::animateFade() {

	fadeOpacity = 1.0-fadeTime.getTotalTime()/DkSettings::display.fadeSec;
	
	if (fadeOpacity <= 0) {
		fadeBuffer = QImage();
		fadeTimer->stop();
		fadeOpacity = 0;
	}
	else if (DkSettings::slideShow.moveSpeed) {
		float factor = 1.0f+(targetScale-worldMatrix.m11()) * fadeTime.getTotalTime()/DkSettings::display.fadeSec;
		zoom(factor, QPoint(0,0));
		qDebug() << "zoom factor: " << factor;
		// TODO: maybe the timer is not stable? - painting seems to be stable
	}

	qDebug() << "new opacity: " << fadeOpacity;

	update();
}

void DkViewPort::animateMove() {

	moveView(moveStep/worldMatrix.m11());
}

void DkViewPort::togglePattern(bool show) {

	controller->setInfo((show) ? tr("Transparency Pattern Enabled") : tr("Transparency Pattern Disabled"));

	DkBaseViewPort::togglePattern(show);
}

// edit image --------------------------------------------------------------------
void DkViewPort::rotateCW() {

	applyPluginChanges();

	if (loader != 0)
		loader->rotateImage(90);

}

void DkViewPort::rotateCCW() {

	applyPluginChanges();

	if (loader != 0)
		loader->rotateImage(-90);

}

void DkViewPort::rotate180() {

	applyPluginChanges();

	if (loader != 0)
		loader->rotateImage(180);

}

// file handling --------------------------------------------------------------------
void DkViewPort::loadLena() {

	bool ok;
	QString text = QInputDialog::getText(this, tr("Lena"), tr("A remarkable woman"), QLineEdit::Normal, 0, &ok);

	// pass phrase
	if (ok && !text.isEmpty() && text == "lena") {
		testLoaded = true;
		toggleLena();
	}
	else if (!ok && parent) {
		QMessageBox warningDialog(parent);
		warningDialog.setIcon(QMessageBox::Warning);
		warningDialog.setText(tr("you cannot cancel this"));
		warningDialog.exec();
		loadLena();
	}
	else {
		QApplication::beep();
		
		if (text.isEmpty())
			controller->setInfo(tr("did you understand the brainteaser?"));
		else
			controller->setInfo(tr("%1 is wrong...").arg(text));
	}
}

void DkViewPort::toggleLena() {

	if (!testLoaded)
		return;

	if (loader) {
		if (parent && parent->isFullScreen())
			loader->load(QFileInfo(":/nomacs/img/lena-full.jpg"));
		else
			loader->load(QFileInfo(":/nomacs/img/lena.jpg"));
	}
}

void DkViewPort::settingsChanged() {

	reloadFile();

	altMod = DkSettings::global.altMod;
	ctrlMod = DkSettings::global.ctrlMod;

	controller->settingsChanged();
}

void DkViewPort::downloadFile(const QUrl& url) {

	if (!fileDownloader) {
		fileDownloader = new FileDownloader(url, this);
		connect(fileDownloader, SIGNAL(downloaded()), this, SLOT(fileDownloaded()));
		qDebug() << "trying to download: " << url;
	}
	else
		fileDownloader->downloadFile(url);

}

void DkViewPort::fileDownloaded() {

	if (!fileDownloader) {
		qDebug() << "empty fileDownloader, where it should not be";
		return;
	}

	QSharedPointer<QByteArray> ba = fileDownloader->downloadedData();

	if (!ba || ba->isEmpty()) {
		qDebug() << fileDownloader->getUrl() << " not downloaded...";
		controller->setInfo(tr("Sorry, I could not download:\n%1").arg(fileDownloader->getUrl().toString()));
		return;
	}

	DkBasicLoader loader;
	if (loader.loadGeneral(QFileInfo(), ba, true)) {
		loadImage(loader.image());
		qDebug() << (tr("Package size: %1").arg(ba->size()));
	}
	else
		controller->setInfo(tr("Sorry, I could not load:\n%1\nPackage size:%2").arg(fileDownloader->getUrl().toString()).arg(ba->size()));
}

void DkViewPort::setEditedImage(QImage newImg) {

	if (newImg.isNull()) {
		controller->setInfo(tr("Attempted to set NULL image"));	// not sure if users understand that
		return;
	}

	QFileInfo file = loader->file();
	unloadImage(false);
	loader->setImage(newImg, file);
	setImage(newImg);
	qDebug() << "loader gets this size: " << newImg.size();

	// TODO: contrast viewport does not add * 

	// TODO: add functions such as save file on unload
}

void DkViewPort::applyPluginChanges() {

	DkNoMacs* noMacs = dynamic_cast<DkNoMacs*>(parent);

	if (!noMacs) return;

	if(!noMacs->getCurrRunningPlugin().isEmpty()) noMacs->applyPluginChanges(true, false);
}

bool DkViewPort::unloadImage(bool fileChange) {


	// TODO: we have to check here - why loading is not stopped by applyPluginChanges()
	/*if (!pluginImageWasApplied)*/ applyPluginChanges(); //prevent recursion
	
	if (DkSettings::display.fadeSec && (controller->getPlayer()->isPlaying() || parent && parent->isFullScreen())) {
		fadeBuffer = imgStorage.getImage(imgMatrix.m11()*worldMatrix.m11());
		fadeImgViewRect = imgViewRect;
		fadeImgRect = imgRect;
		fadeOpacity = 1.0f;
	}

	int success = true;
	if (fileChange)
		success = loader->unloadFile();		// returns false if the user cancels
	
	if (movie && success) {
		movie->stop();
		delete movie; 
		movie = 0;
	}

	return success;
}

void DkViewPort::loadFile(QFileInfo file) {

	if (!unloadImage())
		return;

	testLoaded = false;

	if (loader && file.isDir()) {
		QDir dir = QDir(file.absoluteFilePath());

		//// TODO: create this behavior again
		//if (loader && controller && controller->getThumbWidget()->isVisible()) {
		//	loader->loadDir(dir);
		//	//controller->getThumbWidget()->getThumbWidget()->setFile(file);
		//}
		//else
			loader->setDir(dir);

	} else if (loader)
		loader->load(file);

	qDebug() << "sync mode: " << (DkSettings::sync.syncMode == DkSettings::sync_mode_remote_display);
	if ((qApp->keyboardModifiers() == altMod || DkSettings::sync.syncMode == DkSettings::sync_mode_remote_display) && (hasFocus() || controller->hasFocus()) && loader->hasFile())
		tcpLoadFile(0, file.absoluteFilePath());
}

void DkViewPort::reloadFile() {

	if (loader) {

		if (unloadImage())
			loader->reloadImage();
	}
}

void DkViewPort::loadFile(int skipIdx) {

	if (!unloadImage())
		return;

	if (loader && !testLoaded)
		loader->changeFile(skipIdx);

	// alt mod
	if ((qApp->keyboardModifiers() == altMod || DkSettings::sync.syncMode == DkSettings::sync_mode_remote_display || DkSettings::sync.syncActions) && (hasFocus() || controller->hasFocus())) {
		emit sendNewFileSignal(skipIdx);
		qDebug() << "emitting load next";
	}
}

void DkViewPort::loadPrevFileFast() {

	loadFileFast(-1);
}

void DkViewPort::loadNextFileFast() {

	loadFileFast(1);
}


void DkViewPort::loadFileFast(int skipIdx, int rec) {

	if (!unloadImage())
		return;

	if (!((qApp->keyboardModifiers() == altMod || DkSettings::sync.syncActions) &&
		DkSettings::sync.syncMode == DkSettings::sync_mode_remote_control)) {
		QApplication::sendPostedEvents();

		for (int idx = 0; idx < loader->getImages().size(); idx++) {
			QSharedPointer<DkImageContainerT> imgC = loader->getSkippedImage(skipIdx);
		
			if (!imgC)
				break;

			loader->setCurrentImage(imgC);

			if (imgC && imgC->getLoadState() != DkImageContainer::exists_not) {
				loader->load(imgC);
				break;
			}
			else
				qDebug() << "image does not exist - skipping";
		}
	}	

	if (((qApp->keyboardModifiers() == altMod || DkSettings::sync.syncActions) &&
		DkSettings::sync.syncMode != DkSettings::sync_mode_remote_display) && 
		(hasFocus() || 
		controller->hasFocus())) {
		emit sendNewFileSignal(skipIdx);
		QCoreApplication::sendPostedEvents();
	}

	//skipImageTimer->start(50);	// load full image in 50 ms if there is not a fast load again
}

// deprecated
void DkViewPort::loadFullFile() {

	if (thumbFile.exists()) {
		//unloadImage();	// TODO: unload image clears the image -> makes an empty file
		loader->load(thumbFile);
	}
}

void DkViewPort::loadFirst() {

	if (!unloadImage())
		return;

	if (loader && !testLoaded)
		loader->firstFile();

	if ((qApp->keyboardModifiers() == altMod || DkSettings::sync.syncMode == DkSettings::sync_mode_remote_display || DkSettings::sync.syncActions) && (hasFocus() || controller->hasFocus()))
		emit sendNewFileSignal(SHRT_MIN);
}

void DkViewPort::loadLast() {

	if (!unloadImage())
		return;

	if (loader && !testLoaded)
		loader->lastFile();

	if ((qApp->keyboardModifiers() == altMod || DkSettings::sync.syncMode == DkSettings::sync_mode_remote_display || DkSettings::sync.syncActions) && (hasFocus() || controller->hasFocus()))
		emit sendNewFileSignal(SHRT_MAX);

}

void DkViewPort::loadSkipPrev10() {

	loadFileFast(-DkSettings::global.skipImgs);
	//unloadImage();

	//if (loader && !testLoaded)
	//	loader->changeFile(-DkSettings::global.skipImgs, (parent->isFullScreen() && DkSettings::slideShow.silentFullscreen));

	if (qApp->keyboardModifiers() == altMod && (hasFocus() || controller->hasFocus()))
		emit sendNewFileSignal(-DkSettings::global.skipImgs);
}

void DkViewPort::loadSkipNext10() {

	loadFileFast(DkSettings::global.skipImgs);
	//unloadImage();

	//if (loader && !testLoaded)
	//	loader->changeFile(DkSettings::global.skipImgs, (parent->isFullScreen() && DkSettings::slideShow.silentFullscreen));

	if (qApp->keyboardModifiers() == altMod && (hasFocus() || controller->hasFocus()))
		emit sendNewFileSignal(DkSettings::global.skipImgs);
}

void DkViewPort::tcpLoadFile(qint16 idx, QString filename) {

	qDebug() << "I got a file request??";

	// some hack: set the mode to default in order to prevent loops (if both are auto connected)
	// should be mostly harmless
	//int oldMode = DkSettings::sync.syncMode;
	//DkSettings::sync.syncMode = DkSettings::sync_mode_receiveing_command;

	if (filename.isEmpty()) {

		// change the file idx according to my brother
		switch (idx) {
			case SHRT_MIN:
				loadFirst();
				break;
			case SHRT_MAX:
				loadLast();
				break;
			//case 1:
			//	loadNextFileFast();
			//	break;
			//case -1:
			//	loadPrevFileFast();
			//	break;
			default:
				loadFileFast(idx, true);
				//if (loader) loader->loadFileAt(idx);
		}
	}
	else 
		loadFile(QFileInfo(filename));

	qDebug() << "loading file: " << filename;

	//DkSettings::sync.syncMode = oldMode;
}

DkImageLoader* DkViewPort::getImageLoader() {

	return loader;
}

DkControlWidget* DkViewPort::getController() {
	
	return controller;
}

void DkViewPort::cropImage(DkRotatingRect rect, const QColor& bgCol) {

	QTransform tForm; 
	QPointF cImgSize;

	rect.getTransform(tForm, cImgSize);

	if (cImgSize.x() < 0.5f || cImgSize.y() < 0.5f) {
		controller->setInfo(tr("I cannot crop an image that has 0 px, sorry."));
		return;
	}

	qDebug() << cImgSize;

	double angle = DkMath::normAngleRad(rect.getAngle(), 0, CV_PI*0.5);
	double minD = qMin(abs(angle), abs(angle-CV_PI*0.5));

	QImage img = QImage(cImgSize.x(), cImgSize.y(), QImage::Format_ARGB32);
	img.fill(bgCol.rgba());

	// render the image into the new coordinate system
	QPainter painter(&img);
	painter.setWorldTransform(tForm);
	
	// for rotated rects we want perfect anti-aliasing
	if (minD > FLT_EPSILON)
		painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
	
	painter.drawImage(QRect(QPoint(), getImage().size()), getImage(), QRect(QPoint(), getImage().size()));
	painter.end();

	setEditedImage(img);
	
	//imgQt = img;
	//update();

	qDebug() << "cropping...";
}

// DkViewPortFrameless --------------------------------------------------------------------
DkViewPortFrameless::DkViewPortFrameless(QWidget *parent, Qt::WindowFlags flags) : DkViewPort(parent) {
	
#ifdef Q_WS_MAC
	parent->setAttribute(Qt::WA_MacNoShadow);
#endif

	setAttribute(Qt::WA_TranslucentBackground, true);
	setCursor(Qt::OpenHandCursor);
    
//#ifdef Q_WS_MAC
	// setAttribute( Qt::WA_MacNoShadow );
//#endif

	imgBg.load(":/nomacs/img/splash-screen.png");

	mainScreen = geometry();

	//show();

	// TODO: just set the left - upper - lower offset for all labels (according to viewRect)
	// always set the size to be full screen -> bad for OS that are not able to show transparent frames!!
}

DkViewPortFrameless::~DkViewPortFrameless() {

	release();
}

void DkViewPortFrameless::release() {

	DkViewPort::release();
}

void DkViewPortFrameless::addStartActions(QAction* startAction, QIcon* startIcon) {

	startActions.append(startAction);
	startIcons.append(startIcon);
}

void DkViewPortFrameless::setImage(QImage newImg) {

	DkViewPort::setImage(newImg);
}

void DkViewPortFrameless::zoom(float factor, QPointF center) {

	if (!imgStorage.hasImage() || blockZooming)
		return;

	//limit zoom out ---
	if (worldMatrix.m11()*factor <= minZoom && factor < 1)
		return;

	//if (worldMatrix.m11()*factor < 1) {
	//	resetView();
	//	return;
	//}

	// reset view & block if we pass the 'image fit to screen' on zoom out
	if (worldMatrix.m11() > 1 && worldMatrix.m11()*factor < 1) {

		blockZooming = true;
		zoomTimer->start(500);
		//resetView();
		//return;
	}

	//limit zoom in ---
	if (worldMatrix.m11()*imgMatrix.m11() > maxZoom && factor > 1)
		return;

	QRectF viewRect = worldMatrix.mapRect(imgViewRect);

	// if no center assigned: zoom in at the image center
	if (center.x() == -1 || center.y() == -1)
		center = viewRect.center();

	
	//if (factor < 1) {
	//	
	//	QRectF imgWorldRect = worldMatrix.mapRect(imgViewRect);
	//	float ipl = imgViewRect.width()/imgWorldRect.width();	// size ratio
	//	center = (imgViewRect.center() - imgWorldRect.center()) + imgViewRect.center();
	//}

	if (center.x() < viewRect.left())			center.setX(viewRect.left());
	else if (center.x() > viewRect.right())		center.setX(viewRect.right());
	if (center.y() < viewRect.top())			center.setY(viewRect.top());
	else if (center.y() > viewRect.bottom())	center.setY(viewRect.bottom());

	//inverse the transform
	int a, b;
	worldMatrix.inverted().map(center.x(), center.y(), &a, &b);

	worldMatrix.translate(a-factor*a, b-factor*b);
	worldMatrix.scale(factor, factor);

	controlImagePosition();
	showZoom();
	changeCursor();

	update();

	tcpSynchronize();

}

void DkViewPortFrameless::resetView() {

	// maybe we can delete this function...
	DkViewPort::resetView();
}

void DkViewPortFrameless::paintEvent(QPaintEvent* event) {

	if (parent && !parent->isFullScreen()) {

		QPainter painter(viewport());
		painter.setWorldTransform(worldMatrix);
		drawFrame(&painter);
		painter.end();
	}

	DkViewPort::paintEvent(event);
}

void DkViewPortFrameless::draw(QPainter *painter, float opacity) {
	
	if (parent && parent->isFullScreen()) {
		QColor col = QColor(0,0,0);
		col.setAlpha(150);
		painter->setWorldMatrixEnabled(false);
		painter->fillRect(QRect(QPoint(), size()), col);
		painter->setWorldMatrixEnabled(true);
	}

	if (!movie || !movie->isValid()) {
		QImage imgQt = imgStorage.getImage(imgMatrix.m11()*worldMatrix.m11());

		if (DkSettings::display.tpPattern && imgQt.hasAlphaChannel()) {

			// don't scale the pattern...
			QTransform scaleIv;
			scaleIv.scale(worldMatrix.m11(), worldMatrix.m22());
			pattern.setTransform(scaleIv.inverted());

			painter->setPen(QPen(Qt::NoPen));	// no border
			painter->setBrush(pattern);
			painter->drawRect(imgViewRect);
		}

		painter->drawImage(imgViewRect, imgQt, QRect(QPoint(), imgQt.size()));
	}
	else {
		painter->drawPixmap(imgViewRect, movie->currentPixmap(), movie->frameRect());
	}

}

void DkViewPortFrameless::drawBackground(QPainter *painter) {
	
	painter->setWorldTransform(imgMatrix);
	painter->setRenderHint(QPainter::SmoothPixmapTransform);
	painter->setBrush(QColor(127, 144, 144, 200));
	painter->setPen(QColor(100, 100, 100, 255));

	QRectF initialRect = mainScreen;
	QPointF oldCenter = initialRect.center();

	QTransform cT;
	cT.scale(400/initialRect.width(), 400/initialRect.width());
	initialRect = cT.mapRect(initialRect);
	initialRect.moveCenter(oldCenter);

	// fit to viewport
	QSize s = imgBg.size();

	QRectF bgRect(QPoint(), s);
	bgRect.moveCenter(initialRect.center());//moveTopLeft(QPointF(size().width(), size().height())*0.5 - initialRect.bottomRight()*0.5);

	//painter->drawRect(initialRect);
	painter->drawImage(bgRect, imgBg, QRect(QPoint(), imgBg.size()));

	if (startActions.isEmpty())
		return;

	// first time?
	if (startActionsRects.isEmpty()) {
		float margin = 40;
		float iconSizeMargin = (initialRect.width()-3*margin)/startActions.size();
		QSize iconSize = QSize(iconSizeMargin - margin, iconSizeMargin - margin);
		QPointF offset = QPointF(bgRect.left() + 50, initialRect.center().y()+iconSizeMargin*0.25f);

		for (int idx = 0; idx < startActions.size(); idx++) {

			QRectF iconRect = QRectF(offset, iconSize);
			QPixmap ci = startIcons[idx] ? startIcons[idx]->pixmap(iconSize) : startActions[idx]->icon().pixmap(iconSize);
			startActionsRects.push_back(iconRect);
			startActionsIcons.push_back(ci);

			offset.setX(offset.x() + margin + iconSize.width());
		}
	}

	// draw start actions
	for (int idx = 0; idx < startActions.size(); idx++) {
		
		if (startIcons[idx])
			painter->drawPixmap(startActionsRects[idx], startActionsIcons[idx], QRect(QPoint(), startActionsIcons[idx].size()));
		else
			painter->drawPixmap(startActionsRects[idx], startActionsIcons[idx], QRect(QPoint(), startActionsIcons[idx].size()));
		
		QRectF tmpRect = startActionsRects[idx];
		QString text = startActions[idx]->text().replace("&", "");
		tmpRect.moveTop(tmpRect.bottom()+10);
		painter->drawText(tmpRect, text);
	}

	QString infoText = tr("Press F10 to exit Frameless view");
	QRectF tmpRect(bgRect.left()+50, bgRect.bottom()-60, bgRect.width()-100, 20);
	painter->drawText(tmpRect, infoText);
}

void DkViewPortFrameless::drawFrame(QPainter* painter) {

	// TODO: replace hasAlphaChannel with has alphaBorder
	if (imgStorage.hasImage() && imgStorage.getImage().hasAlphaChannel() || !DkSettings::display.showBorder)
		return;

	painter->setBrush(QColor(255, 255, 255, 200));
	painter->setPen(QColor(100, 100, 100, 255));

	QRectF frameRect;

	float fs = qMin(imgViewRect.width(), imgViewRect.height())*0.1f;

	// looks pretty bad if the frame is too small
	if (fs < 4)
		return;

	frameRect = imgViewRect;
	frameRect.setSize(frameRect.size() + QSize(fs, fs));
	frameRect.moveCenter(imgViewRect.center());

	painter->drawRect(frameRect);
}

void DkViewPortFrameless::mousePressEvent(QMouseEvent *event) {
	
	// move the window - todo: NOT full screen, window inside...
	setCursor(Qt::ClosedHandCursor);
	posGrab = event->pos();

	DkViewPort::mousePressEvent(event);
}

void DkViewPortFrameless::mouseReleaseEvent(QMouseEvent *event) {
	
	if (!imgStorage.hasImage()) {

		qDebug() << "mouse released";
		QPointF pos = imgMatrix.inverted().map(event->pos());

		for (int idx = 0; idx < startActionsRects.size(); idx++) {

			if (startActionsRects[idx].contains(pos)) {
				qDebug() << "toggle..." << idx;
				
				startActions[idx]->trigger();
				break;
			}
		}
	}

	setCursor(Qt::OpenHandCursor);
	DkViewPort::mouseReleaseEvent(event);
}


void DkViewPortFrameless::mouseMoveEvent(QMouseEvent *event) {
	
	if (!imgStorage.hasImage()) {

		QPointF pos = imgMatrix.inverted().map(event->pos());

		int idx;
		for (idx = 0; idx < startActionsRects.size(); idx++) {

			if (startActionsRects[idx].contains(pos)) {
				setCursor(Qt::PointingHandCursor);
				break;
			}
		}

		// TODO: change if closed hand cursor is present...
		if (idx == startActionsRects.size())
			setCursor(Qt::OpenHandCursor);
	}

	if (visibleStatusbar)
		getPixelInfo(event->pos());

	if (event->buttons() == Qt::LeftButton) {

		QPointF cPos = event->pos();
		QPointF dxy = (cPos - posGrab);
		posGrab = cPos;
		moveView(dxy/worldMatrix.m11());
	}

	//// scroll thumbs preview
	//if (filePreview && filePreview->isVisible() && event->buttons() == Qt::MiddleButton) {

	//	float dx = std::fabs(enterPos.x() - event->pos().x())*0.015;
	//	dx = std::exp(dx);
	//	if (enterPos.x() - event->pos().x() < 0)
	//		dx = -dx;

	//	filePreview->setCurrentDx(dx);	// update dx
	//}

	QGraphicsView::mouseMoveEvent(event);
}

void DkViewPortFrameless::resizeEvent(QResizeEvent *event) {

	DkViewPort::resizeEvent(event);

	// controller should only be on the main screen...
	QDesktopWidget* dw = QApplication::desktop();
	controller->setGeometry(dw->screenGeometry());
	qDebug() << "controller resized to: " << controller->geometry();
}

void DkViewPortFrameless::moveView(QPointF delta) {

	// if no zoom is present -> the translation is like a move window
	if (worldMatrix.m11() == 1.0f) {
		float s = imgMatrix.m11();
		imgMatrix.translate(delta.x()/s, delta.y()/s);
		imgViewRect = imgMatrix.mapRect(imgRect);
	}
	else
		worldMatrix.translate(delta.x(), delta.y());

	controlImagePosition();
	update();
}


void DkViewPortFrameless::controlImagePosition(float lb, float ub) {
	// dummy method
}

void DkViewPortFrameless::centerImage() {

}

void DkViewPortFrameless::updateImageMatrix() {

	if (!imgStorage.hasImage())
		return;

	QRectF oldImgRect = imgViewRect;
	QTransform oldImgMatrix = imgMatrix;

	imgMatrix.reset();

	// if the image is smaller or zoom is active: paint the image as is
	if (!getMainGeometry().contains(imgRect.toRect()))
		imgMatrix = getScaledImageMatrix();
	else {

		QPointF p = (imgViewRect.isEmpty()) ? getMainGeometry().center() : imgViewRect.center();
		p -= imgStorage.getImage().rect().center();
		imgMatrix.translate(p.x()-1, p.y()-1);	// -1 is needed due to float -> int
		imgMatrix.scale(1.0f, 1.0f);
	}

	imgViewRect = imgMatrix.mapRect(imgRect);

	// update world matrix
	if (worldMatrix.m11() != 1) {

		float scaleFactor = oldImgMatrix.m11()/imgMatrix.m11();
		double dx = oldImgRect.x()/scaleFactor-imgViewRect.x();
		double dy = oldImgRect.y()/scaleFactor-imgViewRect.y();

		worldMatrix.scale(scaleFactor, scaleFactor);
		worldMatrix.translate(dx, dy);
	}
}

QTransform DkViewPortFrameless::getScaledImageMatrix() {

	QRectF initialRect = mainScreen;
	QPointF oldCenter = imgViewRect.isEmpty() ? initialRect.center() : imgViewRect.center();
	qDebug() << "initial rect: " << initialRect;

	QTransform cT;
	cT.scale(800/initialRect.width(), 800/initialRect.width());
	cT.translate(initialRect.center().x(), initialRect.center().y());
	initialRect = cT.mapRect(initialRect);
	initialRect.moveCenter(oldCenter);

	// the image resizes as we zoom
	float ratioImg = imgRect.width()/imgRect.height();
	float ratioWin = initialRect.width()/initialRect.height();

	QTransform imgMatrix;
	float s;
	if (imgRect.width() == 0 || imgRect.height() == 0)
		s = 1.0f;
	else
		s = (ratioImg > ratioWin) ? initialRect.width()/imgRect.width() : initialRect.height()/imgRect.height();

	imgMatrix.scale(s, s);

	QRectF imgViewRect = imgMatrix.mapRect(imgRect);
	QSizeF sDiff = (initialRect.size() - imgViewRect.size())*0.5f/s;
	imgMatrix.translate(initialRect.left()/s+sDiff.width(), initialRect.top()/s+sDiff.height());

	return imgMatrix;
}

DkViewPortContrast::DkViewPortContrast(QWidget *parent, Qt::WindowFlags flags) : DkViewPort(parent) {

	isColorPickerActive = false;
	activeChannel = 0;
	
	colorTable = QVector<QRgb>(256);
	for (int i = 0; i < colorTable.size(); i++) 
		colorTable[i] = qRgb(i, i, i);
	
	drawFalseColorImg = false;

}

DkViewPortContrast::~DkViewPortContrast() {

	release();
}

void DkViewPortContrast::release() {

	DkViewPort::release();
}

void DkViewPortContrast::changeChannel(int channel) {

	if (channel < 0 || channel >= imgs.size())
		return;

	if (imgStorage.hasImage()) {

		falseColorImg = imgs[channel];
		falseColorImg.setColorTable(colorTable);
		drawFalseColorImg = true;

		update();

		drawImageHistogram();
	}

}


void DkViewPortContrast::changeColorTable(QGradientStops stops) {

	
	qreal pos = stops.at(0).first;
	qreal fac;

	qreal actPos, leftStop, rightStop;
	QColor tmp;

	int rLeft, gLeft, bLeft, rRight, gRight, bRight;
	int rAct, gAct, bAct;

	// At least one stop has to be set:
	tmp = stops.at(0).second;
	tmp.getRgb(&rLeft, &gLeft, &bLeft);
	leftStop = stops.at(0).first;

	// If just one stop is set, we can speed things up:
	if (stops.size() == 1) {
		for (int i = 0; i < colorTable.size(); i++)
			colorTable[i] = qRgb(rLeft, gLeft, bLeft);
	}
	// Otherwise interpolate:
	else {

		int rightStopIdx = 1;
		tmp = stops.at(rightStopIdx).second;
		tmp.getRgb(&rRight, &gRight, &bRight);
		rightStop = stops.at(rightStopIdx).first;
	
		for (int i = 0; i < colorTable.size(); i++) {
			actPos = (qreal) i / colorTable.size();

			if (actPos > rightStop) {
				leftStop = rightStop;	

				rLeft = rRight;
				gLeft = gRight;
				bLeft = bRight;

				if (stops.size() > rightStopIdx + 1) {
					rightStopIdx++;
					rightStop = stops.at(rightStopIdx).first;
					tmp = stops.at(rightStopIdx).second;
					tmp.getRgb(&rRight, &gRight, &bRight);
				}

			}
		
			if (actPos <= leftStop)
				colorTable[i] = qRgb(rLeft, gLeft, bLeft);
			else if (actPos >= rightStop)
				colorTable[i] = qRgb(rRight, gRight, bRight);
			else {
				fac = (actPos - leftStop) / (rightStop - leftStop);
				rAct = qRound(rLeft + (rRight - rLeft) * fac);
				gAct = qRound(gLeft + (gRight - gLeft) * fac);
				bAct = qRound(bLeft + (bRight - bLeft) * fac);
				colorTable[i] = qRgb(rAct, gAct, bAct);
			}	
		}
	}


	falseColorImg.setColorTable(colorTable);
	
	update();
	
}

void DkViewPortContrast::draw(QPainter *painter, float opacity) {

	if (parent && parent->isFullScreen()) {
		painter->setWorldMatrixEnabled(false);
		painter->fillRect(QRect(QPoint(), size()), DkSettings::slideShow.backgroundColor);
		painter->setWorldMatrixEnabled(true);
	}

	QImage imgQt = imgStorage.getImage(imgMatrix.m11()*worldMatrix.m11());

	if (DkSettings::display.tpPattern && imgQt.hasAlphaChannel()) {

		// don't scale the pattern...
		QTransform scaleIv;
		scaleIv.scale(worldMatrix.m11(), worldMatrix.m22());
		pattern.setTransform(scaleIv.inverted());

		painter->setPen(QPen(Qt::NoPen));	// no border
		painter->setBrush(pattern);
		painter->drawRect(imgViewRect);
	}

	if (drawFalseColorImg)
		painter->drawImage(imgViewRect, falseColorImg, imgRect);		// TODO: add storage class for falseColorImg
	else 
		painter->drawImage(imgViewRect, imgQt, QRect(QPoint(), imgQt.size()));

}

void DkViewPortContrast::setImage(QImage newImg) {

	DkViewPort::setImage(newImg);

	if (newImg.isNull())
		return;

	if (imgStorage.getImage().format() == QImage::Format_Indexed8) {
		int format = imgStorage.getImage().format();
		imgs = QVector<QImage>(1);
		imgs[0] = imgStorage.getImage();
		activeChannel = 0;
	}

#ifdef WITH_OPENCV

	else {	
					
			imgs = QVector<QImage>(4);
			vector<Mat> planes;
			
			Mat imgUC3 = DkImage::qImage2Mat(imgStorage.getImage());
			//int format = imgQt.format();
			//if (format == QImage::Format_RGB888)
			//	imgUC3 = Mat(imgQt.height(), imgQt.width(), CV_8UC3, (uchar*)imgQt.bits(), imgQt.bytesPerLine());
			//else
			//	imgUC3 = Mat(imgQt.height(), imgQt.width(), CV_8UC4, (uchar*)imgQt.bits(), imgQt.bytesPerLine());
			split(imgUC3, planes);
			// Store the 3 channels in a QImage Vector.
			//Be aware that OpenCV 'swaps' the rgb triplet, hence process it in a descending way:
			int idx = 1;
			for (int i = 2; i >= 0; i--) {

				// dirty hack
				if (i >= (int)planes.size()) i = 0;
				imgs[idx] = QImage((const unsigned char*)planes[i].data, (int)planes[i].cols, (int)planes[i].rows, (int)planes[i].step,  QImage::Format_Indexed8);
				imgs[idx] = imgs[idx].copy();
				idx++;

			}
			// The first element in the vector contains the gray scale 'average' of the 3 channels:
			Mat grayMat;
			cv::cvtColor(imgUC3, grayMat, CV_BGR2GRAY);
			imgs[0] = QImage((const unsigned char*)grayMat.data, (int)grayMat.cols, (int)grayMat.rows, (int)grayMat.step,  QImage::Format_Indexed8);
			imgs[0] = imgs[0].copy();
			planes.clear();

	}
#else

	else {
		drawFalseColorImg = false;
		emit imageModeSet(mode_invalid_format);	
		return;
	}

#endif
	
	
	falseColorImg = imgs[activeChannel];
	falseColorImg.setColorTable(colorTable);
	
	// images with valid color table return img.isGrayScale() false...
	if (imgs.size() == 1) 
		emit imageModeSet(mode_gray);
	else
		emit imageModeSet(mode_rgb);

	update();

	
}

void DkViewPortContrast::pickColor() {

	isColorPickerActive = true;
	this->setCursor(Qt::CrossCursor);

}

void DkViewPortContrast::enableTF(bool enable) {

	drawFalseColorImg = enable;
	update();

	drawImageHistogram();

}

void DkViewPortContrast::mousePressEvent(QMouseEvent *event) {


	if (isColorPickerActive) {

		QPointF imgPos = worldMatrix.inverted().map(event->pos());
		imgPos = imgMatrix.inverted().map(imgPos);

		QPoint xy = imgPos.toPoint();

		bool isPointValid = true;

		if (xy.x() < 0 || xy.y() < 0 || xy.x() >= imgStorage.getImage().width() || xy.y() >= imgStorage.getImage().height())
			isPointValid = false;

		if (isPointValid) {

			int colorIdx = imgs[activeChannel].pixelIndex(xy);
			qreal normedPos = (qreal) colorIdx / 255;
			emit tFSliderAdded(normedPos);
		}

		unsetCursor();
		isColorPickerActive = false;

	} 

	// always forward events (fixes #397)
	DkViewPort::mousePressEvent(event);
}

void DkViewPortContrast::keyPressEvent(QKeyEvent* event) {

	if ((event->key() == Qt::Key_Escape) && isColorPickerActive) {
		unsetCursor();
		isColorPickerActive = false;
		update();
		return;
	}
	else
		DkViewPort::keyPressEvent(event);
}

QImage DkViewPortContrast::getImage() {

	if (drawFalseColorImg)
		return falseColorImg;
	else
		return imgStorage.getImage();

}

// in contrast mode: if the histogram widget is visible redraw the histogram from the selected image channel data
void DkViewPortContrast::drawImageHistogram() {

	if (controller->getHistogram() && controller->getHistogram()->isVisible()) {
		if(drawFalseColorImg) controller->getHistogram()->drawHistogram(falseColorImg);
		else controller->getHistogram()->drawHistogram(imgStorage.getImage());
	}

}

// custom events --------------------------------------------------------------------
//QEvent::Type DkInfoEvent::infoEventType = static_cast<QEvent::Type>(QEvent::registerEventType());
//QEvent::Type DkLoadImageEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());


}
