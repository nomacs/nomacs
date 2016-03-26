/*******************************************************************************************************
 DkMarsPlugin.cpp

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

#include "DkMarsPlugin.h"
#include "DkImageStorage.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#pragma warning(pop)		// no warnings from includes - end

#ifdef WITH_OPENCV

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

static void lcl_applyMarsFilterOpenCV (QImage &img);

#else

#include <algorithm>

static void lcl_applyMarsFilterQt (QImage &img);

#endif

namespace nmc {

/**
*	Constructor
**/
DkMarsPlugin::DkMarsPlugin(QObject* parent) : QObject(parent) {

    // create run IDs
    QVector<QString> runIds;
    runIds.resize(id_end);

    runIds[ID_MARS] = "9e67b2694c1b427796e287414bd93ae6";
    mRunIDs = runIds.toList();

    // create menu actions
    QVector<QString> menuNames;
    menuNames.resize(id_end);

    menuNames[ID_MARS] = tr("Mars");
    mMenuNames = menuNames.toList();

    // create menu status tips
    QVector<QString> statusTips;
    statusTips.resize(id_end);

    statusTips[ID_MARS] = tr("Apply Mars filter to image.");
    mMenuStatusTips = statusTips.toList();
}
/**
*	Destructor
**/
DkMarsPlugin::~DkMarsPlugin() {
}


/**
* Returns unique ID for the generated dll
**/
QString DkMarsPlugin::id() const {

    return PLUGIN_ID;
}

/**
* Returns descriptive image for every ID
**/

QImage DkMarsPlugin::image() const {

    return QImage(":/MarsPlugin/img/preview.jpg");
}

/**
* Returns plugin version for every ID
* @param plugin ID
**/
QString DkMarsPlugin::version() const {

    return PLUGIN_VERSION;
}

QList<QAction*> DkMarsPlugin::createActions(QWidget* parent) {

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

QList<QAction*> DkMarsPlugin::pluginActions() const {

    return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin run ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> DkMarsPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC) const {

    if (!imgC)
        return imgC;

    if(runID == mRunIDs[ID_MARS]) {
        QImage img = imgC->image();

#ifdef WITH_OPENCV
        lcl_applyMarsFilterOpenCV(img);
#else
        lcl_applyMarsFilterQt(img);
#endif

        imgC->setImage(img,tr("Mars"));
    }
    else
        qWarning() << "Illegal run ID...";

    // wrong runID? - do nothing
    return imgC;
}

}

#ifdef WITH_OPENCV

void lcl_applyMarsFilterOpenCV (QImage &img)
{
    ///Multiply each pixel with the RGB mask (1.5,0.9,0.9)
    ///Give more weight to red pixels and less to green and blue.
    cv::Mat mat = nmc::DkImage::qImage2Mat(img);
    std::vector<cv::Mat> channels(3);
    cv::split(mat,channels);

    double factors[3] = { 0.9, 0.9, 1.9 }; ///BGR
    for (int i = 0; i < 3; ++i) {
        cv::convertScaleAbs(channels[i],channels[i],factors[i]);
    }

    cv::merge(channels,mat);
    img = nmc::DkImage::mat2QImage(mat);
}

#else

void lcl_applyMarsFilterQt (QImage &img)
{
    ///Multiply each pixel with the RGB mask (1.5,0.9,0.9)
    ///Give more weight to red and less to green and blue.
    for (int i = 0; i < img.width(); ++i) {
        for (int j = 0; j < img.height(); ++j) {
            QRgb val = img.pixel(i,j);
            int r = qRed(val);
            int g = qGreen(val);
            int b = qBlue(val);

            r = std::min(255,15*r/10);
            g = 9*g/10;
            b = 9*b/10;

            img.setPixel(i,j,qRgb(r,g,b));
        }
    }
}

#endif
