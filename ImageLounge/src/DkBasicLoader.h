/*******************************************************************************************************
 DkBasicLoader.h
 Created on:	21.02.2014
 
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

#pragma once

#include <QObject.h>
#include <QFileInfo.h>
#include <QImage.h>
#include <QImageReader.h>
#include <QImageWriter.h>

#include "DkImageStorage.h"
#include "DkSettings.h"
#include "DkError.h"
#include "DkTimer.h"
#include "../libqpsd/qpsdhandler.h"

#ifdef WITH_WEBP
#include "webp/decode.h"
#include "webp/encode.h"
#endif

// opencv
#ifdef WITH_OPENCV

#ifdef WIN32
#pragma warning(disable: 4996)
#endif

#include <libraw/libraw.h>
#ifdef DISABLE_LANCZOS // opencv 2.1.0 is used, does not have opencv2 includes
#include "opencv/cv.h"
#else
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#endif

#ifdef WITH_LIBTIFF
#ifdef WIN32
#include "tif_config.h"	
#endif

#ifdef Q_WS_MAC
#define uint64 uint64_hack_
#define int64 int64_hack_
#endif // Q_WS_MAC

#include "tiffio.h"

#ifdef Q_WS_MAC
#undef uint64
#undef int64
#endif // Q_WS_MAC
#endif

#endif

namespace nmc {

/**
 * This class provides image loading and editing capabilities.
 * It additionally stores the currently loaded image.
 **/ 
class DkBasicLoader : public QObject {
	Q_OBJECT

public:

	enum mode {
		mode_default,
		mode_mat_preferred,
		mode_end
	};

	enum loaderID {
		no_loader = 0,
		qt_loader,
		psd_loader,
		webp_loader,
		raw_loader,
		roh_loader,
		hdr_loader,
	};

	DkBasicLoader(int mode = mode_default);

	~DkBasicLoader() {
		release();
	};

	/**
	 * Loads the image for the given file
	 * @param file an image file
	 * @param skipIdx the number of (internal) pages to be skipped
	 * @return bool true if the image was loaded
	 **/ 
	bool loadGeneral(const QFileInfo& file, bool loadMetaData = false, bool fast = false);
	bool loadGeneral(const QFileInfo& file, const QByteArray& ba, bool loadMetaData = false, bool fast = false);

	/**
	 * Loads the page requested (with respect to the current page)
	 * @param skipIdx number of pages to skip
	 * @return bool true if we could load the page requested
	 **/ 
	bool loadPage(int skipIdx = 0);

	int getNumPages() const {
		return numPages;
	};

	int getPageIdx() const {
		return pageIdx;
	};

	bool setPageIdx(int skipIdx);

	QFileInfo save(const QFileInfo& fileInfo, const QImage& img, int compression = -1);
	bool save(const QFileInfo& fileInfo, const QImage& img, QByteArray& ba, int compression = -1);

	/**
	 * Sets a new image (if edited outside the basicLoader class)
	 * @param img the new image
	 * @param file assigns the current file name
	 **/ 
	void setImage(QImage img, QFileInfo file) {

		this->file = file;
		qImg = img;
	};

	void setTraining(bool training) {
		training = true;
	};

	bool getTraining() {
		return training;
	};

	int getLoader() {
		return loader;
	};

	DkMetaDataT getMetaData() {
		return metaData;
	};

	/**
	 * Returns the 8-bit image, which is rendered.
	 * @return QImage an 8bit image
	 **/ 
	QImage image() {
		return qImg;
	};

	QFileInfo getFile() {
		return file;
	};

	bool isDirty() {
		return pageIdxDirty;
	};

	/**
	 * Returns the current image size.
	 * @return QSize the image size.
	 **/ 
	QSize size() {
		return qImg.size();
	};

	/**
	 * Returns true if an image is currently loaded.
	 * @return bool true if an image is loaded.
	 **/ 
	bool hasImage() {
		return !qImg.isNull();
	};

	void loadFileToBuffer(const QFileInfo& fileInfo, QByteArray& ba);

	void release();

#ifdef WITH_OPENCV
	Mat getImageCv() { return cv::Mat(); };	// we should not need this
#endif

	bool loadPSDFile(const QFileInfo& fileInfo);
	bool loadPSDFile(const QByteArray& ba);
#ifdef WITH_WEBP
	bool loadWebPFile(const QFileInfo& fileInfo);
	bool loadWebPFile(const QByteArray& ba);
	bool saveWebPFile(const QFileInfo& fileInfo, QImage img, int compression);
	bool saveWebPFile(QImage img, QByteArray& ba, int compression, int speed = 4);
#else
	bool loadWebPFile(const QFileInfo& fileInfo) {return false;};	// not supported if webP was not linked
	bool loadWebPFile(const QByteArray& ba) {return false;};
	bool saveWebPFile(const QFileInfo& fileInfo, QImage img, int compression) {return false;};
	bool saveWebPFile(QImage img, QByteArray& ba, int compression, int speed = 4) {return false;};
#endif

signals:
	void errorDialogSignal(const QString& msg);

public slots:
	void rotate(int orientation);
	void resize(QSize size, float factor = 1.0f, QImage* img = 0, int interpolation = DkImage::ipl_cubic, bool silent = false);

protected:
	bool loadRohFile(const QFileInfo& fileInfo);
	bool loadRohFile(const QByteArray& ba);
	bool loadRawFile(const QFileInfo& fileInfo, bool fast = false);
	bool loadRawFile(const QByteArray& ba, bool fast = false);
	void indexPages(const QFileInfo& fileInfo);
	void convert32BitOrder(void *buffer, int width);

	int loader;
	bool training;
	int mode;
	QImage qImg;
	QFileInfo file;
	int numPages;
	int pageIdx;
	bool pageIdxDirty;
	DkMetaDataT metaData;

#ifdef WITH_OPENCV
	cv::Mat cvImg;
#endif
};

};
