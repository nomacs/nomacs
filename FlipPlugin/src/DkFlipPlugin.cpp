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
* Returns plug-in name
* @param plug-in ID
**/
QString DkFlipPlugin::pluginName() const {

   return "Flip plug-in";
};

/**
* Returns long description for every ID
* @param plug-in ID
**/
QString DkFlipPlugin::pluginDescription() const {

   return "<b>Created by:</b> Tim Jerman<br><b> Flip images hirizontally or vertically.";
};

/**
* Returns descriptive iamge for every ID
* @param plug-in ID
**/
QImage DkFlipPlugin::pluginDescriptionImage() const {

   return QImage(":/nomacsPlugin/img/flipPlugin.png");
};

/**
* Returns plug-in version for every ID
* @param plug-in ID
**/
QString DkFlipPlugin::pluginVersion() const {

   return "0.0.1";
};

/**
* Returns unique IDs for every plug-in in this dll
**/
QStringList DkFlipPlugin::runID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QStringList() << "e7630f0f28c34df2b5a3f6d1fbd131aa" << "9b262d0429c14464be6144340e5be66c";
};

/**
* Returns plug-in name for every ID
* @param plug-in ID
**/
QString DkFlipPlugin::pluginMenuName(const QString &runID) const {

   if (runID=="e7630f0f28c34df2b5a3f6d1fbd131aa") return "Flip Horizontally";
   else if (runID=="9b262d0429c14464be6144340e5be66c") return "Flip Vertically";
   return "Wrong GUID!";
};

/**
* Returns short description for status tip for every ID
* @param plug-in ID
**/
QString DkFlipPlugin::pluginStatusTip(const QString &runID) const {

   if (runID=="e7630f0f28c34df2b5a3f6d1fbd131aa") return "Flip image horizontally";
   else if (runID=="9b262d0429c14464be6144340e5be66c") return "Flip image vertically";
   return "Wrong GUID!";
};

/**
* Main function: runs plug-in based on its ID
* @param plug-in ID
* @param current image in the Nomacs viewport
**/
QImage DkFlipPlugin::runPlugin(const QString &runID, const QImage &image) const {

	if(!image.isNull()) {
		bool horizontally = !(runID=="e7630f0f28c34df2b5a3f6d1fbd131aa");
		QImage result(image.width(), image.height(), image.format());

		for(int y=0; y<image.height(); y++)
			for(int x=0; x<image.width(); x++)
				result.setPixel(horizontally?x:(image.width()-1-x), horizontally?(image.height()-1-y):y, image.pixel( x, y ));
		return result;
	}
	else {
		 QMessageBox msgBox;
		 msgBox.setText("No image in the viewport!\nThe plug-in will now close.");
		 msgBox.setIcon(QMessageBox::Warning);
		 msgBox.exec();
	}

	return image;
};

Q_EXPORT_PLUGIN2(DkFlipPlugin, DkFlipPlugin)

};

