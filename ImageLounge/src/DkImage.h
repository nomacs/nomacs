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

#include <QWidget>
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


#include <set>

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport
#endif
#endif

// TODO: ifdef
//#include <ShObjIdl.h>
//#include <ShlObj.h>
//#include <Windows.h>

// my classes
//#include "DkNoMacs.h"
#include "DkImageStorage.h"
#include "DkBasicLoader.h"
#include "DkTimer.h"
#include "DkSettings.h"
#include "DkThumbs.h"
#include "DkMetaData.h"
#include "DkBasicLoader.h"
#include "DkImageContainer.h"

#ifdef Q_WS_X11
	typedef  unsigned char byte;
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


namespace nmc {

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
	QFileInfoList updateSubFolders(QDir rootDir);

	// deprecated
	static QStringList getFilteredFileList(QDir dir, QStringList ignoreKeywords = QStringList(), QStringList keywords = QStringList(), QStringList folderKeywords = QStringList());
	QFileInfoList getFilteredFileInfoList(const QDir& dir, QStringList ignoreKeywords = QStringList(), QStringList keywords = QStringList(), QStringList folderKeywords = QStringList());

	//static DkMetaData imgMetaData;	// static class so that the metadata is only loaded once (performance)

	void rotateImage(double angle);
	void saveFile(QFileInfo filename, QImage saveImg = QImage(), QString fileFilter = "", int compression = -1);
	void setFile(QFileInfo& filename);
	QSharedPointer<DkImageContainerT> getCurrentImage() const;
	QFileInfo file() const;
	QStringList getFileNames();
	QVector<QSharedPointer<DkImageContainerT> > getImages();
	void nextFile(bool silent = false);
	void previousFile(bool silent = false);
	void firstFile();
	void lastFile();
	void loadFileAt(int idx);
	void clearPath();
	void clearFileWatcher();
	QString getCurrentFilter();
	QDir getDir() const;
	QDir getSaveDir();
	void loadLastDir();
	void setDir(QDir& dir);
	void setSaveDir(QDir& dir);
	QSharedPointer<DkImageContainerT> setImage(QImage img, QFileInfo editFile = QFileInfo());
	//QImage loadThumb(QFileInfo& file, bool silent = false);
	bool hasFile();
	bool hasMovie();
	//bool isCached(QFileInfo& file);
	//void updateCacheIndex();
	QString fileName();
	//QFileInfo getChangedFileInfo(int skipIdx, bool silent = false, bool searchFile = true);		//deprecated
	QSharedPointer<DkImageContainerT> getSkippedImage(int skipIdx, bool silent = false, bool searchFile = true);
	//static QStringList sort(const QStringList& files, const QDir& dir);	// deprecated
	void sort();
	QVector<QSharedPointer<DkImageContainerT> > sort(const QVector<QSharedPointer<DkImageContainerT> >& images, const QDir& dir);
	QSharedPointer<DkImageContainerT> findOrCreateFile(const QFileInfo& file) const;
	QSharedPointer<DkImageContainerT> findFile(const QFileInfo& file) const;
	int findFileIdx(const QFileInfo& file, const QVector<QSharedPointer<DkImageContainerT> >& images) const;
	void setCurrentImage(QSharedPointer<DkImageContainerT> newImg);
	
	static void initFileFilters();	// add special file filters

	bool hasImage();
	bool isEdited() const;
	int numFiles() const;
	QImage getImage();
	bool dirtyTiff();

	static bool restoreFile(const QFileInfo &fileInfo);

signals:
	void folderFiltersChanged(QStringList filters);
	void updateImageSignal();
	void updateInfoSignal(QString msg, int time = 3000, int position = 0);
	void updateInfoSignalDelayed(QString msg, bool start = false, int timeDelayed = 700);
	void updateSpinnerSignalDelayed(bool start = false, int timeDelayed = 700);
	void updateFileSignal(QFileInfo file, QSize s = QSize(), bool edited = false, QString attr = QString());
	void updateDirSignal(QFileInfo file, int force = DkThumbsLoader::not_forced);
	void updateDirSignal(QVector<QSharedPointer<DkImageContainerT> > images);
	void newErrorDialog(QString msg, QString title = "Error");
	void fileNotLoadedSignal(QFileInfo file);
	void setPlayer(bool play);
	void updateFileWatcherSignal(QFileInfo file);

	// new signals
	void imageUpdatedSignal(QSharedPointer<DkImageContainerT> image);
	void imageLoadedSignal(QSharedPointer<DkImageContainerT> image, bool loaded = true);
	void showInfoSignal(QString msg, int time = 3000, int position = 0);
	void errorDialogSignal(const QString&);

public slots:
	//QImage changeFileFast(int skipIdx, QFileInfo& fileInfo, bool silent = false);
	void changeFile(int skipIdx, bool silent = false, int force = cache_default);
	//void fileChanged(const QString& path);
	void directoryChanged(const QString& path = QString());
	//void saveFileSilentIntern(QFileInfo file, QImage saveImg = QImage());
	void saveFileIntern(QFileInfo filename, QImage saveImg = QImage(), QString fileFilter = "", int compression = -1);
	//void load(QFileInfo file, bool silent = false, int cacheState = cache_default);	// deprecated
	void load(QSharedPointer<DkImageContainerT> image = QSharedPointer<DkImageContainerT>(), bool silent = false);
	void load(const QFileInfo& file, bool silent = false);
	//virtual bool loadFile(QFileInfo file, bool silent = false, int cacheState = cache_default);
	//void saveRating(int rating);
	void deleteFile();
	QFileInfo saveTempFile(QImage img, QString name = "img", QString fileExt = ".png", bool force = false, bool threaded = true);
	void setFolderFilters(QStringList filters);
	QStringList getFolderFilters();
	void updateFileWatcher(QFileInfo filePath);
	void disableFileWatcher();

	// new slots
	void imageLoaded(bool loaded = false);
	void imageSaved(QFileInfo file, bool saved = true);
	void unloadFile();
	void reloadImage();

protected:

	//DkBasicLoader basicLoader;
	//DkCacher* cacher;

	QTimer delayedUpdateTimer;
	bool timerBlockedUpdate;
	
	//QFileInfo editFile;
	//QFileInfo lastFileLoaded;
	QDir dir;
	QDir saveDir;
	QFileSystemWatcher *watcher;
	QFileSystemWatcher *dirWatcher;
	QStringList subFolders;
	//QStringList files;
	QVector<QSharedPointer<DkImageContainerT > > images;
	QSharedPointer<DkImageContainerT > currentImage;
	QSharedPointer<DkImageContainerT > lastImageLoaded;
	bool folderUpdated;
	//int cFileIdx; // deprecated?
	int tmpFileIdx; // deprecated?

	// functions
	int getNextFolderIdx(int folderIdx);
	int getPrevFolderIdx(int folderIdx);
	bool loadDir(QFileInfo newFile, bool scanRecursive = true);
	bool loadDir(QDir newDir, bool scanRecursive = true);
	//void saveFileSilentThreaded(QFileInfo file, QImage img = QImage());
	void updateHistory();
	void sendFileSignal();
	QString getTitleAttributeString();
	void createImages(const QFileInfoList& files);
};

};
