/*******************************************************************************************************
 DkImageStitchingPlugin.cpp

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2016 Rafael Dominguez

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

#include "DkImageStitchingPlugin.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#pragma warning(pop)		// no warnings from includes - end

#include <QFileDialog>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/stitching/stitcher.hpp>

#include <DkImageStorage.h>

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
DkImageStitchingPlugin::DkImageStitchingPlugin(QObject* parent) : QObject(parent)
{
    // create run IDs
    QVector<QString> runIds;
    runIds.resize(id_end);

    //runIds[ID_GENERATE_PANORAMIC] = "ef24147e87014fb1ad54ee0b5b683269";
    mRunIDs = runIds.toList();

    // create menu actions
    QVector<QString> menuNames;
    menuNames.resize(id_end);

    //menuNames[ID_GENERATE_PANORAMIC] = tr("Generate Panoramic Image");
    mMenuNames = menuNames.toList();

    // create menu status tips
    QVector<QString> statusTips;
    statusTips.resize(id_end);

    //statusTips[ID_GENERATE_PANORAMIC] = tr("Generate a panoramic image from a photo collection");
    mMenuStatusTips = statusTips.toList();
}

/**
*	Destructor
**/
DkImageStitchingPlugin::~DkImageStitchingPlugin()
{
}


/**
* Returns unique ID for the generated dll
**/
QString DkImageStitchingPlugin::id() const
{
    return PLUGIN_ID;
}

/**
* Returns descriptive image for every ID
* @param plugin ID
**/
QImage DkImageStitchingPlugin::image() const
{
    return QImage(":/#PLUGIN_NAME/img/your-image.png");
}

QList<QAction*> DkImageStitchingPlugin::createActions(QWidget* parent)
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

QList<QAction*> DkImageStitchingPlugin::pluginActions() const
{
    return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> DkImageStitchingPlugin::runPlugin(const QString& /*runID*/, QSharedPointer<nmc::DkImageContainer> imgC) const
{
    QStringList files = QFileDialog::getOpenFileNames(Q_NULLPTR,"Select photos");

    std::vector<cv::Mat> inputImgs;
    for (QString &file : files)
    {
        inputImgs.emplace_back(cv::imread(file.toStdString()));
    }


    cv::Stitcher stitcher = cv::Stitcher::createDefault();

    cv::Mat pano;
    cv::Stitcher::Status status = stitcher.stitch(inputImgs,pano);

    if (status != cv::Stitcher::Status::OK)
        qWarning() << "Failed to create panoramic image";
    else
    {
        cv::cvtColor(pano,pano,CV_BGR2RGB);
        imgC = QSharedPointer<nmc::DkImageContainer>(new nmc::DkImageContainer(""));
        imgC->setImage(nmc::DkImage::mat2QImage(pano),"Image");
    }

    return imgC;
}

}

