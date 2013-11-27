/*******************************************************************************************************
 DkFakeMiniaturesPlugin.cpp
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

#include "DkFakeMiniaturesPlugin.h"

namespace nmc {

/**
* Returns unique ID for the generated dll
**/
QString DkFakeMiniaturesPlugin::pluginID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QString("a2ac7b68866b4ab29fb1df3e170b8f0d");
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

   return QString("<b>Created by:</b> Tim Jerman<br><b>Modified:</b> November 2013<br><b>Description:</b> Apply a fake miniature filter (tilt shift effect) to the image.<br><b>Usage:</b> On the ") +
	    QString("preview image select (by mouse click move and release) the region without blurring. A blur is applyied depending on the distance from this region. ") +
		QString("The amount of blur and saturation can be changed with the sliders on the right of the dialog.");
};

/**
* Returns descriptive iamge for every ID
* @param plug-in ID
**/
QImage DkFakeMiniaturesPlugin::pluginDescriptionImage() const {

   return QImage(":/nomacsPlugin/img/fakeMinDesc.png");
};

/**
* Returns plug-in version for every ID
* @param plug-in ID
**/
QString DkFakeMiniaturesPlugin::pluginVersion() const {

   return "1.0.0";
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
* @param current image in the Nomacs viewport
**/
QImage DkFakeMiniaturesPlugin::runPlugin(const QString &runID, const QImage &image) const {

	if (runID == "4d29da2b322f44979c55ea0ed4ff158b") {
		DkFakeMiniaturesDialog* fakeMiniaturesDialog = new DkFakeMiniaturesDialog();

		fakeMiniaturesDialog->setImage(&image);

		bool done = fakeMiniaturesDialog->exec();

		QImage returnImg(image);
		if (fakeMiniaturesDialog->wasOkPressed()) returnImg = fakeMiniaturesDialog->getImage();

		fakeMiniaturesDialog->deleteLater();

		if(!returnImg.isNull()) return returnImg;
	}
	else {
		QMessageBox msgBox;
		msgBox.setText("Wrong GUID!");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
	return image;
};

Q_EXPORT_PLUGIN2("com.nomacs.ImageLounge.DkFakeMiniaturesPlugin/1.0", DkFakeMiniaturesPlugin)

};

