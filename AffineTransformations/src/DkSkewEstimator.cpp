/*******************************************************************************************************
 DkImgTransformationsPlugin.cpp
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

#include "DkSkewEstimator.h"

namespace nmc {

DkSkewEstimator::DkSkewEstimator(QImage inImage) {

	// method parameters
	nIter = 300;
	sigma = 0.3;
	sepThr = 0.1,
	epsilon = 2;
	kMax = 7;

	matImg = DkImage::qImage2Mat(inImage);
	
	sepDims = QSize(qRound(inImage.width()/1430.0*49.0),qRound(inImage.height()/700.0*12.0));
	delta = qRound(inImage.height()/1430.0*20.0);

	if (inImage.width() < inImage.height()) {

		matImg = matImg.t();
		sepDims = QSize(qRound(inImage.width()/1430.0*49.0),qRound(inImage.height()/700.0*12.0));
		delta = qRound(inImage.height()/1430.0*20.0);
	}

	if (sepDims.width() < 1) sepDims.setWidth(1);
	if (sepDims.height() < 1) sepDims.setHeight(1);

	QTime time = QTime::currentTime();
	qsrand((uint)time.msec());
}

DkSkewEstimator::~DkSkewEstimator() {

}

double DkSkewEstimator::getSkewAngle() {

	cv::Mat matGray;

	if (matImg.channels() > 1)
		cv::cvtColor(matImg, matGray, CV_BGR2GRAY);
	else matGray = matImg;

	cv::Mat integral, integralSq;
	cv::integral(matGray, integral, integralSq, CV_64F);
	if (integral.channels() > 1) qDebug() << "Error! integral image has more than one channel";

	cv::Mat separabilityHor = computeSeparability(integral, integralSq, dir_horizontal);
	cv::Mat separabilityVer = computeSeparability(integral, integralSq,  dir_vertical);

	double min, max;
	cv::minMaxLoc(separabilityHor, &min, &max);	
	cv::Mat edgeMapHor = computeEdgeMap(separabilityHor, sepThr * max, dir_horizontal);
	//cv::Mat edgeMapHor = computeEdgeMap(separabilityHor, 0.1, dir_horizontal);
	cv::minMaxLoc(separabilityVer, &min, &max);
	cv::Mat edgeMapVer = computeEdgeMap(separabilityVer, sepThr * max, dir_vertical);
	//cv::Mat edgeMapVer = computeEdgeMap(separabilityVer, 0.1, dir_vertical);

	QVector<QVector3D> weightsHor = computeWeights(edgeMapHor, dir_horizontal);
	qDebug() << weightsHor.size();
	QVector<QVector3D> weightsVer = computeWeights(edgeMapVer, dir_vertical);
	qDebug() << weightsVer.size();

	weightsHor += weightsVer;

	return computeSkewAngle(weightsHor, qSqrt(matGray.rows*matGray.rows + matGray.cols*matGray.cols));
}

cv::Mat DkSkewEstimator::computeSeparability(cv::Mat integral, cv::Mat integralSq, int direction) {

	cv::Mat separability = Mat::zeros(integral.rows, integral.cols, CV_32FC1);

	int W2 = qCeil(sepDims.width()/2);
	int H2 = qCeil(sepDims.height()/2);

	if (direction == dir_horizontal) {

		for (int r = H2 + qCeil(delta/2); r < integral.rows - H2 - qCeil(delta/2); r++) {
			for (int c = W2 + qCeil(delta/2); c < integral.cols - W2 - qCeil(delta/2); c++) {

				double mean1 = integral.at<double>(r - H2, c - W2) + integral.at<double>(r - 1, c + W2) - integral.at<double>(r - H2, c + W2) - integral.at<double>(r - 1, c - W2);
				double mean2 = integral.at<double>(r + 1, c - W2) + integral.at<double>(r + H2, c + W2) - integral.at<double>(r + 1, c + W2) - integral.at<double>(r + H2, c - W2);
				mean1 /= (2 * W2 * H2);
				mean2 /= (2 * W2 * H2);

				double var1 = integralSq.at<double>(r - H2, c - W2) + integralSq.at<double>(r - 1, c + W2) - integralSq.at<double>(r - H2, c + W2) - integralSq.at<double>(r - 1, c - W2);
				double var2 = integralSq.at<double>(r + 1, c - W2) + integralSq.at<double>(r + H2, c + W2) - integralSq.at<double>(r + 1, c + W2) - integralSq.at<double>(r + H2, c - W2);
				var1 /= (2 * W2 * H2);
				var2 /= (2 * W2 * H2);

				var1 = var1 - (mean1 * mean1);
				var2 = var2 - (mean2 * mean2);

				separability.at<float>(r, c) = (float)((mean1 - mean2) * (mean1 - mean2) / (var1 + var2));
			}
		}
	}
	else {
		for (int r = W2 + qCeil(delta/2); r < integral.rows - W2 - qCeil(delta/2); r++) {
			for (int c = H2 + qCeil(delta/2); c < integral.cols - H2 - qCeil(delta/2); c++) {

				double mean1 = integral.at<double>(r - W2, c - H2) + integral.at<double>(r + W2, c - 1) - integral.at<double>(r + W2, c - H2) - integral.at<double>(r - W2, c - 1);
				double mean2 = integral.at<double>(r - W2, c + 1) + integral.at<double>(r + W2, c + H2) - integral.at<double>(r + W2, c + 1) - integral.at<double>(r - W2, c + H2);
				mean1 /= (2 * W2 * H2);
				mean2 /= (2 * W2 * H2);

				double var1 = integralSq.at<double>(r - W2, c - H2) + integralSq.at<double>(r + W2, c - 1) - integralSq.at<double>(r + W2, c - H2) - integralSq.at<double>(r - W2, c - 1);
				double var2 = integralSq.at<double>(r - W2, c + 1) + integralSq.at<double>(r + W2, c + H2) - integralSq.at<double>(r + W2, c + 1) - integralSq.at<double>(r - W2, c + H2);
				var1 /= (2 * W2 * H2);
				var2 /= (2 * W2 * H2);

				var1 = var1 - (mean1 * mean1);
				var2 = var2 - (mean2 * mean2);

				separability.at<float>(r, c) = (float)((mean1 - mean2) * (mean1 - mean2) / (var1 + var2));
			}
		}
	}

	// for displaying:
	// cv::normalize(separability, separability, 0, 255, NORM_MINMAX, CV_8UC1);
	// cvtColor(separability, separability, CV_GRAY2RGB);

	return separability;
}

cv::Mat DkSkewEstimator::computeEdgeMap(cv::Mat separability, double thr, int direction) {

	int tmpStatus;

	int W2 = qCeil(sepDims.width()/2);
	int H2 = qCeil(sepDims.height()/2);

	cv::Mat edgeMap = Mat::zeros(separability.rows, separability.cols, CV_8UC1);

	if (direction == dir_horizontal) {

		float* p;
		for (int r = H2 + kMax; r < separability.rows - H2 - kMax; r++) {

			p = separability.ptr<float>(r);
			for (int c = W2; c < separability.cols - W2; c++) {

				if (p[c] > thr) {
					tmpStatus = 1;
					for (int k = -kMax; k <= kMax; k++) {
						if (k==0) k++;
						float* pK;
						pK = separability.ptr<float>(r + k);
						if (pK[c] > p[c]) {
							tmpStatus = 0;
							break;
						}
					}

					if(tmpStatus) {
						uchar* pEM;
						pEM = edgeMap.ptr<uchar>(r);
						pEM[c] = 1;
					}
				}
			}
		}
	}
	else  {

		float* p;
		for (int r = W2; r < separability.rows - W2; r++) {

			p = separability.ptr<float>(r);
			for (int c = H2 + kMax; c < separability.cols - H2 - kMax; c++) {

				if (p[c] > thr) {
					tmpStatus = 1;
					for (int k = -kMax; k <= kMax; k++) {
						if (k==0) k++;
						if (p[c+k] > p[c]) {
							tmpStatus = 0;
							break;
						}
					}

					if(tmpStatus) {
						uchar* pEM;
						pEM = edgeMap.ptr<uchar>(r);
						pEM[c] = 1;
					}
				}
			}
		}
	}

	return edgeMap;
}

int DkSkewEstimator::randInt(int low, int high) {

	return qrand() % ((high + 1) - low) + low;
}

QVector<QVector3D> DkSkewEstimator::computeWeights(cv::Mat edgeMap, int direction) {

	vector<Vec4i> lines;
	HoughLinesP(edgeMap, lines, 1, CV_PI/180, 50, 20, 20 ); //params: rho resolution, theta resolution, threshold, min Line length, max line gap

	QVector<QVector3D> computedWeights = QVector<QVector3D>();

	for(size_t i = 0; i < lines.size(); i++) {

		Vec4i l = lines[i];		
		QVector3D currMax = QVector3D(0.0, 0.0, 0.0);

		if (direction == dir_horizontal) {

			for (int K = 0; K < nIter; K++) {
		
				int xr1 = randInt(l[0],l[2]);
				while(xr1 >= edgeMap.cols) xr1 = randInt(l[0],l[2]);
				int xr2 = randInt(l[0],l[2]);
				while (xr1 == xr2 || xr2 >= edgeMap.cols) xr2 = randInt(l[0],l[2]); // the line should be enough long

				double slope = (l[3] - l[1]) / (double)(l[2] - l[0]);
				double inters = l[1] - slope * l[0];
				int y1 = qRound(slope * xr1 + inters);
				int y2 = qRound(slope * xr2 + inters);

				if (y1 >= edgeMap.rows) y1 = edgeMap.rows - 1; // doesn't happen very often - just a precaution
				if (y2 >= edgeMap.rows) y2 = edgeMap.rows - 1;

				QVector<int> yrPoss1 = QVector<int>();
				for (int di = -delta; di <= delta; di++) {
					if (edgeMap.at<uchar>(y1 + di, xr1) == 1) yrPoss1.append(y1+di);
				}
				
				QVector<int> yrPoss2 = QVector<int>();
				for (int di = -delta; di <= delta; di++) {
					if (edgeMap.at<uchar>(y2 + di, xr2) == 1) yrPoss2.append(y2+di);
				}

				if (yrPoss1.size() > 0 && yrPoss2.size() > 0) {

					int yr1 = yrPoss1.at(randInt(0,yrPoss1.size()-1));
					int yr2 = yrPoss2.at(randInt(0,yrPoss2.size()-1));

					slope = (yr2 - yr1) / (double)(xr2 - xr1);
					inters = yr2 - slope * xr2;
			
					double sumVal = 0;
					for (int xi = xr1; xi <= xr2; xi++)
						for (double yi = -epsilon; yi <= epsilon + 0.001; yi += epsilon/2.0) {
							int xc = qRound(slope * xi + inters) + yi;
							if(xc < edgeMap.rows && xi < edgeMap.cols && xc > 0 && xi > 0) sumVal += edgeMap.at<uchar>(xc, xi);
						}

					if (sumVal > currMax.x()) {
				
						QPointF centerPoint = QPointF(0.5*(xr1 + xr2), 0.5*(yr1 + yr2));
						currMax = QVector3D(sumVal, slope, (float) qSqrt( (edgeMap.cols*0.5 - centerPoint.x()) * (edgeMap.cols*0.5 - centerPoint.x()) + (edgeMap.rows*0.5 - centerPoint.y()) * (edgeMap.rows*0.5 - centerPoint.y()) ));
					}
				}
			}
		}
		else {

			for (int K = 0; K < nIter; K++) {

				int xr1 = randInt(l[1],l[3]);
				while(xr1 >= edgeMap.rows) xr1 = randInt(l[1],l[3]);
				int xr2 = randInt(l[1],l[3]);
				while (xr1 == xr2 || xr2 >= edgeMap.rows) xr2 = randInt(l[1],l[3]);

				double slope = (l[2] - l[0]) / (double)(l[3] - l[1]);
				double inters = l[0] - slope * l[1];
				int y1 = qRound(slope * xr1 + inters);
				int y2 = qRound(slope * xr2 + inters);

				if (y1 >= edgeMap.cols) y1 = edgeMap.cols - 1; // doesn't happen very often - just a precaution
				if (y2 >= edgeMap.cols) y2 = edgeMap.cols - 1;

				QVector<int> yrPoss1 = QVector<int>();
				for (int di = -delta; di <= delta; di++) {
					if (edgeMap.at<uchar>(xr1, y1 + di) == 1) yrPoss1.append(y1+di);
				}
				
				QVector<int> yrPoss2 = QVector<int>();
				for (int di = -delta; di <= delta; di++) {
					if (edgeMap.at<uchar>(xr2, y2 + di) == 1) yrPoss2.append(y2+di);
				}

				if (yrPoss1.size() > 0 && yrPoss2.size() > 0) {

					int yr1 = yrPoss1.at(randInt(0,yrPoss1.size()-1));
					int yr2 = yrPoss2.at(randInt(0,yrPoss2.size()-1));

					slope = (yr2 - yr1) / (double)(xr2 - xr1);
					inters = yr2 - slope * xr2;
			
					double sumVal = 0;
					for (int xi = xr1; xi <= xr2; xi++)
						for (double yi = -epsilon; yi <= epsilon + 0.001; yi += epsilon/2.0) {
							int yc = qRound(slope * xi + inters) + yi;
							if(yc < edgeMap.cols && xi < edgeMap.rows && yc > 0 && xi > 0) sumVal += edgeMap.at<uchar>(xi, yc);
						}

					if (sumVal > currMax.x()) {
				
						QPointF centerPoint = QPointF(0.5*(xr1 + xr2), 0.5*(yr1 + yr2));
						currMax = QVector3D(sumVal, slope, (float) qSqrt( (edgeMap.rows *0.5 - centerPoint.x()) * (edgeMap.rows*0.5 - centerPoint.x()) + (edgeMap.cols*0.5 - centerPoint.y()) * (edgeMap.cols*0.5 - centerPoint.y()) ));
					}
				}
			}
		}

		if (currMax.x() > 0) computedWeights.append(currMax);
	}
	return computedWeights;
}


double DkSkewEstimator::computeSkewAngle(QVector<QVector3D> weights, double imgDiagonal) {

	if (weights.size() < 1) return 0;

	int maxWeight = 0;
	for (int i = 0; i < weights.size(); i++)
		if (weights.at(i).x() > maxWeight)
			maxWeight = weights.at(i).x();

	double eta = 0.5 * maxWeight;

	QVector<QVector3D> thrWeights = QVector<QVector3D> ();
	for (int i = 0; i < weights.size(); i++)
		if (weights.at(i).x() > eta)
			thrWeights.append(QVector3D((weights.at(i).x() - eta) * (weights.at(i).x() - eta), qAtan(weights.at(i).y()) / M_PI * 180, weights.at(i).z() / imgDiagonal));

	QVector<QPointF> saliencyVec = QVector<QPointF>();

	for (double skewAngle = -20; skewAngle <= 20.001; skewAngle += 0.1) {

		double saliency = 0;

		for (int i = 0; i < thrWeights.size(); i++) {

			saliency += thrWeights.at(i).x() * qExp(-thrWeights.at(i).z()) * qExp(- 0.5 * ((skewAngle - thrWeights.at(i).y()) * (skewAngle - thrWeights.at(i).y())) / (sigma * sigma));
		}

		saliencyVec.append(QPointF(skewAngle, saliency));
	}

	//for (int i = 0; i < saliencyVec.size(); i++) qDebug() << saliencyVec.at(i);

	double maxSaliency = 0;
	double salSkewAngle = 0;

	for (int i = 0; i < saliencyVec.size(); i++)  {
		if (maxSaliency < saliencyVec.at(i).y()) {
			maxSaliency = saliencyVec.at(i).y();
			salSkewAngle = saliencyVec.at(i).x();
		}
	}

	if (maxSaliency == 0) return 0;

	salSkewAngle = -salSkewAngle;
	if (salSkewAngle < 0) salSkewAngle += 360;

	return salSkewAngle;
}

};
