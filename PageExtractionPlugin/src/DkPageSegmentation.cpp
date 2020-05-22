/*******************************************************************************************************
 DkPageSegmentation.cpp

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2015 Markus Diem

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

#include "DkPageSegmentation.h"
#include "DkPageSegmentationUtils.h"
#include "DkMath.h"	// nomacs

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QDebug>
#include <QPainter>

#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#pragma warning(pop)		// no warnings from includes - end

namespace nmp {

// DkSegmentBurger --------------------------------------------------------------------
// This code is based on OpenCV's rectangle sample (squares.cpp)
DkPageSegmentation::DkPageSegmentation(const cv::Mat& colImg /* = cv::Mat */, bool alternativeMethod /* = false */) : alternativeMethod(alternativeMethod) {

	this->mImg = colImg;
}

cv::Mat DkPageSegmentation::getDebugImg() const {

	return dbgImg;	// is NULL if releaseDebug is DK_RELEASE_IMGS
}

DkPolyRect DkPageSegmentation::getMaxRect() const {

	// find the largest rectangle
	DkPolyRect largeRect;
	double maxArea = -1;

	for (const DkPolyRect& p : mRects) {

		double ca = p.getAreaConst();

		if (ca > maxArea) {
			maxArea = ca;
			largeRect = p.toCvPoints();
		}
	}

	return largeRect;
}

QImage DkPageSegmentation::getCropped(const QImage & img) const {

	if (!mRects.empty()) {
		nmc::DkRotatingRect rr = getMaxRect().toRotatingRect();
		return cropToRect(img, rr);
	}

	return img;	// no document page found
}

void DkPageSegmentation::compute() {

	cv::Mat lImg;
	if (alternativeMethod) {
		if (scale == 1.0f && mImg.rows > 700.0f)
			scale = 700.0f / mImg.rows;
			
		lImg = findRectanglesAlternative(mImg, mRects);
	} else {
		cv::Mat imgLab;
		
		if (scale == 1.0f && 960.0f/mImg.cols < 0.8f)
			scale = 960.0f/mImg.cols;
			
		cv::cvtColor(mImg, imgLab, CV_RGB2Lab);	// boost colors
		lImg = findRectangles(mImg, mRects);
	}

	qDebug() << "[DkPageSegmentation] " << mRects.size() << " rectangles circles found resize factor: " << scale;
}

cv::Mat DkPageSegmentation::findRectangles(const cv::Mat& img, std::vector<DkPolyRect>& rects) const {
	
	cv::Mat tImg, gray;


	if (scale != 1.0f)
		cv::resize(img, tImg, cv::Size(), scale, scale, CV_INTER_AREA);	// inter nn -> assuming resize to be 1/(2^n)
	else
		tImg = img;

	std::vector<std::vector<cv::Point> > contours;

	cv::Mat gray0(tImg.size(), CV_8UC1);
	cv::Mat lImg(tImg.size(), CV_8UC1);

	// find squares in every color plane of the image
	for( int c = 0; c < 3; c++ ) {

		int ch[] = {c, 0};
		mixChannels(&tImg, 1, &gray0, 1, ch, 1);
		cv::normalize(gray0, gray0, 255, 0, cv::NORM_MINMAX);

		if (c == 0)	// back-up the luminance channel - we use it as precomputed image for the circle detection
			lImg = gray0.clone();

		int nT = numThresh;//(c == 0) ? numThresh*2 : numThresh;	// more luminance thresholds

							// try several threshold levels
		for( int l = 0; l < nT; l++ ) {

			// hack: use Canny instead of zero threshold level.
			// Canny helps to catch squares with gradient shading
			if( l == 0 ) {

				Canny(gray0, gray, thresh, thresh*3, 5);
				// dilate canny output to remove potential
				// holes between edge segments
				dilate(gray, gray, cv::Mat(), cv::Point(-1,-1));

				//DkIP::imwrite("edgeImg.png", gray);
			}
			else {
				gray = gray0 >= (l+1)*255/numThresh;
			}

			// find contours and store them all as a list
			findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

			if (looseDetection) {
				std::vector<std::vector<cv::Point> > hull;
				for (int i = 0; i < (int)contours.size(); i++) { 

					double cArea = contourArea(cv::Mat(contours[i]));

					if (fabs(cArea) > mMinArea*scale*scale && (!mMaxArea || fabs(cArea) < mMaxArea*(scale*scale))) {
						std::vector<cv::Point> cHull;
						cv::convexHull(cv::Mat(contours[i]), cHull, false);
						hull.push_back(cHull);
					}
				}

				contours = hull;
			}

			std::vector<cv::Point> approx;

			// DEBUG ------------------------
			//cv::Mat pImg = image.clone();
			//cv::cvtColor(pImg, pImg, CV_Lab2RGB);
			// DEBUG ------------------------

			// test each contour
			for( size_t i = 0; i < contours.size(); i++ ) {
				// approxicv::Mate contour with accuracy proportional
				// to the contour perimeter
				approxPolyDP(cv::Mat(contours[i]), approx, arcLength(cv::Mat(contours[i]), true)*0.02, true);

				double cArea = contourArea(cv::Mat(approx));

				// DEBUG ------------------------
				//if (fabs(cArea) < mMaxArea)
				//	fillConvexPoly(pImg, &approx[0], (int)approx.size(), DkUtils::blue);
				// DEBUG ------------------------

				// square contours should have 4 vertices after approxicv::Mation
				// relatively large area (to filter out noisy contours)
				// and be convex.
				// Note: absolute value of an area is used because
				// area may be positive or negative - in accordance with the
				// contour orientation
				if( approx.size() == 4 &&
					fabs(cArea) > mMinArea*scale*scale &&
					(!mMaxArea || fabs(cArea) < mMaxArea*scale*scale) && 
					isContourConvex(cv::Mat(approx)) ) {

					DkPolyRect cr(approx);
					//moutc << mMinArea*scale*scale << " < " << fabs(cArea) << " < " << mMaxArea*scale*scale << dkendl;

					// if cosines of all angles are small
					// (all angles are ~90 degree)
					if(/*cr.maxSide() < std::max(tImg.rows, tImg.cols)*maxSideFactor && */
						(!maxSide || cr.maxSide() < maxSide*scale) && 
						cr.getMaxCosine() < 0.3 ) {
						rects.push_back(cr);
					}
				}
			}
			// DEBUG ------------------------
			//cv::cvtColor(pImg, pImg, CV_RGB2BGR);
			//DkIP::imwrite("polyImg" + DkUtils::stringify(c) + "-" + DkUtils::stringify(l) + ".png", pImg);
			// DEBUG ------------------------
		}
	}

	for (size_t idx = 0; idx < rects.size(); idx++)
		rects[idx].scale(1.0f/scale);


	// filter rectangles which are found because of the image border
	std::vector<DkPolyRect> noLargeRects;
	for (const DkPolyRect& p : rects) {

		DkBox b = p.getBBox();

		if (b.size().height < img.rows*maxSideFactor &&
			b.size().width < img.cols*maxSideFactor) {
			noLargeRects.push_back(p);
		}
	}

	rects = noLargeRects;

	return lImg;
}

cv::Mat DkPageSegmentation::findRectanglesAlternative(const cv::Mat& img, std::vector<DkPolyRect>& rects) const {
	PageExtractor extractor;
	extractor.findPage(img, scale, rects);

	return img;
}

QImage DkPageSegmentation::cropToRect(const QImage & img, const nmc::DkRotatingRect & rect, const QColor & bgCol) const {
	
	QTransform tForm; 
	QPointF cImgSize;

	rect.getTransform(tForm, cImgSize);

	if (cImgSize.x() < 0.5f || cImgSize.y() < 0.5f) {
		return img;
	}

	qDebug() << cImgSize;
	qDebug() << "transform: " << tForm;

	double angle = nmc::DkMath::normAngleRad(rect.getAngle(), 0, CV_PI*0.5);
	double minD = qMin(abs(angle), abs(angle-CV_PI*0.5));

	QImage cImg = QImage(qRound(cImgSize.x()), qRound(cImgSize.y()), QImage::Format_ARGB32);
	cImg.fill(bgCol.rgba());

	// render the image into the new coordinate system
	QPainter painter(&cImg);
	painter.setWorldTransform(tForm);

	// for rotated mRects we want perfect anti-aliasing
	if (minD > FLT_EPSILON)
		painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);

	painter.drawImage(QRect(QPoint(),img.size()), img, QRect(QPoint(), img.size()));
	painter.end();

	return cImg;

	//QImage dImg = mImg;
	//QPainter p1(&dImg);
	//p1.drawPolygon(rect.getPoly());

	//return dImg;
}

void DkPageSegmentation::filterDuplicates(float overlap, float areaRatio) {

	filterDuplicates(mRects, overlap, areaRatio);
}

void DkPageSegmentation::filterDuplicates(std::vector<DkPolyRect>& rects, float overlap, float areaRatio) const {

	std::vector<int> delIdx;
	std::sort(rects.rbegin(), rects.rend(), &DkPolyRect::compArea);	// rbegin() -> sort descending

	for (int idx = 0; idx < (int)rects.size(); idx++) {

		// if we already deleted a rectangle, we can safely skip it
		if (std::find(delIdx.begin(), delIdx.end(), idx) != delIdx.end())
			continue;

		DkPolyRect& cR = rects[idx];
		double cA = cR.getArea();

		std::vector<int> tmpDelIdx;

		for (int oIdx = idx+1; oIdx < (int)rects.size(); oIdx++) {

			// if we already deleted a rectangle, we can safely skip it
			if (idx == oIdx || std::find(delIdx.begin(), delIdx.end(), oIdx) != delIdx.end())
				continue;

			DkPolyRect& oR = rects[oIdx];
			double oA = oR.getArea();

			// ignore rectangles with totally different area
			if (oA/cA < areaRatio)	// since we sort, we know that oA is larger
				continue;

			double intersection = abs(oR.intersectArea(cR));

			if (std::max(intersection/cR.getArea(), intersection/oR.getArea()) > overlap) {

				double cVal, oVal;

				// if the cosine is more or less the same, take the larger rectangle
				//if (fabs(cR.getMaxCosine() - oR.getMaxCosine()) < 0.02) {
				//	cVal = oA;	// inverse since we want to prefer larger areas
				//	oVal = cA;
				//}
				//else {
				cVal = cR.getMaxCosine();
				oVal = oR.getMaxCosine();
				//}

				// delete the rect which has an inferior cosine value
				if (cVal > oVal) {
					delIdx.push_back(idx);
					tmpDelIdx.clear();
					break; // we're done if we delete the current rect
				}
				else {
					tmpDelIdx.push_back(oIdx);
				}
			}
		}

		delIdx.insert(delIdx.end(), tmpDelIdx.begin(), tmpDelIdx.end());
	}

	if (!delIdx.empty()) {
		std::vector<DkPolyRect> filtered;

		for (int idx = 0; idx < (int)rects.size(); idx++) {

			if (std::find(delIdx.begin(), delIdx.end(), idx) == delIdx.end())
				filtered.push_back(rects[idx]);
		}

		qDebug() << "[DkPageSegmentation] " << rects.size() - filtered.size() << " rectangles removed, remaining: " << filtered.size();
		rects = filtered;
	}
}

void DkPageSegmentation::draw(cv::Mat& img, const cv::Scalar& col) const {

	draw(img, mRects, col);
}

void DkPageSegmentation::draw(QImage& img, const QColor& col) const {

	double mA = getMaxRect().getArea();
	std::vector<DkPolyRect> fRects;

	for (const DkPolyRect& r : mRects) {

		if (r.getAreaConst() > mA*0.9)
			fRects.push_back(r);
	}

	QPainter p(&img);
	drawRects(&p, mRects);
	drawRects(&p, fRects, col);
}

void DkPageSegmentation::drawRects(QPainter * p, const std::vector<DkPolyRect>& rects, const QColor & col) const {

	QColor colA = col;
	colA.setAlpha(30);

	QPen pen;
	pen.setColor(col);
	pen.setWidth(10);

	p->setPen(pen);

	for (const DkPolyRect& r : rects) {
		
		QPolygonF poly = r.toPolygon();
		p->drawPolygon(poly);

		QPainterPath pa;
		pa.addPolygon(poly);
		p->fillPath(pa, colA);
	}
}

void DkPageSegmentation::draw(cv::Mat& img, const std::vector<DkPolyRect>& rects, const cv::Scalar& col) const {

	for(const DkPolyRect& r : rects) {
		r.draw(img, col);
	}
}

};

