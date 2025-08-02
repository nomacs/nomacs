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

#include <algorithm>

#include "DkPageSegmentationUtils.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QDebug>
#include <opencv2/imgproc/imgproc.hpp>
#pragma warning(pop) // no warnings from includes - end

namespace nmp
{

// DkIntersectPoly --------------------------------------------------------------------
DkIntersectPoly::DkIntersectPoly() = default;

DkIntersectPoly::DkIntersectPoly(std::vector<nmc::DkVector> vecA, std::vector<nmc::DkVector> vecB)
{
    this->vecA = vecA;
    this->vecB = vecB;
    interArea = 0;
}

double DkIntersectPoly::compute()
{
    // defines
    gamut = 500000000;
    mMinRange = nmc::DkVector(FLT_MAX, FLT_MAX);
    mMaxRange = nmc::DkVector(-FLT_MAX, -FLT_MAX);
    computeBoundingBox(vecA, &mMinRange, &mMaxRange);
    computeBoundingBox(vecB, &mMinRange, &mMaxRange);

    scale = mMaxRange - mMinRange;

    if (scale.minCoord() == 0)
        return 0; // rechteck mit hoehe oder breite = 0

    scale.x = gamut / scale.x;
    scale.y = gamut / scale.y;

    float ascale = scale.x * scale.y;

    // check input
    if (vecA.size() < 3 || vecB.size() < 3) {
        qDebug() << "The polygons must consist of at least 3 points but they are: (vecA: " << vecA.size()
                 << ", vecB: " << vecB.size();
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
                            cross(ipA[idxA],
                                  ipA[idxA + 1],
                                  ipB[idxB],
                                  ipB[idxB + 1],
                                  (double)a1,
                                  (double)a2,
                                  (double)a3,
                                  (double)a4);
                            ipA[idxA].in++;
                            ipB[idxB].in--;
                        } else {
                            cross(ipB[idxB],
                                  ipB[idxB + 1],
                                  ipA[idxA],
                                  ipA[idxA + 1],
                                  (double)a3,
                                  (double)a4,
                                  (double)a1,
                                  (double)a2);
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

void DkIntersectPoly::inness(std::vector<DkVertex> ipA, std::vector<DkVertex> ipB)
{
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

void DkIntersectPoly::cross(DkVertex a, DkVertex b, DkVertex c, DkVertex d, double a1, double a2, double a3, double a4)
{
    double r1 = a1 / ((double)a1 + a2 + DBL_EPSILON);
    double r2 = a3 / ((double)a3 + a4 + DBL_EPSILON);

    cntrib((int)a.ip.x + cvRound((double)(r1 * (double)(b.ip.x - a.ip.x))),
           (int)a.ip.y + cvRound((double)(r1 * (double)(b.ip.y - a.ip.y))),
           b.ip.x,
           b.ip.y,
           1);
    cntrib(d.ip.x,
           d.ip.y,
           (int)c.ip.x + cvRound((double)(r2 * (double)(d.ip.x - c.ip.x))),
           (int)c.ip.y + cvRound((double)(r2 * (double)(d.ip.y - c.ip.y))),
           1);
};

void DkIntersectPoly::cntrib(int fx, int fy, int tx, int ty, int w)
{
    interArea += (int64)w * (tx - fx) * (ty + fy) / 2;
};

int64 DkIntersectPoly::area(DkIPoint a, DkIPoint p, DkIPoint q)
{
    return (int64)p.x * q.y - (int64)p.y * q.x + (int64)a.x * (p.y - q.y) + (int64)a.y * (q.x - p.x);
};

bool DkIntersectPoly::ovl(DkIPoint p, DkIPoint q)
{
    return (p.x < q.y && q.x < p.y);
};

void DkIntersectPoly::getVertices(const std::vector<nmc::DkVector> &vec, std::vector<DkVertex> *ip, int noise)
{
    std::vector<DkIPoint> vecTmp;

    // transform the coordinates and modify the least significant bits (that's fun)
    for (int idx = 0; idx < (int)vec.size(); idx++) {
        DkIPoint cp;
        cp.x = ((int)((vec[idx].x - mMinRange.x) * scale.x - gamut / 2) & ~7) | noise | (idx & 1);
        cp.y = ((int)((vec[idx].y - mMinRange.y) * scale.y - gamut / 2) & ~7) | noise | (idx & 1);

        vecTmp.push_back(cp);
    }

    vecTmp.push_back(*(vecTmp.begin())); // append the first element

    for (int idx = 0; idx < (int)vecTmp.size(); idx++) {
        int nIdx = idx % (int)(vecTmp.size() - 1)
            + 1; // the last element should refer to the second (first & last are the very same)

        DkIPoint cEdgeX = (vecTmp[idx].x < vecTmp[nIdx].x) ? DkIPoint(vecTmp[idx].x, vecTmp[nIdx].x)
                                                           : DkIPoint(vecTmp[nIdx].x, vecTmp[idx].x);
        DkIPoint cEdgeY = (vecTmp[idx].y < vecTmp[nIdx].y) ? DkIPoint(vecTmp[idx].y, vecTmp[nIdx].y)
                                                           : DkIPoint(vecTmp[nIdx].y, vecTmp[idx].y);

        ip->push_back(DkVertex(vecTmp[idx], cEdgeX, cEdgeY));
    }
};

void DkIntersectPoly::computeBoundingBox(std::vector<nmc::DkVector> vec,
                                         nmc::DkVector *minRange,
                                         nmc::DkVector *maxRange)
{
    for (unsigned int idx = 0; idx < vec.size(); idx++) {
        *minRange = minRange->minVec(vec[idx]);
        *maxRange = maxRange->maxVec(vec[idx]); // in our case it's the max vector
    }
};

// DkPolyRect --------------------------------------------------------------------
DkPolyRect::DkPolyRect(const std::vector<cv::Point> &pts)
{
    toDkVectors(pts, this->mPts);
    computeMaxCosine();
    area = DBL_MAX;
}

DkPolyRect::DkPolyRect(const std::vector<nmc::DkVector> &pts)
{
    this->mPts = pts;
    computeMaxCosine();
    area = DBL_MAX;
}

bool DkPolyRect::empty() const
{
    return mPts.empty();
}

void DkPolyRect::toDkVectors(const std::vector<cv::Point> &pts, std::vector<nmc::DkVector> &dkPts) const
{
    for (int idx = 0; idx < (int)pts.size(); idx++)
        dkPts.push_back(nmc::DkVector(pts.at(idx)));
}

void DkPolyRect::computeMaxCosine()
{
    maxCosine = 0;

    for (int idx = 2; idx < (int)mPts.size() + 2; idx++) {
        nmc::DkVector &c = mPts[(idx - 1) % mPts.size()]; // current corner;
        nmc::DkVector &c1 = mPts[idx % mPts.size()];
        nmc::DkVector &c2 = mPts[idx - 2];

        double cosine = abs((c1 - c).cosv(c2 - c));

        maxCosine = std::max(maxCosine, cosine);
    }
}

double DkPolyRect::intersectArea(const DkPolyRect &pr) const
{
    return DkIntersectPoly(mPts, pr.mPts).compute();
}

void DkPolyRect::scale(float s)
{
    for (size_t idx = 0; idx < mPts.size(); idx++)
        mPts[idx] = mPts[idx] * s;

    area = DBL_MAX; // update
}

void DkPolyRect::scaleCenter(float s)
{
    nmc::DkVector c = center();

    for (size_t idx = 0; idx < mPts.size(); idx++) {
        mPts[idx] = nmc::DkVector(mPts[idx] - c) * s + c;
    }

    area = DBL_MAX; // update
}

nmc::DkVector DkPolyRect::center() const
{
    nmc::DkVector c;

    for (size_t idx = 0; idx < mPts.size(); idx++)
        c += mPts[idx];

    c /= (float)mPts.size();

    return c;
}

bool DkPolyRect::inside(const nmc::DkVector &vec) const
{
    float lastsign = 0;

    // we assume, that the polygon is convex
    // so if the point has a different scalar product
    // for one side of the polygon - it is not inside
    for (size_t idx = 1; idx < mPts.size() + 1; idx++) {
        nmc::DkVector dv(mPts[idx - 1] - mPts[idx % mPts.size()]);
        float csign = dv.scalarProduct(vec - mPts[idx % mPts.size()]);

        if (lastsign * csign < 0) {
            return false;
        }

        lastsign = csign;
    }

    return true;
}

float DkPolyRect::maxSide() const
{
    float ms = 0;

    for (size_t idx = 1; idx < mPts.size() + 1; idx++) {
        float cs = nmc::DkVector(mPts[idx - 1] - mPts[idx % mPts.size()]).norm();

        if (ms < cs)
            ms = cs;
    }

    return ms;
}

double DkPolyRect::getArea()
{
    if (area == DBL_MAX)
        area = abs(intersectArea(*this));

    return area;
}

double DkPolyRect::getAreaConst() const
{
    if (area == DBL_MAX)
        return abs(intersectArea(*this));

    return area;
}

bool DkPolyRect::compArea(const DkPolyRect &pl, const DkPolyRect &pr)
{
    return pl.getAreaConst() < pr.getAreaConst();
}

nmc::DkRotatingRect DkPolyRect::toRotatingRect() const
{
    if (empty())
        return nmc::DkRotatingRect();

    // find the largest rectangle
    std::vector<cv::Point> largeRect = toCvPoints();

    cv::RotatedRect rect = cv::minAreaRect(largeRect);

    // convert to corners
    nmc::DkVector xVec = nmc::DkVector(rect.size.width * 0.5f, 0);
    xVec.rotate(-rect.angle * DK_DEG2RAD);

    nmc::DkVector yVec = nmc::DkVector(0, rect.size.height * 0.5f);
    yVec.rotate(-rect.angle * DK_DEG2RAD);

    QPolygonF poly;
    poly.append(nmc::DkVector(rect.center - xVec + yVec).toQPointF());
    poly.append(nmc::DkVector(rect.center + xVec + yVec).toQPointF());
    poly.append(nmc::DkVector(rect.center + xVec - yVec).toQPointF());
    poly.append(nmc::DkVector(rect.center - xVec - yVec).toQPointF());

    nmc::DkRotatingRect rr;
    rr.setPoly(poly);

    return rr;
}

void DkPolyRect::draw(cv::Mat &img, const cv::Scalar &col) const
{
    std::vector<cv::Point> cvPts = toCvPoints();
    if (cvPts.empty())
        return;

    const cv::Point *p = &cvPts[0];
    int n = (int)cvPts.size();
    cv::polylines(img, &p, &n, 1, true, col, 4);
}

std::vector<nmc::DkVector> DkPolyRect::getCorners() const
{
    return mPts;
}

DkBox DkPolyRect::getBBox() const
{
    nmc::DkVector uc(FLT_MAX, FLT_MAX), lc(-FLT_MAX, -FLT_MAX);

    for (size_t idx = 0; idx < mPts.size(); idx++) {
        uc = uc.minVec(mPts[idx]);
        lc = lc.maxVec(mPts[idx]);
    }

    if (mPts.empty())
        qDebug() << "bbox of empty poly rect requested!!";

    DkBox box(uc, lc - uc);

    return box;
}

std::vector<cv::Point> DkPolyRect::toCvPoints() const
{
    std::vector<cv::Point> cvPts;
    for (int idx = 0; idx < (int)mPts.size(); idx++) {
        cvPts.push_back(mPts[idx].getCvPoint());
    }

    return cvPts;
}

QPolygonF DkPolyRect::toPolygon() const
{
    QPolygonF poly;

    for (const nmc::DkVector &v : mPts) {
        poly.append(v.toQPointF());
    }

    return poly;
}

void PageExtractor::findPage(cv::Mat img, float scale, std::vector<DkPolyRect> &rects)
{
    cv::Mat gray, bw;

    cv::cvtColor(img, gray, CV_RGB2GRAY);
    if (scale != 1.0f) {
        cv::resize(gray, gray, cv::Size(), scale, scale, CV_INTER_AREA); // inter nn -> assuming resize to be 1/(2^n)
    }
    const int smallerSide = std::min(gray.size().width, gray.size().height);

    cv::equalizeHist(gray, gray);
    bw = removeText(gray, 2.0f, 5, 2);
    //	cv::imshow("bw after removeText", bw);
    //	cv::waitKey(0);

    cv::dilate(bw, bw, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));

    cv::Mat lineImg;
    int accMin = (int)(houghPeakThresholdRel * std::min(bw.size().width, bw.size().height));
    std::vector<HoughLine> lines = houghTransform(bw, 1, (float)(CV_PI / 180.0), accMin, maxLinesHough);
    if (lines.empty()) {
        qDebug() << "no hough lines detected";
        return;
    }

    // find line segments in image
    int maxGapLength = (int)(maxGapLengthRel * smallerSide);
    std::vector<LineSegment> lineSegments = findLineSegments(bw, lines, minLineSegmentLength, maxGapLength);
    if (lineSegments.empty()) {
        qDebug() << "findLineSegments has not found any line segments, even though hough lines were detected.";
        return;
    }

    // 4.3 transform domain peak filtering
    // iterate through all pairs of lines and build pairs of parallel line segments called extended peak pairs (EPs)
    std::vector<ExtendedPeak> EPs;
    for (size_t i = 0; i < lines.size() - 1; i++) {
        for (size_t j = i + 1; j < lines.size(); j++) {
            // test for parallelity
            if (angleDiff(lines[i].angle, lines[j].angle) < t_theta
                && std::abs(lines[i].acc - lines[j].acc) < t_l * 0.5 * (lines[i].acc + lines[j].acc)) {
                // 'parallel' line segments must not intersect
                ExtendedPeak ep(lines[i], lineSegments[i], lines[j], lineSegments[j]);
                if (ep.intersectionPoint.first) {
                    std::vector<cv::Point2f> epHull;
                    cv::convexHull(std::vector<cv::Point2f>{lineSegments[i].p1,
                                                            lineSegments[i].p2,
                                                            lineSegments[j].p1,
                                                            lineSegments[j].p2},
                                   epHull);
                    if (cv::pointPolygonTest(epHull, ep.intersectionPoint.second, false) >= 0) {
                        continue;
                    }
                }
                EPs.push_back(ep);
            }
        }
    }

    // iterate through pairs of EPs and combine them to intermediate peak pairs (IPs) if they form a rectangular shape
    std::vector<IntermediatePeak> IPs;
    for (size_t i = 0; i < EPs.size(); i++) {
        for (size_t j = i + 1; j < EPs.size(); j++) {
            // test for orthogonality
            if (abs(angleDiff(EPs[i].theta_k, EPs[j].theta_k) - (CV_PI * 0.5)) < orthoTol) {
                IPs.push_back(IntermediatePeak{EPs[i], EPs[j]});
            }
        }
    }

    // test IP corners
    std::vector<Rectangle> candidates;
    for (const IntermediatePeak &ip : IPs) {
        std::vector<cv::Point2f> corners;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                auto r = findLineIntersection(ip.ep1.spatialLines[i], ip.ep2.spatialLines[j]);
                // since the lines of different EPs can not be parallel, they have to intersect at some point
                if (r.first != true) {
                    qDebug() << "no intersection was found for two lines that should not be parallel";
                    return;
                }
                cv::Point2f p = r.second;
                if (pointToLineDistance(ip.ep1.spatialLines[i], p) < cornerGapTol
                    && pointToLineDistance(ip.ep2.spatialLines[j], p) < cornerGapTol) {
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
    for (const Rectangle &c : candidates) {
        // check if sides are large enough
        bool largeEnough = true;
        for (int i = 0; i < 4; i++) {
            if (cv::norm(c.corners[i] - c.corners[(i + 1) % 4]) < minRelSideLength * smallerSide) {
                largeEnough = false;
            }
        }
        if (!largeEnough) {
            continue;
        }
        rectangles.push_back(c);
    }

    if (rectangles.empty()) {
        qDebug() << "no valid rectangles have been detected!";
        return;
    }

    // sort rectangles by overall accumulator value in descending order
    std::sort(rectangles.begin(), rectangles.end(), [](const Rectangle &a, const Rectangle &b) {
        return (a.ip.ep1.line1.acc + a.ip.ep1.line2.acc + a.ip.ep2.line1.acc + a.ip.ep2.line2.acc) > // descending
            (b.ip.ep1.line1.acc + b.ip.ep1.line2.acc + b.ip.ep2.line1.acc + b.ip.ep2.line2.acc);
    });

    if (rectangles.size() > numFinalRects) {
        rectangles.erase(rectangles.begin() + numFinalRects, rectangles.end());
    }

    // construct DkPolyRects for the rectangles to be returned
    for (Rectangle rect : rectangles) {
        std::vector<cv::Point> cornerPoints;
        for (int i = 0; i < 4; i++) {
            cornerPoints.emplace_back((int)round(rect.corners[i].x), (int)round(rect.corners[i].y));
        }
        DkPolyRect r(cornerPoints);
        r.scale(1.0f / scale);
        rects.push_back(r);
    }
}

float PageExtractor::pointToLineDistance(LineSegment ls, cv::Point2f p)
{
    return (float)(cv::Mat(p - ls.p1).dot(cv::Mat(p - ls.p2)) / std::pow(cv::norm(ls.p2 - ls.p1), 2));
}

/**
 * Hough transform, similar to the OpenCV implementation, returns a vector of the linesMax lines, sorted by accumulator
 * value in descending order.
 */
std::vector<PageExtractor::HoughLine> PageExtractor::houghTransform(cv::Mat bwImg,
                                                                    float rho,
                                                                    float theta,
                                                                    int threshold,
                                                                    int linesMax) const
{
    // the implementation is very similar to the one from opencv 2, but it returns the accumulator values and uses some
    // different data structures

    if (bwImg.type() != CV_8U) {
        qDebug() << "custom houghTransform only supports CV_8U input images";
        return std::vector<PageExtractor::HoughLine>();
    }

    int width = bwImg.cols;
    int height = bwImg.rows;
    std::vector<HoughLine> lines;

    int numAngle = cvRound(CV_PI / theta) + 2;
    int numRho = (width + height) * 2 + 2; // always even
    cv::Mat accum = cv::Mat::zeros(numRho, numAngle, CV_16U);
    std::vector<double> tabSin(numAngle - 2);
    std::vector<double> tabCos(numAngle - 2);

    float angle = 0.0f;
    for (int n = 0; n < numAngle - 2; n++, angle += theta) {
        tabSin[n] = sin(static_cast<double>(angle));
        tabCos[n] = cos(static_cast<double>(angle));
    }

    // fill the accumulator
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (bwImg.at<unsigned char>(i, j) != 0) {
                for (int n = 0; n < numAngle - 2; n++) {
                    int r = cvRound((j * tabCos[n] + i * tabSin[n]) / rho) + numRho / 2;
                    accum.at<std::uint16_t>(r + 1, n + 1)++;
                }
            }
        }
    }

    // find local maxima
    for (int r = 1; r < numRho - 1; r++) {
        for (int n = 1; n < numAngle - 1; n++) {
            int val = accum.at<std::uint16_t>(r, n);
            int valRl = accum.at<std::uint16_t>(r - 1, n);
            int valRr = accum.at<std::uint16_t>(r + 1, n);
            int valNl = accum.at<std::uint16_t>(r, n - 1);
            int valNr = accum.at<std::uint16_t>(r, n + 1);
            if (val > threshold && val > valRl && val > valRr && val > valNl && val > valNr) {
                HoughLine l;
                l.acc = val;
                l.rho = ((r - 1) - numRho / 2) * rho;
                l.angle = (n - 1) * theta;
                lines.push_back(l);
            }
        }
    }

    // sort by accumulator value
    std::sort(lines.begin(), lines.end(), [](HoughLine l1, HoughLine l2) {
        return l1.acc > l2.acc;
    });
    lines.resize(linesMax);

    return lines;
}

/**
 * Returns the shorter distance between angles a and b, which are in the interval [0, pi]
 */
double PageExtractor::angleDiff(double a, double b)
{
    return std::min(std::abs(a - b), static_cast<float>(CV_PI) - std::abs(a - b));
}

/**
 * Finds the corresponding line segments (the largest ones) to all houghLines in the binary image bwImg.
 * @param bwImg the binary image on which the hough transform was performed
 * @param houghLines vector of hough lines
 * @param minLength the minimum line length
 * @param maxGap the tolerance for gaps in the line segments
 */
std::vector<PageExtractor::LineSegment> PageExtractor::findLineSegments(cv::Mat bwImg,
                                                                        const std::vector<HoughLine> &houghLines,
                                                                        int minLength,
                                                                        int maxGap) const
{
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
        bool notYetInImageRange = true;

        // in vertical mode, the x values are calculated for every y
        if (abs(line.angle - CV_PI / 2) > CV_PI / 4) {
            mode = LineFindingMode::Vertical;
            dimRange = bwImg.size().height;
        } else { // in horizontal mode, the y values are calculated for every x
            mode = LineFindingMode::Horizontal;
            dimRange = bwImg.size().width;
        }

        float x;
        float y;
        // go through all x or y values and calculate the corresponding coordinate
        for (int i = 0; i < dimRange; i++) {
            if (mode == LineFindingMode::Horizontal) {
                x = (float)i;
                y = (line.rho - x * cos(line.angle)) / (sin(line.angle));
                if (notYetInImageRange && y <= bwImg.rows - 1 && y >= 0) {
                    notYetInImageRange = false;
                }
            } else {
                y = (float)i;
                ;
                x = (line.rho - y * sin(line.angle)) / (cos(line.angle));
                if (notYetInImageRange && x <= bwImg.cols - 1 && x >= 0) {
                    notYetInImageRange = false;
                }
            }
            if (notYetInImageRange) {
                continue;
            }

            // close open lines at the end
            if (i == dimRange - 1 || x > bwImg.cols - 1 || x < 0 || y > bwImg.rows - 1 || y < 0) {
                if (active) {
                    LineSegment l;
                    if (!inGap) {
                        l.p1 = startPos;
                        l.p2 = cv::Point2f(x, y);
                    } else {
                        l.p1 = startPos;
                        l.p2 = stopPos;
                    }

                    l.length = (float)cv::norm(l.p1 - l.p2);
                    if (l.length > minLength) {
                        lineSegmentsCurrent.push_back(l);
                    }
                }
                break;
            }

            // test if (x, y) is an edge pixel. account for small errors by checking all possible positions
            if (bwImg.at<unsigned char>((int)ceil(y), (int)ceil(x)) != 0
                || bwImg.at<unsigned char>((int)ceil(y), (int)floor(x)) != 0
                || bwImg.at<unsigned char>((int)floor(y), (int)ceil(x)) != 0
                || bwImg.at<unsigned char>((int)floor(y), (int)floor(x)) != 0) {
                if (!active) {
                    startPos = cv::Point2f(x, y);
                    active = true;
                }
                inGap = false;
            } else { // position is not an edge pixel
                // assume that the line segment is just interrupted (we are in a gap)
                if (!inGap) {
                    gapCounter = 0;
                    inGap = true;
                    stopPos = prevPos;
                }
                gapCounter++;
                // if the gap is too large, the line segment gets closed
                if (gapCounter >= maxGap && active) {
                    if (cv::norm(stopPos - startPos) > minLength) {
                        lineSegmentsCurrent.push_back(
                            LineSegment{startPos, stopPos, static_cast<float>(cv::norm(stopPos - startPos))});
                    }
                    active = false;
                }
            }
            prevPos = cv::Point2f(x, y);
        }

        // for every line in houghLines add only the longest line (including gaps) that was found in the image
        if (!lineSegmentsCurrent.empty()) {
            auto longestLineSegmentIt = std::max_element(lineSegmentsCurrent.begin(),
                                                         lineSegmentsCurrent.end(),
                                                         [](LineSegment l1, LineSegment l2) {
                                                             return l1.length < l2.length;
                                                         });
            lineSegments.push_back(*longestLineSegmentIt);
        }
    }

    return lineSegments;
}

PageExtractor::ExtendedPeak::ExtendedPeak(const HoughLine &line1,
                                          const LineSegment &ls1,
                                          const HoughLine &line2,
                                          const LineSegment &ls2)
    : line1(line1)
    , line2(line2)
    , spatialLines(std::vector<LineSegment>{ls1, ls2})
    , intersectionPoint(findLineIntersection(ls1, ls2))
{
    // store mean angle
    if (abs(line1.angle - line2.angle)
        > CV_PI * 0.5) { // if angle difference is large enough, the angles become closer to each other
        // note: lines are always in [0, pi]
        theta_k = 0.5 * ((std::min(line1.angle, line2.angle) + CV_PI) + std::max(line1.angle, line2.angle));
        if (theta_k > CV_PI) {
            theta_k -= CV_PI;
        }
    } else {
        theta_k = 0.5 * (line1.angle + line2.angle);
    }
    A_k = 0.5 * (line1.acc + line2.acc);
}

/**
 * Finds the intersection point of ls1 and ls2 when extended to infinity. If the lines don't intersect, the boolean part
 * is false.
 */
std::pair<bool, cv::Point2f> PageExtractor::findLineIntersection(const LineSegment &ls1, const LineSegment &ls2)
{
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

/**
 * Generates an edge image of gray, tries to remove small text-like structures and returns it.
 */
cv::Mat PageExtractor::removeText(cv::Mat gray, float sigma, int selemSize, int threshold)
{
    if (gray.type() != CV_8U) {
        qDebug() << "removeText only supports CV_8U format";
        return gray;
    }

    static const float eps = 0.001f;
    cv::Mat bw;
    cv::Mat sobel_h;
    cv::Mat sobel_v;
    cv::Mat sobel_angle = cv::Mat::zeros(gray.size(), CV_32F);
    cv::GaussianBlur(gray, gray, cv::Size((int)(2 * floor(sigma * 3) + 1), (int)(2 * floor(sigma * 3) + 1)), sigma);
    cv::Canny(gray, bw, 0.1 * 255, 0.2 * 255);
    cv::Sobel(gray, sobel_h, CV_32F, 0, 1, 3);
    cv::Sobel(gray, sobel_v, CV_32F, 1, 0, 3);

    for (int i = 0; i < sobel_angle.rows; i++) {
        for (int j = 0; j < sobel_angle.cols; j++) {
            // calculate gradient angle
            float angle = (atan2(sobel_v.at<float>(i, j), sobel_h.at<float>(i, j)));
            // shift [-pi, 0] to [pi, 2pi]
            if (angle < 0.0f) {
                angle += (float)(2.0f * CV_PI);
            }
            // set 2pi to 0
            if (angle >= 2 * CV_PI) {
                angle = 0.0f;
            }
            sobel_angle.at<float>(i, j) = angle;
        }
    }

    // edge plane decomposition

    std::vector<cv::Mat> E_i(8);
    std::vector<cv::Mat> E_i_ex(8);
    cv::Mat H = cv::Mat::zeros(gray.size(), CV_8U);
    cv::Mat mask;
    cv::Mat mask_factor = (cv::abs(sobel_h) > eps | cv::abs(sobel_v) > eps);
    cv::Mat M_text_inv;
    float rangeStart;
    float rangeEnd;
    // go through angles in pi/4 steps
    for (int i = 0; i < 8; i++) {
        rangeStart = (float)(CV_PI / 4.0f * i);
        rangeEnd = (float)(CV_PI / 4.0f * (i + 1));
        mask = ((sobel_angle >= rangeStart) & (sobel_angle < rangeEnd)) & mask_factor;
        E_i[i] = mask & bw;
        cv::dilate(E_i[i],
                   E_i_ex[i],
                   cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2 * selemSize, 2 * selemSize)));
        cv::threshold(E_i_ex[i],
                      E_i_ex[i],
                      1,
                      1,
                      cv::THRESH_TRUNC); // E_i_ex is binary, true pixels are 255, we have to truncate them to 1
        H += E_i_ex[i];
    }

    // remove text regions
    M_text_inv = H <= threshold;
    cv::Mat E_i_hat = cv::Mat::zeros(bw.size(), CV_8U);
    for (int i = 0; i < 8; i++) {
        E_i_hat = E_i_hat | (E_i[i] & M_text_inv); // equals (E_i[i] AND M_text) XOR E_i[i]
    }

    return E_i_hat;
}

};
