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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QImage>
#include <QtCore/qmath.h>
#include <QtGlobal>
#include <QVector3D>
#include <QVector4D>
#include <cmath>
#include <QProgressDialog>
#include <QWidget>
#include <QDebug>
#pragma warning(pop)		// no warnings from includes - end

// opencv
#ifdef WITH_OPENCV

#ifdef WIN32
#pragma warning(disable: 4996)
#endif

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#endif


namespace nmp {


class DkSkewEstimator {

public:

	enum {
		dir_horizontal = 0,
		dir_vertical,

		dir_end,
	};

	DkSkewEstimator(QWidget* mainWin = 0);
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

	int nIter;
	QSize sepDims;
	int delta;
	double sigma;
	double sepThr;
	int epsilon;
	int kMax;
	int minLineLength;
	int minLineProjLength;
	
	QVector<QVector4D> selectedLines;
	QVector<int> selectedLineTypes;
	cv::Mat matImg;
	int rotationFactor;
	QProgressDialog* progress;
	QWidget* mainWin;
};

};
