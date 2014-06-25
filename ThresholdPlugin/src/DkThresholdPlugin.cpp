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

namespace nmc {

/*-----------------------------------DkThresholdPlugin ---------------------------------------------*/

DkSettings::Display& DkSettings::display = DkSettings::getDisplaySettings();
DkSettings::Global& DkSettings::global = DkSettings::getGlobalSettings();

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
* Returns unique ID for the generated dll
**/
QString DkThresholdPlugin::pluginID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QString("70e2fbe0c913462e846bb91da201ceca");
};


/**
* Returns plugin name
**/
QString DkThresholdPlugin::pluginName() const {

   return "Simple Threshold";
};

/**
* Returns long description
**/
QString DkThresholdPlugin::pluginDescription() const {

   return "<b>Created by:</b> Tim Jerman<br><b>Modified:</b> June 2014<br><b>Description:</b> Fast threshold selection for color and grayscale images. If wanted a ranged threshold can be created by changing the upper threshold value. Threshold can be applied to each of the color channels or to the luminance channel.";
};

/**
* Returns descriptive image
**/
QImage DkThresholdPlugin::pluginDescriptionImage() const {

   return QImage(":/nomacsPluginThr/img/description.png");
};

/**
* Returns plugin version
**/
QString DkThresholdPlugin::pluginVersion() const {

   return "1.0.0";
};

/**
* Returns unique IDs for every plugin in this dll
* plugin can have more the one functionality that are triggered in the menu
* runID differes from pluginID
* viewport plugins can have only one runID and one functionality bound to it
**/
QStringList DkThresholdPlugin::runID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QStringList() << "e46b000ca4804d26b440a7a07c6d9664";
};

/**
* Returns plugin name for every run ID
* @param run ID
**/
QString DkThresholdPlugin::pluginMenuName(const QString &runID) const {

	if (runID=="e46b000ca4804d26b440a7a07c6d9664") return "Threshold on image";
	return "Wrong GUID!";
};

/**
* Returns short description for status tip for every ID
* @param plugin ID
**/
QString DkThresholdPlugin::pluginStatusTip(const QString &runID) const {

	if (runID=="e46b000ca4804d26b440a7a07c6d9664") return "Threshold on image with colored brush";
	return "Wrong GUID!";
};

/**
* Main function: runs plugin based on its ID
* @param run ID
* @param current image in the Nomacs viewport
**/
QImage DkThresholdPlugin::runPlugin(const QString &runID, const QImage &image) const {

	//for a viewport plugin runID and image are null
	if (viewport) {

		DkThresholdViewPort* thresholdViewport = dynamic_cast<DkThresholdViewPort*>(viewport);

		QImage retImg = QImage();
		if (!thresholdViewport->isCanceled()) retImg = thresholdViewport->getThresholdedImage();

		viewport->setVisible(false);

		return retImg;
	}

	return image;
};

/**
* returns ThresholdViewPort
**/
DkPluginViewPort* DkThresholdPlugin::getViewPort() {

	if (!viewport) {
		viewport = new DkThresholdViewPort();
		connect(viewport, SIGNAL(destroyed()), this, SLOT(viewportDestroyed()));
	}
	return viewport;
}

/**
* sets the viewport pointer to NULL after the viewport is destroyed
**/
void DkThresholdPlugin::viewportDestroyed() {

	viewport = 0;
}

void DkThresholdPlugin::deleteViewPort() {

	if (viewport) {
		viewport->deleteLater();
	}
}

/* macro for exporting plugin */
Q_EXPORT_PLUGIN2("com.nomacs.ImageLounge.DkThresholdPlugin/1.0", DkThresholdPlugin)


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

	thresholdToolbar = new DkThresholdToolBar(tr("Threshold Toolbar"), this);

	connect(thresholdToolbar, SIGNAL(thrChannelSignal(int)), this, SLOT(setThrChannel(int)));
	connect(thresholdToolbar, SIGNAL(thrValSignal(int)), this, SLOT(setThrValue(int)));
	connect(thresholdToolbar, SIGNAL(thrValUpperSignal(int)), this, SLOT(setThrValueUpper(int)));
	connect(thresholdToolbar, SIGNAL(calculateAutoThresholdSignal()), this, SLOT(calculateAutoThreshold()));
	connect(thresholdToolbar, SIGNAL(thrEnabledSignal(bool)), this, SLOT(setThrEnabled(bool)));
	connect(thresholdToolbar, SIGNAL(panSignal(bool)), this, SLOT(setPanning(bool)));
	connect(thresholdToolbar, SIGNAL(cancelSignal()), this, SLOT(discardChangesAndClose()));
	connect(thresholdToolbar, SIGNAL(applySignal()), this, SLOT(applyChangesAndClose()));

	DkPluginViewPort::init();
}

void DkThresholdViewPort::mousePressEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->buttons() == Qt::LeftButton && 
		(event->modifiers() == DkSettings::global.altMod || panning)) {
		setCursor(Qt::ClosedHandCursor);
		event->setModifiers(Qt::NoModifier);	// we want a 'normal' action in the viewport
		event->ignore();
		return;
	}

	// no propagation
}

void DkThresholdViewPort::mouseMoveEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->modifiers() == DkSettings::global.altMod ||
		panning) {

		event->setModifiers(Qt::NoModifier);
		event->ignore();
		update();
		return;
	}
}

void DkThresholdViewPort::mouseReleaseEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->modifiers() == DkSettings::global.altMod || panning) {
		setCursor(defaultCursor);
		event->setModifiers(Qt::NoModifier);
		event->ignore();
		return;
	}
}

void DkThresholdViewPort::paintEvent(QPaintEvent *event) {

	QImage thrImage = QImage();
	QRect imgRect = QRect();

	if(parent()) {
		DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());
		if (viewport) {

			imgRect = viewport->getImage().rect();
			thrImage = QImage(viewport->getImage());
			QImage viewportImage;

			//if (thrEnabled) {

				switch (thrImage.depth()) {
					case 8:
						thrImage = QImage(viewport->getImage().size(), QImage::Format_RGB32);
						viewportImage = QImage(viewport->getImage());

						for (int y = 0; y < thrImage.height(); y++) {
							for (int x = 0; x < thrImage.width(); x++) {
								int gray =  (thrEnabled) ? ((thrValue <= qRed(viewportImage.pixel(x, y)) && qRed(viewportImage.pixel(x, y)) <= thrValueUpper) ? 255 : 0)  :  qRed(viewportImage.pixel(x, y));
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
			//}
		}
	}

	QPainter painter(this);
	
	if (worldMatrix)
		painter.setWorldTransform((*imgMatrix) * (*worldMatrix));

	painter.drawImage(imgRect, thrImage);

	painter.end();
	
	DkPluginViewPort::paintEvent(event);
}

QImage DkThresholdViewPort::getThresholdedImage() {

	if(parent()) {
		DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());
		if (viewport) {

			QImage img = viewport->getImage();
			QImage thrImage = QImage(img);

			switch (thrImage.depth()) {
				case 8:
					thrImage = QImage(viewport->getImage().size(), QImage::Format_RGB32);

					for (int y = 0; y < thrImage.height(); y++) {
						for (int x = 0; x < thrImage.width(); x++) {
							int gray =  (thrValue <= qRed(img.pixel(x, y)) && qRed(img.pixel(x, y)) <= thrValueUpper) ? 255 : 0;
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
									gray =  (thrValue <= qGray(pixel) && qGray(pixel) <= thrValueUpper) ? 255 : 0;
									break;
								case channel_red:
									gray =  (thrValue <= qRed(pixel) && qRed(pixel) <= thrValueUpper) ? 255 : 0;
									break;
								case channel_blue:
									gray =  (thrValue <= qBlue(pixel) && qBlue(pixel) <= thrValueUpper) ? 255 : 0;
									break;
								case channel_green:
									gray =  (thrValue <= qGreen(pixel) && qGreen(pixel) <= thrValueUpper) ? 255 : 0;
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
									gray =  (thrValue <= qGray(pixel) && qGray(pixel) <= thrValueUpper) ? 255 : 0;
									break;
								case channel_red:
									gray =  (thrValue <= qRed(pixel) && qRed(pixel) <= thrValueUpper) ? 255 : 0;
									break;
								case channel_blue:
									gray =  (thrValue <= qBlue(pixel) && qBlue(pixel) <= thrValueUpper) ? 255 : 0;
									break;
								case channel_green:
									gray =  (thrValue <= qGreen(pixel) && qGreen(pixel) <= thrValueUpper) ? 255 : 0;
									break;
							}
							int alpha = qAlpha(pixel);
							thrImage.setPixel(x, y, qRgba(gray, gray, gray, alpha));
						}
					}
					break;
			}

			if (img.depth() == 8) {

				QImage outImage = QImage(viewport->getImage().size(), QImage::Format_RGB32);

				QPainter painter(&outImage);

				//if (worldMatrix)
					//painter.setWorldTransform(*worldMatrix);

				painter.drawImage(thrImage.rect(), thrImage);

				painter.end();

				return outImage;
			}

			QPainter painter(&img);

			//if (worldMatrix)
				//painter.setWorldTransform(*worldMatrix);

			painter.drawImage(thrImage.rect(), thrImage);

			painter.end();
			
			return img;
		}
	}
	
	return QImage();
}

void DkThresholdViewPort::setThrValue(int val) {

	this->thrValue = val;
	this->repaint();
}

void DkThresholdViewPort::setThrValueUpper(int val) {

	this->thrValueUpper = val;
	this->repaint();
}

void DkThresholdViewPort::setThrEnabled(bool enabled) {

	this->thrEnabled = enabled;
	this->repaint();
}

void DkThresholdViewPort::setThrChannel(int val) {

	this->thrChannel = val;
	this->repaint();
}

void DkThresholdViewPort::calculateAutoThreshold() {

	double sumPixel = 0;

	if(parent()) {
		DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());
		if (viewport) {

			QImage img = viewport->getImage();		

			if (img.depth() == 8) {
				for (int y = 0; y < img.height(); y++) {
					for (int x = 0; x < img.width(); x++) {
						sumPixel += qRed(img.pixel(x, y));
					}
				}
			}
			else if(img.depth() == 24 || img.depth() == 32) {
				for (int y = 0; y < img.height(); y++) {
					for (int x = 0; x < img.width(); x++) {
						int pixel = img.pixel(x, y);
						switch (thrChannel) {
							case channel_gray:
								sumPixel += qGray(img.pixel(x, y));
								break;
							case channel_red:
								sumPixel += qRed(img.pixel(x, y));
								break;
							case channel_blue:
								sumPixel += qBlue(img.pixel(x, y));
								break;
							case channel_green:
								sumPixel += qGreen(img.pixel(x, y));
								break;
						}
					}
				}
			}

			sumPixel /= (img.height() * img.width());
		}
	}

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
	emit closePlugin();
}

bool DkThresholdViewPort::isCanceled() {
	return cancelTriggered;
}

void DkThresholdViewPort::setVisible(bool visible) {

	if(parent()) {
		DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());
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

	if (DkSettings::display.smallIcons)
		setIconSize(QSize(16, 16));
	else
		setIconSize(QSize(32, 32));

	if (DkSettings::display.toolbarGradient) {

		QColor hCol = DkSettings::display.highlightColor;
		hCol.setAlpha(80);

		setStyleSheet(
			//QString("QToolBar {border-bottom: 1px solid #b6bccc;") +
			QString("QToolBar {border: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); spacing: 3px; padding: 3px;}")
			+ QString("QToolBar::separator {background: #656565; width: 1px; height: 1px; margin: 3px;}")
			//+ QString("QToolButton:disabled{background-color: rgba(0,0,0,10);}")
			+ QString("QToolButton:hover{border: none; background-color: rgba(255,255,255,80);} QToolButton:pressed{margin: 0px; border: none; background-color: " + DkUtils::colorToString(hCol) + ";}")
			);
	}
	else
		setStyleSheet("QToolBar{spacing: 3px; padding: 3px;}");
}

DkThresholdToolBar::~DkThresholdToolBar() {

}

void DkThresholdToolBar::createIcons() {

	// create icons
	icons.resize(icons_end);

	icons[apply_icon] = QIcon(":/nomacsPluginThr/img/apply.png");
	icons[cancel_icon] = QIcon(":/nomacsPluginThr/img/cancel.png");
	icons[pan_icon] = 	QIcon(":/nomacsPluginThr/img/pan.png");
	icons[pan_icon].addPixmap(QPixmap(":/nomacsPluginThr/img/pan_checked.png"), QIcon::Normal, QIcon::On);

	if (!DkSettings::display.defaultIconColor) {
		// now colorize all icons
		for (int idx = 0; idx < icons.size(); idx++) {

			icons[idx].addPixmap(DkImage::colorizePixmap(icons[idx].pixmap(100, QIcon::Normal, QIcon::On), DkSettings::display.iconColor), QIcon::Normal, QIcon::On);
			icons[idx].addPixmap(DkImage::colorizePixmap(icons[idx].pixmap(100, QIcon::Normal, QIcon::Off), DkSettings::display.iconColor), QIcon::Normal, QIcon::Off);
		}
	}
}

void DkThresholdToolBar::createLayout() {

	QList<QKeySequence> enterSc;
	enterSc.append(QKeySequence(Qt::Key_Enter + Qt::SHIFT));
	enterSc.append(QKeySequence(Qt::Key_Return + Qt::SHIFT));

	QAction* applyAction = new QAction(icons[apply_icon], tr("Apply (ENTER + SHIFT)"), this);
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

	//image channel
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
