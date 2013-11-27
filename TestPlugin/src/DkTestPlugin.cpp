/*******************************************************************************************************
 DkTestPlugin.cpp
 Created on:	14.04.2013

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

#include "DkTestPlugin.h"

namespace nmc {
	
DkTestPlugin::DkTestPlugin() {
	viewport = 0;
}

DkTestPlugin::~DkTestPlugin() {

	if (viewport) {
		viewport->deleteLater();
		viewport = 0;
	}
}

/**
* Returns unique ID for the generated dll
**/
QString DkTestPlugin::pluginID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QString("e1dc478b9962473b873e59db0228a22d");
};


/**
* Returns plug-in name
* @param plug-in ID
**/
QString DkTestPlugin::pluginName() const {

   return "Test plugin";
};

/**
* Returns long description for every ID
* @param plug-in ID
**/
QString DkTestPlugin::pluginDescription() const {

   return "<b>Created by:</b> Tim Jerman<br><b>Modified:</b> November 2013<br><b>Description:</b> Template for viewport development. The test plugin shows how to develop a viewport plugin.<br><b>Usage:</b> Draw a rectangle with left mouse click. ENTER applies changes to the image, ESC cancels the action. Middle mouse click rotates the image.";
};

/**
* Returns descriptive iamge for every ID
* @param plug-in ID
**/
QImage DkTestPlugin::pluginDescriptionImage() const {

   return QImage(":/nomacsPlugin/img/plugin1.png");
};

/**
* Returns plug-in version for every ID
* @param plug-in ID
**/
QString DkTestPlugin::pluginVersion() const {

   return "0.1.0";
};

/**
* Returns unique IDs for every plug-in in this dll
* plug-in can have more the one functionality that are triggered in the menu
**/
QStringList DkTestPlugin::runID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QStringList() << "c7019c2172d3474782d91d79be1babfd";
};

/**
* Returns plug-in name for every ID
* @param plug-in ID
**/
QString DkTestPlugin::pluginMenuName(const QString &runID) const {

	if (runID=="c7019c2172d3474782d91d79be1babfd") return "Test viewport plugin";

	return "Wrong GUID!";
};

/**
* Returns short description for status tip for every ID
* @param plug-in ID
**/
QString DkTestPlugin::pluginStatusTip(const QString &runID) const {

	if (runID=="c7019c2172d3474782d91d79be1babfd") return "Draw a rectangle with left mouse click";

	return "Wrong GUID!";
};

/**
* Main function: runs plugin based on its ID
* @param run ID
* @param current image in the Nomacs viewport
**/
QImage DkTestPlugin::runPlugin(const QString &runID, const QImage &image) const {

	//for a viewport plugin runID and image are null
	DkPaintViewPort* paintViewport = dynamic_cast<DkPaintViewPort*>(viewport);

	QImage retImg = QImage();
	bool abc = paintViewport->isCanceled();
	if (!paintViewport->isCanceled()) retImg = paintViewport->getPaintedImage();

	viewport->setVisible(false);
	
	return retImg;
};

/**
* returns paintViewPort
**/
DkPluginViewPort* DkTestPlugin::getViewPort() {

	if (!viewport) {
		viewport = new DkPaintViewPort();		
		connect(viewport, SIGNAL(destroyed()), this, SLOT(viewportDestroyed()));
	}

	return viewport;
}

/**
* sets the viewport pointer to NULL after the viewport is destroyed
**/
void DkTestPlugin::viewportDestroyed() {

	viewport = 0;
}

/* macro for exporting plugin */
Q_EXPORT_PLUGIN2("com.nomacs.ImageLounge.DkTestPlugin/1.0", DkTestPlugin)

/*-----------------------------------DkPaintViewPort ---------------------------------------------*/
DkPaintViewPort::DkPaintViewPort(QWidget* parent, Qt::WindowFlags flags) : DkPluginViewPort(parent, flags) {
	init();
}

void DkPaintViewPort::init() {
	panning = false;	// this should be set to true in a toolbar
	cancelTriggered = true;

	QList<QKeySequence> enterSc;
	enterSc.append(QKeySequence(Qt::Key_Enter));
	enterSc.append(QKeySequence(Qt::Key_Return));

	QAction* applyAction = new QAction("Apply", this);
	applyAction->setShortcuts(enterSc);
	applyAction->setObjectName("applyAction");

	QAction* cancelAction = new QAction("Cancel", this);
	cancelAction->setShortcut(QKeySequence(Qt::Key_Escape));
	cancelAction->setObjectName("cancelAction");

	addAction(applyAction);
	addAction(cancelAction);

	connect(applyAction, SIGNAL(triggered()), this, SLOT(applyChangesAndClose()));
	connect(cancelAction, SIGNAL(triggered()), this, SLOT(discardChangesAndClose()));

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
					
					rectStart = mapToImage(event->posF());
					rectEnd = mapToImage(event->posF());
					update();
				}
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

					rectEnd = mapToImage(event->posF());
					update();
				}
			}
		}
	}

	//QWidget::mouseMoveEvent(event);	// do not propagate mouse event
}

void DkPaintViewPort::mouseReleaseEvent(QMouseEvent *event) {


	// panning -> redirect to viewport
	if (event->modifiers() == DkSettings::global.altMod || panning) {
		event->setModifiers(Qt::NoModifier);
		event->ignore();
		return;
	}
}

void DkPaintViewPort::paintEvent(QPaintEvent *event) {

	QPainter painter(this);
	
	if (worldMatrix)
		painter.setWorldTransform((*imgMatrix) * (*worldMatrix));	// >DIR: using both matrices allows for correct resizing [16.10.2013 markus]

	QPointF rectStartM = rectStart;

	if(rectEnd.x() < rectStart.x()) rectStartM.setX(rectEnd.x());
	if(rectStart.y() > rectEnd.y()) rectStartM.setY(rectEnd.y());

	painter.setPen(Qt::red);
	painter.fillRect(rectStartM.x(), rectStartM.y(), qAbs(rectStart.x()-rectEnd.x()), qAbs(rectStart.y()-rectEnd.y()), QBrush(QColor(0,0,0,80)));
	painter.drawRect(rectStartM.x(), rectStartM.y(), qAbs(rectStart.x()-rectEnd.x()), qAbs(rectStart.y()-rectEnd.y()));

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

			QPointF rectStartM = rectStart;

			if(rectEnd.x() < rectStart.x()) rectStartM.setX(rectEnd.x());
			if(rectStart.y() > rectEnd.y()) rectStartM.setY(rectEnd.y());

			painter.setPen(Qt::red);
			painter.fillRect(rectStartM.x(), rectStartM.y(), qAbs(rectStart.x()-rectEnd.x()), qAbs(rectStart.y()-rectEnd.y()), QBrush(QColor(0,0,0,80)));
			painter.drawRect(rectStartM.x(), rectStartM.y(), qAbs(rectStart.x()-rectEnd.x()), qAbs(rectStart.y()-rectEnd.y()));

			painter.end();

			return img;
		}
	}
	
	return QImage();
}

void DkPaintViewPort::setVisible(bool visible) {

	DkPluginViewPort::setVisible(visible);
}

void DkPaintViewPort::applyChangesAndClose() {

	cancelTriggered = false;
	emit closePlugin();
}

void DkPaintViewPort::discardChangesAndClose() {

	cancelTriggered = true;
	emit closePlugin();
}

bool DkPaintViewPort::isCanceled() {
	return cancelTriggered;
}

};
