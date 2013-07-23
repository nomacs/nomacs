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

#include <QFileInfo>
#include <QImage>
#include <QStringList>
#include <QDir>
#include <QThread>
#include <QMutex>
#include <QImageReader>

#include "DkTimer.h"
#include "DkMetaData.h"
#include "DkSettings.h"

namespace nmc {

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

	friend bool operator==(const DkThumbNail& lt, const DkThumbNail& rt) {

		return lt.file == rt.file;
	};

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
	DkThumbsLoader(std::vector<DkThumbNail>* thumbs = 0, QDir dir = QDir(), QStringList files = QStringList());
	~DkThumbsLoader() {};

	static QImage createThumb(const QImage& image);
	void run();
	int getFileIdx(QFileInfo& file);
	QStringList getFiles() {
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
	static int maxThumbSize;
	int loadLimit;
	int startIdx;
	int endIdx;
	bool loadAllThumbs;
	bool forceSave;
	bool forceLoad;
	QStringList files;

	// function
	QImage getThumbNailQt(QFileInfo file);
	//QImage getThumbNailWin(QFileInfo file);
	void init();
	void loadThumbs();
};

class DkColorLoader : public QThread {
	Q_OBJECT

public:
	DkColorLoader(QDir dir = QDir(), QStringList files = QStringList());
	~DkColorLoader() {};

	void stop();
	void run();

	const QVector<QColor>& getColors() const {
		return cols;
	};

	const QVector<int>& getIndexes() const {
		return indexes;
	};

	int maxFiles() const {
		return maxThumbs;
	};

	QString getFilename(int idx) const {

		if (idx < 0 || idx >= files.size())
			return QString("");

		return files.at(idx);
	}

signals:
	void updateSignal(const QVector<QColor>& cols, const QVector<int>& indexes);

protected:
	void init();
	void loadThumbs();
	void loadColor(int fileIdx);
	QColor computeColor(QImage& thumb);

	QVector<QColor> cols;
	QVector<int> indexes;
	QDir dir;
	QStringList files;
	bool isActive;
	QMutex mutex;
	int maxThumbs;
};

};
