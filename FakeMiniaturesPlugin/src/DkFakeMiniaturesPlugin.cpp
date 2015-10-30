/*******************************************************************************************************
 DkFakeMiniaturesPlugin.cpp
 Created on:	14.04.2013

 nomacs is a fast and small imgC viewer with the capability of synchronizing multiple instances

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

#include "DkFakeMiniaturesPlugin.h"

namespace nmc {

/**
* Returns unique ID for the generated dll
**/
QString DkFakeMiniaturesPlugin::pluginID() const {
	return PLUGIN_ID;
};

/**
* Returns plug-in name
* @param plug-in ID
**/
QString DkFakeMiniaturesPlugin::pluginName() const {

   return tr("Fake Miniatures");
};

/**
* Returns long description for every ID
* @param plug-in ID
**/
QString DkFakeMiniaturesPlugin::pluginDescription() const {

   return QString("<b>Created by:</b> Tim Jerman<br><b>Modified:</b> November 2013<br><b>Description:</b> Apply a fake miniature filter (tilt shift effect) to the imgC.<br><b>Usage:</b> On the ") +
	    QString("preview imgC select (by mouse click move and release) the region without blurring. A blur is applyied depending on the distance from this region. ") +
		QString("The amount of blur and saturation can be changed with the sliders on the right of the dialog.");
};

/**
* Returns descriptive iamge for every ID
* @param plug-in ID
**/
QImage DkFakeMiniaturesPlugin::pluginDescriptionImage() const {

   return QImage(":/nomacsPluginFakeMin/img/fakeMinDesc.png");
};

/**
* Returns plug-in version for every ID
* @param plug-in ID
**/
QString DkFakeMiniaturesPlugin::pluginVersion() const {

   return PLUGIN_VERSION;
};

/**
* Returns unique IDs for every plug-in in this dll
* plug-in can have more the one functionality that are triggered in the menu
**/
QStringList DkFakeMiniaturesPlugin::runID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QStringList() << "4d29da2b322f44979c55ea0ed4ff158b";
};


/**
* Returns plug-in name for the menu
* @param plug-in ID
**/
QString DkFakeMiniaturesPlugin::pluginMenuName(const QString &runID) const {

   if (runID == "4d29da2b322f44979c55ea0ed4ff158b") return tr("Fake Miniatures");
   return "Wrong GUID!";
};

/**
* Returns short description for status tip for every ID
* @param plug-in ID
**/
QString DkFakeMiniaturesPlugin::pluginStatusTip(const QString &runID) const {

   if (runID == "4d29da2b322f44979c55ea0ed4ff158b") return tr("Apply fake miniatures filter");
   return "Wrong GUID!";
};


/**
* Main function: runs plug-in based on its ID
* @param plug-in ID
* @param current imgC in the Nomacs viewport
**/
QSharedPointer<DkImageContainer> DkFakeMiniaturesPlugin::runPlugin(const QString &runID, QSharedPointer<DkImageContainer> imgC) const {

	if (runID == "4d29da2b322f44979c55ea0ed4ff158b" && imgC) {
		QMainWindow* mainWindow = getMainWindow();
		DkFakeMiniaturesDialog* fakeMiniaturesDialog;
		if(mainWindow) 
			fakeMiniaturesDialog = new DkFakeMiniaturesDialog(mainWindow);
		else 
			fakeMiniaturesDialog = new DkFakeMiniaturesDialog();

    QImage img = imgC->image();
		fakeMiniaturesDialog->setImage(&img);

		bool done = fakeMiniaturesDialog->exec();

		QImage returnImg(imgC->image());
		if (fakeMiniaturesDialog->wasOkPressed()) 
			returnImg = fakeMiniaturesDialog->getImage();

		fakeMiniaturesDialog->deleteLater();

		imgC->setImage(returnImg);
	}
	else {
		QMessageBox msgBox;
		msgBox.setText("Wrong GUID!");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}

	return imgC;
};

};

