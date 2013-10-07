/*******************************************************************************************************
 DkImage.h
 Created on:	21.04.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2013 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2013 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2013 Florian Kleber <florian@nomacs.org>

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

#ifdef WIN32
#include "shlwapi.h"
#pragma comment (lib, "shlwapi.lib")
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
#include <QDirIterator>
#include <QProgressDialog>
#include <QReadLocker>
#include <QWriteLocker>
#include <QReadWriteLock>
#include <QTimer>
#include <QMovie>
#include <QByteArray>
#include <QCoreApplication>

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

using namespace cv;
#endif

#ifdef WITH_LIBTIFF
	#ifdef Q_WS_WIN
		#include "tif_config.h"	
	#endif
	#include "tiffio.h"
#endif

#include <set>

//#ifdef DK_DLL
//#define DllExport Q_DECL_EXPORT
//#else
//#define DllExport
//#endif

// TODO: ifdef
//#include <ShObjIdl.h>
//#include <ShlObj.h>
//#include <Windows.h>

// my classes
//#include "DkNoMacs.h"
#include "DkImageStorage.h"
#include "DkTimer.h"
#include "DkSettings.h"
#include "DkThumbs.h"
#include "DkMetaData.h"
#include "../libqpsd/qpsdhandler.h"

#ifdef Q_WS_X11
	typedef  unsigned char byte;
#endif

namespace nmc {

#ifdef Q_WS_WIN
	
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


bool compLogicQString(const QString & lhs, const QString & rhs);

bool compFilename(const QFileInfo & lhf, const QFileInfo & rhf);

bool compFilenameInv(const QFileInfo & lhf, const QFileInfo & rhf);

bool compDateCreated(const QFileInfo& lhf, const QFileInfo& rhf);

bool compDateCreatedInv(const QFileInfo& lhf, const QFileInfo& rhf);

bool compDateModified(const QFileInfo& lhf, const QFileInfo& rhf);

bool compDateModifiedInv(const QFileInfo& lhf, const QFileInfo& rhf);

// basic image processing

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
	bool loadGeneral(QFileInfo file);

	/**
	 * Loads the page requested (with respect to the current page)
	 * @param skipIdx number of pages to skip
	 * @return bool true if we could load the page requested
	 **/ 
	bool loadPage(int skipIdx = 0);

	int getNumPages() {
		return numPages;
	};

	int getPageIdx() {
		return pageIdx;
	};

	bool setPageIdx(int skipIdx);

	bool save(QFileInfo fileInfo, QImage img, int compression = -1);
	
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

	void release();

#ifdef WITH_OPENCV
	Mat getImageCv() { return cv::Mat(); };	// we should not need this
#endif

	bool loadPSDFile(QFileInfo fileInfo);
#ifdef WITH_WEBP
	bool loadWebPFile(QFileInfo fileInfo);
	bool saveWebPFile(QFileInfo fileInfo, QImage img, int compression);
	bool decodeWebP(const QByteArray& buffer);
	bool encodeWebP(QByteArray& buffer, QImage img, int compression, int speed = 4);
#else
	bool loadWebPFile(QFileInfo fileInfo) {return false;};	// not supported if webP was not linked
	bool saveWebPFile(QFileInfo fileInfo, QImage img, int compression) {return false;};
	bool decodeWebP(const QByteArray& buffer) {return false;};
	bool encodeWebP(QByteArray& buffer, QImage img, int compression, int speed = 4) {return false;};
#endif


public slots:
	void rotate(int orientation);
	void resize(QSize size, float factor = 1.0f, QImage* img = 0, int interpolation = DkImage::ipl_cubic, bool silent = false);

protected:
	
	bool loadRohFile(QString fileName);
	bool loadRawFile(QFileInfo file);
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
		rotated = false;
	};

	bool operator==(const DkImageCache& o) const {
		return o.getFile() == getFile();
	}

	void setFileInfo(QFileInfo& file) {
		this->file = file;
	};

	void setImage(QImage img, bool rotated = false) {
		
		if (rotated)
			this->rotated = rotated;
		if (initialSize.isEmpty()) 
			initialSize = img.size();
		else if (img.isNull()) {
			initialSize = QSize();
			this->rotated = false;
		}

		cacheSize = DkImage::getBufferSizeFloat(img.size(), img.depth());
		this->img = img;
	};

	void ignore() {
		cacheState = cache_ignored;
	};

	void clearImage() {
		img = QImage();
		initialSize = QSize();
		rotated = false;
	};

	QFileInfo getFile() const {
		return file;
	};

	QImage getImage() const {
		return img;
	};

	float getCacheSize() const {
		return cacheSize;
	};

	int getCacheState() const {

		if (!img.isNull())
			return cache_loaded;
		else
			return cacheState;
	};

	//void setRotated(bool rotated = true) {
	//	this->rotated = rotated;
	//};

	bool isRotated() {

		qDebug() << "rotated: " << rotated << " size bool: " << (img.size() != initialSize);
		return rotated | img.size() != initialSize;
	};

protected:
	QFileInfo file;
	QImage img;
	QSize initialSize;
	int cacheState;
	float cacheSize;
	bool rotated;
};


/**
 * This class provides a method for caching images.
 **/ 
class DkCacher : public QThread {

	Q_OBJECT

public:
	DkCacher(QDir dir = QDir(), QStringList files = QStringList());
	~DkCacher() {};

	void run();
	void stop();
	void pause();
	void play();
	void start();

	void setCurrentFile(QFileInfo file, QImage img = QImage());
	void setNewDir(QDir& dir, QStringList& files);
	void updateDir(QStringList& files);
	QVector<DkImageCache> getCache() {
		//QReadLocker locker(&lock);
		return cache;
	};

private:
	QVector<DkImageCache> cache;
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

	//QMutex mutex;
	//QReadWriteLock lock;

	void index();
	void load();
	bool cacheImage(DkImageCache& cacheImg);
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

	enum cacheProps {
		cache_force_load = 0,
		cache_default = 1,
		cache_disable_update,
		cache_props_end
	};

	DkImageLoader(QFileInfo file = QFileInfo());

	virtual ~DkImageLoader();

	//static QString saveFilter;		// for system close dialog
	//static QString openFilter;		// for system  open dialog
	static QStringList fileFilters;	// just the filters
	static QStringList openFilters;	// for open dialog
	static QStringList saveFilters;	// for close dialog

	QStringList ignoreKeywords;
	QStringList keywords;
	QStringList folderKeywords;		// are deleted if a new folder is opened

	static bool isValid(const QFileInfo& fileInfo);
	//static int locateFile(QFileInfo& fileInfo, QDir* dir = 0);
	static QStringList getFoldersRecursive(QDir dir);
	static QStringList getFilteredFileList(QDir dir, QStringList ignoreKeywords = QStringList(), QStringList keywords = QStringList(), QStringList folderKeywords = QStringList());

	static DkMetaData imgMetaData;	// static class so that the metadata is only loaded once (performance)

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
	void loadLastDir();
	void setDir(QDir& dir);
	void setSaveDir(QDir& dir);
	void setImage(QImage img, QFileInfo editFile = QFileInfo());
	void load();
	QImage loadThumb(QFileInfo& file, bool silent = false);
	bool hasFile();
	bool hasMovie();
	bool isCached(QFileInfo& file);
	void updateCacheIndex();
	QString fileName();
	QFileInfo getChangedFileInfo(int skipIdx, bool silent = false, bool searchFile = true);
	static QStringList sort(const QStringList& files, const QDir& dir);
	void sort();

	static void initFileFilters();	// add special file filters

	/**
	 * Returns if an image is loaded currently.
	 * @return bool true if an image is loaded.
	 **/ 
	bool hasImage() {
		
		QMutexLocker locker(&mutex);
		return basicLoader.hasImage();
	};

	bool isEdited() {
		return editFile.exists();
	};

	int numFiles() const {
		return files.size();
	};

	/**
	 * Returns the currently loaded image.
	 * @return QImage the current image
	 **/ 
	QImage getImage() {
		
		// >DIR: dead-lock if a function calls getImage() that is called by updateImageSignal or updateDirSignal 
		// and is connected with a Qt::directConnection [7.11.2012 markus]
		QMutexLocker locker(&mutex);	
		return basicLoader.image();
	};

	bool dirtyTiff() {
		return basicLoader.isDirty();
	};

	/**
	 * Returns the image's metadata.
	 * @return nmc::DkMetaData the image metadata.
	 **/ 
	DkMetaData getMetaData() {
		return imgMetaData;
	};

	static bool restoreFile(const QFileInfo &fileInfo);

signals:
	void folderFiltersChanged(QStringList filters);
	void updateImageSignal();
	void updateInfoSignal(QString msg, int time = 3000, int position = 0);
	void updateInfoSignalDelayed(QString msg, bool start = false, int timeDelayed = 700);
	void updateSpinnerSignalDelayed(bool start = false, int timeDelayed = 700);
	void updateFileSignal(QFileInfo file, QSize s = QSize(), bool edited = false, QString attr = QString());
	void updateDirSignal(QFileInfo file, int force = DkThumbsLoader::not_forced);
	void newErrorDialog(QString msg, QString title = "Error");
	void fileNotLoadedSignal(QFileInfo file);
	void setPlayer(bool play);
	void updateFileWatcherSignal(QFileInfo file);

public slots:
	QImage changeFileFast(int skipIdx, QFileInfo& fileInfo, bool silent = false);
	void changeFile(int skipIdx, bool silent = false, int force = cache_default);
	void fileChanged(const QString& path);
	void directoryChanged(const QString& path = QString());
	void saveFileSilentIntern(QFileInfo file, QImage saveImg = QImage());
	void saveFileIntern(QFileInfo filename, QString fileFilter = "", QImage saveImg = QImage(), int compression = -1);
	void load(QFileInfo file, bool silent = false, int cacheState = cache_default);
	virtual bool loadFile(QFileInfo file, bool silent = false, int cacheState = cache_default);
	void saveRating(int rating);
	void deleteFile();
	QFileInfo saveTempFile(QImage img, QString name = "img", QString fileExt = ".png", bool force = false, bool threaded = true);
	void setFolderFilters(QStringList filters);
	QStringList getFolderFilters();
	void updateFileWatcher(QFileInfo filePath);
	void disableFileWatcher();

	//void enableWatcher(bool enable);

protected:

	DkBasicLoader basicLoader;
	DkCacher* cacher;

	QTimer delayedUpdateTimer;
	bool timerBlockedUpdate;
	
	QFileInfo editFile;
	QFileInfo lastFileLoaded;
	QFileInfo file;
	QFileInfo virtualFile;
	QDir dir;
	QDir saveDir;
	QFileSystemWatcher *watcher;
	QFileSystemWatcher *dirWatcher;
	QStringList subFolders;
	QStringList files;
	bool folderUpdated;
	int cFileIdx;

	// threads
	QMutex mutex;
	QThread* loaderThread;

	// functions
	bool loadDir(QDir newDir, bool scanRecursive = true);
	void saveFileSilentThreaded(QFileInfo file, QImage img = QImage());
	void updateHistory();
	void startStopCacher();
	void sendFileSignal();
	QString getTitleAttributeString();
};

};
