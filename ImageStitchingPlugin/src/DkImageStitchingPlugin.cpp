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

#include <iostream>
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

    ///Extract the matching points between the two images
    std::vector<cv::Point2f> inliersSrc(matchesInfo.num_inliers);
    std::vector<cv::Point2f> inliersDst(matchesInfo.num_inliers);

    for (int i = 0, l = 0; i < matchesInfo.inliers_mask.size(); ++i)
    {
        if (matchesInfo.inliers_mask[i])
        {
            cv::DMatch &m = matchesInfo.matches[i];
            cv::Point2f pTarget = features[0].keypoints[m.queryIdx].pt;
            pTarget.x -= features[0].img_size.width*0.5;
            pTarget.y -= features[0].img_size.height*0.5;

            cv::Point2f pReference = features[1].keypoints[m.trainIdx].pt;
            pReference.x -= features[1].img_size.width*0.5;
            pReference.y -= features[1].img_size.height*0.5;

            inliersSrc[l] = pTarget;
            inliersDst[l] = pReference;
            ++l;
        }
    }

    ///Build the A matrix with the matching points
    cv::Mat A(2*matchesInfo.num_inliers,9,CV_32F);
    for (int i = 0; i < matchesInfo.num_inliers; ++i)
    {
        const cv::Point2f &pTarget = inliersSrc[i];
        const cv::Point2f &pReference = inliersDst[i];

        A.at<float>(2*i,0) = 0.0;
        A.at<float>(2*i,1) = 0.0;
        A.at<float>(2*i,2) = 0.0;
        A.at<float>(2*i,3) = -pTarget.x;
        A.at<float>(2*i,4) = -pTarget.y;
        A.at<float>(2*i,5) = -1.0;
        A.at<float>(2*i,6) = pReference.y*pTarget.x;
        A.at<float>(2*i,7) = pReference.y*pTarget.y;
        A.at<float>(2*i,8) = pReference.y;

        A.at<float>(2*i+1,0) = pTarget.x;
        A.at<float>(2*i+1,1) = pTarget.y;
        A.at<float>(2*i+1,2) = 1.0;
        A.at<float>(2*i+1,3) = 0.0;
        A.at<float>(2*i+1,4) = 0.0;
        A.at<float>(2*i+1,5) = 0.0;
        A.at<float>(2*i+1,6) = -pReference.x*pTarget.x;
        A.at<float>(2*i+1,7) = -pReference.x*pTarget.y;
        A.at<float>(2*i+1,8) = -pReference.x;
    }

    std::cout << matchesInfo.H << "\n";

    ///Calculate canvas size using global homography
    double canvasCornersCoords[8] = {
        0.0, 0.0, //TL
        0.0, features[1].img_size.width, //BL
        features[1].img_size.height, 0.0, //TR
        features[1].img_size.height, features[1].img_size.width //BR
    };

    int maxX = 1, minX = 1;
    int maxY = 1, minY = 1;
    for (int i = 0; i < 4; ++i)
    {
        cv::Mat pt = cv::Mat(3,1,CV_64F,1.0);
        pt.at<double>(0,0) = canvasCornersCoords[2*i];
        pt.at<double>(1,0) = canvasCornersCoords[2*i+1];

        cv::Mat corner;
        cv::solve(matchesInfo.H,pt,corner);

        double w = corner.at<double>(2,0);
        int x = ceil(corner.at<double>(0,0)/w);
        int y = ceil(corner.at<double>(1,0)/w);
        maxX = std::max(maxX,x);
        minX = std::min(minX,x);
        maxY = std::max(maxY,y);
        minY = std::min(minY,y);
    }

    int canvasWidth = maxX - minX + 1;
    int canvasHeight = maxY - minY + 1;

    std::cout << canvasWidth << " " << canvasHeight << "\n";

    ///Calculate offset using global homography
    cv::Point2i offset;
    offset.x = 2 - std::min(features[0].img_size.width,minX);
    offset.y = 2 - std::min(features[0].img_size.height,minY);

    std::cout << offset << std::endl;

    ///Divide the reference image into CX*CY cells and calculate their
    ///local homographies.
    const int CX = 100;
    const int CY = 100;

    const int cellWidth = images[1].size[0]/CX;
    const int cellHeight = images[1].size[1]/CY;
    const float sigmaSquared = 12.5*12.5;

    cv::SVD svdSolver;
    std::vector<cv::Mat> localHomographies(CX*CY);
    cv::Mat Wi(2*matchesInfo.num_inliers,2*matchesInfo.num_inliers,CV_32F,0.0);
    for (int i = 0; i < CX; ++i)
    {
        for (int j = 0; j < CY; ++j)
        {
            int centerX = i*cellWidth+cellWidth/2;
            int centerY = j*cellHeight+cellHeight/2;

            ///Build W matrix for each cell center
            for (int k = 0; k < matchesInfo.num_inliers; ++k)
            {
                cv::Point2f xk = inliersSrc[k];
                xk.x = centerX-xk.x-offset.x;
                xk.y = centerY-xk.y-offset.y;

                float w = exp(-1.0*sqrt(xk.x*xk.x+xk.y*xk.y)/sigmaSquared);
                Wi.at<float>(2*k,2*k) = w;
                Wi.at<float>(2*k+1,2*k+1) = w;
            }

            //std::cout << "W = " << Wi << "\n";
            ///Calculate local homography for each cell
            svdSolver(Wi*A);
            cv::normalize(svdSolver.w,localHomographies[i*CY+j]);
        }
    }

    for (int i = 0; i < CX; ++i)
    {
        for (int j = 0; j < CY; ++j)
        {
            float H[9];
            for (int k = 0; k < 9; ++k)
                H[k] = localHomographies[i*CY+j].at<float>(k);

            //std::cout << "Local H = " << localHomographies[i*CY+j] << "\n";

            for (int k = 0; k < cellWidth; ++k)
            {
                for (int l = 0; l < cellHeight; ++l)
                {
                    int pX = i*cellWidth+k-offset.x;
                    int pY = j*cellHeight+l-offset.y;

                    int hX = (H[0]*pX+H[1]*pY+H[2])/(H[6]*pX+H[7]*pY+H[8]);
                    int hY = (H[3]*pX+H[4]*pY+H[5])/(H[6]*pX+H[7]*pY+H[8]);

                    //std::cout << "(" << pX << " , " << pY << ") = " << hX << " " << hY << "\n";
                }
            }
        }
    }

    fflush(stdout);

    return imgC;
}

}

