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
	: mViewport(nullptr) {
}

/**
*	Destructor
**/
DkPatchMatchingPlugin::~DkPatchMatchingPlugin() {

	qDebug() << "[PAINT PLUGIN] deleted...";
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
	if (mViewport) {
		if (!mViewport->isCanceled())
			image->setImage(mViewport->getPaintedImage(), tr("Drawings Added"));

		mViewport->setVisible(false);
	}
	
	return image;
};

/**
* returns paintViewPort
**/
nmc::DkPluginViewPort* DkPatchMatchingPlugin::getViewPort() {

	if (!mViewport) {
		mViewport = new DkPatchMatchingViewPort();			
	}
	return mViewport;
}

void DkPatchMatchingPlugin::deleteViewPort() {

	if (mViewport) {
		mViewport->deleteLater();
		mViewport = nullptr;
	}
}

bool DkPatchMatchingPlugin::closesOnImageChange() const
{
	return false;
}

	/*-----------------------------------DkPatchMatchingViewPort ---------------------------------------------*/

DkPatchMatchingViewPort::DkPatchMatchingViewPort(QWidget* parent, Qt::WindowFlags flags) 
	: DkPluginViewPort(parent, flags), mTransform(std::make_shared<QTransform>())
{

	setObjectName("DkPatchMatchingViewPort");
	init();
	setMouseTracking(true);

	//view = new QGraphicsView(this);
	//scene = new DkGraphics(this);

	//view->setStyleSheet("background: transparent");
	//view->setScene(scene);
	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
	//layout = new QHBoxLayout(this);
	//setLayout(layout);
	//layout->addWidget(view);
	//layout->setMargin(0);
	//
	////scene->addRect(QRect(30, 30, 60, 60), QPen(), QBrush(QColor(0, 255, 0, 40)));
	////view->setTransformationAnchor(QGraphicsView::NoAnchor);
	////view->setTransformationAnchor(QGraphicsView::NoAnchor);
	////scene->setSceneRect(geometr());

	//scene->addRect(0, 0, 20, 20);
	
	//view->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	//view->setTransform(*mWorldMatrix);
	poly = new DkSyncedPolygon(this, mWorldMatrix);
}

DkPatchMatchingViewPort::~DkPatchMatchingViewPort() {
	qDebug() << "[VIEWPORT] deleted...";
	
	saveSettings();

	// active deletion since the MainWindow takes ownership...
	// if we have issues with this, we could disconnect all signals between viewport and toolbar too
	// however, then we have lot's of toolbars in memory if the user opens the plugin again and again
	if (mtoolbar) {
		delete mtoolbar;
		mtoolbar = nullptr;
	}
}


void DkPatchMatchingViewPort::saveSettings() const {

	QSettings& settings = nmc::Settings::instance().getSettings();

	settings.beginGroup(objectName());
	settings.setValue("penColor", mPen.color().rgba());
	settings.setValue("penWidth", mPen.width());
	settings.endGroup();

}

void DkPatchMatchingViewPort::loadSettings() {

	QSettings& settings = nmc:: Settings::instance().getSettings();

	settings.beginGroup(objectName());
	mPen.setColor(QColor::fromRgba(settings.value("penColor", mPen.color().rgba()).toInt()));
	mPen.setWidth(settings.value("penWidth", 15).toInt());
	settings.endGroup();
}

void DkPatchMatchingViewPort::init() {
	
	setAttribute(Qt::WA_MouseTracking);
	mPolygonFinished = false;
	panning = false;
	cancelTriggered = false;
	defaultCursor = Qt::CrossCursor;
	setCursor(defaultCursor);
	mPen = QColor(0,0,0);
	mPen.setCapStyle(Qt::RoundCap);
	mPen.setJoinStyle(Qt::RoundJoin);
	mPen.setBrush(Qt::NoBrush);
	mPen.setWidth(1);
	
	mtoolbar = new DkPatchMatchingToolBar(tr("Paint Toolbar"), this);

	connect(mtoolbar, SIGNAL(colorSignal(QColor)), this, SLOT(setPenColor(QColor)), Qt::UniqueConnection);
	connect(mtoolbar, SIGNAL(widthSignal(int)), this, SLOT(setPenWidth(int)), Qt::UniqueConnection);
	connect(mtoolbar, SIGNAL(panSignal(bool)), this, SLOT(setPanning(bool)), Qt::UniqueConnection);
	connect(mtoolbar, SIGNAL(cancelSignal()), this, SLOT(discardChangesAndClose()), Qt::UniqueConnection);
	connect(mtoolbar, SIGNAL(undoSignal()), this, SLOT(undoLastPaint()), Qt::UniqueConnection);

	connect(mtoolbar, SIGNAL(applySignal()), this, SLOT(applyChangesAndClose()), Qt::UniqueConnection);
	
	loadSettings();
	mtoolbar->setPenColor(mPen.color());
	mtoolbar->setPenWidth(mPen.width());
}

void DkPatchMatchingViewPort::undoLastPaint() {

	if (paths.empty())
		return;		// nothing to undo

	paths.pop_back();
	pathsPen.pop_back();
	update();
}

//void DkPatchMatchingViewPort::controlPointRemoved(DkControlPoint* sender)
//{
//	auto idx = mControlPoints.indexOf(sender);
//	mControlPoints.remove(idx);
//	delete sender;
//	
//	mControlPoints[0]->setType(ControlPointType::circle);
//	
//	update();
//}

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
		QPointF point = mapToViewport(event->pos()); //
		
		poly->addPoint(point);

		/*
		auto cp = new DkControlPoint(point, mWorldMatrix, mTransform, this);
		cp->setVisible(true);
		
		connect(cp, SIGNAL(moved()), this, SLOT(update()));
		connect(cp, &DkControlPoint::removed, this, &DkPatchMatchingViewPort::controlPointRemoved);

		if (mControlPoints.empty()) {
			cp->setType(ControlPointType::circle);
		}
		mControlPoints << cp;
		*/
		update();
	}
}

void DkPatchMatchingViewPort::mouseMoveEvent(QMouseEvent *event) {
	// panning -> redirect to viewport
	if (event->modifiers() == nmc::Settings::param().global().altMod ||
		panning) {

		event->setModifiers(Qt::NoModifier);
		event->ignore();
		update();
	}
}

void DkPatchMatchingViewPort::mouseReleaseEvent(QMouseEvent *event) {
	// panning -> redirect to viewport
	if (event->modifiers() == nmc::Settings::param().global().altMod || panning) {
		setCursor(defaultCursor);
		event->setModifiers(Qt::NoModifier);
		event->ignore();
	}
}

void DkPatchMatchingViewPort::paintEvent(QPaintEvent *event) {
	poly->setWorldMatrix(mWorldMatrix);
	QPainter painter(this);

	//painter.setBrush(QColor(255, 0, 0, 40));
	//painter.drawRect(QRect(QPoint(), size()));

	//auto transform = (*mWorldMatrix);//(*mImgMatrix)
	//QPolygonF poly;
	//for (auto&& p: mControlPoints) {
	//	QRectF g = p->geometry();
	//	auto center = transform.map(p->truPosition());
	//	g.moveCenter(center);
	//	p->setGeometry(g.toRect());
	//	poly << center;
	//}

	//painter.setRenderHint(QPainter::HighQualityAntialiasing);
	//painter.setRenderHint(QPainter::Antialiasing);
	//painter.setPen(mPen);
	//painter.drawPolyline(poly);

	//poly.clear();
	//for (auto&& p : mClonePoints) {
	//	QRectF g = p->geometry();
	//	auto center = transform.map(mTransform->map(p->truPosition()));
	//	g.moveCenter(center);
	//	p->setGeometry(g.toRect());
	//	poly << center;
	//}

	//painter.setRenderHint(QPainter::HighQualityAntialiasing);
	//painter.setRenderHint(QPainter::Antialiasing);
	//painter.setPen(mPen);
	//painter.drawPolyline(poly);

	//auto t = (*mWorldMatrix)*(*mImgMatrix);
	//view->fitInView(geometry(), Qt::KeepAspectRatio);
	//view->setTransform(*mWorldMatrix);
	DkPluginViewPort::paintEvent(event);
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

void DkPatchMatchingViewPort::checkWorldMatrixChange()
{
	if (mWorldMatrix && mWorldMatrixCache != *mWorldMatrix) {
		emit worldMatrixChanged(newMatrix);
	}
}

void DkPatchMatchingViewPort::setBrush(const QBrush& brush) {
	this->brush = brush;
}

void DkPatchMatchingViewPort::setPen(const QPen& pen) {
	this->mPen = pen;
}

void DkPatchMatchingViewPort::setPenWidth(int width) {

	this->mPen.setWidth(width);
}

void DkPatchMatchingViewPort::setPenColor(QColor color) {

	this->mPen.setColor(color);
}

void DkPatchMatchingViewPort::setPanning(bool checked) {

	this->panning = checked;
	if(checked) defaultCursor = Qt::OpenHandCursor;
	else defaultCursor = Qt::CrossCursor;
	setCursor(defaultCursor);
}

void DkPatchMatchingViewPort::applyChangesAndClose() {
	mPolygonFinished = true;
	mTransform->translate(100, 0);
	

	//for (auto point : mClonePoints) {
	//	delete point;
	//}
	//
	//for (auto point : mControlPoints) {
	//	auto clone = new DkControlPoint(point->truPosition(), mWorldMatrix, mTransform, this);
	//	clone->setVisible(true);
	//	mClonePoints.append(clone);
	//	
	//}

	update();
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
	return mPen;
}

bool DkPatchMatchingViewPort::isCanceled() {
	return cancelTriggered;
}

void DkPatchMatchingViewPort::setVisible(bool visible) {

	if (mtoolbar)
		emit showToolbar(mtoolbar, visible);

	DkPluginViewPort::setVisible(visible);
}


//
//void DkControlPoint::mousePressEvent(QMouseEvent* event)
//{
//	if (event->buttons() == Qt::LeftButton) {
//		posGrab = event->globalPos();
//		initialPos = truPosition();
//		update();
//	}
//
//	if (event->button() == Qt::LeftButton && event->modifiers() == Qt::CTRL) {
//		emit removed(this);
//	}
//	qDebug() << "mouse pressed control point";
//	//QWidget::mousePressEvent(event);	// diem: eat this event - so the parent does not create a new point...
//}
//
//void DkControlPoint::mouseMoveEvent(QMouseEvent* event)
//{
//	if (event->buttons() == Qt::LeftButton) {
//		mPosition = initialPos + mapToViewport(event->globalPos()) - mapToViewport(posGrab);
//		update();
//		emit moved();
//		qDebug() << "accepted false...";
//	}
//}
//
//void DkControlPoint::mouseReleaseEvent(QMouseEvent* event)
//{	
//	if (event->buttons() == Qt::LeftButton) {
//		update();
//	}
//}
//
//void DkControlPoint::enterEvent(QEvent* event)
//{
//}
//
//
//const QPointF& DkControlPoint::truPosition() const
//{
//	return mPosition;
//}
//
//void DkControlPoint::setPosition(const QPointF& point)
//{
//	mPosition = point;
//}
//
//void DkControlPoint::setType(ControlPointType t)
//{
//	mOriginal->setType(t);
//	mClone->setType(t);
//}
//
//void DkControlPoint::paintEvent(QPaintEvent* event)
//{
//	mClone->setGeometry(mlocal->mapRect(mOriginal->geometry()));
//	QWidget::paintEvent(event);
//}
//
//
//DkControlPoint::DkControlPoint(const QPointF& center, QTransform* worldMatrix, std::shared_ptr<QTransform> local, QWidget* parent)
//	: QWidget(parent), mWorldMatrix(worldMatrix), 
//		mPosition(center), mlocal(local), 
//		mSize(20), 
//		mOriginal(new DkControlPointRepresentation(this)),
//		mClone(new DkControlPointRepresentation(this))
//{
//	QRect cpr(QPoint(), QSize(mSize, mSize));
//	mOriginal->setGeometry(cpr);
//	setCursor(Qt::CrossCursor);
//}


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

	// pen colorc
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
