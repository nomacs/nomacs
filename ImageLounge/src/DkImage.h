/*******************************************************************************************************
 DkImage.h
 Created on:	21.04.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2012 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2012 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2012 Florian Kleber <florian@nomacs.org>

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

#ifdef WIN32		// why is Q_WS_WIN32 not defined here?
#include "shlwapi.h"
//#pragma comment (lib, "shlwapi.lib")
#endif

#include <QtGui/QWidget>
#include <QImageWriter>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QFile>
#include <QSettings>
#include <QImageReader>
#include <QDir>
#include <QThread>
#include <QBuffer>
#include <QStringBuilder>
#include <QDebug>
#include <QMutex>
#include <QFileIconProvider>
#include <QStringList>
#include <QMessageBox>

#ifdef HAVE_EXIV2_HPP
#include <exiv2/exiv2.hpp>
#else
#include <exiv2/image.hpp>
#include <iomanip>
#endif

// opencv
#ifdef WITH_OPENCV

#ifdef Q_WS_WIN
#pragma warning(disable: 4996)
#endif

#include <libraw/libraw.h>
#include <opencv/cv.h>
using namespace cv;
#endif

#include <set>

#ifdef DK_DLL
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif

// TODO: ifdef
//#include <ShObjIdl.h>
//#include <ShlObj.h>
//#include <Windows.h>

// my classes
//#include "DkNoMacs.h"
#include "DkTimer.h"
#include "DkSettings.h"

#ifdef Q_WS_X11
	typedef  unsigned char byte;
#endif

namespace nmc {

#ifdef WIN32
	
	/**
	 * Logical string compare function.
	 * This function is used to sort:
	 * a1.png
	 * a2.png
	 * a10.png
	 * instead of:
	 * a1.png
	 * a10.png
	 * a2.png
	 * @param lhs left string
	 * @param rhs right string
	 * @return bool true if left string < right string
	 **/ 
	bool wCompLogic(const std::wstring & lhs, const std::wstring & rhs);
#endif

// basic image processing

/**
 * DkImage holds some basic image processing
 * methods that are generally needed.
 **/ 
class DkImage {

public:

	/**< interpolation mapping OpenCV -> Qt */
	enum{ipl_nearest, ipl_area, ipl_linear, ipl_cubic, ipl_lanczos, ipl_end};

#ifdef WITH_OPENCV
	
	/**
	 * Converts a QImage to a Mat
	 * @param img formats supported: ARGB32 | RGB32 | RGB888 | Indexed8
	 * @return cv::Mat the corresponding Mat
	 **/ 
	static Mat qImage2Mat(QImage img) {

		Mat mat2;
		QImage cImg;
		if (img.format() == QImage::Format_ARGB32 || img.format() == QImage::Format_RGB32 ) {
			mat2 = Mat(img.height(), img.width(), CV_8UC4, (uchar*)img.bits(), img.bytesPerLine());
			qDebug() << "ARGB32 or RGB32";
		}
		else if (img.format() == QImage::Format_RGB888) {
			mat2 = Mat(img.height(), img.width(), CV_8UC3, (uchar*)img.bits(), img.bytesPerLine());
			qDebug() << "RGB888";
		}
		else if (img.format() == QImage::Format_Indexed8) {
			mat2 = Mat(img.height(), img.width(), CV_8UC1, (uchar*)img.bits(), img.bytesPerLine());
			qDebug() << "indexed...";
		}
		else {
			qDebug() << "image flag: " << img.format();
			cImg = img.convertToFormat(QImage::Format_RGB888);
			mat2 = Mat(cImg.height(), cImg.width(), CV_8UC3, (uchar*)cImg.bits(), cImg.bytesPerLine());
			qDebug() << "I need to convert the QImage to RGB888";
		}

		mat2 = mat2.clone();	// we need to own the pointer

		return mat2; 
	}

	/**
	 * Converts a cv::Mat to a QImage.
	 * @param img supported formats CV8UC1 | CV_8UC3 | CV_8UC4
	 * @return QImage the corresponding QImage
	 **/ 
	static QImage mat2QImage(Mat img) {

		QImage qImg;

		// since Mat header is copied, a new buffer should be allocated (check this!)
		if (img.depth() == CV_32F)
			img.convertTo(img, CV_8U, 255);

		if (img.type() == CV_8UC1) {
			qImg = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_Indexed8);
			//Mat tmp;
			//cvtColor(img, tmp, CV_GRAY2RGB);	// Qt does not support writing to index8 images
			//img = tmp;
		}
		if (img.type() == CV_8UC3) {
			
			cv::cvtColor(img, img, CV_RGB2BGR);
			qImg = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888);
		}
		if (img.type() == CV_8UC4) {
			qImg = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_ARGB32);
		}

		qImg = qImg.copy();

		return qImg;
	}
#endif

	/**
	 * Returns a string with the buffer size of an image.
	 * @param img a QImage
	 * @return QString a human readable string containing the buffer size
	 **/ 
	static QString getBufferSize(const QImage& img) {

		return getBufferSize(img.size(), img.depth());
	}

	/**
	 * Returns a string with the buffer size of an image.
	 * @param imgSize the image size
	 * @param depth the image depth
	 * @return QString a human readable string containing the buffer size
	 **/ 
	static QString getBufferSize(const QSize imgSize, const int depth) {

		double size = (double)imgSize.width() * (double)imgSize.height() * (double)(depth/8.0f);
		QString sizeStr;
		qDebug() << "dimension: " << size;

		if (size >= 1024*1024*1024) {
			return QString::number(size/(1024.0f*1024.0f*1024.0f), 'f', 2) + " GB";
		}
		else if (size >= 1024*1024) {
			return QString::number(size/(1024.0f*1024.0f), 'f', 2) + " MB";
		}
		else if (size >= 1024) {
			return QString::number(size/1024.0f, 'f', 2) + " KB";
		}
		else {
			return QString::number(size, 'f', 2) + " B";
		}
	}

	/**
	 * Returns a the buffer size of an image.
	 * @param imgSize the image size
	 * @param depth the image depth
	 * @return buffer size in MB
	 **/ 
	static float getBufferSizeFloat(const QSize imgSize, const int depth) {

		double size = (double)imgSize.width() * (double)imgSize.height() * (double)(depth/8.0f);
		QString sizeStr;
		//qDebug() << "dimension: " << size;

		return size/(1024.0f*1024.0f);
	}


	/**
	 * This function resizes an image according to the interpolation method specified.
	 * @param img the image to resize
	 * @param newSize the new size
	 * @param factor the resize factor
	 * @param interpolation the interpolation method
	 * @return QImage the resized image
	 **/ 
	static QImage resizeImage(const QImage &img, const QSize &newSize, float factor = 1.0f, int interpolation = ipl_cubic) {
		
		QSize nSize = newSize;

		// nothing to do
		if (img.size() == nSize && factor == 1.0f)
			return img;

		if (factor != 1.0f)
			nSize = QSize(img.width()*factor, img.height()*factor);

		if (nSize.width() < 1 || nSize.height() < 1) {
			return QImage();
		}

		Qt::TransformationMode iplQt;
		switch(interpolation) {
		case ipl_nearest:	
		case ipl_area:		iplQt = Qt::FastTransformation; break;
		case ipl_linear:	
		case ipl_cubic:		
		case ipl_lanczos:	iplQt = Qt::SmoothTransformation; break;
		}
#ifdef WITH_OPENCV

		int ipl = CV_INTER_CUBIC;
		switch(interpolation) {
		case ipl_nearest:	ipl = CV_INTER_NN; break;
		case ipl_area:		ipl = CV_INTER_AREA; break;
		case ipl_linear:	ipl = CV_INTER_LINEAR; break;
		case ipl_cubic:		ipl = CV_INTER_CUBIC; break;
#ifdef DISABLE_LANCZOS
		case ipl_lanczos:	ipl = CV_INTER_CUBIC; break;
#else
		case ipl_lanczos:	ipl = CV_INTER_LANCZOS4; break;
#endif
		}


		try {
			Mat resizeImage = DkImage::qImage2Mat(img);

			// is the image convertible?
			if (resizeImage.empty()) {
				return img.scaled(newSize, Qt::IgnoreAspectRatio, iplQt);
			}
			else {

				Mat tmp;
				cv::resize(resizeImage, tmp, cv::Size(nSize.width(), nSize.height()), 0, 0, ipl);
				resizeImage = tmp;
				return DkImage::mat2QImage(resizeImage);
			}

		}catch (std::exception se) {

			return QImage();
		}

#else

		return img.scaled(nSize, Qt::IgnoreAspectRatio, iplQt);

#endif
	}

};


class DllExport DkMetaData {

public:
	DkMetaData(QFileInfo file = QFileInfo()) {
		this->file = file;
		mdata = false;
	};
	
	DkMetaData(const DkMetaData& metaData);

	void setFileName(QFileInfo file) {
		this->file = file;
		mdata = false;
	};
	~DkMetaData() {};

	DkMetaData& operator=(const DkMetaData& metadata) {

		if (this == &metadata)
			return *this;

		this->file = metadata.file;
		this->mdata = false;
		
		return *this;
	};

	void reloadImg();

	void saveMetaDataToFile(QFileInfo fileN = QFileInfo(), int orientation = 0);

	std::string getNativeExifValue(std::string key);
	std::string getExifValue(std::string key);
	bool setExifValue(std::string key, std::string taginfo);
	std::string getIptcValue(std::string key);
	int getOrientation();
	QImage getThumbnail();
	void saveThumbnail(QImage thumb);
	void saveOrientation(int o);
	int getHorizontalFlipped();
	void saveHorizontalFlipped(int f);
	float getRating();
	void setRating(int r);
	bool isTiff();
	bool isJpg();
	bool isRaw();
	void printMetaData(); //only for debug
	QStringList getExifKeys();
	QStringList getExifValues();
	QStringList getIptcKeys();
	QStringList getIptcValues();


private:

	void readMetaData();

	Exiv2::Image::AutoPtr exifImg;
	QFileInfo file;

	bool mdata;

};

/**
 * This class holds thumbnails.
 **/ 
class DkThumbNail {

public:
	enum {
		exists_not = -1,
		not_loaded,
		loaded
	};
	
	/**
	 * Default constructor.
	 * @param file the corresponding file
	 * @param img the thumbnail image
	 **/ 
	DkThumbNail(QFileInfo file = QFileInfo(), QImage img = QImage()) {
		this->img = img;
		this->file = file;
		imgExists = true;
		s = qMax(img.width(), img.height());
	};

	/**
	 * Default destructor.
	 * @return 
	 **/ 
	~DkThumbNail() {};

	/**
	 * Sets the thumbnail image.
	 * @param img the thumbnail
	 **/ 
	void setImage(QImage img) {
		this->img = img;
	}

	/**
	 * Returns the thumbnail.
	 * @return QImage the thumbnail.
	 **/ 
	QImage getImage() {
		return img;
	};

	/**
	 * Returns the file information.
	 * @return QFileInfo the thumbnail file
	 **/ 
	QFileInfo getFile() {
		return file;
	};

	/**
	 * Returns whether the thumbnail was loaded, or does not exist.
	 * @return int a status (loaded | not loaded | exists not)
	 **/ 
	int hasImage() {
		
		if (!img.isNull())
			return loaded;
		else if (img.isNull() && imgExists)
			return not_loaded;
		else
			return exists_not;
	};

	/**
	 * Manipulates the file loaded status.
	 * @param exists a status (loaded | not loaded | exists not)
	 **/ 
	void setImgExists(bool exists) {
		imgExists = exists;
	}

	/**
	 * Returns the thumbnail size.
	 * @return int the maximal side (either width or height)
	 **/ 
	int size() {
		return s;
	};

private:
	QImage img;
	QFileInfo file;
	int s;
	bool imgExists;

};

/**
 * This class provides a method for reading thumbnails.
 * If the a thumbnail is provided in the metadata,
 * it can be loaded very fast. Additionally,
 * the thumbnails are loaded in a separate thread (in the 
 * background)
 **/ 
class DkThumbsLoader : public QThread {

	Q_OBJECT

public:
	DkThumbsLoader(std::vector<DkThumbNail>* thumbs = 0, QDir dir = QDir());
	~DkThumbsLoader() {};

	void run();
	void stop();
	int getFileIdx(QFileInfo& file);

signals:
	void updateSignal();

public slots:
	void setLoadLimits(int start = 0, int end = 20);

private:
	std::vector<DkThumbNail>* thumbs;
	QDir dir;
	bool isActive;
	bool somethingTodo;
	QMutex mutex;
	int maxThumbSize;
	int loadLimit;
	int startIdx;
	int endIdx;

	// function
	QImage getThumbNailQt(QFileInfo file);
	//QImage getThumbNailWin(QFileInfo file);
	void init();
	void loadThumbs();

};

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

	DkBasicLoader(int mode = mode_default);

	bool loadGeneral(QFileInfo file);
	
	/**
	 * Sets a new image (if edited outside the basicLoader class)
	 * @param img the new image
	 * @param file assigns the current file name
	 **/ 
	void setImage(QImage& img, QFileInfo file) {

		this->file = file;
		qImg = img;
	}

	/**
	 * Returns the 8-bit image, which is rendered.
	 * @return QImage& an 8bit image
	 **/ 
	QImage image() {
	
		return qImg;
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

	void release();

#ifdef WITH_OPENCV
	Mat getImageCv() { return cv::Mat(); };	// we should not need this
#endif

public slots:
	void rotate(int orientation);
	void resize(QSize size, float factor = 1.0f, QImage* img = 0, int interpolation = DkImage::ipl_cubic, bool silent = false);

protected:
	
	bool loadRohFile(QString fileName);
	bool loadRawFile(QFileInfo file);

	int mode;
	QImage qImg;
	QFileInfo file;

#ifdef WITH_OPENCV
	cv::Mat cvImg;
#endif

};

class DkImageCache {

public:

	enum cache_state {cache_ignored = -1, cache_not_loaded = 0, cache_loaded, cache_end };

	DkImageCache(QFileInfo file = QFileInfo(), QImage img = QImage()) {
		this->file = file;
		this->img = img;
		cacheState = cache_not_loaded;
		cacheSize = 0.0f;
	};

	void setFileInfo(QFileInfo& file) {
		this->file = file;
	};

	void setImage(QImage& img) {
		cacheSize = DkImage::getBufferSizeFloat(img.size(), img.depth());
		this->img = img;
	};

	void ignore() {
		cacheState = cache_ignored;
	};

	QFileInfo getFile() {
		return file;
	};

	void clearImage() {
		img = QImage();
	};

	QImage getImage() {
		return img;
	};

	float getCacheSize() {
		return cacheSize;
	};

	int getCacheState() {
		
		if (!img.isNull())
			return cache_loaded;
		else
			return cacheState;
	};

protected:
	QFileInfo file;
	QImage img;
	int cacheState;
	float cacheSize;
};


/**
 * This class provides a method for caching images.
 **/ 
class DkCacher : public QThread {

	Q_OBJECT

public:
	DkCacher(std::vector<DkImageCache>* cache = 0, QDir dir = QDir(), QStringList files = QStringList());
	~DkCacher() {};

	void run();
	void stop();
	void pause();
	void play();
	void start();

	void setCurrentFile(QFileInfo& file, QImage img = QImage());
	void setNewDir(QDir& dir, QStringList& files);
	void updateDir(QStringList& files);

private:
	std::vector<DkImageCache>* cache;
	int curFileIdx;

	float maxFileSize;
	float curCache;

	int maxNumFiles;

	QDir dir;
	bool isActive;
	bool somethingTodo;
	bool newDir;
	bool updateFiles;

	DkBasicLoader loader;
	QStringList files;

	QMutex mutex;

	void index();
	void load();
	bool cacheImage(DkImageCache* cacheImg);
	bool clean(int curCacheIdx);
};



/**
 * This class is a basic image loader class.
 * It takes care of the file watches for the current folder,
 * holds the currently displayed image,
 * calls the load routines
 * and saves the image or the image metadata.
 **/ 
class DllExport DkImageLoader : public QObject {

	Q_OBJECT

public:
	DkImageLoader(QFileInfo file = QFileInfo());

	virtual ~DkImageLoader();

	static QString saveFilter;		// for system close dialog
	static QString openFilter;		// for system  open dialog
	static QStringList fileFilters;	// just the filters
	static QStringList openFilters;	// for open dialog
	static QStringList saveFilters;	// for close dialog
	QStringList ignoreKeywords;
	QStringList keywords;

	static bool isValid(QFileInfo& fileInfo);
	//static int locateFile(QFileInfo& fileInfo, QDir* dir = 0);
	static QStringList getFilteredFileList(QDir dir, QStringList ignoreKeywords = QStringList(), QStringList keywords = QStringList());

	static DkMetaData imgMetaData;	// static class so that the metadata is only loaded once (performance)

	bool silent;

	void rotateImage(double angle);
	void saveFile(QFileInfo filename, QString fileFilter = "", QImage saveImg = QImage(), int compression = -1);
	void setFile(QFileInfo& filename);
	QFileInfo getFile();
	QStringList getFiles();
	void nextFile(bool silent = false);
	void previousFile(bool silent = false);
	void firstFile();
	void lastFile();
	void loadFileAt(int idx);
	void clearPath();
	void clearFileWatcher();
	QString getCurrentFilter();
	QDir getDir();
	QDir getSaveDir();
	void setDir(QDir& dir);
	void setSaveDir(QDir& dir);
	void setImage(QImage& img);
	void load();
	void load(QFileInfo file, bool silent = false);
	QImage loadThumb(QFileInfo& file, bool silent = false);
	bool hasFile();
	bool isCached(QFileInfo& file);
	QString fileName();
	QFileInfo getChangedFileInfo(int skipIdx, bool silent = false);


	/**
	 * Returns if an image is loaded currently.
	 * @return bool true if an image is loaded.
	 **/ 
	bool hasImage() {
		
		QMutexLocker locker(&mutex);
		return basicLoader.hasImage();
	};

	/**
	 * Returns the currently loaded image.
	 * @return QImage& the current image
	 **/ 
	QImage getImage() {
		
		QMutexLocker locker(&mutex);
		return basicLoader.image();
	};

	/**
	 * Returns the image's metadata.
	 * @return nmc::DkMetaData the image metadata.
	 **/ 
	DkMetaData getMetaData() {
		return imgMetaData;
	};

signals:
	void updateImageSignal();
	void updateInfoSignal(QString msg, int time = 3000, int position = 0);
	void updateInfoSignalDelayed(QString msg, bool start = false, int timeDelayed = 700);
	void updateSpinnerSignalDelayed(bool start = false, int timeDelayed = 700);
	void updateFileSignal(QFileInfo file, QSize s);
	void updateDirSignal(QFileInfo file, bool force = false);
	void newErrorDialog(QString msg, QString title = "Error");
	void fileNotLoadedSignal(QFileInfo file);

public slots:
	QImage changeFileFast(int skipIdx, QFileInfo& fileInfo, bool silent = false);
	void changeFile(int skipIdx, bool silent = false);
	void fileChanged(const QString& path);
	void directoryChanged(const QString& path);
	void saveFileSilentIntern(QFileInfo file, QImage saveImg = QImage());
	void saveFileIntern(QFileInfo filename, QString fileFilter = "", QImage saveImg = QImage(), int compression = -1);
	virtual bool loadFile(QFileInfo file);
	void saveRating(int rating);
	void deleteFile();
	void saveTempFile(QImage img);
	//void enableWatcher(bool enable);

protected:

	DkBasicLoader basicLoader;
	DkCacher* cacher;
	std::vector<DkImageCache> cache;
	
	QFileInfo lastFileLoaded;
	QFileInfo file;
	QFileInfo virtualFile;
	QDir dir;
	QDir saveDir;
	QFileSystemWatcher *watcher;
	QFileSystemWatcher *dirWatcher;
	QStringList files;
	bool folderUpdated;

	// threads
	QMutex mutex;
	QThread* loaderThread;

	// functions
	bool loadDir(QDir newDir);
	void saveFileSilentThreaded(QFileInfo file, QImage img = QImage());
	void updateHistory();
	bool restoreFile(const QFileInfo &fileInfo);
	void startStopCacher();
};

};
