/*******************************************************************************************************
 DkFlipPlugin.cpp
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

#include "DkFlipPlugin.h"

namespace nmc {

/**
* Returns unique ID for the generated dll
**/
QString DkFlipPlugin::pluginID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QString("35f8d00939bb493dad6b9363bbe70d31");
};


/**
* Returns plugin name
* @param plugin ID
**/
QString DkFlipPlugin::pluginName() const {

	return tr("Flip plugin");
};

/**
* Returns long description for every ID
* @param plugin ID
**/
QString DkFlipPlugin::pluginDescription() const {

	return "<b>Created by:</b> Tim Jerman<br><b>Modified:</b> November 2013<br><b>Description:</b> Flip images hirizontally or vertically.";
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage DkFlipPlugin::pluginDescriptionImage() const {

	return QImage(":/nomacsPlugin/img/flipPlugin.png");
};

/**
* Returns plugin version for every ID
* @param plugin ID
**/
QString DkFlipPlugin::pluginVersion() const {

	return "1.0.0";
};

/**
* Returns unique IDs for every plugin in this dll
**/
QStringList DkFlipPlugin::runID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QStringList() << "e7630f0f28c34df2b5a3f6d1fbd131aa" << "9b262d0429c14464be6144340e5be66c";
};

/**
* Returns plugin name for every ID
* @param plugin ID
**/
QString DkFlipPlugin::pluginMenuName(const QString &runID) const {

	/* //one way to do it:
	if (runID=="e7630f0f28c34df2b5a3f6d1fbd131aa") return "Flip Horizontally";
	else if (runID=="9b262d0429c14464be6144340e5be66c") return "Flip Vertically";
	return "Wrong GUID!";
	*/
	
	/*
		//another way is to return a submenu from pluginActions function
	*/
	return tr("Flip image");
};

/**
* Returns short description for status tip for every ID
* @param plugin ID
**/
QString DkFlipPlugin::pluginStatusTip(const QString &runID) const {

	/* //one way to do it:
	if (runID=="e7630f0f28c34df2b5a3f6d1fbd131aa") return "Flip image horizontally";
	else if (runID=="9b262d0429c14464be6144340e5be66c") return "Flip image vertically";
	return "Wrong GUID!";
	*/
	
	/*
		//another way is to return a submenu from pluginActions function
		//
	*/
	return tr("Flip image horizontally or vertically");
};

QList<QAction*> DkFlipPlugin::pluginActions(QWidget* parent) {

	QList<QAction*> myActions;

	QAction* ca = new QAction(tr("Horizontally"), parent);
	ca->setObjectName("flipHorizontally");
	ca->setStatusTip(tr("flip image horizontally"));
	ca->setData("e7630f0f28c34df2b5a3f6d1fbd131aa");	// runID needed for calling function runPlugin()
	myActions.append(ca);
	
	ca = new QAction(tr("Vertically"), parent);
	ca->setObjectName("flipVertically");
	ca->setStatusTip(tr("flip image vertically"));
	ca->setData("9b262d0429c14464be6144340e5be66c");	// runID needed for calling function runPlugin()
	myActions.append(ca);
	

	return myActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param current image in the Nomacs viewport
**/
QImage DkFlipPlugin::runPlugin(const QString &runID, const QImage &image) const {

	if(!runID.isEmpty()) {
		bool horizontally = (runID=="e7630f0f28c34df2b5a3f6d1fbd131aa");
		return image.mirrored(horizontally, !horizontally);
	}

	return image;
};

Q_EXPORT_PLUGIN2("com.nomacs.ImageLounge.DkFlipPlugin/1.0", DkFlipPlugin)

};

