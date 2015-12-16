/*******************************************************************************************************
 DkPaintPlugin.cpp
 Created on:	14.07.2013

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

#include "DkPaintPlugin.h"

#include <QDebug>
#include <QMouseEvent>

namespace nmc {

/*-----------------------------------DkPaintPlugin ---------------------------------------------*/

DkSettings::Display& DkSettings::display = DkSettings::getDisplaySettings();
DkSettings::Global& DkSettings::global = DkSettings::getGlobalSettings();
DkSettings::App& DkSettings::app = DkSettings::getAppSettings();

/**
*	Constructor
**/
DkPaintPlugin::DkPaintPlugin() {

	viewport = 0;
}

/**
*	Destructor
**/
DkPaintPlugin::~DkPaintPlugin() {

	qDebug() << "[PAINT PLUGIN] deleted...";

	if (viewport) {
		viewport->deleteLater();
		viewport = 0;
	}
}

/**
* Returns unique ID for the generated dll
**/
QString DkPaintPlugin::pluginID() const {
	return PLUGIN_ID;
};


/**
* Returns plugin name
**/
QString DkPaintPlugin::pluginName() const {

   return "Paint";
};

/**
* Returns long description
**/
QString DkPaintPlugin::pluginDescription() const {

   return "<b>Created by:</b> Tim Jerman<br><b>Modified:</b> May 2014<br><b>Description:</b> Paint on an opened image. The color, size and opacity of the brush can be changed.";
};

/**
* Returns descriptive image
**/
QImage DkPaintPlugin::pluginDescriptionImage() const {

   return QImage(":/nomacsPluginPaint/img/description.png");
};

/**
* Returns plugin version
**/
QString DkPaintPlugin::pluginVersion() const {

   return PLUGIN_VERSION;
};

/**
* Returns unique IDs for every plugin in this dll
* plugin can have more the one functionality that are triggered in the menu
* runID differes from pluginID
* viewport plugins can have only one runID and one functionality bound to it 
**/
QStringList DkPaintPlugin::runID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QStringList() << PLUGIN_ID;
};

/**
* Returns plugin name for every run ID
* @param run ID
**/
QString DkPaintPlugin::pluginMenuName(const QString &runID) const {

	if (runID==PLUGIN_ID) return "Paint on image";
	return "Wrong GUID!";
};

/**
* Returns short description for status tip for every ID
* @param plugin ID
**/
QString DkPaintPlugin::pluginStatusTip(const QString &runID) const {

	if (runID==PLUGIN_ID) return "Paint on image with colored brush";
	return "Wrong GUID!";
};

/**
* Main function: runs plugin based on its ID
* @param run ID
* @param current image in the Nomacs viewport
**/
QSharedPointer<DkImageContainer> DkPaintPlugin::runPlugin(const QString &runID, QSharedPointer<DkImageContainer> image) const {
	
	if (!image)
		return image;

	//for a viewport plugin runID and image are null
	if (viewport) {

		DkPaintViewPort* paintViewport = dynamic_cast<DkPaintViewPort*>(viewport);

		if (!paintViewport->isCanceled()) 
			image->setImage(paintViewport->getPaintedImage());

		viewport->setVisible(false);
		
	}
	
	return image;
};

/**
* returns paintViewPort
**/
DkPluginViewPort* DkPaintPlugin::getViewPort() {

	if (!viewport) {
		viewport = new DkPaintViewPort();
		//connect(viewport, SIGNAL(destroyed()), this, SLOT(viewportDestroyed()));
	}
	return viewport;
}

/**
* sets the viewport pointer to NULL after the viewport is destroyed
**/
void DkPaintPlugin::viewportDestroyed() {

	viewport = 0;
}

void DkPaintPlugin::deleteViewPort() {

	if (viewport) {
		viewport->deleteLater();
		viewport = 0;
	}
}

/*-----------------------------------DkPaintViewPort ---------------------------------------------*/

DkPaintViewPort::DkPaintViewPort(QWidget* parent, Qt::WindowFlags flags) : DkPluginViewPort(parent, flags) {

	setObjectName("DkPaintViewPort");
	init();
}

DkPaintViewPort::~DkPaintViewPort() {
	qDebug() << "[PAINT VIEWPORT] deleted...";
	
	saveSettings();

	// active deletion since the MainWindow takes ownership...
	// if we have issues with this, we could disconnect all signals between viewport and toolbar too
	// however, then we have lot's of toolbars in memory if the user opens the plugin again and again
	if (paintToolbar) {
		delete paintToolbar;
		paintToolbar = 0;
	}
}


void DkPaintViewPort::saveSettings() const {

	QSettings& settings = Settings::instance().getSettings();

	settings.beginGroup(objectName());
	settings.setValue("penColor", pen.color().rgba());
	settings.setValue("penWidth", pen.width());
	settings.endGroup();

}

void DkPaintViewPort::loadSettings() {

	QSettings& settings = Settings::instance().getSettings();

	settings.beginGroup(objectName());
	pen.setColor(QColor::fromRgba(settings.value("penColor", pen.color().rgba()).toInt()));
	pen.setWidth(settings.value("penWidth", 15).toInt());
	settings.endGroup();

}

void DkPaintViewPort::init() {
	
	panning = false;
	cancelTriggered = false;
	isOutside = false;
	defaultCursor = Qt::CrossCursor;
	setCursor(defaultCursor);
	pen = QColor(0,0,0);
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	pen.setWidth(1);
	
	paintToolbar = new DkPaintToolBar(tr("Paint Toolbar"), this);

	connect(paintToolbar, SIGNAL(colorSignal(QColor)), this, SLOT(setPenColor(QColor)));
	connect(paintToolbar, SIGNAL(widthSignal(int)), this, SLOT(setPenWidth(int)));
	connect(paintToolbar, SIGNAL(panSignal(bool)), this, SLOT(setPanning(bool)));
	connect(paintToolbar, SIGNAL(cancelSignal()), this, SLOT(discardChangesAndClose()));
	connect(paintToolbar, SIGNAL(undoSignal()), this, SLOT(undoLastPaint()));
	connect(paintToolbar, SIGNAL(applySignal()), this, SLOT(applyChangesAndClose()));
	
	DkPluginViewPort::init();

	loadSettings();
	paintToolbar->setPenColor(pen.color());
	paintToolbar->setPenWidth(pen.width());
}

void DkPaintViewPort::undoLastPaint() {

	if (paths.empty())
		return;		// nothing to undo

	paths.pop_back();
	pathsPen.pop_back();
	update();
}

void DkPaintViewPort::mousePressEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->buttons() == Qt::LeftButton && 
		(event->modifiers() == DkSettings::global.altMod || panning)) {
		setCursor(Qt::ClosedHandCursor);
		event->setModifiers(Qt::NoModifier);	// we want a 'normal' action in the viewport
		event->ignore();
		return;
	}

	if (event->buttons() == Qt::LeftButton) {
		if(parent()) {

			DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());
			if(viewport) {
		
				if(QRectF(QPointF(), viewport->getImage().size()).contains(mapToImage(event->pos()))) {
					
					isOutside = false;
					paths.append(QPainterPath());
					paths.last().moveTo(mapToImage(event->pos()));
					paths.last().lineTo(mapToImage(event->pos())+QPointF(0.1,0));
					pathsPen.append(pen);
					update();
				}
				else isOutside = true;
			}
		}
	}

	// no propagation
}

void DkPaintViewPort::mouseMoveEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->modifiers() == DkSettings::global.altMod ||
		panning) {

		event->setModifiers(Qt::NoModifier);
		event->ignore();
		update();
		return;
	}

	if (event->buttons() == Qt::LeftButton) {
		if(parent()) {
			DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());

			if(viewport) {
		
				if(QRectF(QPointF(), viewport->getImage().size()).contains(mapToImage(event->pos()))) {
					if (isOutside) {
						paths.append(QPainterPath());
						paths.last().moveTo(mapToImage(event->pos()));
						pathsPen.append(pen);
					}
					else {
						QPointF point = mapToImage(event->pos());
						paths.last().lineTo(point);
						update();
					}
					isOutside = false;
				}
				else isOutside = true;
			}
		}
	}

	//QWidget::mouseMoveEvent(event);	// do not propagate mouse event
}

void DkPaintViewPort::mouseReleaseEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->modifiers() == DkSettings::global.altMod || panning) {
		setCursor(defaultCursor);
		event->setModifiers(Qt::NoModifier);
		event->ignore();
		return;
	}
}

void DkPaintViewPort::paintEvent(QPaintEvent *event) {

	QPainter painter(this);
	
	if (mWorldMatrix)
		painter.setWorldTransform((*mImgMatrix) * (*mWorldMatrix));	// >DIR: using both matrices allows for correct resizing [16.10.2013 markus]

	for (int idx = 0; idx < paths.size(); idx++) {

		painter.setPen(pathsPen.at(idx));
		painter.drawPath(paths.at(idx));
	}

	painter.end();

	DkPluginViewPort::paintEvent(event);
}

QImage DkPaintViewPort::getPaintedImage() {

	if(parent()) {
		DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());
		if (viewport) {

			if (!paths.isEmpty()) {   // if nothing is drawn there is no need to change the image

				QImage img = viewport->getImage();

				QPainter painter(&img);

				// >DIR: do not apply world matrix if painting in the image [14.10.2014 markus]
				//if (worldMatrix)
				//	painter.setWorldTransform(*worldMatrix);

				painter.setRenderHint(QPainter::HighQualityAntialiasing);
				painter.setRenderHint(QPainter::Antialiasing);

				for (int idx = 0; idx < paths.size(); idx++) {
					painter.setPen(pathsPen.at(idx));
					painter.drawPath(paths.at(idx));
				}
				painter.end();

				return img;
			}
		}
	}
	
	return QImage();
}

void DkPaintViewPort::setBrush(const QBrush& brush) {
	this->brush = brush;
}

void DkPaintViewPort::setPen(const QPen& pen) {
	this->pen = pen;
}

void DkPaintViewPort::setPenWidth(int width) {

	this->pen.setWidth(width);
}

void DkPaintViewPort::setPenColor(QColor color) {

	this->pen.setColor(color);
}

void DkPaintViewPort::setPanning(bool checked) {

	this->panning = checked;
	if(checked) defaultCursor = Qt::OpenHandCursor;
	else defaultCursor = Qt::CrossCursor;
	setCursor(defaultCursor);
}

void DkPaintViewPort::applyChangesAndClose() {

	cancelTriggered = false;
	emit closePlugin();
}

void DkPaintViewPort::discardChangesAndClose() {

	cancelTriggered = true;
	emit closePlugin();
}

QBrush DkPaintViewPort::getBrush() const {
	return brush;
}

QPen DkPaintViewPort::getPen() const {
	return pen;
}

bool DkPaintViewPort::isCanceled() {
	return cancelTriggered;
}

void DkPaintViewPort::setVisible(bool visible) {

	if (paintToolbar)
		emit showToolbar(paintToolbar, visible);

	DkPluginViewPort::setVisible(visible);
}

/*-----------------------------------DkPaintToolBar ---------------------------------------------*/
DkPaintToolBar::DkPaintToolBar(const QString & title, QWidget * parent /* = 0 */) : QToolBar(title, parent) {

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

	qDebug() << "[PAINT TOOLBAR] created...";
}

DkPaintToolBar::~DkPaintToolBar() {

	qDebug() << "[PAINT TOOLBAR] deleted...";
}

void DkPaintToolBar::createIcons() {

	// create icons
	icons.resize(icons_end);

	icons[apply_icon] = QIcon(":/nomacsPluginPaint/img/apply.png");
	icons[cancel_icon] = QIcon(":/nomacsPluginPaint/img/cancel.png");
	icons[pan_icon] = 	QIcon(":/nomacsPluginPaint/img/pan.png");
	icons[pan_icon].addPixmap(QPixmap(":/nomacsPluginPaint/img/pan_checked.png"), QIcon::Normal, QIcon::On);
	icons[undo_icon] = 	QIcon(":/nomacsPluginPaint/img/undo.png");

	if (!DkSettings::display.defaultIconColor || DkSettings::app.privateMode) {
		// now colorize all icons
		for (int idx = 0; idx < icons.size(); idx++) {

			icons[idx].addPixmap(DkImage::colorizePixmap(icons[idx].pixmap(100, QIcon::Normal, QIcon::On), DkSettings::display.iconColor), QIcon::Normal, QIcon::On);
			icons[idx].addPixmap(DkImage::colorizePixmap(icons[idx].pixmap(100, QIcon::Normal, QIcon::Off), DkSettings::display.iconColor), QIcon::Normal, QIcon::Off);
		}
	}
}

void DkPaintToolBar::createLayout() {

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

	// pen color
	penCol = QColor(0,0,0);
	penColButton = new QPushButton(this);
	penColButton->setObjectName("penColButton");
	penColButton->setStyleSheet("QPushButton {background-color: " + DkUtils::colorToString(penCol) + "; border: 1px solid #888;}");
	penColButton->setToolTip(tr("Background Color"));
	penColButton->setStatusTip(penColButton->toolTip());

	// undo Button
	undoAction = new QAction(icons[undo_icon], tr("Undo (CTRL+Z)"), this);
	undoAction->setShortcut(QKeySequence::Undo);
	undoAction->setObjectName("undoAction");

	colorDialog = new QColorDialog(this);
	colorDialog->setObjectName("colorDialog");

	// pen width
	widthBox = new QSpinBox(this);
	widthBox->setObjectName("widthBox");
	widthBox->setSuffix("px");
	widthBox->setMinimum(1);
	widthBox->setMaximum(500);	// huge sizes since images might have high resolutions

	// pen alpha
	alphaBox = new QSpinBox(this);
	alphaBox->setObjectName("alphaBox");
	alphaBox->setSuffix("%");
	alphaBox->setMinimum(0);
	alphaBox->setMaximum(100);

	addAction(applyAction);
	addAction(cancelAction);
	addSeparator();
	addAction(panAction);
	addAction(undoAction);
	addSeparator();
	addWidget(widthBox);
	addWidget(penColButton);
	addWidget(alphaBox);
}

void DkPaintToolBar::setVisible(bool visible) {

	//if (!visible)
	//	emit colorSignal(QColor(0,0,0));
	if (visible) {
		//emit colorSignal(penCol);
		//widthBox->setValue(10);
		//alphaBox->setValue(100);
		panAction->setChecked(false);
	}

	qDebug() << "[PAINT TOOLBAR] set visible: " << visible;

	QToolBar::setVisible(visible);
}

void DkPaintToolBar::setPenColor(const QColor& col) {

	penCol = col;
	penColButton->setStyleSheet("QPushButton {background-color: " + DkUtils::colorToString(penCol) + "; border: 1px solid #888;}");
	penAlpha = col.alpha();
	alphaBox->setValue(col.alphaF()*100);
}

void DkPaintToolBar::setPenWidth(int width) {

	widthBox->setValue(width);
}

void DkPaintToolBar::on_undoAction_triggered() {
	emit undoSignal();
}

void DkPaintToolBar::on_applyAction_triggered() {
	emit applySignal();
}

void DkPaintToolBar::on_cancelAction_triggered() {
	emit cancelSignal();
}

void DkPaintToolBar::on_panAction_toggled(bool checked) {

	emit panSignal(checked);
}

void DkPaintToolBar::on_widthBox_valueChanged(int val) {

	emit widthSignal(val);
}

void DkPaintToolBar::on_alphaBox_valueChanged(int val) {

	penAlpha = val;
	QColor penColWA = penCol;
	penColWA.setAlphaF(penAlpha/100.0);
	emit colorSignal(penColWA);
}

void DkPaintToolBar::on_penColButton_clicked() {

	QColor tmpCol = penCol;
	
	colorDialog->setCurrentColor(tmpCol);
	int ok = colorDialog->exec();

	if (ok == QDialog::Accepted) {
		penCol = colorDialog->currentColor();
		penColButton->setStyleSheet("QPushButton {background-color: " + DkUtils::colorToString(penCol) + "; border: 1px solid #888;}");
		
		QColor penColWA = penCol;
		penColWA.setAlphaF(penAlpha/100.0);
		emit colorSignal(penColWA);
	}

}

};
