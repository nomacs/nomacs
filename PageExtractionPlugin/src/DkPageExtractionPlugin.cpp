/*******************************************************************************************************
 DkPageExtractionPlugin.cpp

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2015 Markus Diem

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

#include "DkPageExtractionPlugin.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#pragma warning(pop)		// no warnings from includes - end

 /*******************************************************************************************************
  * DkPageExtractionPlugin	- enter the plugin class name (e.g. DkPageExtractionPlugin)
  * Markus Diem			- your name/pseudonym whatever
  * 27.08.2015				- today...
  * Detects document pages in images		- describe your plugin in one sentence
  * Document Page Extraction			- a user friendly name (e.g. Flip Image)
  * Detects page borders in images (or more generally speaking: rectangles)	- status tip of your plugin
  * 1638a7f56b814ee48c6eb8a7710e74b4			- generate an ID using: GUID without hyphens generated at http://www.guidgenerator.com/
  * id_crop_to_page			- your action name (e.g. id_flip_horizontally)
  * Crop to Page		- your action name (e.g. Flip Horizotally - user friendly!)
  * Finds a page in a document image and then crops the image to that page.		- your action status tip (e.g. Flips an image horizontally - user friendly!)
  *******************************************************************************************************/

namespace nmc {

/**
*	Constructor
**/
DkPageExtractionPlugin::DkPageExtractionPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	runIds[id_crop_to_page] = "1638a7f56b814ee48c6eb8a7710e74b4";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_crop_to_page] = tr("Crop to Page");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_crop_to_page] = tr("Finds a page in a document image and then crops the image to that page.");
	mMenuStatusTips = statusTips.toList();
}

/**
*	Destructor
**/
DkPageExtractionPlugin::~DkPageExtractionPlugin() {
}


/**
* Returns unique ID for the generated dll
**/
QString DkPageExtractionPlugin::pluginID() const {

	return PLUGIN_ID;
};


/**
* Returns plugin name
* @param plugin ID
**/
QString DkPageExtractionPlugin::pluginName() const {

	return tr("Document Page Extraction");
};

/**
* Returns long description for every ID
* @param plugin ID
**/
QString DkPageExtractionPlugin::pluginDescription() const {

	return "<b>Created by:</b> Markus Diem <br><b>Modified:</b>27.08.2015<br><b>Description:</b> Detects document pages in images.";
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage DkPageExtractionPlugin::pluginDescriptionImage() const {

	return QImage(":/#PLUGIN_NAME/img/your-image.png");
};

/**
* Returns plugin version for every ID
* @param plugin ID
**/
QString DkPageExtractionPlugin::pluginVersion() const {

	return PLUGIN_VERSION;
};

/**
* Returns unique IDs for every plugin in this dll
**/
QStringList DkPageExtractionPlugin::runID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QStringList() << "#RUN_GUID";
};

/**
* Returns plugin name for every ID
* @param plugin ID
**/
QString DkPageExtractionPlugin::pluginMenuName(const QString &runID) const {

	return tr("Document Page Extraction");
};

/**
* Returns short description for status tip for every ID
* @param plugin ID
**/
QString DkPageExtractionPlugin::pluginStatusTip(const QString &runID) const {

	return tr("#MENU_STATUS_TIP");
};

QList<QAction*> DkPageExtractionPlugin::pluginActions(QWidget* parent) {

	if (mActions.empty()) {
		QAction* ca = new QAction(mMenuNames[id_crop_to_page], this);
		ca->setObjectName(mMenuNames[id_crop_to_page]);
		ca->setStatusTip(mMenuStatusTips[id_crop_to_page]);
		ca->setData(mRunIDs[id_crop_to_page]);	// runID needed for calling function runPlugin()
		mActions.append(ca);

		//QAction* ca = new QAction(mMenuNames[ID_ACTION2], this);
		//ca->setObjectName(mMenuNames[ID_ACTION2]);
		//ca->setStatusTip(mMenuStatusTips[ID_ACTION2]);
		//ca->setData(mRunIDs[ID_ACTION2]);	// runID needed for calling function runPlugin()
		//mActions.append(ca);
	}

	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QImage DkPageExtractionPlugin::runPlugin(const QString &runID, const QImage &image) const {

	if(!runID.isEmpty()) {
		bool horizontally = (runID == mRunIDs[id_crop_to_page]);
		return image.mirrored(horizontally, !horizontally);
	}

	// wrong runID? - do nothing
	return image;
};

};

