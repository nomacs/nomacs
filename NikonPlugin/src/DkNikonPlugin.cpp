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

namespace nmc {

/*-----------------------------------DkNikonPlugin ---------------------------------------------*/
DkSettings::Display& DkSettings::display = DkSettings::getDisplaySettings();
DkSettings::Global& DkSettings::global = DkSettings::getGlobalSettings();

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

/**
* Returns unique ID for the generated dll
**/
QString DkNikonPlugin::pluginID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QString("87c51045b38744caa9f389e000d64f3f");
};


/**
* Returns plugin name
**/
QString DkNikonPlugin::pluginName() const {

   return "Nikon";
};

/**
* Returns long description
**/
QString DkNikonPlugin::pluginDescription() const {

   return "<b>Created by:</b> Thomas Lang<br><b>Modified:</b> June 2014<br><b>Description:</b> Control Nikon D4 using nomacs.";
};

/**
* Returns descriptive image
**/
QImage DkNikonPlugin::pluginDescriptionImage() const {

   return QImage(":/nomacsPluginNikon/img/description.png");
};

/**
* Returns plugin version
**/
QString DkNikonPlugin::pluginVersion() const {

   return "1.0.0";
};

/**
* Returns unique IDs for every plugin in this dll
* plugin can have more the one functionality that are triggered in the menu
* runID differes from pluginID
* viewport plugins can have only one runID and one functionality bound to it 
**/
QStringList DkNikonPlugin::runID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QStringList() << "87c51045b38744caa9f389e000d64f3f";
};

/**
* Returns plugin name for every run ID
* @param run ID
**/
QString DkNikonPlugin::pluginMenuName(const QString &runID) const {

	if (runID=="87c51045b38744caa9f389e000d64f3f") return "Nikon";
	return "Wrong GUID!";
};

/**
* Returns short description for status tip for every ID
* @param plugin ID
**/
QString DkNikonPlugin::pluginStatusTip(const QString &runID) const {

	if (runID=="87c51045b38744caa9f389e000d64f3f") return "Control your Nikon D4";
	return "Wrong GUID!";
};

/**
* Main function: runs plugin based on its ID
* @param run ID
* @param current image in the Nomacs viewport
**/
QImage DkNikonPlugin::runPlugin(const QString &runID, const QImage &image) const {
	
	//for a viewport plugin runID and image are null
	if (viewport) {

		DkNikonViewPort* paintViewport = dynamic_cast<DkNikonViewPort*>(viewport);

		QImage retImg = QImage();
		//if (!paintViewport->isCanceled()) retImg = paintViewport->getPaintedImage();

		viewport->setVisible(false);
	
		return retImg;
	}
	
	return image;
};

/**
* returns paintViewPort
**/
DkPluginViewPort* DkNikonPlugin::getViewPort() {

	if (!viewport) {
		// >DIR: dummy viewport [4.7.2014 markus]
		viewport = new DkNikonViewPort();
	}

	if (!maidFacade) {
		QMainWindow* mainWindow = getMainWidnow();

		maidFacade = new MaidFacade();

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

		} else {
			QMessageBox warningDialog(mainWindow);
			warningDialog.setWindowTitle(tr("MAID Library could not be opened"));
			warningDialog.setText(tr("The MAID library could not be opened. Camera controls will be disabled."));
			warningDialog.setIcon(QMessageBox::Warning);
			warningDialog.setStandardButtons(QMessageBox::Ok);
			warningDialog.setDefaultButton(QMessageBox::Ok);
			warningDialog.show();
			warningDialog.exec();
		}
	}

	return viewport;
}

/**
* sets the viewport pointer to NULL after the viewport is destroyed
**/
void DkNikonPlugin::viewportDestroyed() {

	viewport = 0;
	camControls = 0;
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

/* macro for exporting plugin */
Q_EXPORT_PLUGIN2("com.nomacs.ImageLounge.DkNikonPlugin/1.0", DkNikonPlugin)


/*-----------------------------------DkPaintViewPort ---------------------------------------------*/

DkNikonViewPort::DkNikonViewPort(QWidget* parent, Qt::WindowFlags flags) : DkPluginViewPort(parent, flags) {

	init();
}

DkNikonViewPort::~DkNikonViewPort() {
	qDebug() << "[PAINT VIEWPORT] deleted...";
}

void DkNikonViewPort::init() {
		
	DkPluginViewPort::init();
}

};
