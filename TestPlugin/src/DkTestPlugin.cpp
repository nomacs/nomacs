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

   return "Start Painting";
};

/**
* Returns long description for every ID
* @param plug-in ID
**/
QString DkTestPlugin::pluginDescription() const {

   return "<b>Created by:</b> name<br><b>Description:</b> One plug-in with 5 actions that are added into the menu.";
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

   return "2.2.1";
};

/**
* Returns unique IDs for every plug-in in this dll
* plug-in can have more the one functionality that are triggered in the menu
**/
QStringList DkTestPlugin::runID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QStringList() << "c7019c2172d3474782d91d79be1babfd" << "b49683ec27824e3fa5f5d1abf37517f4" << "ccc725b251ba4c23ab7f596401ce1d92" <<
						"0e0adc9f38284447960e528efb2bb3b4" << "3ab568d753c345f69fdea7aa29ba18fa";
};

/**
* Returns plug-in name for every ID
* @param plug-in ID
**/
QString DkTestPlugin::pluginMenuName(const QString &runID) const {

	return "Paint";

	if (runID=="c7019c2172d3474782d91d79be1babfd") return "Test plug-in 1";
	else if (runID=="b49683ec27824e3fa5f5d1abf37517f4") return "Test plug-in 2";
	else if (runID=="ccc725b251ba4c23ab7f596401ce1d92") return "Test plug-in 3";
	else if (runID=="0e0adc9f38284447960e528efb2bb3b4") return "Test plug-in 4";
	else if (runID=="3ab568d753c345f69fdea7aa29ba18fa") return "Test plug-in 5";
	return "Wrong GUID!";
};

/**
* Returns short description for status tip for every ID
* @param plug-in ID
**/
QString DkTestPlugin::pluginStatusTip(const QString &runID) const {

	if (runID=="c7019c2172d3474782d91d79be1babfd") return "Status tip for plug-in 1";
	else if (runID=="b49683ec27824e3fa5f5d1abf37517f4") return "Status tip for plug-in 2";
	else if (runID=="ccc725b251ba4c23ab7f596401ce1d92") return "Status tip for plug-in 3";
	else if (runID=="0e0adc9f38284447960e528efb2bb3b4") return "Status tip for plug-in 4";
	else if (runID=="3ab568d753c345f69fdea7aa29ba18fa") return "Status tip for plug-in 5";
	return "Wrong GUID!";
};

QList<QAction*> DkTestPlugin::pluginActions(QWidget* parent) {

	QList<QAction*> myActions;

	// destruction?
	QAction* ca = new QAction(tr("Paint"), parent);
	ca->setObjectName("paintAction");
	connect(ca, SIGNAL(triggered()), this, SLOT(emitRunPlugin()));
	myActions.append(ca);
	
	ca = new QAction(tr("Josef"), parent);
	ca->setObjectName("josefAction");
	ca->setStatusTip("tim");
	myActions.append(ca);
	
	ca = new QAction(tr("Ana"), parent);
	ca->setObjectName("anaAction");
	myActions.append(ca);

	return myActions;
}

/**
* Main function: runs plug-in based on its ID
* @param plug-in ID
* @param current image in the Nomacs viewport
**/
QImage DkTestPlugin::runPlugin(const QString &runID, const QImage &image) const {

	if(!image.isNull()) {
		 QMessageBox msgBox;
		 msgBox.setText(QString("You selected plugin %1.").arg(pluginMenuName(runID)));
		 msgBox.setIcon(QMessageBox::Warning);
		 msgBox.exec();
	}
	else {
		 QMessageBox msgBox;
		 msgBox.setText("No image in the viewport!\nThe plug-in will now close.");
		 msgBox.setIcon(QMessageBox::Warning);
		 msgBox.exec();
	}

	return image;
};

DkPluginViewPort* DkTestPlugin::getViewPort() {

	if (!viewport)
		viewport = new DkPaintViewPort();

	return viewport;
}

Q_EXPORT_PLUGIN2(DkTestPlugin, DkTestPlugin)


// My ViewPort --------------------------------------------------------------------
DkPaintViewPort::DkPaintViewPort(QWidget* parent, Qt::WindowFlags flags) : DkPluginViewPort(parent, flags) {
	init();
}

void DkPaintViewPort::init() {
	panning = false;	// this should be set to true in a toolbar

	DkPluginViewPort::init();
}

void DkPaintViewPort::mousePressEvent(QMouseEvent *event) {

	// allow zoom/pan
	if (panning || event->buttons() == Qt::LeftButton && event->modifiers() == DkSettings::global.altMod) {
		event->setModifiers(Qt::NoModifier);		// we want a 'normal' action in the viewport
		QWidget::mousePressEvent(event);
		return;
	}

	if (event->buttons() == Qt::LeftButton) {
		paths.append(QPainterPath());
		paths.last().moveTo(mapToViewport(event->posF()));
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

	// allow zoom/pan
	if (panning || event->buttons() == Qt::LeftButton && event->modifiers() == DkSettings::global.altMod) {
		event->setModifiers(Qt::NoModifier);		// we want a 'normal' action in the viewport
		DkPaintViewPort::mouseMoveEvent(event);
		return;
	}

	if (event->buttons() == Qt::LeftButton) {
		QPointF point = mapToViewport(event->posF());
		paths.last().lineTo(point);
		update();
	}

	//QWidget::mouseMoveEvent(event);	// do not propagate mouse event
}

void DkPaintViewPort::mouseReleaseEvent(QMouseEvent *event) {

	// allow zoom/pan
	if (panning || event->buttons() == Qt::LeftButton && event->modifiers() == DkSettings::global.altMod) {
		event->setModifiers(Qt::NoModifier);		// we want a 'normal' action in the viewport
		DkPluginViewPort::mouseReleaseEvent(event);
		return;
	}
}

void DkPaintViewPort::paintEvent(QPaintEvent *event) {

	QPainter painter(this);
	
	if (worldMatrix)
		painter.setWorldTransform(*worldMatrix);

	for (int idx = 0; idx < paths.size(); idx++)
		painter.drawPath(paths.at(idx));
	painter.end();

	DkPluginViewPort::paintEvent(event);
}

void DkPaintViewPort::setBrush(const QBrush& brush) {
	this->brush = brush;
}

void DkPaintViewPort::setPen(const QPen& pen) {
	this->pen = pen;
}

QBrush DkPaintViewPort::getBrush() const {
	return brush;
}

QPen DkPaintViewPort::getPen() const {
	return pen;
}

};
