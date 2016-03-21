/*******************************************************************************************************
 DkInstagramLikePlugin.cpp

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

#include "DkInstagramLikePlugin.h"

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

namespace nmc
{

/**
*	Constructor
**/
DkInstagramLikePlugin::DkInstagramLikePlugin(QObject* parent) : QObject(parent)
{

    // create run IDs
    QVector<QString> runIds;
    runIds.resize(id_end);

    runIds[ID_LILY] = "7da191b417794ebc8b88947fbe75910c";
    mRunIDs = runIds.toList();

    // create menu actions
    QVector<QString> menuNames;
    menuNames.resize(id_end);

    menuNames[ID_LILY] = tr("Lily");
    mMenuNames = menuNames.toList();

    // create menu status tips
    QVector<QString> statusTips;
    statusTips.resize(id_end);

    statusTips[ID_LILY] = tr("Instagram Like Filter Lily");
    mMenuStatusTips = statusTips.toList();
}
/**
*	Destructor
**/
DkInstagramLikePlugin::~DkInstagramLikePlugin()
{
}


/**
* Returns unique ID for the generated dll
**/
QString DkInstagramLikePlugin::id() const
{

    return PLUGIN_ID;
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage DkInstagramLikePlugin::image() const
{

    return QImage(":/InstragramLikeFilterPlugin/img/InstagramLikeFilter.png");
};

/**
* Returns plugin version for every ID
* @param plugin ID
**/
QString DkInstagramLikePlugin::version() const
{

    return PLUGIN_VERSION;
};

QList<QAction*> DkInstagramLikePlugin::createActions(QWidget* parent)
{

    if (mActions.empty())
    {

        for (int idx = 0; idx < id_end; idx++)
        {
            QAction* ca = new QAction(mMenuNames[idx], parent);
            ca->setObjectName(mMenuNames[idx]);
            ca->setStatusTip(mMenuStatusTips[idx]);
            ca->setData(mRunIDs[idx]);	// runID needed for calling function runPlugin()
            mActions.append(ca);
        }
    }

    return mActions;
}

QList<QAction*> DkInstagramLikePlugin::pluginActions() const
{

    return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> DkInstagramLikePlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC) const
{

    if (!imgC)
        return imgC;

    if(runID == mRunIDs[ID_LILY])
    {
        QImage returnImg(imgC->image());
        returnImg = DkInstagramLikePlugin::applyLilyFilter(imgC->image());
        imgC->setImage(returnImg, tr("Lily"));
    }
    else
        qWarning() << "Illegal run ID...";

    // wrong runID? - do nothing
    return imgC;
};
QImage DkInstagramLikePlugin::applyLilyFilter(QImage inImg)
{
    cv::Mat src=nmc::DkImage::qImage2Mat(inImg);
    int width=src.cols;
    int heigh=src.rows;
    RNG rng;
    if(src.type() == CV_8UC3)
    {
        Mat img(src.size(),CV_8UC3);
        for (int y=0; y<heigh; y++)
        {
            uchar* P0 = src.ptr<uchar>(y);
            uchar* P1 = img.ptr<uchar>(y);
            for (int x=0; x<width; x++)
            {
                float B=P0[3*x];
                float G=P0[3*x+1];
                float R=P0[3*x+2];
                float newB=0.272*R+0.534*G+0.131*B;
                float newG=0.349*R+0.686*G+0.168*B;
                float newR=0.393*R+0.769*G+0.189*B;
                if(newB<0)newB=0;
                if(newB>255)newB=255;
                if(newG<0)newG=0;
                if(newG>255)newG=255;
                if(newR<0)newR=0;
                if(newR>255)newR=255;
                P1[3*x] = (uchar)newB;
                P1[3*x+1] = (uchar)newG;
                P1[3*x+2] = (uchar)newR;
            }

        }

        QImage out = nmc::DkImage::mat2QImage(img);
        return out;
    }
    else
    {
        Mat img(src.size(),CV_8UC4);
        for (int y=0; y<heigh; y++)
        {
            uchar* P0 = src.ptr<uchar>(y);
            uchar* P1 = img.ptr<uchar>(y);
            for (int x=0; x<width; x++)
            {
                float B=P0[4*x];
                float G=P0[4*x+1];
                float R=P0[4*x+2];
                float newB=0.272*R+0.534*G+0.131*B;
                float newG=0.349*R+0.686*G+0.168*B;
                float newR=0.393*R+0.769*G+0.189*B;
                if(newB<0)newB=0;
                if(newB>255)newB=255;
                if(newG<0)newG=0;
                if(newG>255)newG=255;
                if(newR<0)newR=0;
                if(newR>255)newR=255;
                P1[4*x] = (uchar)newB;
                P1[4*x+1] = (uchar)newG;
                P1[4*x+2] = (uchar)newR;
                P1[4*x+3] = P0[4*x+3];
            }

        }

        QImage out = nmc::DkImage::mat2QImage(img);
        return out;
    }
};

};

