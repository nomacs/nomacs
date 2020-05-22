/*******************************************************************************************************
 DkSkewEstimator.cpp
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
#include "DkImageStorage.h"

#include <QDebug>

namespace nmp {

DkSkewEstimator::DkSkewEstimator(QWidget* mainWin) {

	this->mainWin = mainWin;

	// method parameters
	nIter = 200;
	sigma = 0.3;
	sepThr = 0.1,
	epsilon = 2;
	kMax = 7;
	sepDims = QSize(0,0); // based on image size
	delta = 0; // based on image size
	minLineLength = 10;
	minLineProjLength = minLineLength/4;
	rotationFactor = 1;

	selectedLines.clear();
}

DkSkewEstimator::~DkSkewEstimator() {

}

void DkSkewEstimator::setImage(QImage inImage) {

	matImg = nmc::DkImage::qImage2Mat(inImage);
	
	sepDims = QSize(qRound(inImage.width()/1430.0*49.0),qRound(inImage.height()/700.0*12.0));
	delta = qRound(inImage.width()/1430.0*20.0);
	minLineLength = qRound(inImage.width()/1430.0 * 20.0);
	rotationFactor = 1;

	if (inImage.width() < inImage.height()) {

		matImg = matImg.t();
		sepDims = QSize(qRound(inImage.width()/1430.0*49.0),qRound(inImage.height()/700.0*12.0));
		delta = qRound(inImage.height()/1430.0*20.0);
		minLineLength = qRound(inImage.height()/1430.0 * 20.0);
		rotationFactor = -1;
	}

	if (sepDims.width() < 1) sepDims.setWidth(1);
	if (sepDims.height() < 1) sepDims.setHeight(1);

	minLineProjLength = minLineLength/4;
}

double DkSkewEstimator::getSkewAngle() {

	if (!matImg.empty()) {
		progress = new QProgressDialog(QT_TRANSLATE_NOOP("nmc::DkSkewEstimator", "Calculating angle..."), QT_TRANSLATE_NOOP("nmc::DkSkewEstimator", "Cancel"), 0, 100, mainWin);
		progress->setMinimumDuration(250);
		progress->setMaximum(100);
		progress->setValue(0);
		progress->setWindowModality(Qt::WindowModal);
		progress->setModal(true);
		progress->hide();
		progress->show();

		cv::Mat matGray;

		if (matImg.channels() > 1)
			cv::cvtColor(matImg, matGray, CV_BGR2GRAY);
		else matGray = matImg;

		cv::Mat integral, integralSq;
		cv::integral(matGray, integral, integralSq, CV_64F);
		if (integral.channels() > 1) qDebug() << "Error! integral image has more than one channel";

		cv::Mat separabilityHor = computeSeparability(integral, integralSq, dir_horizontal);
		if (progress->wasCanceled()) {
			progress->deleteLater();
			return 0;
		}

		cv::Mat separabilityVer = computeSeparability(integral, integralSq,  dir_vertical);
		if (progress->wasCanceled()) {
			progress->deleteLater();
			return 0;
		}

		double min, max;
		cv::minMaxLoc(separabilityHor, &min, &max);	
		cv::Mat edgeMapHor = computeEdgeMap(separabilityHor, sepThr * max, dir_horizontal);
		//cv::Mat edgeMapHor = computeEdgeMap(separabilityHor, 0.1, dir_horizontal);
		if (progress->wasCanceled()) {
			progress->deleteLater();
			return 0;
		}

		cv::minMaxLoc(separabilityVer, &min, &max);
		cv::Mat edgeMapVer = computeEdgeMap(separabilityVer, sepThr * max, dir_vertical);
		//cv::Mat edgeMapVer = computeEdgeMap(separabilityVer, 0.1, dir_vertical);
		if (progress->wasCanceled()) {
			progress->deleteLater();
			return 0;
		}

		selectedLines.clear();

		QVector<QVector3D> weightsHor = computeWeights(edgeMapHor, dir_horizontal);
		qDebug() << weightsHor.size();
		QVector<QVector3D> weightsVer = computeWeights(edgeMapVer, dir_vertical);
		qDebug() << weightsVer.size();
		if (progress->wasCanceled()) {
			selectedLines.clear();
			progress->deleteLater();
			return 0;
		}

		weightsHor += weightsVer;
 
		double retAngle = computeSkewAngle(weightsHor, qSqrt(matGray.rows*matGray.rows + matGray.cols*matGray.cols));

		progress->setValue(100);
		progress->deleteLater();

		return retAngle;
	}
	else return 0;
}

cv::Mat DkSkewEstimator::computeSeparability(cv::Mat integral, cv::Mat integralSq, int direction) {

	cv::Mat separability = cv::Mat::zeros(integral.rows, integral.cols, CV_32FC1);

	int W2 = qCeil(sepDims.width()/2);
	int H2 = qCeil(sepDims.height()/2);

	if (direction == dir_horizontal) {
		int progressStep = integral.rows - 2 * H2 - 2 * qCeil(delta/2);
		int lastValue = progress->value();

		for (int r = H2 + qCeil(delta/2); r < integral.rows - H2 - qCeil(delta/2); r++) {
			progress->setValue(lastValue + qRound(30.0 * (r - H2 - qCeil(delta/2)) / (double)progressStep));
			if (progress->wasCanceled()) break;
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
		int progressStep = integral.rows - 2 * W2 - 2 * qCeil(delta/2);
		int lastValue = progress->value();

		for (int r = W2 + qCeil(delta/2); r < integral.rows - W2 - qCeil(delta/2); r++) {
			progress->setValue(lastValue + qRound(30.0 * (r - W2 - qCeil(delta/2)) / (double)progressStep));
			if (progress->wasCanceled()) break;

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

	cv::Mat edgeMap = cv::Mat::zeros(separability.rows, separability.cols, CV_8UC1);

	if (direction == dir_horizontal) {
		int progressStep = separability.rows - 2 * H2 - 2 * kMax;
		int lastValue = progress->value();

		float* p;
		for (int r = H2 + kMax; r < separability.rows - H2 - kMax; r++) {
			progress->setValue(lastValue + qRound(5.0 * (r - H2 - kMax) / (double)progressStep));
			if (progress->wasCanceled()) break;

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
		int progressStep = separability.rows - 2 * W2 - 2 * kMax;
		int lastValue = progress->value();

		float* p;
		for (int r = W2; r < separability.rows - W2; r++) {
			progress->setValue(lastValue + qRound(5.0 * (r - W2 - kMax) / (double)progressStep));
			if (progress->wasCanceled()) break;

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

	std::vector<cv::Vec4i> lines;
	QVector4D maxLine = QVector4D();
	HoughLinesP(edgeMap, lines, 1, CV_PI/180, 50, minLineLength, 20 ); //params: rho resolution, theta resolution, threshold, min Line length, max line gap

	QVector<QVector3D> computedWeights = QVector<QVector3D>();
	int lastValue = progress->value();

	for(size_t i = 0; i < lines.size(); i++) {
		progress->setValue(lastValue + qRound(15.0 * (float)i / lines.size()));
		if (progress->wasCanceled()) break;

		cv::Vec4i l = lines[i];		
		QVector3D currMax = QVector3D(0.0, 0.0, 0.0);

		if (direction == dir_horizontal) {

			int K = 0;

			if (l[2] < l[0]) l = cv::Vec4i(l[2],l[3],l[0],l[1]);

			int x1 = l[0];
			int x2 = l[2];

			while(x2 > edgeMap.cols) x2--;
			while(x1 < 0) x1--;

			double lineAngle =  atan2((l[3] - l[1]), (l[2] - l[0]));
			double slope = qTan(lineAngle);

			while (qAbs(x1-x2) > minLineProjLength && K < nIter) {

				int y1 = qRound(l[1] + (x1 - l[0]) * slope);
				int y2 = qRound(l[1] + (x2 - l[0]) * slope);

				QVector<int> yrPoss1 = QVector<int>();
				for (int di = -delta; di <= delta && y1 + di < edgeMap.rows; di++) {
					if (y1 + di >= 0)
						if (edgeMap.at<uchar>(y1 + di, x1) == 1) yrPoss1.append(y1+di);
				}
				
				QVector<int> yrPoss2 = QVector<int>();
				for (int di = -delta; di <= delta && y2 + di < edgeMap.rows; di++) {
					if (y2 + di >= 0)
						if (edgeMap.at<uchar>(y2 + di, x2) == 1) yrPoss2.append(y2+di);
				}

				if (yrPoss1.size() > 0 && yrPoss2.size() > 0) {
					for (int y1i = 0; y1i < yrPoss1.size(); y1i++) {

						//int ys1 = yrPoss1.at(y1i);
						for (int y2i = 0; y2i < yrPoss2.size(); y2i++) {
							//int ys2 = yrPoss2.at(y2i);

							double sumVal = 0;
							for (int xi = x1; xi <= x2; xi++)
								for (int yi = -epsilon; yi <= epsilon; yi++) {
									int yc = qRound(l[1] + (xi - l[0]) * slope) + yi;
									if(yc < edgeMap.rows && xi < edgeMap.cols && yc > 0 && xi > 0) sumVal += edgeMap.at<uchar>(yc, xi);
								}

							if (sumVal > currMax.x()) {
				
								QPointF centerPoint = QPointF(0.5*(x1 + x2), 0.5*(y1 + y2));
								currMax = QVector3D(
									(float)sumVal, 
									(float) (- rotationFactor * lineAngle), 
									(float) qSqrt( (edgeMap.cols*0.5 - centerPoint.x()) * (edgeMap.cols*0.5 - centerPoint.x()) + (edgeMap.rows*0.5 - centerPoint.y()) * (edgeMap.rows*0.5 - centerPoint.y()) ));
								maxLine = QVector4D((float)x1, (float)y1, (float)x2, (float)y2);
							}

							K++;
						}
					}
				}

				x1++;
				x2--;				
			}
		}
		else {

			int K = 0;
			if (l[3] < l[1]) l = cv::Vec4i(l[2],l[3],l[0],l[1]);

			int x1 = l[1];
			int x2 = l[3];

			while(x2 > edgeMap.rows) x2--;
			while(x1 < 0) x1--;

			double lineAngle =  atan2((l[2] - l[0]), (l[3] - l[1]));
			double slope = qTan(lineAngle);
			

			while (qAbs(x1-x2) > minLineProjLength && K < nIter) {

				int y1 = qRound(l[0] + (x1 - l[1]) * slope);
				int y2 = qRound(l[0] + (x2 - l[1]) * slope);

				QVector<int> yrPoss1 = QVector<int>();
				for (int di = -delta; di <= delta && y1 + di < edgeMap.cols; di++) {
					if (y1 + di >= 0)
						if (edgeMap.at<uchar>(x1, y1 + di) == 1) yrPoss1.append(y1+di);
				}
				
				QVector<int> yrPoss2 = QVector<int>();
				for (int di = -delta; di <= delta && y2 + di < edgeMap.cols; di++) {
					if (y2 + di >= 0)
						if (edgeMap.at<uchar>(x2, y2 + di) == 1) yrPoss2.append(y2+di);
				}

				if (yrPoss1.size() > 0 && yrPoss2.size() > 0) {
					for (int y1i = 0; y1i < yrPoss1.size(); y1i++) {

						//int ys1 = yrPoss1.at(y1i);
						for (int y2i = 0; y2i < yrPoss2.size(); y2i++) {
							//int ys2 = yrPoss2.at(y2i);

							double sumVal = 0;
							for (int xi = x1; xi <= x2; xi++)
								for (int yi = -epsilon; yi <= epsilon; yi++) {
									int yc = qRound(l[0] + (xi - l[1]) * slope) + yi;
									if(yc < edgeMap.cols && xi < edgeMap.rows && yc > 0 && xi > 0) sumVal += edgeMap.at<uchar>(xi, yc);
								}

							if (sumVal > currMax.x()) {
				
								QPointF centerPoint = QPointF(0.5*(x1 + x2), 0.5*(y1 + y2));
								currMax = QVector3D((float)sumVal, (float)(rotationFactor * lineAngle), (float) qSqrt( (edgeMap.rows *0.5 - centerPoint.x()) * (edgeMap.rows*0.5 - centerPoint.x()) + (edgeMap.cols*0.5 - centerPoint.y()) * (edgeMap.cols*0.5 - centerPoint.y()) ));
								maxLine = QVector4D((float)y1, (float)x1, (float)y2, (float)x2);
							}

							K++;
						}
					}
				}

				x1++;
				x2--;				
			}
		}

		if (currMax.x() > 0) {
			computedWeights.append(currMax);
			if (rotationFactor == -1) maxLine = QVector4D(maxLine.y(), maxLine.x(), maxLine.w(), maxLine.z());
			selectedLines.append(maxLine);
			selectedLineTypes.append(0);
		}
	}
	return computedWeights;
}


double DkSkewEstimator::computeSkewAngle(QVector<QVector3D> weights, double imgDiagonal) {

	if (weights.size() < 1) return 0;

	double maxWeight = 0;
	for (int i = 0; i < weights.size(); i++)
		if (weights.at(i).x() > maxWeight)
			maxWeight = weights.at(i).x();

	double eta = 0.35;

	QVector<QVector3D> thrWeights = QVector<QVector3D> ();
	for (int i = 0; i < weights.size(); i++)
		if (weights.at(i).x()/maxWeight > eta) {
			thrWeights.append(QVector3D((float)qSqrt((weights.at(i).x()/maxWeight - eta)/(1 - eta)), (float)(weights.at(i).y() / M_PI * 180), (float)(weights.at(i).z() / imgDiagonal)));
			//thrWeights.append(QVector3D((weights.at(i).x()/maxWeight - eta) * (weights.at(i).x()/maxWeight - eta), weights.at(i).y() / M_PI * 180, weights.at(i).z() / imgDiagonal));
		}

	QVector<QPointF> saliencyVec = QVector<QPointF>();

	for (double skewAngle = -30; skewAngle <= 30.001; skewAngle += 0.1) {

		double saliency = 0;

		for (int i = 0; i < thrWeights.size(); i++) {
			saliency += thrWeights.at(i).x() * qExp(-thrWeights.at(i).z()) * qExp(-0.5 * ((skewAngle - thrWeights.at(i).y()) * (skewAngle - thrWeights.at(i).y())) / (sigma * sigma));
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

	for (int i = 0; i < weights.size(); i++)
		if (weights.at(i).x() > eta && qAbs(weights.at(i).y() / M_PI * 180 - salSkewAngle) < 0.15)
			selectedLineTypes.replace(i,1);

	if (maxSaliency == 0) return 0;

	return salSkewAngle;
}

QVector<QVector4D> DkSkewEstimator::getLines() {

	return selectedLines;
}

QVector<int> DkSkewEstimator::getLineTypes() {

	return selectedLineTypes;
}


};
