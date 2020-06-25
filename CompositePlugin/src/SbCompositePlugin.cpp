/*******************************************************************************************************
 SbCompositePlugin.cpp

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2015 #YOUR_NAME

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

#include "SbCompositePlugin.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#pragma warning(pop)		// no warnings from includes - end

 /*******************************************************************************************************
  * SbCompositePlugin	- enter the plugin class name (e.g. DkPageExtractionPlugin)
  * Simon Brenner		- your name/pseudonym whatever
  * #DATE				- today...
  * #DESCRIPTION		- describe your plugin in one sentence
  * #MENU_NAME			- a user friendly name (e.g. Flip Image)
  * #MENU_STATUS_TIPP	- status tip of your plugin
  * #RUN_ID_1			- generate an ID using: GUID without hyphens generated at http://www.guidgenerator.com/
  * ID_ACTION1			- your action name (e.g. id_flip_horizontally)
  * #ACTION_NAME1		- your action name (e.g. Flip Horizotally - user friendly!)
  * #ACTION_TIPP1		- your action status tip (e.g. Flips an image horizontally - user friendly!)
  *******************************************************************************************************/

namespace nmc {

/**
* Returns descriptive image for every ID
* @param plugin ID
**/
QImage SbCompositePlugin::image() const {

	return QImage(":/CompositePlugin/img/description.png");
};


/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> SbCompositePlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC) const {

	Q_UNUSED(runID);
	
	qDebug() << "called runPlugin";
	if (!imgC)
		qDebug() << "imgC was null";

	if (viewport && imgC) {
		if (apply) {
			imgC->setImage(buildComposite(), tr("composite"));	//put input image names here?
		}
		viewport->setVisible(false);
		dockWidget->setVisible(false);
	}

	return imgC;
}

bool SbCompositePlugin::createViewPort(QWidget* parent)
{
	qDebug() << "called createViewPort";

	if (!viewport) {
		viewport = new SbViewPort(parent);
		connect(viewport, SIGNAL(gotImage()), this, SLOT(onViewportGotImage()));
	}
	if (!dockWidget) {
		buildUI();
	}
	setVisible(true);
	return false;
}

DkPluginViewPort* SbCompositePlugin::getViewPort()
{
	qDebug() << "called getViewPort";
	return viewport;
}

void SbCompositePlugin::setVisible(bool visible)
{
	qDebug() << "called setVisible";
	if (viewport)
		viewport->setVisible(visible);
	if(dockWidget)
		dockWidget->setVisible(visible);
	
	if (!visible) {
		//cleanup
		for (SbChannelWidget* cw : channelWidgets) {
			cw->setImg();
		}
		for (int i = 0; i < 3; i++) {
			channels[i] = cv::Mat();
		}
	}
}


//##############################################################


void SbCompositePlugin::buildUI()
{
	mainWidget = new QWidget();

	outerLayout = new QBoxLayout(QBoxLayout::Direction::TopToBottom);	//default
	outerLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

	for (int i = 0; i < 3; i++) {
		SbChannelWidget* cw = new SbChannelWidget(static_cast<SbChannelWidget::Channel>(i), mainWidget);
		channelWidgets.append(cw);
	}

	for(SbChannelWidget * s: channelWidgets) 
	{
		connect(s, SIGNAL(imageChanged(int)), this, SLOT(onImageChanged(int)));
		connect(s, SIGNAL(newAlpha(QImage)), this, SLOT(onNewAlpha(QImage)));
		outerLayout->addWidget(s);
	}

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	QPushButton* applyButton = new QPushButton("apply");
	applyButton->setIcon(QIcon(":/CompositePlugin/img/description.png"));
	applyButton->setIconSize(QSize(24, 24));
	connect(applyButton, SIGNAL(released()), this, SLOT(onPushButtonApply()));
	QPushButton* cancelButton = new QPushButton("cancel");
	cancelButton->setIcon(QIcon(":/CompositePlugin/img/close.svg"));
	cancelButton->setIconSize(QSize(24, 24));
	connect(cancelButton, SIGNAL(released()), this, SLOT(onPushButtonCancel()));
	buttonLayout->addWidget(applyButton);
	buttonLayout->addWidget(cancelButton);
	buttonLayout->setAlignment(Qt::AlignBottom);
	outerLayout->addItem(buttonLayout);
	outerLayout->addStretch();

	mainWidget->setLayout(outerLayout);


	// dock widget & scroll area setup
	dockWidget = new SbCompositeDockWidget(tr("Composite Plugin"));
	QSettings settings;
	Qt::DockWidgetArea dockLocation = static_cast<Qt::DockWidgetArea>(settings.value("sbCompWidgetLocation", Qt::LeftDockWidgetArea).toInt());

	scrollArea = new QScrollArea(dockWidget);
	scrollArea->setMinimumSize(SbChannelWidget::THUMB_MAX_SIZE+50, SbChannelWidget::THUMB_MAX_SIZE+100);	//very dirty
	scrollArea->setWidget(mainWidget);
	scrollArea->setWidgetResizable(true);
	
	dockWidget->setWidget(scrollArea);

	connect(dockWidget, SIGNAL(closed()), this, SLOT(onDockWidgetClose()));
	connect(dockWidget, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(onDockLocationChanged(Qt::DockWidgetArea)));
	
	QMainWindow* mainWindow = getMainWindow();
	if (mainWindow)
		mainWindow->addDockWidget(dockLocation, dockWidget);
}

QImage SbCompositePlugin::buildComposite() const
{
	cv::Mat composite;
	if (alpha.empty()) {
		cv::merge(channels, 3, composite);
	}
	else {
		cv::Mat bgra[4] = { channels[2], channels[1], channels[0], alpha };	//when merging 4 channels, blue and red are reversed again.. why..
		cv::merge(bgra, 4, composite);
	}
	return DkImage::mat2QImage(composite);
}

void SbCompositePlugin::onImageChanged(int c) {
	qDebug() << "image changed in channel " << c;
	channels[c] = channelWidgets[c]->getImg();

	//set all channels with non-matching sizes to zeros of matching size
	for (int i = 0; i < 3; i++) {
		if (i != c) {
			if (!(channels[i].rows == channels[c].rows &&
				channels[i].cols == channels[c].cols)) {
				channels[i] = cv::Mat::zeros(channels[c].rows, channels[c].cols, channels[c].type());
				channelWidgets[i]->setImg();
			}
		}
	}
	viewport->loadImage(buildComposite());
}

void SbCompositePlugin::onNewAlpha(QImage _alpha)
{

	if (_alpha == QImage()) {
		qDebug() << "got empty alpha";
		alpha = cv::Mat();
	}
	else {
		qDebug() << "got full alpha";
		alpha = DkImage::qImage2Mat(_alpha);
		//currently it seems like qImage2Mat converts a single-channel QImage to a multi-channel Mat. so..
		if (alpha.channels() == 4)
			cv::cvtColor(alpha, alpha, CV_RGBA2GRAY);
		else if (alpha.channels() == 3)
			cv::cvtColor(alpha, alpha, CV_RGB2GRAY);
	}
}

void SbCompositePlugin::onViewportGotImage()
{
	//put that image into the three channels
	QSharedPointer<DkImageContainerT> imgC = viewport->getImgC();
	QImage newImage = imgC->image();
	cv::Mat rgb = DkImage::qImage2Mat(newImage);
	if (rgb.channels() >= 3) {
		std::vector<cv::Mat> c;
		split(rgb, c);
		for (int i = 0; i < 3; i++) {
			channels[i] = c[2 - i];	//channels are BGR.. why?
			channelWidgets[i]->setImg(c[2 - i], imgC->fileName());
		}
		if (rgb.channels() >= 4) {
			alpha = c[3];
		}
	}
	//else? i don't think this can happen..
	emit viewport->loadImage(buildComposite());
}

void SbCompositePlugin::onDockWidgetClose()
{
	emit viewport->closePlugin(true);
}

void SbCompositePlugin::onDockLocationChanged(Qt::DockWidgetArea a)
{
	//vertical layout
	if (a == Qt::DockWidgetArea::LeftDockWidgetArea || a == Qt::DockWidgetArea::RightDockWidgetArea) {
		scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
		outerLayout->setDirection(QBoxLayout::Direction::TopToBottom);
	}
	//horizontal layout
	else {
		scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
		outerLayout->setDirection(QBoxLayout::Direction::LeftToRight);
	}
}

void SbCompositePlugin::onPushButtonApply()
{
	apply = true;
	emit viewport->closePlugin(false);
}

void SbCompositePlugin::onPushButtonCancel()
{
	apply = false;
	emit viewport->closePlugin(false);
}

};

