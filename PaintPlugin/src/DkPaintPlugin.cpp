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

namespace nmp {

/*-----------------------------------DkPaintPlugin ---------------------------------------------*/

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
* Returns descriptive image
**/
QImage DkPaintPlugin::image() const {

   return QImage(":/nomacsPluginPaint/img/description.png");
}

bool DkPaintPlugin::hideHUD() const {
	return false;
}

/**
* Main function: runs plugin based on its ID
* @param run ID
* @param current image in the Nomacs viewport
**/
QSharedPointer<nmc::DkImageContainer> DkPaintPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> image) const {
	
	if (!image)
		return image;

	//for a viewport plugin runID and image are null
	if (viewport) {

		DkPaintViewPort* paintViewport = dynamic_cast<DkPaintViewPort*>(viewport);

		if (!paintViewport->isCanceled()) 
			image->setImage(paintViewport->getPaintedImage(), tr("Drawings Added"));

		viewport->setVisible(false);
		
	}
	
	return image;
};

/**
* returns paintViewPort
**/
nmc::DkPluginViewPort* DkPaintPlugin::getViewPort() {
	return viewport;
}

DkPaintViewPort * DkPaintPlugin::getPaintViewPort() {
	return dynamic_cast<DkPaintViewPort*>(viewport);
}

bool DkPaintPlugin::createViewPort(QWidget * parent) {
	
	viewport = new DkPaintViewPort(parent);
	
	return true;
}

void DkPaintPlugin::setVisible(bool visible) {

	if (!viewport)
		return;

	viewport->setVisible(visible);
	if (!visible)
		getPaintViewPort()->clear();
	
}

/*-----------------------------------DkPaintViewPort ---------------------------------------------*/

DkPaintViewPort::DkPaintViewPort(QWidget* parent, Qt::WindowFlags flags) : DkPluginViewPort(parent, flags) {

	setObjectName("DkPaintViewPort");
	init();
	setMouseTracking(true);
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

	nmc::DefaultSettings settings;

	settings.beginGroup(objectName());
	settings.setValue("penColor", pen.color().rgba());
	settings.setValue("penWidth", pen.width());
	settings.endGroup();

}

void DkPaintViewPort::loadSettings() {

	nmc::DefaultSettings settings;

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

	connect(paintToolbar, SIGNAL(colorSignal(QColor)), this, SLOT(setPenColor(QColor)), Qt::UniqueConnection);
	connect(paintToolbar, SIGNAL(widthSignal(int)), this, SLOT(setPenWidth(int)), Qt::UniqueConnection);
	connect(paintToolbar, SIGNAL(panSignal(bool)), this, SLOT(setPanning(bool)), Qt::UniqueConnection);
	connect(paintToolbar, SIGNAL(cancelSignal()), this, SLOT(discardChangesAndClose()), Qt::UniqueConnection);
	connect(paintToolbar, SIGNAL(undoSignal()), this, SLOT(undoLastPaint()), Qt::UniqueConnection);
	connect(paintToolbar, SIGNAL(applySignal()), this, SLOT(applyChangesAndClose()), Qt::UniqueConnection);
	
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
		(event->modifiers() == nmc::DkSettingsManager::param().global().altMod || panning)) {
		setCursor(Qt::ClosedHandCursor);
		event->setModifiers(Qt::NoModifier);	// we want a 'normal' action in the viewport
		event->ignore();
		return;
	}

	if (event->buttons() == Qt::LeftButton && parent()) {

		nmc::DkBaseViewPort* viewport = dynamic_cast<nmc::DkBaseViewPort*>(parent());
		if(viewport) {
		
			if(QRectF(QPointF(), viewport->getImage().size()).contains(mapToImage(event->pos()))) {
					
				isOutside = false;
				paths.append(QPainterPath());
				paths.last().moveTo(mapToImage(event->pos()));
				paths.last().lineTo(mapToImage(event->pos())+QPointF(0.1,0));
				pathsPen.append(pen);
				update();
			}
			else 
				isOutside = true;
		}
	}

	// no propagation
}

void DkPaintViewPort::mouseMoveEvent(QMouseEvent *event) {

	//qDebug() << "paint viewport...";

	// panning -> redirect to viewport
	if (event->modifiers() == nmc::DkSettingsManager::param().global().altMod ||
		panning) {

		event->setModifiers(Qt::NoModifier);
		event->ignore();
		update();
		return;
	}

	if (parent()) {
		nmc::DkBaseViewPort* viewport = dynamic_cast<nmc::DkBaseViewPort*>(parent());

		if (viewport) {
			viewport->unsetCursor();

			if (event->buttons() == Qt::LeftButton && parent()) {

				if (QRectF(QPointF(), viewport->getImage().size()).contains(mapToImage(event->pos()))) {
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
				else 
					isOutside = true;
			}
		}
	}
	//QWidget::mouseMoveEvent(event);	// do not propagate mouse event
}

void DkPaintViewPort::mouseReleaseEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->modifiers() == nmc::DkSettingsManager::param().global().altMod || panning) {
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
		nmc::DkBaseViewPort* viewport = dynamic_cast<nmc::DkBaseViewPort*>(parent());
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

void DkPaintViewPort::clear() {
	paths.clear();
	pathsPen.clear();
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
	if(checked) 
		defaultCursor = Qt::OpenHandCursor;
	else 
		defaultCursor = Qt::CrossCursor;
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
		emit showToolBar(paintToolbar, visible);

	DkPluginViewPort::setVisible(visible);
}

/*-----------------------------------DkPaintToolBar ---------------------------------------------*/
DkPaintToolBar::DkPaintToolBar(const QString & title, QWidget * parent /* = 0 */) : QToolBar(title, parent) {

	setObjectName("paintToolBar");
	createIcons();
	createLayout();
	QMetaObject::connectSlotsByName(this);

	qDebug() << "[PAINT TOOLBAR] created...";
}

DkPaintToolBar::~DkPaintToolBar() {

	qDebug() << "[PAINT TOOLBAR] deleted...";
}

void DkPaintToolBar::createIcons() {

	// create icons
	icons.resize(icons_end);

	icons[apply_icon]	= nmc::DkImage::loadIcon(":/nomacs/img/save.svg");
	icons[cancel_icon]	= nmc::DkImage::loadIcon(":/nomacs/img/cancel.svg");
	icons[pan_icon]		= nmc::DkImage::loadIcon(":/nomacs/img/pan.svg");
	icons[pan_icon].addPixmap(nmc::DkImage::loadIcon(":/nomacs/img/pan_checked.svg"), QIcon::Normal, QIcon::On);
	icons[undo_icon]	= nmc::DkImage::loadIcon(":/nomacs/img/rotate-cc.svg");
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
	penColButton->setStyleSheet("QPushButton {background-color: " + nmc::DkUtils::colorToString(penCol) + "; border: 1px solid #888;}");
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
	penColButton->setStyleSheet("QPushButton {background-color: " + nmc::DkUtils::colorToString(penCol) + "; border: 1px solid #888;}");
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
		penColButton->setStyleSheet("QPushButton {background-color: " + nmc::DkUtils::colorToString(penCol) + "; border: 1px solid #888;}");
		
		QColor penColWA = penCol;
		penColWA.setAlphaF(penAlpha/100.0);
		emit colorSignal(penColWA);
	}

}

};
