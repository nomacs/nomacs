/*******************************************************************************************************
 DkSkewEstimator.h
 Created on:	28.06.2014

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2014 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2014 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2014 Florian Kleber <florian@nomacs.org>

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

#pragma once

#include <QDebug>
#include <QImage>
#include <QProgressDialog>
#include <QVector3D>
#include <QVector4D>
#include <QWidget>
#include <QtCore/qmath.h>
#include <QtGlobal>
#include <cmath>

#ifdef WITH_OPENCV
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#endif

namespace nmp
{

class DkSkewEstimator
{
public:
    enum {
        dir_horizontal = 0,
        dir_vertical,

        dir_end,
    };

    explicit DkSkewEstimator(QWidget *mainWin = nullptr);
    ~DkSkewEstimator();

    double getSkewAngle();
    QVector<QVector4D> getLines();
    QVector<int> getLineTypes();
    void setImage(QImage inImage);

private:
    cv::Mat computeSeparability(cv::Mat integral, cv::Mat integralSq, int direction);
    cv::Mat computeEdgeMap(cv::Mat separability, double thr, int direction);
    QVector<QVector3D> computeWeights(cv::Mat edgeMap, int direction);
    double computeSkewAngle(QVector<QVector3D> weights, double imgDiagonal);
    int randInt(int low, int high);

    static constexpr int kIter = 200;
    static constexpr double kSigma = 0.3;
    static constexpr double kSepThr = 0.1;
    static constexpr int kEpsilon = 2;
    static constexpr int kMaxK = 7;

    QSize mSepDims{0, 0};
    int mDelta = 0;
    int mMinLineLength = 10;
    int mMinLineProjLength = mMinLineLength / 2;
    int mRotationFactor = 1;

    QVector<QVector4D> mSelectedLines;
    QVector<int> mSelectedLineTypes;
    cv::Mat mMatImg;
    QProgressDialog *mProgress;
    QWidget *mMainWin;
};

};
