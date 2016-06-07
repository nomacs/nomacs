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
#include <opencv2/stitching/detail/matchers.hpp>

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

    mRunIDs = runIds.toList();

    // create menu actions
    QVector<QString> menuNames;
    menuNames.resize(id_end);

    mMenuNames = menuNames.toList();

    // create menu status tips
    QVector<QString> statusTips;
    statusTips.resize(id_end);

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

    cv::Ptr<cv::detail::FeaturesFinder> featureFinder = cv::makePtr<cv::detail::OrbFeaturesFinder>();
    std::vector<cv::detail::ImageFeatures> features(files.size());
    std::vector<cv::Mat> images;
    for (int i = 0; i < files.size(); ++i)
    {
        cv::Mat img = cv::imread(files[i].toStdString());

        if (img.empty())
            continue;

        images.emplace_back(img);

        (*featureFinder)(img,features[i]);
    }

    featureFinder->collectGarbage();

    cv::detail::BestOf2NearestMatcher matcher;

    cv::detail::MatchesInfo matchesInfo;
    matcher(features[0],features[1],matchesInfo);

    ///Build the A matrix with the matching points

    cv::Mat A(2*matchesInfo.num_inliers,9,CV_32F);
    for (int i = 0; i < matchesInfo.inliers_mask.size(); ++i)
    {
        if (!matchesInfo.inliers_mask[i])
            continue;

        cv::DMatch &m = matchesInfo.matches[i];
        cv::Point2f pTarget = features[0].keypoints[m.queryIdx].pt;
        cv::Point2f pReference = features[1].keypoints[m.trainIdx].pt;

        A.at<float>(0,2*i) = 0.0;
        A.at<float>(1,2*i) = 0.0;
        A.at<float>(2,2*i) = 0.0;
        A.at<float>(3,2*i) = -pReference.x;
        A.at<float>(4,2*i) = -pReference.y;
        A.at<float>(5,2*i) = 1.0;
        A.at<float>(6,2*i) = pTarget.y*pReference.x;
        A.at<float>(7,2*i) = pTarget.y*pReference.y;
        A.at<float>(8,2*i) = pTarget.y;

        A.at<float>(0,2*i+1) = pReference.x;
        A.at<float>(1,2*i+1) = pReference.y;
        A.at<float>(2,2*i+1) = 1.0;
        A.at<float>(3,2*i+1) = 0.0;
        A.at<float>(4,2*i+1) = 0.0;
        A.at<float>(5,2*i+1) = 0.0;
        A.at<float>(6,2*i+1) = -pTarget.x*pReference.x;
        A.at<float>(7,2*i+1) = -pTarget.x*pReference.y;
        A.at<float>(8,2*i+1) = -pTarget.x;
    }

    cv::Mat Wi(2*matchesInfo.num_inliers,2*matchesInfo.num_inliers,CV_32F);

    const int CX = 25;
    const int CY = 25;

    const int cellWidth = images[1].size[0]/CX;
    const int cellHeight = images[1].size[1]/CY;
    const float sigmaSquared = 12.0*12.0;

    cv::SVD svdSolver;
    std::vector<cv::Mat> localHomographies(CX*CY);
    for (int i = 0; i < CX; ++i)
    {
        for (int j = 0; j < CY; ++j)
        {
            int centerX = i*cellWidth+cellWidth/2;
            int centerY = j*cellHeight+cellHeight/2;

            ///Build W matrix for each cell center
            for (int k = 0; k < matchesInfo.num_inliers; ++k)
            {
                cv::DMatch &m = matchesInfo.matches[i];
                cv::Point2f xk = features[1].keypoints[m.trainIdx].pt;

                float dx = xk.x-centerX;
                float dy = xk.y-centerY;
                float w = exp(-1.0*sqrt(dx*dx+dy*dy)/sigmaSquared);
                Wi.at<float>(2*k,2*k) = w;
                Wi.at<float>(2*k+1,2*k+1) = w;
            }

            ///Calculate local homography for each cell
            svdSolver(Wi*A,cv::SVD::FULL_UV);
            localHomographies[i*CX+j] = svdSolver.w;
        }
    }

    return imgC;
}

}

