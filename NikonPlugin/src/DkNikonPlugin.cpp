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

#include "DkNikonPlugin.h"

#include "MaidError.h"

namespace nmp {

/*-----------------------------------DkNikonPlugin ---------------------------------------------*/


/**
*	Constructor
**/
DkNikonPlugin::DkNikonPlugin() {

	viewport = 0;
	camControls = 0;
	maidFacade = 0;
}

/**
*	Destructor
**/
DkNikonPlugin::~DkNikonPlugin() {

	if (viewport) {
		viewport->deleteLater();
		viewport = 0;
	}

	if (maidFacade) {
		maidFacade->deleteLater();
		maidFacade = 0;
	}

	if (camControls) {
		camControls->deleteLater();
		camControls = 0;
	}
}

void DkNikonPlugin::setVisible(bool visible) {

	if (!viewport)
		return;

	viewport->setVisible(visible);
}

/**
* Returns descriptive image
**/
QImage DkNikonPlugin::image() const {

   return QImage(":/nomacsPluginNikon/img/description.png");
};

/**
* Main function: runs plugin based on its ID
* @param run ID
* @param current image in the Nomacs viewport
**/
QSharedPointer<nmc::DkImageContainer> DkNikonPlugin::runPlugin(const QString &runID, const QSharedPointer<nmc::DkImageContainer> image) const {
	
	//for a viewport plugin runID and image are null
	if (viewport) {

		DkNikonViewPort* paintViewport = dynamic_cast<DkNikonViewPort*>(viewport);

		//if (!paintViewport->isCanceled()) retImg = paintViewport->getPaintedImage();
		viewport->setVisible(false);
	
		return image;
	}
	
	return image;
};

bool DkNikonPlugin::createViewPort(QWidget * parent) {

	viewport = new DkNikonViewPort(parent);

	if (!maidFacade) {
		QMainWindow* mainWindow = getMainWindow();

		maidFacade = new MaidFacade(mainWindow);

		if (maidFacade->init()) {


			if (!camControls) {

				// get last location
				QSettings settings;
				int dockLocation = settings.value("camControlsLocation", Qt::RightDockWidgetArea).toInt();

				camControls = new DkCamControls(maidFacade, tr("Camera Controls"));

				if (mainWindow)
					mainWindow->addDockWidget((Qt::DockWidgetArea)dockLocation, camControls);

				connect(camControls, SIGNAL(updateImage(QImage)), getViewPort(), SIGNAL(loadImage(QImage)));
				connect(camControls, SIGNAL(loadFile(QFileInfo)), getViewPort(), SIGNAL(loadFile(QFileInfo)));
				connect(camControls, SIGNAL(closeSignal()), getViewPort(), SIGNAL(closePlugin()));
			}

			camControls->setVisible(true);

		}
		else {
			QMessageBox warningDialog(mainWindow);
			warningDialog.setWindowTitle(tr("MAID Library could not be opened"));
			warningDialog.setText(tr("The MAID library could not be opened. Camera controls will be disabled."));
			warningDialog.setIcon(QMessageBox::Warning);
			warningDialog.setStandardButtons(QMessageBox::Ok);
			warningDialog.setDefaultButton(QMessageBox::Ok);
			warningDialog.show();
			warningDialog.exec();

			return 0;
		}
	}

	return true;
}

/**
* returns paintViewPort
**/
nmc::DkPluginViewPort* DkNikonPlugin::getViewPort() {

	return viewport;
}

void DkNikonPlugin::deleteViewPort() {

	if (viewport) {
		viewport->deleteLater();
		viewport = 0;
	}

	if (camControls) {
		camControls->deleteLater();
		camControls = 0;
	}

	if (maidFacade) {
		delete maidFacade;
		maidFacade = 0;
	}

}

/*-----------------------------------DkPaintViewPort ---------------------------------------------*/

DkNikonViewPort::DkNikonViewPort(QWidget* parent, Qt::WindowFlags flags) : DkPluginViewPort(parent, flags) {
}

DkNikonViewPort::~DkNikonViewPort() {
	qDebug() << "[PAINT VIEWPORT] deleted...";
}

};
