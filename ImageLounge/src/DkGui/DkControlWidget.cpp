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
#include "DkPluginManager.h"
#include "DkMessageBox.h"
#include "DkActionManager.h"
#include "DkCropWidgets.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QStackedLayout>
#include <QSharedPointer>
#include <QGridLayout>
#include <QTransform>
#include <QMouseEvent>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

// DkControlWidget --------------------------------------------------------------------
DkControlWidget::DkControlWidget(DkViewPort *parent, Qt::WindowFlags flags) : DkWidget(parent, flags) {

	mViewport = parent;
	setObjectName("DkControlWidget");

	// cropping
	// TODO: add lazy initialization here
	mCropWidget = new DkCropWidget(this);
	
	// thumbnails, metadata
	mFilePreview = new DkFilePreview(this, flags);
	mMetaDataInfo = new DkMetaDataHUD(this);
	mZoomWidget = new DkZoomWidget(this);
	mPlayer = new DkPlayer(this);
	mPlayer->setMaximumHeight(90);

	mFolderScroll = new DkFolderScrollBar(this);

	// file info - overview
	mFileInfoLabel = new DkFileInfoLabel(this);
	mRatingLabel = new DkRatingLabelBg(2, this, flags);
	mCommentWidget = new DkCommentWidget(this);

	// delayed info
	mDelayedInfo = new DkDelayedMessage(this); // TODO: make a nice constructor

	// info labels
	mBottomLabel = new DkLabelBg(this, "");
	mBottomLeftLabel = new DkLabelBg(this, "");

	// wheel label
	QPixmap wp = QPixmap(":/nomacs/img/thumbs-move.svg");

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
	mFilePreview->setDisplaySettings(&DkSettingsManager::param().app().showFilePreview);
	mMetaDataInfo->setDisplaySettings(&DkSettingsManager::param().app().showMetaData);
	mFileInfoLabel->setDisplaySettings(&DkSettingsManager::param().app().showFileInfoLabel);
	mPlayer->setDisplaySettings(&DkSettingsManager::param().app().showPlayer);
	mHistogram->setDisplaySettings(&DkSettingsManager::param().app().showHistogram);
	mCommentWidget->setDisplaySettings(&DkSettingsManager::param().app().showComment);
	mZoomWidget->setDisplaySettings(&DkSettingsManager::param().app().showOverview);
	mFolderScroll->setDisplaySettings(&DkSettingsManager::param().app().showScroller);

	// some adjustments
	mBottomLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	mBottomLeftLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	mRatingLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	mZoomWidget->setContentsMargins(10, 10, 0, 0);
	mCropWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	mCommentWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	// register actions
	DkActionManager& am = DkActionManager::instance();
	mFilePreview->registerAction(am.action(DkActionManager::menu_panel_preview));
	mMetaDataInfo->registerAction(am.action(DkActionManager::menu_panel_exif));
	mPlayer->registerAction(am.action(DkActionManager::menu_panel_player));
	mCropWidget->registerAction(am.action(DkActionManager::menu_edit_crop));
	mFileInfoLabel->registerAction(am.action(DkActionManager::menu_panel_info));
	mHistogram->registerAction(am.action(DkActionManager::menu_panel_histogram));
	mCommentWidget->registerAction(am.action(DkActionManager::menu_panel_comment));
	mFolderScroll->registerAction(am.action(DkActionManager::menu_panel_scroller));

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

	// center column
	QWidget* center = new QWidget(this);
	center->setMouseTracking(true);
	QVBoxLayout* cLayout = new QVBoxLayout(center);
	cLayout->setContentsMargins(0,0,0,0);
	cLayout->setAlignment(Qt::AlignBottom);
	cLayout->addWidget(mPlayer);
	
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

	//// debug code...
	//centerLabel->setText("ich bin richtig...", -1);
	//bottomLeftLabel->setText("topLeft label...", -1);
	//spinnerLabel->show();

	show();
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
	connect(mZoomWidget, SIGNAL(zoomSignal(double)), mViewport, SLOT(zoomTo(double)));
	connect(mViewport, SIGNAL(zoomSignal(double)), mZoomWidget, SLOT(updateZoom(double)));

	// waiting
	connect(mDelayedInfo, SIGNAL(infoSignal(const QString&, int)), this, SLOT(setInfo(const QString&, int)));
	
	// rating
	connect(mFileInfoLabel->getRatingLabel(), SIGNAL(newRatingSignal(int)), this, SLOT(updateRating(int)));
	connect(mRatingLabel, SIGNAL(newRatingSignal(int)), this, SLOT(updateRating(int)));
	//connect(ratingLabel, SIGNAL(newRatingSignal(int)), metaDataInfo, SLOT(setRating(int)));

	// playing
	connect(mPlayer, SIGNAL(previousSignal()), mViewport, SLOT(loadPrevFileFast()));
	connect(mPlayer, SIGNAL(nextSignal()), mViewport, SLOT(loadNextFileFast()));

	// cropping
	connect(mCropWidget, SIGNAL(cropImageSignal(const DkRotatingRect&, const QColor&, bool)), mViewport, SLOT(cropImage(const DkRotatingRect&, const QColor&, bool)));
	connect(mCropWidget, SIGNAL(hideSignal()), this, SLOT(hideCrop()));

	// comment widget
	connect(mCommentWidget, SIGNAL(showInfoSignal(const QString&)), this, SLOT(setInfo(const QString&)));
	connect(mCommentWidget, SIGNAL(commentSavedSignal()), this, SLOT(setCommentSaved()));
	connect(this, SIGNAL(imageUpdatedSignal()), mCommentWidget, SLOT(resetComment()));

	// mViewport
	connect(mViewport, SIGNAL(infoSignal(const QString&)), this, SLOT(setInfo(const QString&)));

	DkActionManager& am = DkActionManager::instance();

	// plugins
	if (am.pluginActionManager()) {
		connect(am.pluginActionManager(), SIGNAL(runPlugin(DkViewPortInterface*, bool)), this, SLOT(setPluginWidget(DkViewPortInterface*, bool)));
		connect(am.pluginActionManager(), SIGNAL(applyPluginChanges(bool)), this, SLOT(applyPluginChanges(bool)));
	}

	// actions
	connect(am.action(DkActionManager::menu_edit_crop), SIGNAL(triggered(bool)), this, SLOT(showCrop(bool)));
	connect(am.action(DkActionManager::menu_panel_overview), SIGNAL(toggled(bool)), this, SLOT(showOverview(bool)));
	connect(am.action(DkActionManager::menu_panel_player), SIGNAL(toggled(bool)), this, SLOT(showPlayer(bool)));
	connect(am.action(DkActionManager::menu_panel_preview), SIGNAL(toggled(bool)), this, SLOT(showPreview(bool)));
	connect(am.action(DkActionManager::menu_panel_scroller), SIGNAL(toggled(bool)), this, SLOT(showScroller(bool)));
	connect(am.action(DkActionManager::menu_panel_exif), SIGNAL(toggled(bool)), this, SLOT(showMetaData(bool)));
	connect(am.action(DkActionManager::menu_panel_info), SIGNAL(toggled(bool)), this, SLOT(showFileInfo(bool)));
	connect(am.action(DkActionManager::menu_panel_histogram), SIGNAL(toggled(bool)), this, SLOT(showHistogram(bool)));
	connect(am.action(DkActionManager::menu_panel_comment), SIGNAL(toggled(bool)), this, SLOT(showCommentWidget(bool)));
	connect(am.action(DkActionManager::menu_panel_toggle), SIGNAL(toggled(bool)), this, SLOT(toggleHUD(bool)));
}

void DkControlWidget::setCommentSaved() {

	mViewport->imageContainer()->setMetaData(tr("File comment"));
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

	// do not show if hide panels is checked
	if (DkActionManager::instance().action(DkActionManager::menu_panel_toggle)->isChecked())
		return;

	//qDebug() << "current app mode: " << DkSettingsManager::param().app().currentAppMode;

	showOverview(mZoomWidget->getCurrentDisplaySetting());
	showPreview(mFilePreview->getCurrentDisplaySetting());
	showMetaData(mMetaDataInfo->getCurrentDisplaySetting());
	showFileInfo(mFileInfoLabel->getCurrentDisplaySetting());
	showPlayer(mPlayer->getCurrentDisplaySetting());
	showHistogram(mHistogram->getCurrentDisplaySetting());
	showCommentWidget(mCommentWidget->getCurrentDisplaySetting());
	showScroller(mFolderScroll->getCurrentDisplaySetting());
}

void DkControlWidget::toggleHUD(bool hide) {
	
	if (hide)
		setWidgetsVisible(false, false);
	else	
		showWidgetsSettings();
}

void DkControlWidget::setWidgetsVisible(bool visible, bool saveSettings) {

	mFilePreview->setVisible(visible, saveSettings);
	mFolderScroll->setVisible(visible, saveSettings);
	mMetaDataInfo->setVisible(visible, saveSettings);
	mFileInfoLabel->setVisible(visible, saveSettings);
	mPlayer->setVisible(visible, saveSettings);
	mZoomWidget->setVisible(visible, saveSettings);
	mHistogram->setVisible(visible, saveSettings);
	mCommentWidget->setVisible(visible, saveSettings);
}

void DkControlWidget::showPreview(bool visible) {

	if (!mFilePreview)
		return;

	if (visible && !mFilePreview->isVisible())
		mFilePreview->show();
	else if (!visible && mFilePreview->isVisible())
		mFilePreview->hide(!mViewport->getImage().isNull());	// do not save settings if we have no image in the viewport
}

void DkControlWidget::showScroller(bool visible) {

	if (!mFolderScroll)
		return;

	if (visible && !mFolderScroll->isVisible())
		mFolderScroll->show();
	else if (!visible && mFolderScroll->isVisible())
		mFolderScroll->hide(!mViewport->getImage().isNull());	// do not save settings if we have no image in the viewport
}

void DkControlWidget::showMetaData(bool visible) {

	if (!mMetaDataInfo)
		return;

	if (visible && !mMetaDataInfo->isVisible()) {
		mMetaDataInfo->show();
		qDebug() << "showing metadata...";
	}
	else if (!visible && mMetaDataInfo->isVisible())
		mMetaDataInfo->hide(!mViewport->getImage().isNull());	// do not save settings if we have no image in the viewport
}

void DkControlWidget::showFileInfo(bool visible) {

	if (!mFileInfoLabel)
		return;

	if (visible && !mFileInfoLabel->isVisible()) {
		mFileInfoLabel->show();
		mRatingLabel->block(mFileInfoLabel->isVisible());
	}
	else if (!visible && mFileInfoLabel->isVisible()) {
		mFileInfoLabel->hide(!mViewport->getImage().isNull());	// do not save settings if we have no image in the viewport
		mRatingLabel->block(false);
	}
}

void DkControlWidget::showPlayer(bool visible) {

	if (!mPlayer)
		return;

	if (visible)
		mPlayer->show();
	else
		mPlayer->hide(!mViewport->getImage().isNull());	// do not save settings if we have no image in the viewport
}

void DkControlWidget::startSlideshow(bool start) {

	mPlayer->play(start);
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

	if (visible) {
		mViewport->zoomToFit(100);
		mCropWidget->reset();
		switchWidget(mWidgets[crop_widget]);
		//connect(mCropWidget->getToolbar(), SIGNAL(colorSignal(const QBrush&)), mViewport, SLOT(setBackgroundBrush(const QBrush&)));
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

	if (mLayout->currentWidget() == widget || (!widget && mLayout->currentWidget() == mWidgets[hud_widget]))
		return;

	if (widget != mWidgets[hud_widget])
		applyPluginChanges(true);

	if (widget)
		mLayout->setCurrentWidget(widget);
	else
		mLayout->setCurrentWidget(mWidgets[hud_widget]);

	//if (mLayout->currentWidget())
	//	qDebug() << "changed to widget: " << mLayout->currentWidget();

	// ok, this is really nasty... however, the pImagfileInfo layout is destroyed otherwise
	if (mLayout->currentIndex() == hud_widget && mFileInfoLabel->isVisible()) {
		mFileInfoLabel->setVisible(false);
		showFileInfo(true);
	}

}

bool DkControlWidget::closePlugin(bool askForSaving, bool force) {

#ifdef WITH_PLUGINS

	QSharedPointer<DkPluginContainer> plugin = DkPluginManager::instance().getRunningPlugin();

	if (!plugin)
		return true;
	
	DkViewPortInterface* vPlugin = plugin->pluginViewPort();

	if (!vPlugin) 
		return true;

	// this is that complicated because we do not want plugins to have threaded containers - this could get weird
	QSharedPointer<DkImageContainerT> pluginImage;

	if (!force) {
		
		if (mViewport->imageContainer()) {

			bool applyChanges = true;

			if (askForSaving) {

				DkMessageBox* msgBox = new DkMessageBox(
					QMessageBox::Question,
					tr("Closing Plugin"),
					tr("Apply plugin changes?"),
					QMessageBox::Yes | QMessageBox::No,
					this);
				msgBox->setDefaultButton(QMessageBox::Yes);
				msgBox->setObjectName("SavePluginChanges");

				int answer = msgBox->exec();
				applyChanges = (answer == QMessageBox::Accepted || answer == QMessageBox::Yes);
			}

			if (applyChanges)
				pluginImage = DkImageContainerT::fromImageContainer(vPlugin->runPlugin("", mViewport->imageContainer()));
		}
		else
			qDebug() << "[DkControlWidget] I cannot close a plugin if the image container is NULL";
	}

	setPluginWidget(vPlugin, true);
	plugin->setActive(false);		// handles states

	if (!force && pluginImage) {
		mViewport->setEditedImage(pluginImage);
		return false;
	}

	return true;
#else
	Q_UNUSED(askForSaving);
	Q_UNUSED(force);

	return false;
#endif // WITH_PLUGINS
}

bool DkControlWidget::applyPluginChanges(bool askForSaving) {

#ifdef WITH_PLUGINS
	QSharedPointer<DkPluginContainer> plugin = DkPluginManager::instance().getRunningPlugin();

	if (!plugin)
		return true;

	// does the plugin want to be closed on image changes?
	if (!plugin->plugin()->closesOnImageChange())
		return true;

	return closePlugin(askForSaving);
#else
	Q_UNUSED(askForSaving);

	return true;
#endif // WITH_PLUGINS
}

void DkControlWidget::setPluginWidget(DkViewPortInterface* pluginWidget, bool removeWidget) {

	mPluginViewport = pluginWidget->getViewPort();

	if (!mPluginViewport) {
		qDebug() << "cannot set plugin widget since the viewport is NULL" << pluginWidget;
		return;
	}

	if (!removeWidget) {
		mPluginViewport->setWorldMatrix(mViewport->getWorldMatrixPtr());
		mPluginViewport->setImgMatrix(mViewport->getImageMatrixPtr());
		mPluginViewport->updateImageContainer(mViewport->imageContainer());

		connect(mPluginViewport, SIGNAL(closePlugin(bool)), this, SLOT(closePlugin(bool)), Qt::UniqueConnection);
		connect(mPluginViewport, SIGNAL(loadFile(const QString&)), mViewport, SLOT(loadFile(const QString&)), Qt::UniqueConnection);
		connect(mPluginViewport, SIGNAL(loadImage(const QImage&)), mViewport, SLOT(setImage(const QImage&)), Qt::UniqueConnection);
		connect(mPluginViewport, SIGNAL(showInfo(const QString&)), this, SLOT(setInfo(const QString&)), Qt::UniqueConnection);
	}

	setAttribute(Qt::WA_TransparentForMouseEvents, !removeWidget && pluginWidget->hideHUD());
	
	if (pluginWidget->hideHUD() && !removeWidget) {
		setWidgetsVisible(false, false);
	}
	else if (pluginWidget->hideHUD())
		showWidgetsSettings();

	mViewport->setPaintWidget(dynamic_cast<QWidget*>(mPluginViewport), removeWidget);

	if (removeWidget) {
		mPluginViewport = 0;
	}
}

void DkControlWidget::updateImage(QSharedPointer<DkImageContainerT> imgC) {

	mImgC = imgC;

	if (mPluginViewport)
		mPluginViewport->updateImageContainer(imgC);

	if (!imgC)
		return;

	QSharedPointer<DkMetaDataT> metaData = imgC->getMetaData();

	QString dateString = metaData->getExifValue("DateTimeOriginal");
	mFileInfoLabel->updateInfo(imgC->filePath(), "", dateString, metaData->getRating());
	mFileInfoLabel->setEdited(imgC->isEdited());
	mCommentWidget->setMetaData(metaData); // reset
	updateRating(metaData->getRating());

	connect(imgC.get(), SIGNAL(imageUpdatedSignal()), this, SIGNAL(imageUpdatedSignal()));

}

void DkControlWidget::setInfo(const QString& msg, int time, int location) {

	if (location == bottom_left_label && mBottomLabel)
		mBottomLabel->setText(msg, time);
	else if (location == top_left_label && mBottomLeftLabel)
		mBottomLeftLabel->setText(msg, time);

	update();
}

void DkControlWidget::setInfoDelayed(const QString& msg, bool start, int delayTime) {

	if (start)
		mDelayedInfo->setInfo(msg, delayTime);
	else
		mDelayedInfo->stop();
}

void DkControlWidget::changeMetaDataPosition(int pos) {

	if (pos == DkFadeWidget::pos_west) {
		mHudLayout->addWidget(mMetaDataInfo, top_metadata, left_metadata, bottom_metadata-top_metadata, 1);	
	}
	else if (pos == DkFadeWidget::pos_east) {
		mHudLayout->addWidget(mMetaDataInfo, top_metadata, right_metadata, bottom_metadata-top_metadata, 1);	
	}
	else if (pos == DkFadeWidget::pos_north) {
		mHudLayout->addWidget(mMetaDataInfo, top_metadata, left_metadata, 1, hor_pos_end-2);	
	}
	else if (pos == DkFadeWidget::pos_south) {
		mHudLayout->addWidget(mMetaDataInfo, bottom_metadata, left_metadata, 1, hor_pos_end-2);	
	}
}

void DkControlWidget::changeThumbNailPosition(int pos) {

	if (pos == DkFadeWidget::pos_west) {
		mHudLayout->addWidget(mFilePreview, top_thumbs, left_thumbs, ver_pos_end, 1);	
	}
	else if (pos == DkFadeWidget::pos_east) {
		mHudLayout->addWidget(mFilePreview, top_thumbs, right_thumbs, ver_pos_end, 1);	
	}
	else if (pos == DkFadeWidget::pos_north) {
		mHudLayout->addWidget(mFilePreview, top_thumbs, left_thumbs, 1, hor_pos_end);	
	}
	else if (pos == DkFadeWidget::pos_south) {
		mHudLayout->addWidget(mFilePreview, bottom_thumbs, left_thumbs, 1, hor_pos_end);	
	}
	else 
		mFilePreview->hide();

}

void DkControlWidget::stopLabels() {

	mBottomLeftLabel->stop();
	mBottomLabel->stop();
	// TODO
	//mProgressBar->hide();

	switchWidget();
}

void DkControlWidget::settingsChanged() {

	if (mFileInfoLabel && mFileInfoLabel->isVisible()) {
		showFileInfo(false);	// just a hack but all states are preserved this way
		showFileInfo(true);
	}

}

void DkControlWidget::setTransforms(const QTransform* worldMatrix, const QTransform* imgMatrix) {

	mZoomWidget->getOverview()->setTransforms(worldMatrix, imgMatrix);
	mCropWidget->setTransforms(worldMatrix, imgMatrix);
}

void DkControlWidget::setImageRect(const QRectF* imgRect) {
	mCropWidget->setImageRect(imgRect);
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

	if (DkSettingsManager::param().slideShow().showPlayer &&
		fullscreen &&
		!mPlayer->isVisible())
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

	mEnterPos = event->pos();

	if (mFilePreview && mFilePreview->isVisible() && event->buttons() == Qt::MiddleButton) {

		QTimer* mImgTimer = mFilePreview->getMoveImageTimer();
		mImgTimer->start(1);

		// show icon
		mWheelButton->move(event->pos().x()-16, event->pos().y()-16);
		mWheelButton->show();
	}

	if (mPluginViewport)
		QCoreApplication::sendEvent(mPluginViewport, event);
	else
		QWidget::mousePressEvent(event);

}

void DkControlWidget::mouseReleaseEvent(QMouseEvent *event) {

	if (mFilePreview && mFilePreview->isVisible()) {
		mFilePreview->setCurrentDx(0);
		QTimer* mImgTimer = mFilePreview->getMoveImageTimer();
		mImgTimer->stop();
		mWheelButton->hide();
	}

	if (mPluginViewport)
		QCoreApplication::sendEvent(mPluginViewport, event);
	else
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

	if (mPluginViewport)
		QCoreApplication::sendEvent(mPluginViewport, event);
	else
		QWidget::mouseMoveEvent(event);
}



void DkControlWidget::keyPressEvent(QKeyEvent *event) {
	
	QWidget::keyPressEvent(event);
}

void DkControlWidget::keyReleaseEvent(QKeyEvent *event) {

	QWidget::keyReleaseEvent(event);
}

}
