/*******************************************************************************************************
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2016 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2016 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2016 Florian Kleber <florian@nomacs.org>

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

 related links:
 [1] http://www.nomacs.org/
 [2] https://github.com/nomacs/
 [3] http://download.nomacs.org
 *******************************************************************************************************/

#include "DkManipulatorsIpl.h"

#include "DkImageStorage.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QSharedPointer>
#include <QDebug>
#pragma warning(pop)

namespace nmc {

// DkGrayScaleManipulator --------------------------------------------------------------------
DkGrayScaleManipulator::DkGrayScaleManipulator(QAction * action) : DkBaseManipulator(action) {
}

QImage DkGrayScaleManipulator::apply(const QImage& img) const {

	if (img.isNull())
		return img;

	QImage imgR;

#ifdef WITH_OPENCV

	cv::Mat cvImg = DkImage::qImage2Mat(img);
	cv::cvtColor(cvImg, cvImg, CV_RGB2Lab);

	std::vector<cv::Mat> imgs;
	cv::split(cvImg, imgs);

	// get the luminance channel
	if (!imgs.empty())
		cvImg = imgs[0];

	// convert it back for the painter
	cv::cvtColor(cvImg, cvImg, CV_GRAY2RGB);

	imgR = DkImage::mat2QImage(cvImg);
#else

	QVector<QRgb> table(256);
	for(int i=0;i<256;++i)
		table[i]=qRgb(i,i,i);

	imgR = img.convertToFormat(QImage::Format_Indexed8,table);
#endif

	return imgR;
}

QString DkGrayScaleManipulator::errorMessage() const {
	return QObject::tr("Could not convert to grayscale");
}

}