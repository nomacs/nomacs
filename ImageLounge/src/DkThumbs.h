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
#include <QFutureWatcher>
#include <QSharedPointer>
#include <QColor>
#include <QDir>
#include <QThread>
#include <QImage>
#pragma warning(pop)		// no warnings from includes - end

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport
#endif
#endif

namespace nmc {

#define max_thumb_size 160

/**
 * This class holds thumbnails.
 **/ 
class DllExport DkThumbNail {

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
	DkThumbNail(const QString& filePath = QString(), const QImage& img = QImage());

	/**
	 * Default destructor.
	 * @return 
	 **/ 
	virtual ~DkThumbNail();

	friend bool operator==(const DkThumbNail& lt, const DkThumbNail& rt) {

		return lt.mFile == rt.mFile;
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
		
		return mImg;
	};

	/**
	 * Returns the file information.
	 * @return QFileInfo the thumbnail file
	 **/ 
	QString getFilePath() const {
		return mFile;
	};

	void compute(int forceLoad = do_not_force);

	/**
	 * Returns whether the thumbnail was loaded, or does not exist.
	 * @return int a status (loaded | not loaded | exists not)
	 **/ 
	int hasImage() const {
		
		if (!mImg.isNull())
			return loaded;
		else if (mImg.isNull() && mImgExists)
			return not_loaded;
		else
			return exists_not;
	};

	void setMaxThumbSize(int maxSize) {
		mMaxThumbSize = maxSize;
	};

	int getMaxThumbSize() const {
		return mMaxThumbSize;
	};

	void setMinThumbSize(int minSize) {
		mMinThumbSize = minSize;
	};

	int getMinThumbSize() const {
		return mMinThumbSize;
	};
	
	void setRescale(bool rescale) {
		mRescale = rescale;
	};

	/**
	 * Manipulates the file loaded status.
	 * @param exists a status (loaded | not loaded | exists not)
	 **/ 
	void setImgExists(bool exists) {
		mImgExists = exists;
	};

	enum {
		do_not_force,
		force_exif_thumb,
		force_full_thumb,
		save_thumb,
		force_save_thumb,
	};

protected:
	QImage computeIntern(const QString& file, QSharedPointer<QByteArray> ba, int forceLoad, int maxThumbSize, int minThumbSize, bool rescale);
	QColor computeColorIntern();

	QImage mImg;
	QString mFile;
	//int s;
	bool mImgExists;
	int mMaxThumbSize;
	int mMinThumbSize;
	bool mRescale;
	bool mColorExists;
};

class DkThumbNailT : public QObject, public DkThumbNail {
	Q_OBJECT

public:
	DkThumbNailT(const QString& mFile = QString(), const QImage& mImg = QImage());
	~DkThumbNailT();

	bool fetchThumb(int forceLoad = do_not_force, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>());

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

protected slots:
	void thumbLoaded();

protected:
	QImage computeCall(int forceLoad, QSharedPointer<QByteArray> ba);

	QFutureWatcher<QImage> thumbWatcher;
	bool mFetching;
	int mForceLoad;
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

	int getFileIdx(const QString& filePath) const;
	
	QFileInfoList getFiles() {
		return mFiles;
	};
	QDir getDir() {
		return mDir;
	};
	bool isWorking() {
		return mSomethingTodo;
	};

	enum ForceUpdate {
		not_forced,
		dir_updated,
		user_updated,
	};

	void setForceLoad(bool forceLoad) {
		mForceLoad = forceLoad;
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
	std::vector<DkThumbNail>* mThumbs;
	QDir mDir;
	bool mIsActive;
	bool mSomethingTodo;
	int mNumFilesLoaded;
	QMutex mMutex;
	int mLoadLimit;
	int mStartIdx;
	int mEndIdx;
	bool mLoadAllThumbs;
	bool mForceSave;
	bool mForceLoad;
	QFileInfoList mFiles;

	// functions
	void init();
	void loadThumbs();
};

};
