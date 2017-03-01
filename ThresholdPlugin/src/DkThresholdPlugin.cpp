/*******************************************************************************************************
 DkThresholdPlugin.cpp
 Created on:	20.05.2014

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2014 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2014 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2014 Florian Kleber <florian@nomacs.org>

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

#include "DkThresholdPlugin.h"

#include <QMouseEvent>

namespace nmp {

/*-----------------------------------DkThresholdPlugin ---------------------------------------------*/

/**
*	Constructor
**/
DkThresholdPlugin::DkThresholdPlugin() {

	viewport = 0;
}

/**
*	Destructor
**/
DkThresholdPlugin::~DkThresholdPlugin() {

	if (viewport) {
		viewport->deleteLater();
		viewport = 0;
	}
}

/**
* Returns descriptive image
**/
QImage DkThresholdPlugin::image() const {

   return QImage(":/nomacsPluginThr/img/description.png");
}

bool DkThresholdPlugin::hideHUD() const {
	return false;
}

/**
* Main function: runs plugin based on its ID
* @param run ID
* @param current image in the Nomacs viewport
**/
QSharedPointer<nmc::DkImageContainer> DkThresholdPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC) const {

	//for a viewport plugin runID and imgC are null
	if (viewport && imgC) {

		DkThresholdViewPort* thresholdViewport = dynamic_cast<DkThresholdViewPort*>(viewport);

		QImage retImg;
		if (!thresholdViewport->isCanceled()) 
			imgC->setImage(thresholdViewport->getThresholdedImage(true), tr("Thresholded"));
		else {
			if (parent()) {
				nmc::DkBaseViewPort* viewport = dynamic_cast<nmc::DkBaseViewPort*>(parent());
				if (viewport) 
					viewport->setImage(thresholdViewport->getOriginalImage());
			}
		}

		viewport->setVisible(false);
	}

	return imgC;
};

/**
* returns ThresholdViewPort
**/
nmc::DkPluginViewPort* DkThresholdPlugin::getViewPort() {

	if (!viewport)
		viewport = new DkThresholdViewPort();
	return viewport;
}

void DkThresholdPlugin::deleteViewPort() {

	if (viewport) {
		viewport->deleteLater();
		viewport = 0;
	}
}

/*-----------------------------------DkThresholdViewPort ---------------------------------------------*/

DkThresholdViewPort::DkThresholdViewPort(QWidget* parent, Qt::WindowFlags flags) : DkPluginViewPort(parent, flags) {

	init();
}

DkThresholdViewPort::~DkThresholdViewPort() {

	// acitive deletion since the MainWindow takes ownership...
	// if we have issues with this, we could disconnect all signals between viewport and toolbar too
	// however, then we have lot's of toolbars in memory if the user opens the plugin again and again
	if (thresholdToolbar) {
		delete thresholdToolbar;
		thresholdToolbar = 0;
	}
}

void DkThresholdViewPort::init() {

	panning = false;
	cancelTriggered = false;
	defaultCursor = Qt::ArrowCursor;
	setCursor(defaultCursor);
	thrChannel = channel_gray;
	thrEnabled = true;
	thrValue = 128;
	thrValueUpper = thrValue;	
	origImg = QImage();
	origImgSet = false;

	thresholdToolbar = new DkThresholdToolBar(tr("Threshold Toolbar"), this);

	connect(thresholdToolbar, SIGNAL(thrChannelSignal(int)), this, SLOT(setThrChannel(int)));
	connect(thresholdToolbar, SIGNAL(thrValSignal(int)), this, SLOT(setThrValue(int)));
	connect(thresholdToolbar, SIGNAL(thrValUpperSignal(int)), this, SLOT(setThrValueUpper(int)));
	connect(thresholdToolbar, SIGNAL(calculateAutoThresholdSignal()), this, SLOT(calculateAutoThreshold()));
	connect(thresholdToolbar, SIGNAL(thrEnabledSignal(bool)), this, SLOT(setThrEnabled(bool)));
	connect(thresholdToolbar, SIGNAL(panSignal(bool)), this, SLOT(setPanning(bool)));
	connect(thresholdToolbar, SIGNAL(cancelSignal()), this, SLOT(discardChangesAndClose()));
	connect(thresholdToolbar, SIGNAL(applySignal()), this, SLOT(applyChangesAndClose()));
}

void DkThresholdViewPort::mousePressEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->buttons() == Qt::LeftButton && 
		(event->modifiers() == nmc::DkSettingsManager::param().global().altMod || panning)) {
		setCursor(Qt::ClosedHandCursor);
		event->setModifiers(Qt::NoModifier);	// we want a 'normal' action in the viewport
		event->ignore();
		return;
	}

	// no propagation
}

void DkThresholdViewPort::mouseMoveEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->modifiers() == nmc::DkSettingsManager::param().global().altMod ||
		panning) {

		event->setModifiers(Qt::NoModifier);
		event->ignore();
		update();
		return;
	}
}

void DkThresholdViewPort::mouseReleaseEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->modifiers() == nmc::DkSettingsManager::param().global().altMod || panning) {
		setCursor(defaultCursor);
		event->setModifiers(Qt::NoModifier);
		event->ignore();
		return;
	}
}

void DkThresholdViewPort::paintEvent(QPaintEvent *event) {

	DkPluginViewPort::paintEvent(event);
}

QImage DkThresholdViewPort::getOriginalImage() {
	
	return origImg;
}

QImage DkThresholdViewPort::getThresholdedImage(bool thrEnabled) {


	if(parent() && !origImgSet) {
		nmc::DkBaseViewPort* viewport = dynamic_cast<nmc::DkBaseViewPort*>(parent());
		if (viewport) {
			origImg = viewport->getImage();
			origImgSet = true;
		}
	}

	QImage thrImage = QImage(origImg);

	switch (thrImage.depth()) {
		case 8:
			thrImage = QImage(origImg.size(), QImage::Format_RGB32);

			for (int y = 0; y < thrImage.height(); y++) {
				for (int x = 0; x < thrImage.width(); x++) {
					int gray =  (thrEnabled) ? ((thrValue <= qRed(origImg.pixel(x, y)) && qRed(origImg.pixel(x, y)) <= thrValueUpper) ? 255 : 0)  :  qRed(origImg.pixel(x, y));
					thrImage.setPixel(x, y, qRgb(gray, gray, gray));
				}
			}
			break;
		case 24:
			for (int y = 0; y < thrImage.height(); y++) {
				for (int x = 0; x < thrImage.width(); x++) {
					int pixel = thrImage.pixel(x, y);
					int gray = 0;
					switch (thrChannel) {
						case channel_gray:
							gray =  (thrEnabled) ? ((thrValue <= qGray(pixel) && qGray(pixel) <= thrValueUpper) ? 255 : 0) : qGray(pixel);
							break;
						case channel_red:
							gray =  (thrEnabled) ? ((thrValue <= qRed(pixel) && qRed(pixel) <= thrValueUpper) ? 255 : 0) : qRed(pixel);
							break;
						case channel_blue:
							gray =  (thrEnabled) ? ((thrValue <= qBlue(pixel) && qBlue(pixel) <= thrValueUpper) ? 255 : 0) : qBlue(pixel);
							break;
						case channel_green:
							gray =  (thrEnabled) ? ((thrValue <= qGreen(pixel) && qGreen(pixel) <= thrValueUpper) ? 255 : 0) : qGreen(pixel);
							break;
					}
					thrImage.setPixel(x, y, qRgb(gray, gray, gray));
				}
			}
			break;
		case 32:
			for (int y = 0; y < thrImage.height(); y++) {
				for (int x = 0; x < thrImage.width(); x++) {
					int pixel = thrImage.pixel(x, y);
					int gray = 0;
					switch (thrChannel) {
						case channel_gray:
							gray =  (thrEnabled) ? ((thrValue <= qGray(pixel) && qGray(pixel) <= thrValueUpper) ? 255 : 0) : qGray(pixel);
							break;
						case channel_red:
							gray =  (thrEnabled) ? ((thrValue <= qRed(pixel) && qRed(pixel) <= thrValueUpper) ? 255 : 0) : qRed(pixel);
							break;
						case channel_blue:
							gray =  (thrEnabled) ? ((thrValue <= qBlue(pixel) && qBlue(pixel) <= thrValueUpper) ? 255 : 0) : qBlue(pixel);
							break;
						case channel_green:
							gray =  (thrEnabled) ? ((thrValue <= qGreen(pixel) && qGreen(pixel) <= thrValueUpper) ? 255 : 0) : qGreen(pixel);
							break;
					}
					int alpha = qAlpha(pixel);
					thrImage.setPixel(x, y, qRgba(gray, gray, gray, alpha));
				}
			}
			break;
	}

	if (origImg.depth() == 8) {

		QImage outImage = QImage(origImg.size(), QImage::Format_RGB32);

		QPainter painter(&outImage);

		painter.drawImage(thrImage.rect(), thrImage);

		painter.end();

		return outImage;
	}

	QImage outImage = QImage(origImg);
	QPainter painter(&outImage);

	painter.drawImage(thrImage.rect(), thrImage);

	painter.end();
			
	return outImage;
}

void DkThresholdViewPort::setThrValue(int val) {

	this->thrValue = val;
	if (parent()) {
		nmc::DkBaseViewPort* viewport = dynamic_cast<nmc::DkBaseViewPort*>(parent());
		if (viewport) viewport->setImage(getThresholdedImage(this->thrEnabled));
	}
	this->repaint();
}

void DkThresholdViewPort::setThrValueUpper(int val) {

	this->thrValueUpper = val;
	if (parent()) {
		nmc::DkBaseViewPort* viewport = dynamic_cast<nmc::DkBaseViewPort*>(parent());
		if (viewport) viewport->setImage(getThresholdedImage(this->thrEnabled));
	}
	this->repaint();
}

void DkThresholdViewPort::setThrEnabled(bool enabled) {

	this->thrEnabled = enabled;
	if (parent()) {
		nmc::DkBaseViewPort* viewport = dynamic_cast<nmc::DkBaseViewPort*>(parent());
		if (viewport) viewport->setImage(getThresholdedImage(this->thrEnabled));
	}
	this->repaint();
}

void DkThresholdViewPort::setThrChannel(int val) {

	this->thrChannel = val;
	if (parent()) {
		nmc::DkBaseViewPort* viewport = dynamic_cast<nmc::DkBaseViewPort*>(parent());
		if (viewport) viewport->setImage(getThresholdedImage(this->thrEnabled));
	}
	this->repaint();
}

void DkThresholdViewPort::calculateAutoThreshold() {

	double sumPixel = 0;

	if (origImg.depth() == 8) {
		for (int y = 0; y < origImg.height(); y++) {
			for (int x = 0; x < origImg.width(); x++) {
				sumPixel += qRed(origImg.pixel(x, y));
			}
		}
	}
	else if(origImg.depth() == 24 || origImg.depth() == 32) {
		for (int y = 0; y < origImg.height(); y++) {
			for (int x = 0; x < origImg.width(); x++) {
				int pixel = origImg.pixel(x, y);
				switch (thrChannel) {
					case channel_gray:
						sumPixel += qGray(origImg.pixel(x, y));
						break;
					case channel_red:
						sumPixel += qRed(origImg.pixel(x, y));
						break;
					case channel_blue:
						sumPixel += qBlue(origImg.pixel(x, y));
						break;
					case channel_green:
						sumPixel += qGreen(origImg.pixel(x, y));
						break;
				}
			}
		}
	}

	sumPixel /= (origImg.height() * origImg.width());

	thresholdToolbar->setThrValue(qRound(sumPixel));
}

void DkThresholdViewPort::setPanning(bool checked) {

	this->panning = checked;
	if(checked) defaultCursor = Qt::OpenHandCursor;
	else defaultCursor = Qt::CrossCursor;
	setCursor(defaultCursor);
}

void DkThresholdViewPort::applyChangesAndClose() {

	cancelTriggered = false;
	emit closePlugin();
}

void DkThresholdViewPort::discardChangesAndClose() {

	cancelTriggered = true;
	if(parent() && origImgSet) {
		nmc::DkBaseViewPort* viewport = dynamic_cast<nmc::DkBaseViewPort*>(parent());
		if (viewport) viewport->setImage(origImg);
	}
	emit closePlugin();
}

bool DkThresholdViewPort::isCanceled() {
	return cancelTriggered;
}

void DkThresholdViewPort::setVisible(bool visible) {

	if(parent()) {
		nmc::DkBaseViewPort* viewport = dynamic_cast<nmc::DkBaseViewPort*>(parent());
		if (viewport) {
			if (viewport->getImage().depth() == 8 && thresholdToolbar)
				thresholdToolbar->disableColorChannels();
		}
	}
		
		
	if (thresholdToolbar) emit showToolbar(thresholdToolbar, visible);
	
	DkPluginViewPort::setVisible(visible);
}

/*-----------------------------------DkThresholdToolBar ---------------------------------------------*/
DkThresholdToolBar::DkThresholdToolBar(const QString & title, QWidget * parent /* = 0 */) : QToolBar(title, parent) {

	createIcons();
	createLayout();
	QMetaObject::connectSlotsByName(this);

	setObjectName("ThresholdToolBar");
}

DkThresholdToolBar::~DkThresholdToolBar() {

}

void DkThresholdToolBar::createIcons() {

	// create icons
	icons.resize(icons_end);

	icons[apply_icon]	= nmc::DkImage::loadIcon(":/nomacs/img/save.svg");
	icons[cancel_icon]	= nmc::DkImage::loadIcon(":/nomacs/img/cancel.svg");
	icons[pan_icon]		= nmc::DkImage::loadIcon(":/nomacs/img/pan.svg");
	icons[pan_icon].addPixmap(nmc::DkImage::loadIcon(":/nomacs/img/pan_checked.svg"), QIcon::Normal, QIcon::On);
}

void DkThresholdToolBar::createLayout() {

	QList<QKeySequence> enterSc;
	enterSc.append(QKeySequence(Qt::Key_Enter));
	enterSc.append(QKeySequence(Qt::Key_Return));

	QAction* applyAction = new QAction(icons[apply_icon], tr("Apply (ENTER)"), this);
	applyAction->setShortcuts(enterSc);
	applyAction->setObjectName("applyAction");

	QAction* cancelAction = new QAction(icons[cancel_icon], tr("Cancel (ESC)"), this);
	cancelAction->setShortcut(QKeySequence(Qt::Key_Escape));
	cancelAction->setObjectName("cancelAction");

	panAction = new QAction(icons[pan_icon], tr("Pan"), this);
	panAction->setShortcut(QKeySequence(Qt::Key_P));
	panAction->setObjectName("panAction");
	panAction->setCheckable(true);
	panAction->setChecked(false);

	//imgC channel
	QStringList thrChannels;
	thrChannels.append(QT_TRANSLATE_NOOP("nmc::DkThresholdToolBar", "Gray"));
	thrChannels.append(QT_TRANSLATE_NOOP("nmc::DkThresholdToolBar", "Red"));
	thrChannels.append(QT_TRANSLATE_NOOP("nmc::DkThresholdToolBar", "Green"));
	thrChannels.append(QT_TRANSLATE_NOOP("nmc::DkThresholdToolBar", "Blue"));

	thrChannelBox = new QComboBox(this);
	thrChannelBoxContents = new QListWidget(thrChannelBox);
	thrChannelBoxContents->hide();
	thrChannelBox->setModel(thrChannelBoxContents->model());
	thrChannelBoxContents->addItems(thrChannels);

	//thrChannelBox->addItems(thrChannels);
	thrChannelBox->setObjectName("thrChannelBox");
	thrChannelBox->setToolTip(tr("Thresholding channel"));
	thrChannelBox->setStatusTip(thrChannelBox->toolTip());

	//threshold value
	thrValBox = new QSpinBox(this);
	thrValBox->setObjectName("thrValBox");
	thrValBox->setMinimum(0);
	thrValBox->setMaximum(255);
	thrValBox->setToolTip(tr("Thresholding value"));
	thrValBox->setStatusTip(thrValBox->toolTip());

	thrValSlider = new QSlider(this);
	thrValSlider->setMinimum(0);
	thrValSlider->setMaximum(255);
	thrValSlider->setTickInterval(64);
	thrValSlider->setOrientation(Qt::Horizontal);
	thrValSlider->setTickPosition(QSlider::TicksBelow);

	thrValSlider->setStyleSheet(
		QString("QSlider::groove:horizontal {border: 1px solid #999999; height: 4px; margin: 2px 0;")
		+ QString("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3c3c3c, stop:1 #c8c8c8) ")
		+ QString(";} ")
		+ QString("QSlider::handle:horizontal {background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #d2d2d2, stop:1 #e6e6e6); border: 1px solid #5c5c5c; width: 6px; margin:-4px 0px -6px 0px ;border-radius: 3px;}"));

	connect(thrValBox, SIGNAL(valueChanged(int)),thrValSlider, SLOT(setValue(int)));
    connect(thrValSlider, SIGNAL(valueChanged(int)),thrValBox, SLOT(setValue(int)));

	//threshold upper value / for ranged threshold
	thrValUpperBox = new QSpinBox(this);
	thrValUpperBox->setObjectName("thrValUpperBox");
	thrValUpperBox->setMinimum(0);
	thrValUpperBox->setMaximum(255);
	thrValUpperBox->setToolTip(tr("Threshold upper value"));
	thrValUpperBox->setStatusTip(thrValUpperBox->toolTip());
	
	connect(thrValBox, SIGNAL(valueChanged(int)), this, SLOT(setBoxMinimumValue(int)));

	//auto threshold
	autoThrButton = new QPushButton(tr("Auto"), this);
	autoThrButton->setObjectName("autoThrButton");
	autoThrButton->setToolTip(tr("Automatic threshold calculation"));
	autoThrButton->setStatusTip(autoThrButton->toolTip());

	//display original image
	thrEnableBox = new QCheckBox(tr("Show original"), this);
	thrEnableBox->setObjectName("thrEnableBox");
	thrEnableBox->setCheckState(Qt::Unchecked);
	thrEnableBox->setToolTip(tr("Display original image"));
	thrEnableBox->setStatusTip(thrEnableBox->toolTip());

	addAction(applyAction);
	addAction(cancelAction);
	addSeparator();
	addAction(panAction);
	addSeparator();
	addWidget(thrChannelBox);
	addWidget(thrValBox);
	addWidget(thrValSlider);
	addWidget(thrValUpperBox);
	addWidget(autoThrButton);
	addWidget(thrEnableBox);
}

void DkThresholdToolBar::disableColorChannels() {

	for (int index = 1; index <= 3; index++) {
		QListWidgetItem *item = thrChannelBoxContents->item(index);
		item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
	}
}

void DkThresholdToolBar::setVisible(bool visible) {

	if (visible) {
		thrValBox->setValue(128);
		thrValUpperBox->setValue(255);
		panAction->setChecked(false);
	}

	QToolBar::setVisible(visible);
}

void DkThresholdToolBar::on_applyAction_triggered() {
	emit applySignal();
}

void DkThresholdToolBar::on_cancelAction_triggered() {
	emit cancelSignal();
}

void DkThresholdToolBar::on_panAction_toggled(bool checked) {

	emit panSignal(checked);
}

void DkThresholdToolBar::on_thrValBox_valueChanged(int val) {

	emit thrValSignal(val);
}

void DkThresholdToolBar::on_thrChannelBox_currentIndexChanged(int val) {

	emit thrChannelSignal(val);
}

void DkThresholdToolBar::on_thrValUpperBox_valueChanged(int val) {

	emit thrValUpperSignal(val);
}

void DkThresholdToolBar::on_autoThrButton_clicked() {

	emit calculateAutoThresholdSignal();
}

void DkThresholdToolBar::setThrValue(int val) {

	thrValBox->setValue(val);
}

void DkThresholdToolBar::setBoxMinimumValue(int val) {

	thrValUpperBox->setMinimum(val);
}

void DkThresholdToolBar::on_thrEnableBox_stateChanged(int val) {

	bool enabled = true;

	if (val == Qt::Checked) enabled = false;

	emit thrEnabledSignal(enabled);
}

};
