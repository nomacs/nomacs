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

#include <iostream> // TODO remove; just for debugging purposes
#include <algorithm>
#include <opencv2/highgui/highgui.hpp> // TODO remove; just for debugging purposes

#include "DkPageSegmentationUtils.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QDebug>
#include <opencv2/imgproc/imgproc.hpp>
#pragma warning(pop)		// no warnings from includes - end

namespace nmp {


// DkIntersectPoly --------------------------------------------------------------------
DkIntersectPoly::DkIntersectPoly() {};

DkIntersectPoly::DkIntersectPoly(std::vector<nmc::DkVector> vecA, std::vector<nmc::DkVector> vecB) {

	this->vecA = vecA;
	this->vecB = vecB;
	interArea = 0;
}

double DkIntersectPoly::compute() {

	// defines
	gamut = 500000000;
	minRange = nmc::DkVector(FLT_MAX, FLT_MAX);
	maxRange = nmc::DkVector(-FLT_MAX, -FLT_MAX);
	computeBoundingBox(vecA, &minRange, &maxRange);
	computeBoundingBox(vecB, &minRange, &maxRange);

	scale = maxRange - minRange;

	if (scale.minCoord() == 0) return 0; //rechteck mit höhe oder breite = 0

	scale.x = gamut / scale.x;
	scale.y = gamut / scale.y;

	float ascale = scale.x * scale.y;

	// check input
	if (vecA.size() < 3 || vecB.size() < 3) {
		qDebug() << "The polygons must consist of at least 3 points but they are: (vecA: " << vecA.size() << ", vecB: " << vecB.size();
		return 0;
	}

	// compute edges
	std::vector<DkVertex> ipA;
	std::vector<DkVertex> ipB;

	getVertices(vecA, &ipA, 0);
	getVertices(vecB, &ipB, 2);

	for (unsigned int idxA = 0; idxA < ipA.size() - 1; idxA++) {
		for (unsigned int idxB = 0; idxB < ipB.size() - 1; idxB++) {

			if (ovl(ipA[idxA].rx, ipB[idxB].rx) && ovl(ipA[idxA].ry, ipB[idxB].ry)) {

				int64 a1 = -area(ipA[idxA].ip, ipB[idxB].ip, ipB[idxB + 1].ip);
				int64 a2 = area(ipA[idxA + 1].ip, ipB[idxB].ip, ipB[idxB + 1].ip);

				if (a1 < 0 == a2 < 0) {
					int64 a3 = area(ipB[idxB].ip, ipA[idxA].ip, ipA[idxA + 1].ip);
					int64 a4 = -area(ipB[idxB + 1].ip, ipA[idxA].ip, ipA[idxA + 1].ip);

					if (a3 < 0 == a4 < 0) {

						if (a1 < 0) {
							cross(ipA[idxA], ipA[idxA + 1], ipB[idxB], ipB[idxB + 1], (double)a1, (double)a2, (double)a3, (double)a4);
							ipA[idxA].in++;
							ipB[idxB].in--;
						}
						else {
							cross(ipB[idxB], ipB[idxB + 1], ipA[idxA], ipA[idxA + 1], (double)a3, (double)a4, (double)a1, (double)a2);
							ipA[idxA].in--;
							ipB[idxB].in++;
						}
					}
				}

			}
		}
	}

	inness(ipA, ipB);
	inness(ipB, ipA);

	double areaD = (double)interArea / (ascale + FLT_MIN);

	return areaD;

}

void DkIntersectPoly::inness(std::vector<DkVertex> ipA, std::vector<DkVertex> ipB) {

	int s = 0;

	DkIPoint p = ipA[0].ip;

	for (int idx = (int)ipB.size() - 2; idx >= 0; idx--) {

		if (ipB[idx].rx.x < p.x && p.x < ipB[idx].rx.y) {
			bool sgn = 0 < area(p, ipB[idx].ip, ipB[idx + 1].ip);
			s += (sgn != ipB[idx].ip.x < ipB[idx + 1].ip.x) ? 0 : (sgn ? -1 : 1);
		}
	}

	for (unsigned int idx = 0; idx < ipA.size() - 1; idx++) {
		if (s != 0)
			cntrib(ipA[idx].ip.x, ipA[idx].ip.y, ipA[idx + 1].ip.x, ipA[idx + 1].ip.y, s);
		s += ipA[idx].in;
	}

};

void DkIntersectPoly::cross(DkVertex a, DkVertex b, DkVertex c, DkVertex d, double a1, double a2, double a3, double a4) {

	double r1 = a1 / ((double)a1 + a2 + DBL_EPSILON);
	double r2 = a3 / ((double)a3 + a4 + DBL_EPSILON);

	cntrib((int)a.ip.x + cvRound((double)(r1 * (double)(b.ip.x - a.ip.x))),
		(int)a.ip.y + cvRound((double)(r1 * (double)(b.ip.y - a.ip.y))),
		b.ip.x, b.ip.y, 1);
	cntrib(d.ip.x, d.ip.y,
		(int)c.ip.x + cvRound((double)(r2 * (double)(d.ip.x - c.ip.x))),
		(int)c.ip.y + cvRound((double)(r2 * (double)(d.ip.y - c.ip.y))),
		1);
};

void DkIntersectPoly::cntrib(int fx, int fy, int tx, int ty, int w) {

	interArea += (int64)w * (tx - fx) * (ty + fy) / 2;
};


int64 DkIntersectPoly::area(DkIPoint a, DkIPoint p, DkIPoint q) {

	return (int64)p.x * q.y - (int64)p.y * q.x +
		(int64)a.x * (p.y - q.y) + (int64)a.y * (q.x - p.x);
};

bool DkIntersectPoly::ovl(DkIPoint p, DkIPoint q) {

	return (p.x < q.y && q.x < p.y);
};

void DkIntersectPoly::getVertices(const std::vector<nmc::DkVector>& vec, std::vector<DkVertex> *ip, int noise) {

	std::vector<DkIPoint> vecTmp;

	// transform the coordinates and modify the least significant bits (that's fun)
	for (int idx = 0; idx < (int)vec.size(); idx++) {

		DkIPoint cp;
		cp.x = ((int)((vec[idx].x - minRange.x) * scale.x - gamut / 2) & ~7) | noise | (idx & 1);
		cp.y = ((int)((vec[idx].y - minRange.y) * scale.y - gamut / 2) & ~7) | noise | (idx & 1);

		vecTmp.push_back(cp);
	}

	vecTmp.push_back(*(vecTmp.begin()));	// append the first element

	for (int idx = 0; idx < (int)vecTmp.size(); idx++) {

		int nIdx = idx % (int)(vecTmp.size() - 1) + 1;	// the last element should refer to the second (first & last are the very same)

		DkIPoint cEdgeX = (vecTmp[idx].x < vecTmp[nIdx].x) ?
			DkIPoint(vecTmp[idx].x, vecTmp[nIdx].x) :
			DkIPoint(vecTmp[nIdx].x, vecTmp[idx].x);
		DkIPoint cEdgeY = (vecTmp[idx].y < vecTmp[nIdx].y) ?
			DkIPoint(vecTmp[idx].y, vecTmp[nIdx].y) :
			DkIPoint(vecTmp[nIdx].y, vecTmp[idx].y);

		ip->push_back(DkVertex(vecTmp[idx], cEdgeX, cEdgeY));
	}
};

void DkIntersectPoly::computeBoundingBox(std::vector<nmc::DkVector> vec, nmc::DkVector *minRange, nmc::DkVector *maxRange) {


	for (unsigned int idx = 0; idx < vec.size(); idx++) {

		*minRange = minRange->minVec(vec[idx]);
		*maxRange = maxRange->maxVec(vec[idx]);	// in our case it's the max vector
	}
};

// DkPolyRect --------------------------------------------------------------------
DkPolyRect::DkPolyRect(const std::vector<cv::Point>& pts) {

	toDkVectors(pts, this->pts);
	computeMaxCosine();
	area = DBL_MAX;
}

DkPolyRect::DkPolyRect(const std::vector<nmc::DkVector>& pts) {
	this->pts = pts;
	computeMaxCosine();
	area = DBL_MAX;
}

bool DkPolyRect::empty() const {
	return pts.empty();
}

void DkPolyRect::toDkVectors(const std::vector<cv::Point>& pts, std::vector<nmc::DkVector>& dkPts) const {

	for (int idx = 0; idx < (int)pts.size(); idx++)
		dkPts.push_back(nmc::DkVector(pts.at(idx)));
}

void DkPolyRect::computeMaxCosine() {

	maxCosine = 0;

	for (int idx = 2; idx < (int)pts.size()+2; idx++ ) {

		nmc::DkVector& c = pts[(idx-1)%pts.size()];	// current corner;
		nmc::DkVector& c1 = pts[idx%pts.size()];
		nmc::DkVector& c2 = pts[idx-2];

		double cosine = abs((c1-c).cosv(c2-c));

		maxCosine = std::max(maxCosine, cosine);
	}
}

double DkPolyRect::intersectArea(const DkPolyRect& pr) const {

	return DkIntersectPoly(pts, pr.pts).compute();
}

void DkPolyRect::scale(float s) {

	for (size_t idx = 0; idx < pts.size(); idx++)
		pts[idx] = pts[idx]*s;

	area = DBL_MAX;	// update
}

void DkPolyRect::scaleCenter(float s) {

	nmc::DkVector c = center();

	for (size_t idx = 0; idx < pts.size(); idx++) {
		pts[idx] = nmc::DkVector(pts[idx]-c)*s+c;
	}

	area = DBL_MAX;	// update
}

nmc::DkVector DkPolyRect::center() const {

	nmc::DkVector c;

	for (size_t idx = 0; idx < pts.size(); idx++)
		c += pts[idx];

	c /= (float)pts.size();

	return c;
}


bool DkPolyRect::inside(const nmc::DkVector& vec) const {

	float lastsign = 0;

	// we assume, that the polygon is convex
	// so if the point has a different scalar product
	// for one side of the polygon - it is not inside
	for (size_t idx = 1; idx < pts.size()+1; idx++) {

		nmc::DkVector dv(pts[idx-1] - pts[idx%pts.size()]);
		float csign = dv.scalarProduct(vec - pts[idx%pts.size()]);

		if (lastsign*csign < 0) {
			return false;
		}

		lastsign = csign;
	}

	return true;
}

float DkPolyRect::maxSide() const {

	float ms = 0;

	for (size_t idx = 1; idx < pts.size()+1; idx++) {
		float cs = nmc::DkVector(pts[idx-1] - pts[idx%pts.size()]).norm();

		if (ms < cs)
			ms = cs;
	}

	return ms;
}

double DkPolyRect::getArea() {

	if (area == DBL_MAX)
		area = abs(intersectArea(*this));

	return area;
}

double DkPolyRect::getAreaConst() const {

	if (area == DBL_MAX)
		return abs(intersectArea(*this));

	return area;
}

bool DkPolyRect::compArea(const DkPolyRect& pl, const DkPolyRect& pr) {

	return pl.getAreaConst() < pr.getAreaConst();
}

nmc::DkRotatingRect DkPolyRect::toRotatingRect() const {

	if (empty())
		return nmc::DkRotatingRect();

	// find the largest rectangle
	std::vector<cv::Point> largeRect = toCvPoints();

	cv::RotatedRect rect = cv::minAreaRect(largeRect);

	// convert to corners
	nmc::DkVector xVec = nmc::DkVector(rect.size.width * 0.5f, 0);
	xVec.rotate(-rect.angle*DK_DEG2RAD);

	nmc::DkVector yVec = nmc::DkVector(0, rect.size.height * 0.5f);
	yVec.rotate(-rect.angle*DK_DEG2RAD);

	QPolygonF poly;
	poly.append(nmc::DkVector(rect.center - xVec + yVec).toQPointF());
	poly.append(nmc::DkVector(rect.center + xVec + yVec).toQPointF());
	poly.append(nmc::DkVector(rect.center + xVec - yVec).toQPointF());
	poly.append(nmc::DkVector(rect.center - xVec - yVec).toQPointF());

	nmc::DkRotatingRect rr;
	rr.setPoly(poly);

	return rr;
}

void DkPolyRect::draw(cv::Mat& img, const cv::Scalar& col) const {

	std::vector<cv::Point> cvPts = toCvPoints();
	if (cvPts.empty())
		return;

	const cv::Point* p = &cvPts[0];
	int n = (int)cvPts.size();
	cv::polylines(img, &p, &n, 1, true, col, 4);
}

std::vector<nmc::DkVector> DkPolyRect::getCorners() const {
	return pts;
}

DkBox DkPolyRect::getBBox() const {

	nmc::DkVector uc(FLT_MAX, FLT_MAX), lc(-FLT_MAX, -FLT_MAX);

	for (size_t idx = 0; idx < pts.size(); idx++) {

		uc = uc.minVec(pts[idx]);
		lc = lc.maxVec(pts[idx]);
	}


	if (pts.empty())
		qDebug() << "bbox of empty poly rect requested!!";

	DkBox box(uc, lc-uc);

	return box;
}

std::vector<cv::Point> DkPolyRect::toCvPoints() const {

	std::vector<cv::Point> cvPts;
	for (int idx = 0; idx < (int)pts.size(); idx++) {
		cvPts.push_back(pts[idx].getCvPoint());
	}

	return cvPts;
}

QPolygonF DkPolyRect::toPolygon() const {

	QPolygonF poly;

	for (const nmc::DkVector& v : pts) {
		poly.append(v.toQPointF());
	}

	return poly;
}

void PageExtractor::run(cv::Mat img, float scale) {
	float g_sigma = 2.0;
	cv::Mat gray, bw;

	cv::cvtColor(img, gray, CV_RGB2GRAY);
	if (scale != 1.0f) {
		cv::resize(gray, gray, cv::Size(), scale, scale, CV_INTER_AREA);	// inter nn -> assuming resize to be 1/(2^n)
	}
	
//	std::vector<cv::Point> pts;
//	pts.push_back(cv::Point(0.f, 0.f));
//	pts.push_back(cv::Point(0.f, 100.f));
//	pts.push_back(cv::Point(100.f, 100.f));
//	pts.push_back(cv::Point(100.f, 0.f));
//	DkPolyRect r(pts);
//	rects.push_back(r);
	
	// TODO iterate over sigmas, half size
	
	cv::equalizeHist(gray, gray);
	
	// TODO remove text
	
	cv::GaussianBlur(gray, gray, cv::Size(2 * floor(g_sigma * 3) + 1, 2 * floor(g_sigma * 3) + 1), g_sigma);
//	cv::namedWindow("gray gaussian");
//	cv::imshow("gray gaussian", gray);
//	cv::waitKey();
	cv::Canny(gray, bw, 0.1 * 255, 0.2 * 255);
	cv::dilate(bw, bw, cv::Mat::ones(3, 3, CV_8UC1));
	cv::namedWindow("bw");
	cv::imshow("bw", bw);
	cv::waitKey();
	
	cv::Mat lineImg;
	
//	std::vector<cv::Vec2f> lines1;
//	// we assume that the lines are sorted by accumulator values in descending order
//	cv::HoughLines(bw, lines1, 1, CV_PI / 180.0, 100); 
//	if (lines1.size() > maxLinesHough) {
//		lines1.resize(maxLinesHough);
//	}
//	for (cv::Vec2f l : lines1) {
//		std::cout << l[0] << " " << l[1] << std::endl;
//	}
//	lineImg = gray.clone();
//	// plot lines
//	for( size_t i = 0; i < lines1.size(); i++ )
//	{
//		float rho = lines1[i][0], theta = lines1[i][1];
//		cv::Point pt1, pt2;
//		double a = cos(theta), b = sin(theta);
//		double x0 = a*rho, y0 = b*rho;
//		pt1.x = cvRound(x0 + 1000*(-b));
//		pt1.y = cvRound(y0 + 1000*(a));
//		pt2.x = cvRound(x0 - 1000*(-b));
//		pt2.y = cvRound(y0 - 1000*(a));
//		cv::line( lineImg, pt1, pt2, cv::Scalar(0,0,255), 3, CV_AA);
//	}
//	cv::namedWindow("lineImg");
//	cv::imshow("lineImg", lineImg);
//	cv::waitKey();
	
	std::vector<HoughLine> lines = houghTransform(bw, 1, CV_PI / 180.0, 100, maxLinesHough);
	
	// DEBUG OUTPUT
	std::cout << lines.size() << std::endl;
	lineImg = gray.clone();
	for (size_t i = 0; i < lines.size(); i++) {
		std::cout << lines[i].acc << " " << lines[i].rho << " " << lines[i].angle << std::endl;
		float rho = lines[i].rho, theta = lines[i].angle;
		cv::Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000*(-b));
		pt1.y = cvRound(y0 + 1000*(a));
		pt2.x = cvRound(x0 - 1000*(-b));
		pt2.y = cvRound(y0 - 1000*(a));
		cv::line( lineImg, pt1, pt2, cv::Scalar(0,0,255), 3, CV_AA);
	}
	cv::namedWindow("lineImg");
	cv::imshow("lineImg", lineImg);
	cv::waitKey();
	
	// find line segments in image
	std::vector<LineSegment> lineSegments = findLineSegments(bw, lines, minLineSegmentLength, maxGapLength, false);
//	std::cout << lineSegments.size() << std::endl;
//	for (size_t i = 0; i < lines.size(); i++) {
//		std::cout << "line " << i << ": (" << lines[i].acc << "), " << lines[i].rho << ", " << lines[i].angle << std::endl;
//		lineImg = gray.clone();
//		cv::line(lineImg, lineSegments[i].p1, lineSegments[i].p2, cv::Scalar(0,0,255), 3, CV_AA);
//		cv::namedWindow("lineImg spatial");
//		cv::imshow("lineImg spatial", lineImg);
//		cv::waitKey(0);
//	}
	
	// 4.3 transform domain peak filtering
	// iterate through all pairs of lines and build pairs of parallel line segments called extended peak pairs (EPs)
	std::vector<ExtendedPeak> EPs;
	for (size_t i = 0; i < lines.size() - 1; i++) {
		for (size_t j = i + 1; j < lines.size(); j++) {
			// test for parallelity
			if (angleDiff(lines[i].angle, lines[j].angle) < t_theta && 
					std::abs(lines[i].acc - lines[j].acc) < t_l * 0.5 * (lines[i].acc + lines[j].acc)) {
				
				// 'parallel' line segments must not intersect
				ExtendedPeak ep(lines[i], lineSegments[i], lines[j], lineSegments[j]);
				if (ep.intersectionPoint.first) {
					std::vector<cv::Point2f> epHull;
					cv::convexHull(std::vector<cv::Point2f> {lineSegments[i].p1, lineSegments[i].p2, lineSegments[j].p1, lineSegments[j].p2}, epHull);
					if (cv::pointPolygonTest(epHull, ep.intersectionPoint.second, false) >= 0) {
						continue;
					}
				}
				
				
//				for (LineSegment l : ep.spatialLines) {
//					std::cout << l.p1.x << ", " << l.p1.y << " --- " << l.p2.x << ", " << l.p2.y << std::endl;
//				}
//				
//				lineImg = gray.clone();
//				for (LineSegment l : ep.spatialLines) {
//					cv::line(lineImg, l.p1, l.p2, cv::Scalar(0,0,255), 3, CV_AA);
//				}
//				cv::namedWindow("lineImg spatial");
//				cv::imshow("lineImg spatial", lineImg);
//				cv::waitKey();
				EPs.push_back(ep);
			}
		}
	}
	
	// iterate through pairs of EPs and combine them to intermediate peak pairs (IPs) if they form a rectangular shape 
	std::vector<IntermediatePeak> IPs;
	for (size_t i = 0; i < EPs.size(); i++) {
		for (size_t j = i + 1; j < EPs.size(); j++) {
			// test for orthogonality
			if (abs(angleDiff(EPs[i].theta_k, EPs[j].theta_k) - (CV_PI * 0.5f)) < orthoTol) {
				IPs.push_back(IntermediatePeak {EPs[i], EPs[j]});
			}
		}
	}
	
	// test IP corners
	std::vector<Rectangle> candidates;
	for (const IntermediatePeak& ip : IPs) {
		std::vector<cv::Point2f> corners;
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				auto r = findLineIntersection(ip.ep1.spatialLines[i], ip.ep2.spatialLines[j]);
				// since the lines of different EPs can not be parallel, they have to intersect at some point
				assert(r.first == true); // TODO replace assert with gui error
				cv::Point2f p = r.second;
				if (pointToLineDistance(ip.ep1.spatialLines[i], p) < cornerGapTol &&
						pointToLineDistance(ip.ep2.spatialLines[j], p) < cornerGapTol) {
					
					corners.push_back(p);
				}
			}
		}
		
		std::vector<cv::Point2f> rectCorners;
		if (corners.size() == 4) {
			cv::convexHull(corners, rectCorners);
			candidates.emplace_back(ip, rectCorners);
		}		
	}
	
	// check rectangle candidates for validity
	std::vector<Rectangle> rectangles;
	for (const Rectangle& c : candidates) {
		// check if sides are large enough
		bool largeEnough = true;
		for (int i = 0; i < 4; i++) {
			if (cv::norm(c.corners[i] - c.corners[(i + 1) % 4]) < minLineSegmentLength) {
				largeEnough = false;
			}
		}
		if (!largeEnough) {
			continue;
		}
		rectangles.push_back(c);
	}
	
	if (rectangles.empty()) {
		std::cout << "no valid rectangles have been detected!" << std::endl; // TODO gui output, return value
		return;
	}
	std::cout << "DEBUG: " << rectangles.size() << " valid rectangles have been detected" << std::endl;
	
	// find rectangle with highest overall accumulator value
	auto finalRectangleIt = std::max_element(rectangles.begin(), rectangles.end(), [] (Rectangle a, Rectangle b) { 
		return (a.ip.ep1.line1.acc + a.ip.ep1.line2.acc + a.ip.ep2.line1.acc + a.ip.ep2.line2.acc) < 
			(b.ip.ep1.line1.acc + b.ip.ep1.line2.acc + b.ip.ep2.line1.acc + b.ip.ep2.line2.acc);
	});
	
	Rectangle& finalRect = *finalRectangleIt;
	lineImg = gray.clone();
	for (size_t i = 0; i < finalRect.corners.size(); i++) {
		cv::line(lineImg, finalRect.corners[i], finalRect.corners[(i + 1) % 4], cv::Scalar(0, 0, 255), 3, CV_AA);
	}
	cv::namedWindow("finalRect");
	cv::imshow("finalRect", lineImg);
	cv::waitKey();
}

float PageExtractor::pointToLineDistance(LineSegment ls, cv::Point2f p) {
	return cv::Mat(p - ls.p1).dot(cv::Mat(p - ls.p2)) / std::pow(cv::norm(ls.p2 - ls.p1), 2);
}

std::vector<PageExtractor::HoughLine> PageExtractor::houghTransform(cv::Mat bwImg, float rho, float theta, int threshold, int linesMax) const {
	// the implementation is very similar to the one from opencv 2, but it returns the accumulator values and uses data structures that are easier to handle (but maybe less efficient)
	
	int width = bwImg.size().width;
	int height = bwImg.size().height;
	std::vector<HoughLine> lines;
	
	CV_Assert(bwImg.type() == CV_8UC1);

	int numAngle = cvRound(CV_PI / theta);
	int numRho = (width + height) * 2; // always even
	cv::Mat accum = cv::Mat::zeros(numRho, numAngle, CV_16UC1);
	float tabSin[numAngle];
	float tabCos[numAngle];
	
	float angle = 0.0f;
	for (int n = 0; n < numAngle; n++, angle += theta) {
		tabSin[n] = static_cast<float>(sin(static_cast<double>(angle)));
		tabCos[n] = static_cast<float>(cos(static_cast<double>(angle)));
	}
	
	// fill the accumulator
	for (size_t i = 0; i < height; i++) {
		for (size_t j = 0; j < width; j++) {
			if (bwImg.at<unsigned char>(i, j) != 0) {
				for (int n = 0; n < numAngle; n++) {
					int r = cvRound((j * tabCos[n] + i * tabSin[n]) / rho) + numRho / 2;
					accum.at<std::uint16_t>(r, n)++;
				}
			}
		}
	}
	
//	// show accumulator
//	cv::namedWindow("accumulator");
//	double min_, max_;
//	cv::Mat accumLine = accum.reshape(1);
//	cv::minMaxLoc(accumLine, &min_, &max_);
//	cv::Mat accumDraw;
//	accum.convertTo(accumDraw, CV_32FC1, 1.0 / max_);
//	cv::imshow("accumulator", accumDraw);
//	cv::waitKey();
	
	// find local maxima
	for (int r = 0; r < numRho; r++) {
		for (int n = 0; n < numAngle; n++) {
			int val = accum.at<std::uint16_t>(r, n);
			int valRl = (r - 1 >= 0) ? accum.at<std::uint16_t>(r - 1, n) : 0;
			int valRr = (r + 1 < numRho) ? accum.at<std::uint16_t>(r + 1, n) : 0;
			int valNl = (n - 1 >= 0) ? accum.at<std::uint16_t>(r, n - 1) : 0;
			int valNr = (n + 1 < numAngle) ? accum.at<std::uint16_t>(r, n + 1) : 0;
			if (val > threshold && 
					val > valRl && val > valRr &&
					val > valNl  && val > valNr) {
				
				HoughLine l;
				l.acc = val;
				l.rho = (r - numRho / 2) * rho;
				l.angle = n * theta;
				lines.push_back(l);
			}
		}
	}

	// sort by accumulator value
	std::sort(lines.begin(), lines.end(), [] (HoughLine l1, HoughLine l2) { return l1.acc > l2.acc; });
	lines.resize(linesMax);
	
	return lines;
}

float PageExtractor::angleDiff(float a, float b) {
	return std::min(std::abs(a - b), static_cast<float>(CV_PI) - std::abs(a - b));
}

std::vector<PageExtractor::LineSegment> PageExtractor::findLineSegments(cv::Mat bwImg, const std::vector<HoughLine>& houghLines, int minLength, int maxGap, bool dilate) const {
	if (dilate) {
		bwImg = bwImg.clone();
		cv::dilate(bwImg, bwImg, cv::Mat::ones(3, 3, CV_8UC1));
	}
	
	std::vector<LineSegment> lineSegments; // final line segments
	std::vector<LineSegment> lineSegmentsCurrent; // line segments per line
	LineFindingMode mode;
	int dimRange = 0;
	
	for (HoughLine line : houghLines) {
		lineSegmentsCurrent.clear();
		cv::Point2f startPos;
		bool active = false; // if true: a line is being followed
		bool inGap = false; // if true: a line is being followed and currently not interrupted
		cv::Point2f stopPos;
		cv::Point2f prevPos;
		int gapCounter = 0;
		
		//if (line.angle <= CV_PI / 4 || line.angle > (3 * CV_PI) / 4) {
		if (abs(line.angle - CV_PI / 2) > CV_PI / 4) {
			mode = LineFindingMode::Vertical;
			dimRange = bwImg.size().height;
		} else {
			mode = LineFindingMode::Horizontal;
			dimRange = bwImg.size().width;
		}
		
		for (size_t i = 0; i < dimRange; i++) {
			float x;
			float y;
			if (mode == LineFindingMode::Horizontal) {
				x = i;
				//y = std::max(std::min((line.rho - x * cv::cos(line.angle)) / (cv::sin(line.angle) + 0.000001f), bwImg.size().height - 1.0f), 0.0f);
				y = (line.rho - x * cv::cos(line.angle)) / (cv::sin(line.angle));
			} else {
				y = i;
				//x = std::max(std::min((line.rho - y * cv::sin(line.angle)) / (cv::cos(line.angle) + 0.000001f), bwImg.size().width - 1.0f), 0.0f);
				x = (line.rho - y * cv::sin(line.angle)) / (cv::cos(line.angle));
			}
			if (x > bwImg.size().width - 1.0f || x < 0.0f || y > bwImg.size().height - 1.0f || y < 0.0f) {
				continue;
			}
			
			// close open lines at the end
			if (i == dimRange - 1) {
				if (active) {
					LineSegment l;
					if (!inGap) {
						l.p1 = startPos;
						l.p2 = cv::Point2f(x, y);
					} else {
						l.p1 = startPos;
						l.p2 = stopPos;
					}
					
					l.length = cv::norm(l.p1 - l.p2);
					if (l.length > minLength) {
						lineSegmentsCurrent.push_back(l);
					}
				}
				break;
			}
			
			if (bwImg.at<std::uint8_t>((int) ceil(y), (int) ceil(x)) != 0 ||
					bwImg.at<std::uint8_t>((int) ceil(y), (int) floor(x)) != 0 || 
					bwImg.at<std::uint8_t>((int) floor(y), (int) ceil(x)) != 0 ||
					bwImg.at<std::uint8_t>((int) floor(y), (int) floor(x)) != 0) {
				
				if (!active) {
					startPos = cv::Point2f(x, y);
				}
				active = true;
				inGap = false;
			} else {
				if (!inGap) {
					gapCounter = 0;
					inGap = true;
					stopPos = prevPos;
				}
				gapCounter++;
				if (gapCounter >= maxGap && active) {
					if (cv::norm(stopPos - startPos) > minLength) {
						lineSegmentsCurrent.push_back(LineSegment {startPos, stopPos, static_cast<float>(cv::norm(stopPos - startPos))});
					}
					active = false;
				}
			}
			prevPos = cv::Point2f(x, y);
		}
		
//		std::cout << "DEBUG: acc " << line.acc << " (" << line.rho << ", " << line.angle << "), segments: ";
//		for (int k = 0; k < lineSegmentsCurrent.size(); k++) {
//			std::cout << lineSegmentsCurrent[k].length << " ";
//		}
//		std::cout << std::endl;
		// for every line in houghLines add only the longest line (including gaps) that was found in the image
		assert(lineSegmentsCurrent.size() > 0);
		auto longestLineSegmentIt = std::max_element(lineSegmentsCurrent.begin(), lineSegmentsCurrent.end(), [] (LineSegment l1, LineSegment l2) { return l1.length < l2.length; });
		lineSegments.push_back(*longestLineSegmentIt);
	}
	
	return lineSegments;
}

PageExtractor::ExtendedPeak::ExtendedPeak(const HoughLine& line1, const LineSegment& ls1, const HoughLine& line2, const LineSegment& ls2)
		: line1(line1), 
		line2(line2), 
		spatialLines(std::vector<LineSegment> {ls1, ls2}),
		intersectionPoint(findLineIntersection(ls1, ls2)) {
	
	// store mean angle
	if (abs(line1.angle - line2.angle) > CV_PI * 0.5) { // if angle difference is large enough, the angles become closer to each other 
		// note: lines are always in [0, pi]
		theta_k = 0.5f * ((std::min(line1.angle, line2.angle) + CV_PI) + std::max(line1.angle, line2.angle));
		if (theta_k > CV_PI) {
			theta_k -= CV_PI;
		}
	} else {
		theta_k = 0.5f * (line1.angle + line2.angle);
	}
	A_k = 0.5f * (line1.acc + line2.acc);
}

std::pair<bool, cv::Point2f> PageExtractor::findLineIntersection(const LineSegment& ls1, const LineSegment& ls2) {
	cv::Mat A = cv::Mat::zeros(2, 2, CV_32F);
	A.at<float>(0, 0) = ls1.p1.y - ls1.p2.y;
	A.at<float>(0, 1) = ls1.p2.x - ls1.p1.x;
	A.at<float>(1, 0) = ls2.p1.y - ls2.p2.y;
	A.at<float>(1, 1) = ls2.p2.x - ls2.p1.x;
	cv::Mat b = cv::Mat::zeros(2, 1, CV_32F);
	b.at<float>(0) = ls1.p1.x * (ls1.p1.y - ls1.p2.y) + ls1.p1.y * (ls1.p2.x - ls1.p1.x);
	b.at<float>(1) = ls2.p1.x * (ls2.p1.y - ls2.p2.y) + ls2.p1.y * (ls2.p2.x - ls2.p1.x);
	cv::Mat x;
	bool r = cv::solve(A, b, x);
	return std::pair<bool, cv::Point2f>(r, cv::Point2f(x));
}

};

