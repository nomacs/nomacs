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

namespace nmc {

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

	if (viewport) {
		viewport->deleteLater();
		viewport = 0;
	}
}

/**
* Returns unique ID for the generated dll
**/
QString DkPaintPlugin::pluginID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QString("ad970ef36cc24737afd2b53ad015ff0d");
};


/**
* Returns plug-in name
* @param plug-in ID
**/
QString DkPaintPlugin::pluginName() const {

   return "Paint";
};

/**
* Returns long description for every ID
* @param plug-in ID
**/
QString DkPaintPlugin::pluginDescription() const {

   return "<b>Created by:</b> Tim Jerman<br><b>Description:</b> Simple paint demo. Paints colored lines on image.";
};

/**
* Returns descriptive iamge for every ID
* @param plug-in ID
**/
QImage DkPaintPlugin::pluginDescriptionImage() const {

   return QImage(":/nomacsPlugin/img/description.png");
};

/**
* Returns plug-in version for every ID
* @param plug-in ID
**/
QString DkPaintPlugin::pluginVersion() const {

   return "0.1";
};

/**
* Returns unique IDs for every plug-in in this dll
* plug-in can have more the one functionality that are triggered in the menu
**/
QStringList DkPaintPlugin::runID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QStringList() << "15ac610607d247d7a8b3bb5ea0fef5cc";
};

/**
* Returns plug-in name for every ID
* @param plug-in ID
**/
QString DkPaintPlugin::pluginMenuName(const QString &runID) const {

	if (runID=="15ac610607d247d7a8b3bb5ea0fef5cc") return "Paint on image";
	return "Wrong GUID!";
};

/**
* Returns short description for status tip for every ID
* @param plug-in ID
**/
QString DkPaintPlugin::pluginStatusTip(const QString &runID) const {

	if (runID=="15ac610607d247d7a8b3bb5ea0fef5cc") return "Paint colored lines on image";
	return "Wrong GUID!";
};

/**
* Main function: runs plug-in based on its ID
* @param plug-in ID
* @param current image in the Nomacs viewport
**/
QImage DkPaintPlugin::runPlugin(const QString &runID, const QImage &image) const {

	//for a viewport plugin runID and image are null
	DkPaintViewPort* paintViewport = dynamic_cast<DkPaintViewPort*>(viewport);

	QImage retImg = QImage();
	if (!paintViewport->isCanceled()) retImg = paintViewport->getPaintedImage();

	delete viewport;

	return retImg;
};

/**
* returns paintViewPort
**/
DkPluginViewPort* DkPaintPlugin::getViewPort() {

	if (!viewport)
		viewport = new DkPaintViewPort();

	return viewport;
}

Q_EXPORT_PLUGIN2(DkPaintPlugin, DkPaintPlugin)


/*-----------------------------------DkPaintViewPort ---------------------------------------------*/
DkPaintViewPort::DkPaintViewPort(QWidget* parent, Qt::WindowFlags flags) : DkPluginViewPort(parent, flags) {

	init();
}

void DkPaintViewPort::init() {

	panning = false;	// this should be set to true in a toolbar
	cancelTriggered = false;
	isOutside = false;
	defaultCursor = Qt::CrossCursor;
	setCursor(defaultCursor);
	pen = QColor(0,0,0);
	pen.setWidth(1);
	paintToolbar = new DkPaintToolBar(tr("Paint Toolbar"), this);
	paintToolbar->setVisible(true);
	connect(paintToolbar, SIGNAL(colorSignal(QColor)), this, SLOT(setPenColor(QColor)));
	connect(paintToolbar, SIGNAL(widthSignal(int)), this, SLOT(setPenWidth(int)));
	connect(paintToolbar, SIGNAL(panSignal(bool)), this, SLOT(setPanning(bool)));
	connect(paintToolbar, SIGNAL(cancelSignal()), this, SLOT(discardChangesAndClose()));
	connect(paintToolbar, SIGNAL(applySignal()), this, SLOT(applyChangesAndClose()));
	connect(paintToolbar, SIGNAL(showToolbar(QToolBar*, bool)), this, SIGNAL(showToolbar(QToolBar*, bool)));
	DkPluginViewPort::init();
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
		
				if(QRectF(QPointF(), viewport->getImage().size()).contains(mapToImage(event->posF()))) {
					
					isOutside = false;
					paths.append(QPainterPath());
					paths.last().moveTo(mapToViewport(event->posF()));
					//paths.last().addRect(QRectF(mapToViewport(event->posF()), QSizeF(1,1)));
					paths.last().lineTo(mapToViewport(event->posF())+QPointF(0.1,0));
					//paths.last().lineTo(mapToViewport(event->posF()));
					pathsPen.append(pen);
					update();
				}
				else isOutside = true;
			}
		}
	}

	if (event->buttons() == Qt::MiddleButton && parent()) {

		// small image editing demo
		DkBaseViewPort* viewport = dynamic_cast<DkBaseViewPort*>(parent());
		if (viewport) {
			QImage img = viewport->getImage();
			img = img.mirrored();
			viewport->setImage(img);
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
		
				if(QRectF(QPointF(), viewport->getImage().size()).contains(mapToImage(event->posF()))) {
					if (isOutside) {
						paths.append(QPainterPath());
						paths.last().moveTo(mapToViewport(event->posF()));
						pathsPen.append(pen);
					}
					else {
						QPointF point = mapToViewport(event->posF());
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

	// allow zoom/pan
	/*if (panning) {
		event->setModifiers(Qt::NoModifier);		// we want a 'normal' action in the viewport
		DkPluginViewPort::mouseReleaseEvent(event);
		return;
	}*/

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
	
	if (worldMatrix)
		painter.setWorldTransform(*worldMatrix);

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

			QImage img = viewport->getImage();

			QPainter painter(&img);

			if (worldMatrix)
				painter.setWorldTransform(*worldMatrix);

			for (int idx = 0; idx < paths.size(); idx++) {
				painter.setPen(pathsPen.at(idx));
				painter.drawPath(paths.at(idx));
			}
			painter.end();

			return img;
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
	emit closePlugin(false);	// false - don't ask for saving the image after applying
}

void DkPaintViewPort::discardChangesAndClose() {

	cancelTriggered = true;
	paintToolbar->setVisible(false);
	emit closePlugin(false);	// false - don't ask for saving the image after applying
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
}

DkPaintToolBar::~DkPaintToolBar() {

}

void DkPaintToolBar::createIcons() {

	// create icons
	icons.resize(icons_end);

	icons[apply_icon] = QIcon(":/nomacsPlugin/img/apply.png");
	icons[cancel_icon] = QIcon(":/nomacsPlugin/img/cancel.png");
	icons[pan_icon] = 	QIcon(":/nomacsPlugin/img/pan.png");
	icons[pan_icon].addPixmap(QPixmap(":/nomacsPlugin/img/pan_checked.png"), QIcon::Normal, QIcon::On);

	if (!DkSettings::display.defaultIconColor) {
		// now colorize all icons
		for (int idx = 0; idx < icons.size(); idx++) {

			icons[idx].addPixmap(DkUtils::colorizePixmap(icons[idx].pixmap(100, QIcon::Normal, QIcon::On), DkSettings::display.iconColor), QIcon::Normal, QIcon::On);
			icons[idx].addPixmap(DkUtils::colorizePixmap(icons[idx].pixmap(100, QIcon::Normal, QIcon::Off), DkSettings::display.iconColor), QIcon::Normal, QIcon::Off);
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

	colorDialog = new QColorDialog(this);
	colorDialog->setObjectName("colorDialog");

	widthBox = new QSpinBox(this);
	widthBox->setObjectName("widthBox");
	widthBox->setSuffix("px");
	widthBox->setMinimum(1);
	widthBox->setMaximum(15);

	addAction(applyAction);
	addAction(cancelAction);
	addSeparator();
	addAction(panAction);
	addSeparator();
	addWidget(penColButton);
	addWidget(widthBox);
}

void DkPaintToolBar::setVisible(bool visible) {

	if (!visible)
		emit colorSignal(QColor(0,0,0));
	else {
		emit colorSignal(penCol);
		widthBox->setValue(1);
		panAction->setChecked(false);
	}
	showToolbar(this, visible);
	//QToolBar::addSeparator();
	//QToolBar::setVisible(visible);
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

void DkPaintToolBar::on_penColButton_clicked() {

	QColor tmpCol = penCol;
	
	colorDialog->setCurrentColor(tmpCol);
	int ok = colorDialog->exec();

	if (ok == QDialog::Accepted) {
		penCol = colorDialog->currentColor();
		penColButton->setStyleSheet("QPushButton {background-color: " + DkUtils::colorToString(penCol) + "; border: 1px solid #888;}");
		emit colorSignal(penCol);
	}

}

};
