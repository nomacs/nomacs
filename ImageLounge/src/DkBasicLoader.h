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

#pragma warning(push, 0)    
#include <QNetworkAccessManager>
#include <QSharedPointer>
#include <QUrl>
#include <QFileInfo>
#include <QImage>
#pragma warning(pop)

//#include "DkImageStorage.h"

#ifndef WIN32
#include "qpsdhandler.h"
#endif

// opencv
#ifdef WITH_OPENCV

#ifdef WIN32
#pragma warning(disable: 4996)
#endif

#ifdef DISABLE_LANCZOS // opencv 2.1.0 is used, does not have opencv2 includes
#include "opencv/cv.h"
#else
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#endif
#endif

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport
#endif
#endif

// Qt defines
class QNetworkReply;

namespace nmc {

// results in compiler warning C4150
// as far as I understand, the warning can be ignored because
// the destructor of DkMetaDataT is trivial - could someone check for this so we can suppress it?
class DkMetaDataT;

#ifdef WITH_QUAZIP
class DllExport DkZipContainer {

public:
	DkZipContainer(const QFileInfo& fileInfo);

	bool isZip();
	QFileInfo getZipFileInfo();
	QFileInfo getImageFileInfo();
	QFileInfo getEncodedFileInfo();
	static QString zipMarker();
	static QSharedPointer<QByteArray> extractImage(QFileInfo zipFile, QFileInfo imageFile);
	static void extractImage(QFileInfo zipFile, QFileInfo imageFile, QByteArray& ba);
	static QFileInfo decodeZipFile(const QFileInfo& encodedFileInfo);
	static QFileInfo decodeImageFile(const QFileInfo& encodedFileInfo);
	static QFileInfo encodeZipFile(const QFileInfo& zipFile, const QString& imageFile);

protected:
	QFileInfo encodedFileInfo;
	QFileInfo zipFileInfo;
	QFileInfo imageFileInfo;
	bool imageInZip;
	static QString mZipMarker;

};
#endif

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
		release(true);
	};

	/**
	 * Convenience function.
	 **/ 
	bool loadGeneral(const QFileInfo& file, bool loadMetaData = false, bool fast = false);

	/**
	 * Loads the image for the given file
	 * @param file an image file
	 * @param skipIdx the number of (internal) pages to be skipped
	 * @return bool true if the image was loaded
	 **/ 
	bool loadGeneral(const QFileInfo& file, const QSharedPointer<QByteArray> ba, bool loadMetaData = false, bool fast = false);

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
	void resetPageIdx();

	QFileInfo save(const QFileInfo& fileInfo, const QImage& img, int compression = -1);
	bool saveToBuffer(const QFileInfo& fileInfo, const QImage& img, QSharedPointer<QByteArray>& ba, int compression = -1);
	void saveThumbToMetaData(const QFileInfo& fileInfo, QSharedPointer<QByteArray>& ba);
	void saveMetaData(const QFileInfo& fileInfo, QSharedPointer<QByteArray>& ba);
	void saveThumbToMetaData(const QFileInfo& fileInfo);
	void saveMetaData(const QFileInfo& fileInfo);

	static bool isContainer(const QFileInfo& fileInfo);

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

	QSharedPointer<DkMetaDataT> getMetaData() const {
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

	void loadFileToBuffer(const QFileInfo& fileInfo, QByteArray& ba) const;
	QSharedPointer<QByteArray> loadFileToBuffer(const QFileInfo& fileInfo) const;
	bool writeBufferToFile(const QFileInfo& fileInfo, const QSharedPointer<QByteArray> ba) const;

	void release(bool clear = false);

#ifdef WITH_OPENCV
	cv::Mat getImageCv();
	bool loadOpenCVVecFile(const QFileInfo& fileInfo, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>(), QSize s = QSize());
	cv::Mat getPatch(const unsigned char** dataPtr, QSize patchSize) const;
	int mergeVecFiles(const QVector<QFileInfo>& vecFileInfos, QFileInfo& saveFileInfo) const;
	bool readHeader(const unsigned char** dataPtr, int& fileCount, int& vecSize) const;
	void getPatchSizeFromFileName(const QString& fileName, int& width, int& height) const;
#else
	bool loadOpenCVVecFile(const QFileInfo&, QSharedPointer<QByteArray> = QSharedPointer<QByteArray>(), QSize = QSize()) {return false;};
	int mergeVecFiles(const QVector<QFileInfo>&, QFileInfo&) const {return 0;};
	bool readHeader(const unsigned char**, int&, int&) const {return false;};
	void getPatchSizeFromFileName(const QString&, int&, int&) const {};

#endif

	bool loadPSDFile(const QFileInfo& fileInfo, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>());
#ifdef WITH_WEBP
	bool loadWebPFile(const QFileInfo& fileInfo, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>());
	bool saveWebPFile(const QFileInfo& fileInfo, const QImage img, int compression);
	bool saveWebPFile(const QImage img, QSharedPointer<QByteArray>& ba, int compression, int speed = 4);
#else
	bool loadWebPFile(const QFileInfo&, QSharedPointer<QByteArray> = QSharedPointer<QByteArray>()) {return false;};	// not supported if webP was not linked
	bool saveWebPFile(const QFileInfo&, const QImage, int) {return false;};
	bool saveWebPFile(const QImage, QSharedPointer<QByteArray>&, int, int = 4) {return false;};
#endif

signals:
	void errorDialogSignal(const QString& msg);

public slots:
	void rotate(int orientation);

protected:
	bool loadRohFile(const QFileInfo& fileInfo, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>());
	bool loadRawFile(const QFileInfo& fileInfo, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>(), bool fast = false);
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
	QSharedPointer<DkMetaDataT> metaData;

#ifdef WITH_OPENCV
	cv::Mat cvImg;
#endif
};

// file downloader from: http://qt-project.org/wiki/Download_Data_from_URL
class FileDownloader : public QObject {
	Q_OBJECT

public:
	explicit FileDownloader(QUrl imageUrl, QObject *parent = 0);

	virtual ~FileDownloader();

	QSharedPointer<QByteArray> downloadedData() const;
	QUrl getUrl() const;
	void downloadFile(const QUrl& url);

signals:
	void downloaded();

private slots:
	void fileDownloaded(QNetworkReply* pReply);

private:

	QNetworkAccessManager m_WebCtrl;
	QSharedPointer<QByteArray> m_DownloadedData;
	QUrl url;
};

};
