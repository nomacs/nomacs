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

#include "DkPageSegmentationUtils.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QDebug>
#include <opencv2/imgproc/imgproc.hpp>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {


// DkIntersectPoly --------------------------------------------------------------------
DkIntersectPoly::DkIntersectPoly() {};

DkIntersectPoly::DkIntersectPoly(std::vector<DkVector> vecA, std::vector<DkVector> vecB) {

	this->vecA = vecA;
	this->vecB = vecB;
	interArea = 0;
}

double DkIntersectPoly::compute() {

	// defines
	gamut = 500000000;
	minRange = DkVector(FLT_MAX, FLT_MAX);
	maxRange = DkVector(-FLT_MAX, -FLT_MAX);
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

void DkIntersectPoly::getVertices(const std::vector<DkVector>& vec, std::vector<DkVertex> *ip, int noise) {

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

void DkIntersectPoly::computeBoundingBox(std::vector<DkVector> vec, DkVector *minRange, DkVector *maxRange) {


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

DkPolyRect::DkPolyRect(const std::vector<DkVector>& pts) {
	this->pts = pts;
	computeMaxCosine();
	area = DBL_MAX;
}

bool DkPolyRect::empty() const {
	return pts.empty();
}

void DkPolyRect::toDkVectors(const std::vector<cv::Point>& pts, std::vector<DkVector>& dkPts) const {

	for (int idx = 0; idx < (int)pts.size(); idx++)
		dkPts.push_back(DkVector(pts.at(idx)));
}

void DkPolyRect::computeMaxCosine() {

	maxCosine = 0;

	for (int idx = 2; idx < (int)pts.size()+2; idx++ ) {

		DkVector& c = pts[(idx-1)%pts.size()];	// current corner;
		DkVector& c1 = pts[idx%pts.size()];
		DkVector& c2 = pts[idx-2];

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

	DkVector c = center();

	for (size_t idx = 0; idx < pts.size(); idx++) {
		pts[idx] = DkVector(pts[idx]-c)*s+c;
	}

	area = DBL_MAX;	// update
}

DkVector DkPolyRect::center() const {

	DkVector c;

	for (size_t idx = 0; idx < pts.size(); idx++)
		c += pts[idx];

	c /= (float)pts.size();

	return c;
}


bool DkPolyRect::inside(const DkVector& vec) const {

	float lastsign = 0;

	// we assume, that the polygon is convex
	// so if the point has a different scalar product
	// for one side of the polygon - it is not inside
	for (size_t idx = 1; idx < pts.size()+1; idx++) {

		DkVector dv(pts[idx-1] - pts[idx%pts.size()]);
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
		float cs = DkVector(pts[idx-1] - pts[idx%pts.size()]).norm();

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

void DkPolyRect::draw(cv::Mat& img, const cv::Scalar& col) const {

	std::vector<cv::Point> cvPts = toCvPoints();
	if (cvPts.empty())
		return;

	const cv::Point* p = &cvPts[0];
	int n = (int)cvPts.size();
	cv::polylines(img, &p, &n, 1, true, col, 4);
}

std::vector<DkVector> DkPolyRect::getCorners() const {
	return pts;
}

DkBox DkPolyRect::getBBox() const {

	DkVector uc(FLT_MAX, FLT_MAX), lc(-FLT_MAX, -FLT_MAX);

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

};

