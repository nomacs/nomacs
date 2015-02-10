/*******************************************************************************************************
 DkThumbs.h
 Created on:	19.04.2013
 
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
#include <QFileInfo>
#include <QImage>
#include <QStringList>
#include <QDir>
#include <QThread>
#include <QMutex>
#include <QImageReader>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QSharedPointer>
#include <QTimer>

#ifdef WIN32
	#include <winsock2.h>	// needed since libraw 0.16
#endif
#pragma warning(pop)		// no warnings from includes - end

#include "DkTimer.h"
#include "DkMetaData.h"
#include "DkSettings.h"

namespace nmc {

#define max_thumb_size 160

/**
 * This class holds thumbnails.
 **/ 
class DkThumbNail {

public:
	enum {
		loading = -2,
		exists_not = -1,
		not_loaded,
		loaded,
	};
	
	/**
	 * Default constructor.
	 * @param file the corresponding file
	 * @param img the thumbnail image
	 **/ 
	DkThumbNail(QFileInfo file = QFileInfo(), QImage img = QImage());

	/**
	 * Default destructor.
	 * @return 
	 **/ 
	virtual ~DkThumbNail() {};

	friend bool operator==(const DkThumbNail& lt, const DkThumbNail& rt) {

		return lt.file == rt.file;
	};

	/**
	 * Sets the thumbnail image.
	 * @param img the thumbnail
	 **/ 
	virtual void setImage(const QImage img);

	void removeBlackBorder(QImage& img);

	/**
	 * Returns the thumbnail.
	 * @return QImage the thumbnail.
	 **/ 
	QImage getImage() const {
		
		return img;
	};

	/**
	 * Returns the file information.
	 * @return QFileInfo the thumbnail file
	 **/ 
	QFileInfo getFile() const {
		return file;
	};

	void compute(int forceLoad = do_not_force);

	/**
	 * Returns whether the thumbnail was loaded, or does not exist.
	 * @return int a status (loaded | not loaded | exists not)
	 **/ 
	int hasImage() const {
		
		if (!img.isNull())
			return loaded;
		else if (img.isNull() && imgExists)
			return not_loaded;
		else
			return exists_not;
	};

	QColor getMeanColor() const {
		return meanColor;
	};

	void setMaxThumbSize(int maxSize) {
		this->maxThumbSize = maxSize;
	};

	int getMaxThumbSize() const {
		return maxThumbSize;
	};

	void setMinThumbSize(int minSize) {
		this->minThumbSize = minSize;
	};

	int getMinThumbSize() const {
		return this->minThumbSize;
	};
	
	void setRescale(bool rescale) {
		this->rescale = rescale;
	};

	/**
	 * Manipulates the file loaded status.
	 * @param exists a status (loaded | not loaded | exists not)
	 **/ 
	void setImgExists(bool exists) {
		imgExists = exists;
	};

	///**
	// * Returns the thumbnail size.
	// * @return int the maximal side (either width or height)
	// **/ 
	//int size() {
	//	return s;
	//};

	enum {
		do_not_force,
		force_exif_thumb,
		force_full_thumb,
		save_thumb,
		force_save_thumb,
	};

protected:
	QImage computeIntern(QFileInfo file, QSharedPointer<QByteArray> ba, int forceLoad, int maxThumbSize, int minThumbSize, bool rescale);
	QColor computeColorIntern();

	QImage img;
	QFileInfo file;
	int s;
	bool imgExists;
	int maxThumbSize;
	int minThumbSize;
	bool rescale;
	QColor meanColor;
	bool colorExists;
};

class DkThumbNailT : public QObject, public DkThumbNail {
	Q_OBJECT

public:
	DkThumbNailT(QFileInfo file = QFileInfo(), QImage img = QImage());
	~DkThumbNailT();

	bool fetchThumb(int forceLoad = do_not_force, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>());
	void fetchColor();

	/**
	 * Returns whether the thumbnail was loaded, or does not exist.
	 * @return int a status (loaded | not loaded | exists not | loading)
	 **/ 
	int hasImage() const {
		
		if (thumbWatcher.isRunning())
			return loading;
		else
			return DkThumbNail::hasImage();
	};

	void setImage(const QImage img) {
		DkThumbNail::setImage(img);
		emit thumbLoadedSignal(true);
	};

signals:
	void thumbLoadedSignal(bool loaded = true);
	void colorUpdated();

protected slots:
	void thumbLoaded();
	void colorLoaded();

protected:
	QImage computeCall(int forceLoad, QSharedPointer<QByteArray> ba);
	QColor computeColorCall();

	QFutureWatcher<QImage> thumbWatcher;
	QFutureWatcher<QColor> colorWatcher;
	bool fetching;
	bool fetchingColor;
	int forceLoad;
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
	DkThumbsLoader(std::vector<DkThumbNail>* thumbs = 0, QDir dir = QDir(), QFileInfoList files = QFileInfoList());
	~DkThumbsLoader() {};

	void run();
	int getFileIdx(QFileInfo& file);
	QFileInfoList getFiles() {
		return files;
	};
	QDir getDir() {
		return dir;
	};
	bool isWorking() {
		return somethingTodo;
	};

	enum ForceUpdate {
		not_forced,
		dir_updated,
		user_updated,
	};

	void setForceLoad(bool forceLoad) {
		this->forceLoad = forceLoad;
	};

signals:
	void updateSignal();
	void progressSignal(int percent);
	void numFilesSignal(int numFiles);

public slots:
	void setLoadLimits(int start = 0, int end = 20);
	void loadAll();
	void stop();

private:
	std::vector<DkThumbNail>* thumbs;
	QDir dir;
	bool isActive;
	bool somethingTodo;
	int numFilesLoaded;
	QMutex mutex;
	int loadLimit;
	int startIdx;
	int endIdx;
	bool loadAllThumbs;
	bool forceSave;
	bool forceLoad;
	QFileInfoList files;

	//// function
	//QImage getThumbNailQt(QFileInfo file);
	//QImage getThumbNailWin(QFileInfo file);
	void init();
	void loadThumbs();
};

};
