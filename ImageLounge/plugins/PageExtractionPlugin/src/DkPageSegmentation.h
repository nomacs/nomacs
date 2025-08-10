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

#include "DkPageSegmentationUtils.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include <QColor>
#include <QImage>

namespace nmp
{

class DkPageSegmentation
{
public:
    explicit DkPageSegmentation(const cv::Mat &colImg = cv::Mat(), bool alternativeMethod = false);
    virtual ~DkPageSegmentation() = default;

    virtual void compute();
    virtual void filterDuplicates(float overlap = 0.6f, float areaRatio = 0.5f);
    virtual void filterDuplicates(std::vector<DkPolyRect> &rects, float overlap = 0.6f, float areaRatio = 0.1f) const;

    virtual std::vector<DkPolyRect> getRects() const
    {
        return mRects;
    }

    virtual cv::Mat getDebugImg() const;
    virtual QImage getCropped(const QImage &img) const;
    virtual void draw(cv::Mat &img, const cv::Scalar &col = cv::Scalar(255, 222, 0)) const;
    virtual void draw(QImage &img, const QColor &col = QColor(255, 222, 0)) const;
    virtual void draw(cv::Mat &img,
                      const std::vector<DkPolyRect> &rects,
                      const cv::Scalar &col = cv::Scalar(255, 222, 0)) const;
    DkPolyRect getMaxRect() const;

protected:
    cv::Mat mImg;
    cv::Mat mDbgImg;

    static constexpr int kThresh = 80;
    static constexpr int kNumThresh = 10;
    static constexpr double kMinArea = 12000;
    static constexpr double kMaxArea = 0;
    static constexpr float kMaxSide = 0;
    static constexpr float kMaxSideFactor = 0.97f;

    bool mLooseDetection = false;
    float mScale = 1.0f;
    bool mAlternativeMethod = false;
    std::vector<DkPolyRect> mRects;

    virtual cv::Mat findRectangles(const cv::Mat &img, std::vector<DkPolyRect> &squares) const;
    virtual cv::Mat findRectanglesAlternative(const cv::Mat &img, std::vector<DkPolyRect> &squares) const;
    QImage cropToRect(const QImage &img, const nmc::DkRotatingRect &rect, const QColor &bgCol = QColor(0, 0, 0)) const;
    void drawRects(QPainter *p, const std::vector<DkPolyRect> &rects, const QColor &col = QColor(100, 100, 100)) const;
};

};
