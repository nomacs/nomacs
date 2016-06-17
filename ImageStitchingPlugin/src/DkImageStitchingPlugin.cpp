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

#include <opencv2/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <QFileDialog>

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

    if (files.size() != 2)
        return imgC;

    cv::Mat reference = cv::imread(files[0].toStdString());
    cv::Mat target = cv::imread(files[1].toStdString());

    cv::Mat grayRef, grayTarget;
    cv::cvtColor(reference,grayRef,CV_BGR2GRAY);
    cv::cvtColor(target,grayTarget,CV_BGR2GRAY);

    cv::SiftFeatureDetector detector;
    std::vector<cv::KeyPoint> keypoints1, keypoints2;
    detector.detect(grayRef,keypoints1);
    detector.detect(grayTarget,keypoints2);

    cv::SiftDescriptorExtractor extractor;
    cv::Mat descriptor1, descriptor2;
    extractor.compute(reference,keypoints1,descriptor1);
    extractor.compute(target,keypoints2,descriptor2);

    cv::BFMatcher matcher(cv::NORM_L2);
    std::vector<cv::DMatch> matches;
    matcher.match(descriptor1,descriptor2,matches);

    if (matches.empty())
        return imgC;

    double minDist = matches[0].distance;
    for (int i = 1; i < matches.size(); ++i)
    {
        if (matches[i].distance < minDist)
            minDist = matches[i].distance;
    }

    std::vector<cv::Point2f> queryPts;
    std::vector<cv::Point2f> trainPts;
    for (int i = 0; i < matches.size(); ++i)
    {
        if (matches[i].distance < 3.0*minDist)
        {
            int queryIdx = matches[i].queryIdx;
            int trainIdx = matches[i].trainIdx;
            queryPts.push_back(keypoints1[queryIdx].pt);
            trainPts.push_back(keypoints2[trainIdx].pt);
        }
    }

    ///Obtain the global homography and inliers
    std::vector<uchar> inliers_mask;
    cv::Mat globalH = cv::findHomography(queryPts,trainPts, inliers_mask, CV_RANSAC);

    std::vector<cv::Point2f> inliersTarget;
    std::vector<cv::Point2f> inliersReference;
    for (int i = 0; i < inliers_mask.size(); ++i)
    {
        if (inliers_mask[i])
        {
            inliersTarget.emplace_back(queryPts[i]);
            inliersReference.emplace_back(trainPts[i]);
        }
    }

    ///Build the A matrix with the matching points
    cv::Mat A(2*inliersTarget.size(),9,CV_32F);
    for (int i = 0; i < inliersTarget.size(); ++i)
    {
        const cv::Point2f &pTarget = inliersTarget[i];
        const cv::Point2f &pReference = inliersReference[i];

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

    ///Divide the reference image into CX*CY cells and calculate their
    ///local homographies.
    const int CX = 100;
    const int CY = 100;

    const int cellWidth = (reference.cols+CX-1)/CX;
    const int cellHeight = (reference.rows+CY-1)/CY;
    const float sigmaSquared = 12.5*12.5;

    std::vector<cv::Mat> localHomographies(CX*CY);
    cv::Mat Wi(2*inliersTarget.size(),2*inliersTarget.size(),CV_32F,0.0);
    for (int i = 0; i < CX; ++i)
    {
        for (int j = 0; j < CY; ++j)
        {
            int centerX = i*cellHeight;
            int centerY = j*cellWidth;

            ///Build W matrix for each cell center
            for (int k = 0; k < inliersTarget.size(); ++k)
            {
                cv::Point2f xk = inliersTarget[k];
                xk.x = centerX-xk.x;
                xk.y = centerY-xk.y;

                float w = exp(-1.0*sqrt(xk.x*xk.x+xk.y*xk.y)/sigmaSquared);
                Wi.at<float>(2*k,2*k) = w;
                Wi.at<float>(2*k+1,2*k+1) = w;
            }

            ///Calculate local homography for each cell
            cv::Mat w,u,vt;
            cv::SVD::compute(Wi*A,w,u,vt);

            float smallestSv = w.at<float>(0,0);
            int indexSmallestSv = 0;
            for (int k = 1; k < w.rows; ++k)
            {
                if (w.at<float>(k,0) < smallestSv)
                {
                    smallestSv = w.at<float>(k,0);
                    indexSmallestSv = k;
                }
            }

            ///Represent the homography as a 3x3 matrix
            cv::Mat localH(3,3,CV_64F,0.0);
            for (int k = 0; k < 9; ++k)
                localH.at<double>(k/3,k%3) = vt.row(indexSmallestSv).at<float>(k);

            if (localH.at<float>(2,2) < 0)
                localH *= -1;

            localHomographies[i*CY+j] = localH;
        }
    }

    ///Calculate canvas size using global homography
    cv::Point2f canvasCorners[4];
    canvasCorners[0] = cv::Point2f(0,0);
    canvasCorners[1] = cv::Point2f(reference.cols,0);
    canvasCorners[2] = cv::Point2f(0,reference.rows);
    canvasCorners[3] = cv::Point2f(reference.cols,reference.rows);

    for (int i = 0; i < 4; ++i)
    {
        cv::Mat pSrc(3,1,CV_64F,1.0);
        pSrc.at<double>(0,0) = canvasCorners[i].x;
        pSrc.at<double>(1,0) = canvasCorners[i].y;

        cv::Mat pDst = globalH*pSrc;

        double w = pDst.at<double>(2,0);
        canvasCorners[i].x = 0.5+(pDst.at<double>(0,0)/w);
        canvasCorners[i].y = 0.5+(pDst.at<double>(1,0)/w);
    }

    int minX = floor(canvasCorners[0].x);
    int minY = floor(canvasCorners[0].y);
    int maxX = minX;
    int maxY = minY;

    for (int i = 1; i < 4; ++i)
    {
        minX = std::min(minX,(int)floor(canvasCorners[i].x));
        minY = std::min(minY,(int)floor(canvasCorners[i].y));
        maxX = std::max(maxX,(int)floor(canvasCorners[i].x));
        maxY = std::max(maxY,(int)floor(canvasCorners[i].y));
    }

    int canvasWidth = std::max(target.cols,maxX)-minX;
    int canvasHeight = std::max(target.rows,maxY)-minY;

    ///Calculate translation vector to properly position the
    ///reference image.
    cv::Mat T = cv::Mat::eye(3,3,CV_64F);

    if (minX < 0)
        T.at<double>(0,2) = -minX;
    else
        canvasWidth += minX;

    if (minY < 0)
        T.at<double>(1,2) = -minY;
    else
        canvasHeight += minY;

    cv::Mat globalTH = T*globalH;

    cv::Mat result(canvasHeight,canvasWidth,CV_8UC3,cv::Scalar(0,0,0));
    for (int i = 0; i < CX; ++i)
    {
        for (int j = 0; j < CY; ++j)
        {
            for (int k = 0; k < cellHeight; ++k)
            {
                int pX = i*cellHeight+k;

                if (pX >= reference.rows)
                    break;

                for (int l = 0; l < cellWidth; ++l)
                {
                    int pY = j*cellWidth+l;

                    if (pY >= reference.cols)
                        break;

                    cv::Mat ptSrc(3,1,CV_64F,1.0);
                    ptSrc.at<double>(0,0) = pY;
                    ptSrc.at<double>(1,0) = pX;

                    cv::Mat ptDst = (T*localHomographies[i*CY+j])*ptSrc;
                    ptDst /= ptDst.at<double>(2,0);

                    int hX = ptDst.at<double>(0,0);
                    int hY = ptDst.at<double>(1,0);

                    if (hX >= 0 && hX < canvasWidth && hY >= 0 && hY < canvasHeight)
                        result.at<cv::Vec3b>(hY,hX) = reference.at<cv::Vec3b>(pX,pY);
                }
            }
        }
    }

    cv::Mat half(result,cv::Rect(std::max(0,-minX),std::max(0,-minY),target.cols,target.rows));
    target.copyTo(half);

    cv::cvtColor(result,result,CV_BGR2RGB);

    if (!imgC)
        imgC = QSharedPointer<nmc::DkImageContainer>(new nmc::DkImageContainer(QString("panoramic")));

    imgC->setImage(nmc::DkImage::mat2QImage(result),"","");
    return  imgC;
}

}

