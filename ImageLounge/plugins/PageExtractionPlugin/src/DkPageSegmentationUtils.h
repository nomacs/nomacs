/*******************************************************************************************************
 DkPageSegmentation.h

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2015 Markus Diem <markus@nomacs.org>

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

#include "DkMath.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QString>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#pragma warning(pop) // no warnings from includes - end

namespace nmp
{

/**
 * Box class DK_CORE_API, defines a non-skewed rectangle e.g. Bounding Box
 **/
class DkBox
{
public:
    /**
     * Default constructor.
     * All values are initialized with zero.
     **/
    DkBox()
        : uc()
        , lc(){};

    /**
     * Constructor.
     * @param uc the upper left corner of the box.
     * @param size the size of the box.
     **/
    DkBox(nmc::DkVector uc, nmc::DkVector size)
    {
        this->uc = uc;
        this->lc = uc + size;

        // if (size.width < 0 || size.height < 0)
        // std::cout << "the size is < 0: " << size << std::endl;
    };
    /**
     * Constructor.
     * @param x value of the upper left corner.
     * @param y value of the upper left corner.
     * @param width of the box.
     * @param height of the box.
     **/
    DkBox(float x, float y, float width, float height)
    {
        nmc::DkVector size = nmc::DkVector(width, height);

        uc = nmc::DkVector(x, y);
        lc = uc + size;

        // if (size.width < 0 || size.height < 0)
        // std::cout << "the size is < 0: " << size << std::endl;
    };
    /**
     * Constructor.
     * @param r box as rect with upperleft corner and width and height.
     **/
    DkBox(cv::Rect r)
    {
        nmc::DkVector size((float)r.width, (float)r.height);

        uc.x = (float)r.x;
        uc.y = (float)r.y;

        lc = uc + size;

        // if (size.width < 0 || size.height < 0)
        //	std::cout << "the size is < 0: " << size << std::endl;
    };

    /**
     * Constructor.
     * @param b box as DkBox.
     **/
    DkBox(const DkBox &b)
    {
        this->uc = b.uc;
        this->lc = b.uc + b.size();

        // if (size().width < 0 || size().height < 0)
        //	std::cout << "the size is < 0: " << size() << std::endl;
    }
    /**
     * Default destructor.
     **/
    ~DkBox(){};

    void getStorageBuffer(char **buffer, size_t &length) const
    {
        size_t newBufferLength = length + 4 * sizeof(float);
        char *newStream = new char[newBufferLength];

        if (*buffer) {
            // copy old stream & clean it
            memcpy(newStream, *buffer, length);
            delete *buffer;
        }

        float *newFStream = (float *)newStream;

        int pos = 0;
        newFStream[pos] = uc.x;
        pos++;
        newFStream[pos] = uc.y;
        pos++;
        newFStream[pos] = lc.x;
        pos++;
        newFStream[pos] = lc.y;
        pos++;

        *buffer = newStream;
        length = newBufferLength;
    }

    const char *setSorageBuffer(const char *buffer)
    {
        const float *fBuffer = (const float *)buffer;
        int pos = 0;
        uc.x = fBuffer[pos];
        pos++;
        uc.y = fBuffer[pos];
        pos++;
        lc.x = fBuffer[pos];
        pos++;
        lc.y = fBuffer[pos];
        pos++;

        return buffer + sizeof(float) * pos; // update buffer position
    }

    // friend std::ostream& operator<<(std::ostream& s, DkBox& b) - original
    friend std::ostream &operator<<(std::ostream &s, DkBox b)
    {
        // this makes the operator<< virtual (stroustrup)
        return s << b.toString().toStdString();
    };

    void moveBy(const nmc::DkVector &dxy)
    {
        uc += dxy;
        lc += dxy;
    };

    bool isEmpty() const
    {
        return uc.isEmpty() && lc.isEmpty();
    };

    /**
     * Returns the box as opencv Rect.
     * @return a box as opencv Rect.
     **/
    cv::Rect getCvRect() const
    {
        return cv::Rect(cvRound(uc.x), cvRound(uc.y), cvRound(size().width), cvRound(size().height));
    }

    static DkBox contour2BBox(const std::vector<std::vector<cv::Point>> &pts)
    {
        if (pts.empty())
            return DkBox();

        // TODO: write this in dk style
        int ux = INT_MAX, uy = INT_MAX;
        int lx = 0, ly = 0;

        for (int cIdx = 0; cIdx < (int)pts.size(); cIdx++) {
            const std::vector<cv::Point> &cont = pts[cIdx];

            for (int idx = 0; idx < (int)cont.size(); idx++) {
                cv::Point p = cont[idx];

                if (p.x < ux)
                    ux = p.x;
                if (p.x > lx)
                    lx = p.x;
                if (p.y < uy)
                    uy = p.y;
                if (p.y > ly)
                    ly = p.y;
            }
        }
        DkBox rect((float)ux, (float)uy, (float)lx - ux, (float)ly - uy);

        return rect;
    }

    /**
     * Enlarges the box by the given offset, and the upperleft corner is recalculated.
     * @param offset by which the box is expanded.
     **/
    void expand(float offset)
    {
        uc -= (offset * 0.5f);
    }

    /**
     * Clips the box according the vector s (the box is only clipped but not expanded).
     * @param s the clip vector.
     **/
    void clip(nmc::DkVector s)
    {
        uc.round();
        lc.round();

        uc.clipTo(s);
        lc.clipTo(s);

        // if (lc.x > s.x || lc.y > s.y)
        //	mout << "I did not clip..." << dkendl;
    };

    bool within(const nmc::DkVector &p) const
    {
        return (p.x >= uc.x && p.x < lc.x && p.y >= uc.y && p.y < lc.y);
    };

    nmc::DkVector center() const
    {
        return uc + size() * 0.5f;
    };

    void scaleAboutCenter(float s)
    {
        nmc::DkVector c = center();

        uc = nmc::DkVector(uc - c) * s + c;
        lc = nmc::DkVector(lc - c) * s + c;
    };

    /**
     * Returns the x value of the upper left corner.
     * @return x value in pixel of the upperleft corner.
     **/
    int getX() const
    {
        return cvRound(uc.x);
    };
    /**
     * Returns the y value of the upper left corner.
     * @return y value in pixel of the upperleft corner.
     **/
    int getY() const
    {
        return cvRound(uc.y);
    };
    /**
     * Returns the width of the box.
     * @return the width in pixel of the box.
     **/
    int getWidth() const
    {
        return cvRound(lc.x - uc.x);
    };
    /**
     * Returns the width of the box.
     * @return float the width in pixel fo the box.
     **/
    float getWidthF() const
    {
        return lc.x - uc.x;
    };
    /**
     * Returns the height of the box.
     * @return the height in pixel of the box.
     **/
    int getHeight() const
    {
        return cvRound(lc.y - uc.y);
    };
    /**
     * Returns the height of the box as float
     * @return float height in pixel of the box.
     **/
    float getHeightF() const
    {
        return lc.y - uc.y;
    };
    /**
     * Returns the size of the box.
     * @return size of the box as opencv Size.
     **/
    cv::Size getSize() const
    {
        return cv::Size(getWidth(), getHeight());
    };

    nmc::DkVector size() const
    {
        return lc - uc;
    };

    void setSize(nmc::DkVector size)
    {
        lc = uc + size;
    };

    float area() const
    {
        nmc::DkVector s = size();
        return s.width * s.height;
    };

    float intersectArea(const DkBox &box) const
    {
        nmc::DkVector tmp1 = lc.maxVec(box.lc);
        nmc::DkVector tmp2 = uc.maxVec(box.uc);

        // no intersection?
        if (lc.x < uc.x || lc.y < lc.y)
            return 0;

        tmp1 = tmp2 - tmp1;

        return tmp1.width * tmp1.height;
    };

    QString toString() const
    {
        // QString msg =	"\n upper corner: " + uc.toString();
        // msg +=				"\n size:         " + size().toString();

        return QString();
    };

    nmc::DkVector uc; /**< upper left corner of the box **/
    nmc::DkVector lc; /**< lower right corner of the box **/
};

/**
 * A simple point class DK_CORE_API.
 * This class DK_CORE_API is needed for a fast computation
 * of the polygon overlap.
 **/
class DkIPoint
{
public:
    int x;
    int y;

    DkIPoint()
        : x(0)
        , y(0){};

    DkIPoint(int x, int y)
    {
        this->x = x;
        this->y = y;
    };
};

/**
 * A simple vertex class DK_CORE_API.
 * This class DK_CORE_API is needed for a fast computation
 * of the polygon overlap.
 **/
class DkVertex
{
public:
    DkIPoint ip;
    DkIPoint rx;
    DkIPoint ry;
    int in;

    DkVertex(){};

    DkVertex(DkIPoint ip, DkIPoint rx, DkIPoint ry)
    {
        this->ip = ip;
        this->rx = rx;
        this->ry = ry;
        in = 0;
    };
};

class DkIntersectPoly
{
    // this class DK_CORE_API is based on a method proposed by norman hardy
    // see: http://www.cap-lore.com/MathPhys/IP/aip.c

public:
    DkIntersectPoly();
    DkIntersectPoly(std::vector<nmc::DkVector> vecA, std::vector<nmc::DkVector> vecB);

    double compute();

private:
    std::vector<nmc::DkVector> vecA;
    std::vector<nmc::DkVector> vecB;
    int64 interArea;
    nmc::DkVector mMaxRange;
    nmc::DkVector mMinRange;
    nmc::DkVector scale;
    float gamut;

    void inness(std::vector<DkVertex> ipA, std::vector<DkVertex> ipB);
    void cross(DkVertex a, DkVertex b, DkVertex c, DkVertex d, double a1, double a2, double a3, double a4);
    void cntrib(int fx, int fy, int tx, int ty, int w);
    int64 area(DkIPoint a, DkIPoint p, DkIPoint q);
    bool ovl(DkIPoint p, DkIPoint q);
    void getVertices(const std::vector<nmc::DkVector> &vec, std::vector<DkVertex> *ip, int noise);
    void computeBoundingBox(std::vector<nmc::DkVector> vec, nmc::DkVector *minRange, nmc::DkVector *maxRange);
};

// data class
class DkPolyRect
{
public:
    // DkPolyRect(nmc::DkVector p1, nmc::DkVector p2, nmc::DkVector p3, nmc::DkVector p4);
    DkPolyRect(const std::vector<cv::Point> &pts = std::vector<cv::Point>());
    DkPolyRect(const std::vector<nmc::DkVector> &pts);

    bool empty() const;
    double getMaxCosine() const
    {
        return maxCosine;
    };
    void draw(cv::Mat &img, const cv::Scalar &col = cv::Scalar(0, 100, 255)) const;
    std::vector<cv::Point> toCvPoints() const;
    QPolygonF toPolygon() const;
    std::vector<nmc::DkVector> getCorners() const;
    DkBox getBBox() const;
    double intersectArea(const DkPolyRect &pr) const;
    double getArea();
    double getAreaConst() const;
    void scale(float s);
    void scaleCenter(float s);
    bool inside(const nmc::DkVector &vec) const;
    float maxSide() const;
    nmc::DkVector center() const;
    static bool compArea(const DkPolyRect &pl, const DkPolyRect &pr);
    nmc::DkRotatingRect toRotatingRect() const;

protected:
    std::vector<nmc::DkVector> mPts;
    double maxCosine;
    double area;

    void toDkVectors(const std::vector<cv::Point> &pts, std::vector<nmc::DkVector> &dkPts) const;
    void computeMaxCosine();
};

class PageExtractor
{
public:
    PageExtractor()
    {
    }

    void findPage(cv::Mat img, float scale, std::vector<DkPolyRect> &rects);

protected:
    const int maxLinesHough = 30;
    const float houghPeakThresholdRel = 0.3f; // minimum accumulator value of hough lines, relative to smaller image dimension
    const double t_theta = CV_PI / 9; // angle tolerance for parallel lines
    const float t_l = 0.5f;
    const float maxGapLengthRel = 0.3f; // maximum gap size in findLineSegments, relative to smaller image dimension
    const int minLineSegmentLength = 10;
    const float minRelSideLength = 0.3f; // minimum length of final rectangle sides relative to smaller image dimension
    const double orthoTol = CV_PI / 9; // orthogonality tolerance
    const float cornerGapTol = 3.0f; // tolerance for line segments that almost form a corner
    const int numFinalRects = 3; // number of rectangles to return

    struct HoughLine {
        int acc;
        float rho;
        float angle;
    };

    struct LineSegment {
        cv::Point2f p1;
        cv::Point2f p2;
        float length;
    };

    struct ExtendedPeak {
        ExtendedPeak(const HoughLine &line1, const LineSegment &ls1, const HoughLine &line2, const LineSegment &ls2);
        //~ExtendedPeak() {}

        HoughLine line1;
        HoughLine line2;
        std::vector<LineSegment> spatialLines;
        std::pair<bool, cv::Point2f> intersectionPoint;
        double theta_k;
        double A_k;
    };

    struct IntermediatePeak {
        IntermediatePeak(const ExtendedPeak &ep1, const ExtendedPeak &ep2)
            : ep1(ep1)
            , ep2(ep2)
        {
        }

        ExtendedPeak ep1;
        ExtendedPeak ep2;
    };

    struct Rectangle {
        Rectangle(const IntermediatePeak &ip, const std::vector<cv::Point2f> &corners)
            : ip(ip)
            , corners(corners)
        {
        }

        IntermediatePeak ip;
        std::vector<cv::Point2f> corners;
    };

    enum class LineFindingMode {
        Horizontal,
        Vertical
    };

    static double angleDiff(double a, double b);
    static std::pair<bool, cv::Point2f> findLineIntersection(const LineSegment &ls1, const LineSegment &ls2);
    static float pointToLineDistance(LineSegment ls, cv::Point2f p);
    static cv::Mat removeText(cv::Mat gray, float sigma, int selemSize, int threshold = 2);
    std::vector<HoughLine> houghTransform(cv::Mat bwImg, float rho, float theta, int threshold, int linesMax) const;
    std::vector<LineSegment> findLineSegments(cv::Mat bwImg, const std::vector<HoughLine> &houghLines, int minLength, int maxGap) const;
};

};
