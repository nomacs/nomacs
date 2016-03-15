/*******************************************************************************************************
 DkInstagramPlugin.cpp

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2015 Aly Osama

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

#include "DkInstagramPlugin.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#pragma warning(pop)		// no warnings from includes - end

 /*******************************************************************************************************
  * DkInstagramPlugin	- enter the plugin class name (e.g. DkPageExtractionPlugin)
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
DkInstagramPlugin::DkInstagramPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	runIds[ID_NASHVILLE] = "9efc5613c60649f39c63df684cced2fe";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[ID_NASHVILLE] = tr("Nashville");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[ID_NASHVILLE] = tr("Filter Image like Instagram Nashville");
	mMenuStatusTips = statusTips.toList();
}
/**
*	Destructor
**/
DkInstagramPlugin::~DkInstagramPlugin() {
}


/**
* Returns unique ID for the generated dll
**/
QString DkInstagramPlugin::id() const {

	return PLUGIN_ID;
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage DkInstagramPlugin::image() const {

	return QImage(":/InstagramFilterPlugin/img/instagramFilter.png");
};

/**
* Returns plugin version for every ID
* @param plugin ID
**/
QString DkInstagramPlugin::version() const {

	return PLUGIN_VERSION;
};

QList<QAction*> DkInstagramPlugin::createActions(QWidget* parent) {

	if (mActions.empty()) {

		for (int idx = 0; idx < id_end; idx++) {
			QAction* ca = new QAction(mMenuNames[idx], parent);
			ca->setObjectName(mMenuNames[idx]);
			ca->setStatusTip(mMenuStatusTips[idx]);
			ca->setData(mRunIDs[idx]);	// runID needed for calling function runPlugin()
			mActions.append(ca);
		}
	}

	return mActions;
}

QList<QAction*> DkInstagramPlugin::pluginActions() const {

	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> DkInstagramPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC) const {
	if (!imgC)
		return imgC;
	
	if(runID == mRunIDs[ID_NASHVILLE]) {
		//if (QMessageBox::Yes == QMessageBox(QMessageBox::Information, "Debug", "Run Code", QMessageBox::Yes|QMessageBox::No).exec());
		 QImage returnImg(imgC->image());
		 returnImg = DkInstagramPlugin::applyImageFilter(imgC->image());
		 imgC->setImage(returnImg, tr("Nashville"));
	}
	else
	 qWarning() << "Illegal run ID...";
	return imgC;
};


/**
 * applies Instagram-like filter to an image
 * @param Input image
 * @return image with filter applied
 **/
QImage DkInstagramPlugin::applyImageFilter(const QImage inImg){
#ifdef WITH_OPENCV
	int MAX_KERNEL_LENGTH = 31;
	Mat src =  DkImage::qImage2Mat(inImg);
	Mat dst;
	DkInstagramPlugin::nashville(src,dst);
	return (DkImage::mat2QImage(dst));
#else
	return inImg;
#endif
}


/**
 * Applies Instagram-like nashville filter
 * @param Input Mat image
 * @param Output Mat image
 **/
void DkInstagramPlugin::nashville(const Mat& src, Mat& dst)
{

    dst.create(src.size(), src.type());
    std::vector<Mat> channel(3);
    split(src, channel);

    // Approximated values for Nashville instagram filter 
    // Computed from tested 16 images in InstagramApplication
    uchar mappoint_bgr[3][16]=
    {{4,10,20,34,52,73,94,117,139,157,177,192,204,214,223,229},
    {23,41,63,87,115,139,163,185,204,219,230,238,244,247,250,252},
    {41,48,57,65,72,79,86,91,96,104,109,115,122,132,143,154}};

    for( int c = 0; c < 3; c++ ){
        for( int y = 0; y < src.rows; y++ ){ 
            for( int x = 0; x < src.cols; x++ ){

                // Compute the new color Value from mappoint_bgr Colors 

				uchar i =channel[c].at<uchar>(y,x);
				int mapIndex=i/16;
                float t = (i%16)/16.0; // Mapping Ratio of the new color

                uchar a,b;
                a=mappoint_bgr[c][mapIndex];
                // Check the index is greater or equal 15 
                // Because Number of elements in mappoints_bgr is 16 in each color channel
                if(mapIndex>=15) 
					b=mappoint_bgr[c][mapIndex];
				else
					b=mappoint_bgr[c][mapIndex+1];
          		
          		float newColor=a+t*(b-a);
          		
          		if(newColor<0)newColor=0.0f;
          		else if(newColor>255.0f)newColor=255.0f;
                channel[c].at<uchar>(y,x)= (uchar)qRound(newColor);
            }
        }
    }

    merge(channel,dst);

}


};

