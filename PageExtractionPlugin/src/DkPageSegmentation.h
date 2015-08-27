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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <opencv2/core/core.hpp>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

class DkPolyRect;

class DkBurgerSegmentation {

public:
	DkBurgerSegmentation(const cv::Mat& colImg = cv::Mat());

	virtual void compute();
	virtual void filterDuplicates(float overlap = 0.6f, float areaRatio = 0.1f);
	virtual void filterDuplicates(std::vector<DkPolyRect>& rects, float overlap = 0.6f, float areaRatio = 0.1f) const;

	virtual std::vector<DkPolyRect> getRects() const { return rects; };
	virtual cv::Mat getDebugImg() const;
	virtual void draw(cv::Mat& img, const cv::Scalar& col = cv::Scalar(0, 55, 100)) const;
	virtual void draw(cv::Mat& img, const std::vector<DkPolyRect>& rects, const cv::Scalar& col = cv::Scalar(0, 55, 100)) const;

	bool looseDetection;

protected:
	cv::Mat img;
	cv::Mat dbgImg;
	int thresh;
	int numThresh;
	double minArea;
	double maxArea;
	float maxSide;
	float maxSideFactor;
	float scale;

	std::vector<DkPolyRect> rects;

	virtual cv::Mat findRectangles(const cv::Mat& img, std::vector<DkPolyRect>& squares) const;
};


};
