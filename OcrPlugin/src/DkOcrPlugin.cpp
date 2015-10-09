/*******************************************************************************************************
 DkOcrPlugin.cpp

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2015 #YOUR_NAME

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

#include "DkOcrPlugin.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#pragma warning(pop)		// no warnings from includes - end

 /*******************************************************************************************************
  * PLUGIN_CLASS_NAME	- enter the plugin class name (e.g. DkPageExtractionPlugin)
  * #YOUR_NAME			- your name/pseudonym whatever
  * #DATE				- today...
  * #DESCRIPTION		- describe your plugin in one sentence
  * #MENU_NAME			- a user friendly name (e.g. Flip Image)
  * #MENU_STATUS_TIPP	- status tip of your plugin
  * #RUN_ID_1			- generate an ID using: GUID without hyphens generated at http://www.guidgenerator.com/
  * ID_ACTION1			- your action name (e.g. id_flip_horizontally)
  * #ACTION_NAME1		- your action name (e.g. Flip Horizotally - user friendly!)
  * #ACTION_TIPP1		- your action status tip (e.g. Flips an image horizontally - user friendly!)
  *******************************************************************************************************/

namespace nmc {

/**
*	Constructor
**/
	DkOcrPlugin::DkOcrPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	runIds[ID_ACTION1] = "#RUN_ID_1";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[ID_ACTION1] = tr("#ACTION_NAME1");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[ID_ACTION1] = tr("#ACTION_TIPP1");
	mMenuStatusTips = statusTips.toList();
}
/**
*	Destructor
**/
DkOcrPlugin::~DkOcrPlugin() {
}


/**
* Returns unique ID for the generated dll
**/
QString DkOcrPlugin::pluginID() const {

	return PLUGIN_ID;
};


/**
* Returns plugin name
* @param plugin ID
**/
QString DkOcrPlugin::pluginName() const {

	return tr("#MENU_NAME");
};

/**
* Returns long description for every ID
* @param plugin ID
**/
QString DkOcrPlugin::pluginDescription() const {

	return "<b>Created by:</b> Dominik Schörkhuber <br><b>Modified:</b>9.10.2015<br><b>Description:</b> #DESCRIPTION.";
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage DkOcrPlugin::pluginDescriptionImage() const {

	return QImage(":/#PLUGIN_NAME/img/your-image.png");
};

/**
* Returns plugin version for every ID
* @param plugin ID
**/
QString DkOcrPlugin::pluginVersion() const {

	return PLUGIN_VERSION;
};

/**
* Returns unique IDs for every plugin in this dll
**/
QStringList DkOcrPlugin::runID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QStringList() << "4acb88c461024cb080ae5cd15d0ef0ec";
};

/**
* Returns plugin name for every ID
* @param plugin ID
**/
QString DkOcrPlugin::pluginMenuName(const QString &runID) const {

	return tr("OcrMenu_Placeholder");
};

/**
* Returns short description for status tip for every ID
* @param plugin ID
**/
QString DkOcrPlugin::pluginStatusTip(const QString &runID) const {

	return tr("#MENU_STATUS_TIP");
};

QList<QAction*> DkOcrPlugin::pluginActions(QWidget* parent) {

	if (mActions.empty()) {
		QAction* ca = new QAction(mMenuNames[ID_ACTION1], this);
		ca->setObjectName(mMenuNames[ID_ACTION1]);
		ca->setStatusTip(mMenuStatusTips[ID_ACTION1]);
		ca->setData(mRunIDs[ID_ACTION1]);	// runID needed for calling function runPlugin()
		mActions.append(ca);

		//QAction* ca = new QAction(mMenuNames[ID_ACTION2], this);
		//ca->setObjectName(mMenuNames[ID_ACTION2]);
		//ca->setStatusTip(mMenuStatusTips[ID_ACTION2]);
		//ca->setData(mRunIDs[ID_ACTION2]);	// runID needed for calling function runPlugin()
		//mActions.append(ca);
	}

	return mActions;
}

QSharedPointer<DkImageContainer> DkOcrPlugin::runPlugin(const QString& runID, QSharedPointer<DkImageContainer> imgC) const
{
	return imgC;
}

	/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
/*QImage DkOcrPlugin::runPlugin(const QString &runID, const QImage &image) const {

	if(runID == mRunIDs[ID_ACTION1]) {
		// do what every you want e.g.:
		return image.mirrored(true, false);
	}

	// wrong runID? - do nothing
	return image;
};*/

};

