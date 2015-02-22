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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QColor>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QStringList>
#include <QImage>
#pragma warning(pop)	// no warnings from includes - end

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport
#endif
#endif

// my classes
#include "DkImageContainer.h"

#ifdef Q_WS_X11
	typedef  unsigned char byte;
#endif

//#ifdef WITH_LIBTIFF
//	#ifdef WIN32
//		#include "tif_config.h"	
//	#endif
//
//	#ifdef Q_WS_MAC
//		#define uint64 uint64_hack_
//		#define int64 int64_hack_
//	#endif // Q_WS_MAC
//
//	#include "tiffio.h"
//
//	#ifdef Q_WS_MAC
//		#undef uint64
//		#undef int64
//	#endif // Q_WS_MAC
//#endif

// Qt defines
class QFileSystemWatcher;
class QUrl;

namespace nmc {

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

	QStringList ignoreKeywords;
	QStringList keywords;
	QStringList folderKeywords;		// are deleted if a new folder is opened

	static QStringList getFoldersRecursive(QDir dir);
	QFileInfoList updateSubFolders(QDir rootDir);
	QFileInfoList getFilteredFileInfoList(const QDir& dir, QStringList ignoreKeywords = QStringList(), QStringList keywords = QStringList(), QStringList folderKeywords = QStringList());

	void rotateImage(double angle);
	QSharedPointer<DkImageContainerT> getCurrentImage() const;
	QFileInfo file() const;
	QStringList getFileNames();
	QVector<QSharedPointer<DkImageContainerT> > getImages();
	void setImages(QVector<QSharedPointer<DkImageContainerT> > images);
	void firstFile();
	void lastFile();
	void loadFileAt(int idx);
	void clearPath();
	QDir getDir() const;
	QDir getSaveDir() const;
	void loadLastDir();
	void setDir(QDir& dir);
	void setSaveDir(QDir& dir);
	QSharedPointer<DkImageContainerT> setImage(QImage img, QFileInfo editFile = QFileInfo());
	bool hasFile() const;
	bool hasMovie();
	QString fileName();
	QSharedPointer<DkImageContainerT> getSkippedImage(int skipIdx, bool searchFile = true, bool recursive = false);
	void sort();
	QSharedPointer<DkImageContainerT> findOrCreateFile(const QFileInfo& file) const;
	QSharedPointer<DkImageContainerT> findFile(const QFileInfo& file) const;
	int findFileIdx(const QFileInfo& file, const QVector<QSharedPointer<DkImageContainerT> >& images) const;
	void setCurrentImage(QSharedPointer<DkImageContainerT> newImg);
#ifdef WITH_QUAZIP
	bool loadZipArchive(QFileInfo zipFile);
#endif
	
	void deactivate();
	void activate(bool isActive = true);
	bool hasImage() const;
	bool isEdited() const;
	int numFiles() const;
	QImage getImage();
	bool dirtyTiff();

	static bool restoreFile(const QFileInfo &fileInfo);

signals:
	void folderFiltersChanged(QStringList filters);
	void updateImageSignal();
	void updateInfoSignalDelayed(QString msg, bool start = false, int timeDelayed = 700);
	void updateSpinnerSignalDelayed(bool start = false, int timeDelayed = 700);
	void updateFileSignal(QFileInfo file, QSize s = QSize(), bool edited = false, QString attr = QString());
	void fileNotLoadedSignal(QFileInfo file);
	void setPlayer(bool play);
	void updateFileWatcherSignal(QFileInfo file);

	// new signals
	void imageUpdatedSignal(QSharedPointer<DkImageContainerT> image);
	void imageLoadedSignal(QSharedPointer<DkImageContainerT> image, bool loaded = true);
	void showInfoSignal(QString msg, int time = 3000, int position = 0);
	void updateDirSignal(QVector<QSharedPointer<DkImageContainerT> > images);
	void imageHasGPSSignal(bool hasGPS);

public slots:
	void changeFile(int skipIdx);
	void directoryChanged(const QString& path = QString());
	void saveFileWeb(QImage saveImg);
	void saveUserFileAs(QImage saveImg, bool silent);
	void saveFile(QFileInfo filename, QImage saveImg = QImage(), QString fileFilter = "", int compression = -1);
	void load(QSharedPointer<DkImageContainerT> image = QSharedPointer<DkImageContainerT>());
	void load(const QFileInfo& file);
	void downloadFile(const QUrl& url);
	bool deleteFile();
	QFileInfo saveTempFile(QImage img, QString name = "img", QString fileExt = ".png", bool force = false);
	void setFolderFilters(QStringList filters);
	QStringList getFolderFilters();
	bool loadDir(QFileInfo newFile, bool scanRecursive = true);
	bool loadDir(QDir newDir, bool scanRecursive = true);
	void errorDialog(const QString& msg) const;


	// new slots
	void imageLoaded(bool loaded = false);
	void imageSaved(QFileInfo file, bool saved = true);
	void imagesSorted();
	bool unloadFile();
	void reloadImage();

protected:

	QTimer delayedUpdateTimer;
	bool timerBlockedUpdate;
	QDir dir;
	QDir saveDir;
	QFileSystemWatcher* dirWatcher;
	QStringList subFolders;
	QVector<QSharedPointer<DkImageContainerT > > images;
	QSharedPointer<DkImageContainerT > currentImage;
	QSharedPointer<DkImageContainerT > lastImageLoaded;
	bool folderUpdated;
	int tmpFileIdx;
	bool sortingImages;
	bool sortingIsDirty;
	QFutureWatcher<QVector<QSharedPointer<DkImageContainerT > > > createImageWatcher;

	// functions
	void updateCacher(QSharedPointer<DkImageContainerT> imgC);
	int getNextFolderIdx(int folderIdx);
	int getPrevFolderIdx(int folderIdx);
	void updateHistory();
	void sendFileSignal();
	QString getTitleAttributeString();
	void sortImagesThreaded(QVector<QSharedPointer<DkImageContainerT > > images);
	void createImages(const QFileInfoList& files, bool sort = true);
	QVector<QSharedPointer<DkImageContainerT > > sortImages(QVector<QSharedPointer<DkImageContainerT > > images) const;
};

// deprecated
class DkColorLoader : public QThread {
	Q_OBJECT

public:
	DkColorLoader(QVector<QSharedPointer<DkImageContainerT> > images);
	~DkColorLoader() {};

	void stop();
	void run();

	const QVector<QColor>& getColors() const;
	const QVector<int>& getIndexes() const;
	int maxFiles() const;
	QString getFilename(int idx) const;

signals:
	void updateSignal(const QVector<QColor>& cols, const QVector<int>& indexes);

protected:
	void init();
	void loadThumbs();
	void loadColor(int fileIdx);
	QColor computeColor(QImage& thumb);

	QVector<QSharedPointer<DkImageContainerT> > images;
	QVector<QColor> cols;
	QVector<int> indexes;
	bool isActive;
	bool paused;
	QMutex mutex;
	int maxThumbs;
};

};
