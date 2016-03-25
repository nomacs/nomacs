/*******************************************************************************************************
 PLUGIN_CLASS_NAME.cpp

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

#include "instaFilter.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#pragma warning(pop)		// no warnings from includes - end

 /*******************************************************************************************************
  * PLUGIN_CLASS_NAME	- instaFilterPlugin
  * #YOUR_NAME			- Praveen Agrawal
  * #DATE				- 24-03-2016
  * #DESCRIPTION		- Implmentation of the Xpro-II filter of intagram
  * #MENU_NAME			- Xpro-II
  * #MENU_STATUS_TIPP	- Get a vintage look to your image
  * #RUN_ID_1			- dcf932f26fe54125bf92b5bd55cb9730
  * ID_ACTION1			- XproII Filter
  * #ACTION_NAME1		- your action name (e.g. Flip Horizotally - user friendly!)
  * #ACTION_TIPP1		- your action status tip (e.g. Flips an image horizontally - user friendly!)
  *******************************************************************************************************/

namespace nmc {

/**
*	Constructor
**/
instaFilter::instaFilter(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	runIds[ID_ACTION1] = "dcf932f26fe54125bf92b5bd55cb9730";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[ID_ACTION1] = tr("Xpro-II");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[ID_ACTION1] = tr("Apply the Xpro-II filter of instagram to get a vintage look");
	mMenuStatusTips = statusTips.toList();
}
/**
*	Destructor
**/
instaFilter::~instaFilter() {
}


/**
* Returns unique ID for the generated dll
**/
QString instaFilter::id() const {

	return PLUGIN_ID;
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage instaFilter::image() const {

	return QImage(":/#PLUGIN_NAME/img/your-image.png");
};

/**
* Returns plugin version for every ID
* @param plugin ID
**/
QString instaFilter::version() const {

	return PLUGIN_VERSION;
};

QList<QAction*> instaFilter::createActions(QWidget* parent) {

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

QList<QAction*> instaFilter::pluginActions() const {

	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> instaFilter::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC) const {

	if (!imgC)
		return imgC;

	if(runID == mRunIDs[ID_ACTION1]) {
		QImage img(imgC->image());
		img = instaFilter::xProFilter(img);
		imgC->setImage(img, tr("xPro_II_Filter"));
	}
	else
		qWarning() << "Illegal run ID...";

	// wrong runID? - do nothing
	return imgC;
};

QImage instaFilter::xProFilter(QImage imgInput)
{
    cv::Mat img = nmc::DkImage::qImage2Mat(imgInput);
    
    //Mask to create the vignette effect
    cv::Mat mask(img.size(), CV_64F);
    mask.setTo(cv::Scalar(1));
    
    //The vignette effect is focused on the center of the image. We can change this if we want to focus at any other point
    cv::Point center = cv::Point(mask.size().width/2, mask.size().height/2);

    //The four corners of the image
    std::vector<cv::Point> corners(4);
    corners[0] = cv::Point(0, 0);
    corners[1] = cv::Point(mask.size().width, 0);
    corners[2] = cv::Point(0, mask.size().height);
    corners[3] = cv::Point(mask.size().width, mask.size().height);

    //Finding the maximum radius from the center point (All four radii are same when image center is taken as center of focus of the vignette)
    double maxImageRadius = 0;
    for (int i = 0; i < 4; ++i)
    {
        double d = sqrt(pow((double) (corners[i].x - center.x), 2) + pow((double) (corners[i].y - center.y), 2));
        if (maxImageRadius < d)
            maxImageRadius = d;
    }

    //Creating the vignette mask
    for (int i = 0; i < mask.rows; i++)
    {
        for (int j = 0; j < mask.cols; j++)
        {
            double temp = (sqrt(pow((double) (center.x - j), 2) + pow((double) (center.y - i), 2))) / maxImageRadius;
            temp = temp * 0.5;
            mask.at<double>(i, j) = pow(cos(temp), 4);
        }
    }

    //Convert image from RGB to Lab scale
    //cv::Mat imgLAB(img.size(), CV_64F);
    //cv::cvtColor(img, imgLAB, CV_RGB2Lab);

    //Apply the mask to the L channel (Luminance channel)
    std::vector<cv::Mat> channels(3);
    //cv::split(imgLAB, channels);
    cv::split(img, channels);
    for (int i = 0; i < img.size().height; i++)
    {
        for (int j = 0; j < img.size().width; j++)
        {
            /*cv::Vec3b value = imgLAB.at<cv::Vec3b>(i, j);
            value.val[0] *= mask.at<double>(i, j);  //Changing only the Luminance value so as to change only brightness
            imgLAB.at<cv::Vec3b>(i, j) =  value;*/

            // Vignetting
            channels[0].at<uchar>(i,j) *= mask.at<double>(i, j);
            channels[1].at<uchar>(i,j) *= mask.at<double>(i, j);
            channels[2].at<uchar>(i,j) *= mask.at<double>(i, j);

            //Colour Filter
            if(channels[2].at<uchar>(i,j)>127)  //Red
                channels[2].at<uchar>(i,j) = (int)(-0.00555*pow(channels[2].at<uchar>(i,j),2) + 3.123*channels[2].at<uchar>(i,j) - 180.4579);
            else
                channels[2].at<uchar>(i,j) = (int)(0.00608*pow(channels[2].at<uchar>(i,j),2) + 0.22425*channels[2].at<uchar>(i,j));
            
            if(channels[1].at<uchar>(i,j)>127)  //Green
                channels[1].at<uchar>(i,j) = (int)(-0.00555*pow(channels[1].at<uchar>(i,j),2) + 3.123*channels[1].at<uchar>(i,j) - 180.4579);
            else if(channels[1].at<uchar>(i,j)<=127)
                channels[1].at<uchar>(i,j) = (int)(0.00608*pow(channels[1].at<uchar>(i,j),2) + 0.22425*channels[1].at<uchar>(i,j));

            //Blue
            channels[0].at<uchar>(i,j) = (int)(0.804*channels[0].at<uchar>(i,j) + 20.0);

            /*if(channels[0].at<uchar>(i,j)>127)    //Green
                channels[0].at<uchar>(i,j) = (int)(-0.00555*pow(channels[0].at<uchar>(i,j),2) + 3.123*channels[0].at<uchar>(i,j) - 180.4579);
            else if(channels[0].at<uchar>(i,j)<=127)
                channels[0].at<uchar>(i,j) = (int)(0.00608*pow(channels[0].at<uchar>(i,j),2) + 0.22425*channels[0].at<uchar>(i,j));
*/

            if(channels[0].at<uchar>(i,j)>255)
                channels[0].at<uchar>(i,j) = 255;
            if(channels[0].at<uchar>(i,j)<0)
                channels[0].at<uchar>(i,j) = 0;
            
            if(channels[1].at<uchar>(i,j)>255)
                channels[1].at<uchar>(i,j) = 255;
            if(channels[1].at<uchar>(i,j)<0)
                channels[1].at<uchar>(i,j) = 0;
            
            if(channels[2].at<uchar>(i,j)>255)
                channels[2].at<uchar>(i,j) = 255;
            if(channels[2].at<uchar>(i,j)<0)
                channels[2].at<uchar>(i,j) = 0;

        }
    }



    //cv::merge(channels,imgLAB);
    cv::merge(channels,img);

    //Convert back to the RGB colour scale
    //cv::Mat output(img.size(), CV_8UC3);
    //cv::cvtColor(imgLAB, output, CV_Lab2RGB); //Convert back to RGB

    
    QImage imgOut = nmc::DkImage::mat2QImage(img);
    return imgOut;

    // Initial approach was to convert image to Lab colour scale from RGB and then modify only the L scale (luminance scale) as it only corresponds to the luminance of the image and not the colour. And then finally convert the modified Lab scale image back to RGB.
    //However, doing the above was changing the colours too because of some reasons not yet known to me. So finally the brightness change is done in the RGB scale itself.

}

};