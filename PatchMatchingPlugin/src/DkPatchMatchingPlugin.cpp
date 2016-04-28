/*******************************************************************************************************
 DkPatchMatchingPlugin.cpp
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

#include "DkPatchMatchingPlugin.h"

#include <QDebug>
#include <QMouseEvent>

namespace nmp {

/*-----------------------------------DkPatchMatchingPlugin ---------------------------------------------*/

/**
*	Constructor
**/
DkPatchMatchingPlugin::DkPatchMatchingPlugin() 
	: viewport(nullptr) {
}

/**
*	Destructor
**/
DkPatchMatchingPlugin::~DkPatchMatchingPlugin() {

	qDebug() << "[PAINT PLUGIN] deleted...";

	if (viewport) {
		viewport->deleteLater();
		viewport = nullptr;
	}
}

/**
* Returns unique ID for the generated dll
**/
QString DkPatchMatchingPlugin::id() const {
	return PLUGIN_ID;
};


/**
* Returns descriptive image
**/
QImage DkPatchMatchingPlugin::image() const {

   return QImage(":/nomacsPluginPaint/img/description.png");
};

/**
* Main function: runs plugin based on its ID
* @param run ID
* @param current image in the Nomacs viewport
**/
QSharedPointer<nmc::DkImageContainer> DkPatchMatchingPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> image) const {
	
	if (!image)
		return image;

	//for a viewport plugin runID and image are null
	if (viewport) {

		DkPatchMatchingViewPort* paintViewport = dynamic_cast<DkPatchMatchingViewPort*>(viewport);

		if (!paintViewport->isCanceled()) 
			image->setImage(paintViewport->getPaintedImage(), tr("Drawings Added"));

		viewport->setVisible(false);
		
	}
	
	return image;
};

/**
* returns paintViewPort
**/
nmc::DkPluginViewPort* DkPatchMatchingPlugin::getViewPort() {

	if (!viewport) {
		viewport = new DkPatchMatchingViewPort();
		//connect(viewport, SIGNAL(destroyed()), this, SLOT(viewportDestroyed()));
	}
	return viewport;
}

/**
* sets the viewport pointer to NULL after the viewport is destroyed
**/
void DkPatchMatchingPlugin::viewportDestroyed() {

	viewport = 0;
}

void DkPatchMatchingPlugin::deleteViewPort() {

	if (viewport) {
		viewport->deleteLater();
		viewport = 0;
	}
}

/*-----------------------------------DkPatchMatchingViewPort ---------------------------------------------*/

DkPatchMatchingViewPort::DkPatchMatchingViewPort(QWidget* parent, Qt::WindowFlags flags) : DkPluginViewPort(parent, flags) {

	setObjectName("DkPatchMatchingViewPort");
	init();
	setMouseTracking(true);
}

DkPatchMatchingViewPort::~DkPatchMatchingViewPort() {
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


void DkPatchMatchingViewPort::saveSettings() const {

	QSettings& settings = nmc::Settings::instance().getSettings();

	settings.beginGroup(objectName());
	settings.setValue("penColor", pen.color().rgba());
	settings.setValue("penWidth", pen.width());
	settings.endGroup();

}

void DkPatchMatchingViewPort::loadSettings() {

	QSettings& settings = nmc:: Settings::instance().getSettings();

	settings.beginGroup(objectName());
	pen.setColor(QColor::fromRgba(settings.value("penColor", pen.color().rgba()).toInt()));
	pen.setWidth(settings.value("penWidth", 15).toInt());
	settings.endGroup();

}

void DkPatchMatchingViewPort::init() {
	
	setAttribute(Qt::WA_MouseTracking);
	polygonFinished = false;
	panning = false;
	cancelTriggered = false;
	isOutside = false;
	defaultCursor = Qt::CrossCursor;
	setCursor(defaultCursor);
	pen = QColor(0,0,0);
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	pen.setBrush(Qt::NoBrush);
	pen.setWidth(1);
	
	paintToolbar = new DkPatchMatchingToolBar(tr("Paint Toolbar"), this);

	connect(paintToolbar, SIGNAL(colorSignal(QColor)), this, SLOT(setPenColor(QColor)), Qt::UniqueConnection);
	connect(paintToolbar, SIGNAL(widthSignal(int)), this, SLOT(setPenWidth(int)), Qt::UniqueConnection);
	connect(paintToolbar, SIGNAL(panSignal(bool)), this, SLOT(setPanning(bool)), Qt::UniqueConnection);
	connect(paintToolbar, SIGNAL(cancelSignal()), this, SLOT(discardChangesAndClose()), Qt::UniqueConnection);
	connect(paintToolbar, SIGNAL(undoSignal()), this, SLOT(undoLastPaint()), Qt::UniqueConnection);

	connect(paintToolbar, SIGNAL(applySignal()), this, SLOT(applyChangesAndClose()), Qt::UniqueConnection);
	
	DkPluginViewPort::init();

	loadSettings();
	paintToolbar->setPenColor(pen.color());
	paintToolbar->setPenWidth(pen.width());
}

void DkPatchMatchingViewPort::undoLastPaint() {

	if (paths.empty())
		return;		// nothing to undo

	paths.pop_back();
	pathsPen.pop_back();
	update();
}

void DkPatchMatchingViewPort::mousePressEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->buttons() == Qt::LeftButton && 
		(event->modifiers() == nmc::Settings::param().global().altMod || panning)) {
		setCursor(Qt::ClosedHandCursor);
		event->setModifiers(Qt::NoModifier);	// we want a 'normal' action in the viewport
		event->ignore();
		return;
	}

	//QWidget::mousePressEvent(event);
	if (event->buttons() == Qt::LeftButton && parent()) {

		nmc::DkBaseViewPort* viewport = dynamic_cast<nmc::DkBaseViewPort*>(parent());
		if(viewport) {
			auto point = mapToImage(event->pos()); event->pos();//

			if (controlPoints.empty()) {
				auto cp = new DkControlPoint(point, this,windowFlags());
				cp->setVisible(true);
				controlPoints.push_back(cp);
				mousePos = point;
				
				/*if (QRectF(QPointF(), viewport->getImage().size()).contains(point)) {
					polygon << point;
					update();
				}*/
			} 
		}
	}
}

void DkPatchMatchingViewPort::mouseMoveEvent(QMouseEvent *event) {

	//qDebug() << "paint viewport...";

	// panning -> redirect to viewport
	if (event->modifiers() == nmc::Settings::param().global().altMod ||
		panning) {

		event->setModifiers(Qt::NoModifier);
		event->ignore();
		update();
		return;
	}

	if (parent()) {
		nmc::DkBaseViewPort* viewport = dynamic_cast<nmc::DkBaseViewPort*>(parent());

		if (viewport) {
			if (!polygon.empty()) {
				mousePos = mapToImage(event->pos());
				update();
			}
			/*viewport->unsetCursor(); */
		}
	}
	//QWidget::mouseMoveEvent(event);	// do not propagate mouse event
}

void DkPatchMatchingViewPort::mouseReleaseEvent(QMouseEvent *event) {

	// panning -> redirect to viewport
	if (event->modifiers() == nmc::Settings::param().global().altMod || panning) {
		setCursor(defaultCursor);
		event->setModifiers(Qt::NoModifier);
		event->ignore();
		return;
	}
}

void DkPatchMatchingViewPort::paintEvent(QPaintEvent *event) {

	QPainter painter(this);
	
	if (mWorldMatrix)
		painter.setWorldTransform((*mImgMatrix) * (*mWorldMatrix));	// >DIR: using both matrices allows for correct resizing [16.10.2013 markus]
		
	//for (auto iter = controlPoints.begin(); iter != controlPoints.end(); ++iter) {
	//	(*iter)->draw(&painter);
	//}
	//
	//painter.setRenderHint(QPainter::HighQualityAntialiasing);
	//painter.setRenderHint(QPainter::Antialiasing);
	//painter.setPen(pen);
	//painter.drawPolyline(polygon);

	//	
	//if (!polygon.empty() && !polygonFinished) {
	//	painter.drawLine(QLineF(polygon.last(), mousePos));
	//}

	//for (auto iter = controlPoints.begin(); iter != controlPoints.end(); ++iter) {
	//	(*iter)->draw(&painter);
	//}
	/*auto size = pen.widthF()*2;
	if (!polygon.empty()) {
		painter.drawEllipse(polygon.first(),size,size);
	}

	for (auto iter = polygon.begin(); iter != polygon.end(); iter++) {
		auto x = iter->x();
		auto y = iter->y();
		QRectF rect(x - size, y - size, 2*size, 2*size);
		painter.drawRect(rect);
	}*/
	DkPluginViewPort::paintEvent(event);
}

// mouse events --------------------------------------------------------------------
bool DkPatchMatchingViewPort::event(QEvent *event) {

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
		// mouse events that double are now fixed, since the mViewport is now overlayed by the mController
		return QWidget::event(event);
	}
	else {
		//qDebug() << "not redirecting - type: " << event->type();
		return DkPluginViewPort::event(event);
	}
}

QImage DkPatchMatchingViewPort::getPaintedImage() {

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

void DkPatchMatchingViewPort::setBrush(const QBrush& brush) {
	this->brush = brush;
}

void DkPatchMatchingViewPort::setPen(const QPen& pen) {
	this->pen = pen;
}

void DkPatchMatchingViewPort::setPenWidth(int width) {

	this->pen.setWidth(width);
}

void DkPatchMatchingViewPort::setPenColor(QColor color) {

	this->pen.setColor(color);
}

void DkPatchMatchingViewPort::setPanning(bool checked) {

	this->panning = checked;
	if(checked) defaultCursor = Qt::OpenHandCursor;
	else defaultCursor = Qt::CrossCursor;
	setCursor(defaultCursor);
}

void DkPatchMatchingViewPort::applyChangesAndClose() {
	polygonFinished = true;
	//cancelTriggered = false;
	//emit closePlugin();
}

void DkPatchMatchingViewPort::discardChangesAndClose() {

	cancelTriggered = true;
	emit closePlugin();
}

QBrush DkPatchMatchingViewPort::getBrush() const {
	return brush;
}

QPen DkPatchMatchingViewPort::getPen() const {
	return pen;
}

bool DkPatchMatchingViewPort::isCanceled() {
	return cancelTriggered;
}

void DkPatchMatchingViewPort::setVisible(bool visible) {

	if (paintToolbar)
		emit showToolbar(paintToolbar, visible);

	DkPluginViewPort::setVisible(visible);
}


void DkControlPoint::draw(QPainter* painter)
{
	QPen penNoStroke;
	penNoStroke.setWidth(0);
	penNoStroke.setColor(QColor(0, 0, 0, 0));

	QPen pen;
	//pen.setWidth(1);
	pen.setColor(QColor(255, 255, 0, 100));

	QRectF visibleRect(QPointF(), QSizeF(50,50));
	//QRectF whiteRect(QPointF(), size);
	visibleRect.moveCenter(geometry().center());
	//whiteRect.moveCenter(geometry().center());


	// draw the control point
	//painter->setWorldMatrixEnabled(false);
	painter->setPen(penNoStroke);
	painter->setBrush(QColor(50, 50, 50, 50));
	painter->drawRect(geometry());	// invisible rect for mouseevents...
									//painter->setPen(pen);
	painter->setBrush(QColor(255, 255, 255, 100));
	//painter->drawRect(whiteRect);
	painter->setBrush(QColor(0, 0, 0));
	painter->drawRect(visibleRect);
	//painter->setWorldMatrixEnabled(true);
}

void DkControlPoint::mousePressEvent(QMouseEvent* event)
{
	if (event->buttons() == Qt::LeftButton) {
		posGrab = event->globalPos();
		initialPos = geometry().topLeft();
	}
	qDebug() << "mouse pressed control point";
	QWidget::mousePressEvent(event);
}

void DkControlPoint::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() == Qt::LeftButton) {

		QPointF pt = initialPos + event->globalPos() - posGrab;
		setGeometry(pt.x(),pt.y(),size.width(),size.height());
		emit ctrlMovedSignal(pt, event->modifiers());
		qDebug() << "accepted false...";
	}

	QWidget::mouseMoveEvent(event);
}

void DkControlPoint::mouseReleaseEvent(QMouseEvent* event)
{
	QWidget::mouseReleaseEvent(event);
}

void DkControlPoint::enterEvent(QEvent* event)
{
}

void DkControlPoint::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	draw(&painter);
	QWidget::paintEvent(event);
}

DkControlPoint::DkControlPoint(const QPointF& center, QWidget* parent, Qt::WindowFlags flags)
	:QWidget(parent,flags), size(300,300)
{
	setMouseTracking(true);
	resize(size);
	setGeometry(center.x() - size.width()/2.0, center.y() - size.height()/2.0, size.width(), size.height());
	//setCursor(Qt::CrossCursor);
}

	/*-----------------------------------DkPatchMatchingToolBar ---------------------------------------------*/
DkPatchMatchingToolBar::DkPatchMatchingToolBar(const QString & title, QWidget * parent /* = 0 */) : QToolBar(title, parent) {

	setObjectName("paintToolBar");
	createIcons();
	createLayout();
	QMetaObject::connectSlotsByName(this);

	setIconSize(QSize(nmc::Settings::param().display().iconSize, nmc::Settings::param().display().iconSize));

	if (nmc::Settings::param().display().toolbarGradient) {

		QColor hCol = nmc::Settings::param().display().highlightColor;
		hCol.setAlpha(80);

		setStyleSheet(
			//QString("QToolBar {border-bottom: 1px solid #b6bccc;") +
			QString("QToolBar {border: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); spacing: 3px; padding: 3px;}")
			+ QString("QToolBar::separator {background: #656565; width: 1px; height: 1px; margin: 3px;}")
			//+ QString("QToolButton:disabled{background-color: rgba(0,0,0,10);}")
			+ QString("QToolButton:hover{border: none; background-color: rgba(255,255,255,80);} QToolButton:pressed{margin: 0px; border: none; background-color: " + nmc::DkUtils::colorToString(hCol) + ";}")
			);
	}
	else
		setStyleSheet("QToolBar{spacing: 3px; padding: 3px;}");

	qDebug() << "[PAINT TOOLBAR] created...";
}

DkPatchMatchingToolBar::~DkPatchMatchingToolBar() {

	qDebug() << "[PAINT TOOLBAR] deleted...";
}

void DkPatchMatchingToolBar::createIcons() {

	// create icons
	icons.resize(icons_end);

	icons[apply_icon] = QIcon(":/nomacsPluginPaint/img/apply.png");
	icons[cancel_icon] = QIcon(":/nomacsPluginPaint/img/cancel.png");
	icons[pan_icon] = 	QIcon(":/nomacsPluginPaint/img/pan.png");
	icons[pan_icon].addPixmap(QPixmap(":/nomacsPluginPaint/img/pan_checked.png"), QIcon::Normal, QIcon::On);
	icons[undo_icon] = 	QIcon(":/nomacsPluginPaint/img/undo.png");

	if (!nmc::Settings::param().display().defaultIconColor || nmc::Settings::param().app().privateMode) {
		// now colorize all icons
		for (int idx = 0; idx < icons.size(); idx++) {

			icons[idx].addPixmap(nmc::DkImage::colorizePixmap(icons[idx].pixmap(100, QIcon::Normal, QIcon::On), nmc::Settings::param().display().iconColor), QIcon::Normal, QIcon::On);
			icons[idx].addPixmap(nmc::DkImage::colorizePixmap(icons[idx].pixmap(100, QIcon::Normal, QIcon::Off), nmc::Settings::param().display().iconColor), QIcon::Normal, QIcon::Off);
		}
	}
}

void DkPatchMatchingToolBar::createLayout() {

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

void DkPatchMatchingToolBar::setVisible(bool visible) {

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

void DkPatchMatchingToolBar::setPenColor(const QColor& col) {

	penCol = col;
	penColButton->setStyleSheet("QPushButton {background-color: " + nmc::DkUtils::colorToString(penCol) + "; border: 1px solid #888;}");
	penAlpha = col.alpha();
	alphaBox->setValue(col.alphaF()*100);
}

void DkPatchMatchingToolBar::setPenWidth(int width) {

	widthBox->setValue(width);
}

void DkPatchMatchingToolBar::on_undoAction_triggered() {
	emit undoSignal();
}

void DkPatchMatchingToolBar::on_applyAction_triggered() {
	emit applySignal();
}

void DkPatchMatchingToolBar::on_cancelAction_triggered() {
	emit cancelSignal();
}

void DkPatchMatchingToolBar::on_panAction_toggled(bool checked) {

	emit panSignal(checked);
}

void DkPatchMatchingToolBar::on_widthBox_valueChanged(int val) {

	emit widthSignal(val);
}

void DkPatchMatchingToolBar::on_alphaBox_valueChanged(int val) {

	penAlpha = val;
	QColor penColWA = penCol;
	penColWA.setAlphaF(penAlpha/100.0);
	emit colorSignal(penColWA);
}

void DkPatchMatchingToolBar::on_penColButton_clicked() {

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
