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
#include <QImage>
#pragma warning(pop)

#pragma warning(disable: 4251)	// TODO: remove
//#include "DkImageStorage.h"

#ifndef Q_OS_WIN
#include "qpsdhandler.h"
#endif

// opencv
#ifdef WITH_OPENCV

#ifdef Q_OS_WIN
#pragma warning(disable: 4996)
#endif

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#endif

#ifndef DllLoaderExport
#ifdef DK_LOADER_DLL_EXPORT
#define DllLoaderExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllLoaderExport Q_DECL_IMPORT
#else
#define DllLoaderExport Q_DECL_IMPORT
#endif
#endif

// Qt defines
class QNetworkReply;

namespace nmc {

class DkMetaDataT;

#ifdef WITH_QUAZIP
class DllLoaderExport DkZipContainer {

public:
	DkZipContainer(const QString& fileName);

	bool isZip() const;
	QString getZipFilePath() const;
	QString getImageFileName() const;
	QString getEncodedFilePath() const;
	static QString zipMarker();
	static QSharedPointer<QByteArray> extractImage(const QString& zipFile, const QString& imageFile);
	static void extractImage(const QString& zipFile, const QString& imageFile, QByteArray& ba);
	static QString decodeZipFile(const QString& encodedFileInfo);
	static QString decodeImageFile(const QString& encodedFileInfo);
	static QString encodeZipFile(const QString& zipFile, const QString& imageFile);

protected:
	QString mEncodedFilePath;
	QString mZipFilePath;
	QString mImageFileName;
	bool mImageInZip;
	static QString mZipMarker;
};
#endif

class DllLoaderExport DkEditImage {

public:
	DkEditImage(const QImage& img = QImage(), const QString& editName = "");

	void setImage(const QImage& img);
	QImage image() const;
	QString editName() const;
	int size() const;

protected:
	QImage mImg;
	QString mEditName;

};

/**
 * This class provides image loading and editing capabilities.
 * It additionally stores the currently loaded image.
 **/ 
class DllLoaderExport DkBasicLoader : public QObject {
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
	bool loadGeneral(const QString& filePath, bool loadMetaData = false, bool fast = false);

	/**
	 * Loads the image for the given file
	 * @param file an image file
	 * @param skipIdx the number of (internal) pages to be skipped
	 * @return bool true if the image was loaded
	 **/
	bool loadGeneral(const QString& filePath, const QSharedPointer<QByteArray> ba, bool loadMetaData = false, bool fast = false);

	/**
	 * Loads the page requested (with respect to the current page)
	 * @param skipIdx number of pages to skip
	 * @return bool true if we could load the page requested
	 **/
	bool loadPage(int skipIdx = 0);
	bool loadPageAt(int pageIdx = 0);

	int getNumPages() const {
		return mNumPages;
	};

	int getPageIdx() const {
		return mPageIdx;
	};

	bool setPageIdx(int skipIdx);
	void resetPageIdx();

	QString save(const QString& filePath, const QImage& img, int compression = -1);
	bool saveToBuffer(const QString& filePath, const QImage& img, QSharedPointer<QByteArray>& ba, int compression = -1);
	void saveThumbToMetaData(const QString& filePath, QSharedPointer<QByteArray>& ba);
	void saveMetaData(const QString& filePath, QSharedPointer<QByteArray>& ba);
	void saveThumbToMetaData(const QString& filePath);
	void saveMetaData(const QString& filePath);

	static bool isContainer(const QString& filePath);

	/**
	 * Sets a new image (if edited outside the basicLoader class)
	 * @param img the new image
	 * @param file assigns the current file name
	 **/
	void setImage(const QImage& img, const QString& editName, const QString& file);
	void setEditImage(const QImage& img, const QString& editName = "");

	void setTraining(bool training) {
		training = true;
	};

	bool getTraining() {
		return mTraining;
	};

	int getLoader() {
		return mLoader;
	};

	QSharedPointer<DkMetaDataT> getMetaData() const {
		return mMetaData;
	};

	/**
	 * Returns the 8-bit image, which is rendered.
	 * @return QImage an 8bit image
	 **/
	QImage image() const;

	QString getFile() {
		return mFile;
	};

	bool isDirty() {
		return mPageIdxDirty;
	};

	/**
	 * Returns the current image size.
	 * @return QSize the image size.
	 **/
	QSize size() {
		return image().size();
	};

	/**
	 * Returns true if an image is currently loaded.
	 * @return bool true if an image is loaded.
	 **/
	bool hasImage() {
		return !image().isNull();
	};

	void undo();
	void redo();
	QVector<DkEditImage>* history();
	void setHistoryIndex(int idx);
	int historyIndex() const;

	void loadFileToBuffer(const QString& filePath, QByteArray& ba) const;
	QSharedPointer<QByteArray> loadFileToBuffer(const QString& filePath) const;
	bool writeBufferToFile(const QString& fileInfo, const QSharedPointer<QByteArray> ba) const;

	void release(bool clear = false);


#ifdef WITH_OPENCV
	cv::Mat getImageCv();
	bool loadOpenCVVecFile(const QString& filePath, QImage& img, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>(), QSize s = QSize()) const;
	cv::Mat getPatch(const unsigned char** dataPtr, QSize patchSize) const;
	int mergeVecFiles(const QStringList& vecFilePaths, QString& saveFileInfo) const;
	bool readHeader(const unsigned char** dataPtr, int& fileCount, int& vecSize) const;
	void getPatchSizeFromFileName(const QString& fileName, int& width, int& height) const;
#else
	bool loadOpenCVVecFile(const QString&, QImage&, QSharedPointer<QByteArray> = QSharedPointer<QByteArray>(), QSize = QSize()) { return false; };
	int mergeVecFiles(const QStringList&, QString&) const { return 0; };
	bool readHeader(const unsigned char**, int&, int&) const { return false; };
	void getPatchSizeFromFileName(const QString&, int&, int&) const {};

#endif

	bool loadPSDFile(const QString& filePath, QImage& img, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>()) const;

#ifdef Q_OS_WIN
	bool saveWindowsIcon(const QString& filePath, const QImage& img) const;
	bool saveWindowsIcon(const QImage& img, QSharedPointer<QByteArray>& ba) const;
#else
	bool saveWindowsIcon(const QString& filePath, const QImage& img) const { return false; };
	bool saveWindowsIcon(const QImage& img, QSharedPointer<QByteArray>& ba) const { return false; };
#endif

signals:
	void errorDialogSignal(const QString& msg);

public slots:
	QImage rotate(const QImage& img, int orientation);

protected:
	bool loadRohFile(const QString& filePath, QImage& img, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>()) const;
	bool loadRawFile(const QString& filePath, QImage& img, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>(), bool fast = false) const;
	void indexPages(const QString& filePath);
	void convert32BitOrder(void *buffer, int width);

	int mLoader;
	bool mTraining;
	int mMode;
	
	QString mFile;
	int mNumPages;
	int mPageIdx;
	bool mPageIdxDirty;
	QSharedPointer<DkMetaDataT> mMetaData;
	QVector<DkEditImage> mImages;
	int mImageIndex = 0;
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

	QNetworkAccessManager mWebCtrl;
	QSharedPointer<QByteArray> mDownloadedData;
	QUrl mUrl;
};

};
