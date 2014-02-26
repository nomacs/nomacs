/*******************************************************************************************************
 DkImageContainer.h
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

#include <QObject>
#include <QImage>
#include <QFileInfo>
#include <QFuture>
#include <QFutureWatcher>

#include "DkMetaData.h"
#include "DkBasicLoader.h"
#include "DkThumbs.h"

namespace nmc {

class DkImageContainer {

public:
	enum {
		loading_canceled = -3,
		loading = -2,
		exists_not = -1,
		not_loaded,
		loaded,
	};

	DkImageContainer(const QFileInfo& fileInfo);
	//friend bool operator==(const DkImageContainer& lic, const DkImageContainer& ric);
	bool operator==(const DkImageContainer& ric) const;
	bool operator< (const DkImageContainer& o) const;
	bool operator<= (const DkImageContainer& o) const;
	friend bool imageContainerLessThan(const DkImageContainer& l, const DkImageContainer& r);
	friend bool imageContainerLessThanPtr(const QSharedPointer<DkImageContainer> l, const QSharedPointer<DkImageContainer> r);

	QImage image();

	bool hasImage() const;
	int imgLoaded() const;
	QFileInfo file() const;
	bool isEdited() const;
	int getPageIdx() const;
	QString getTitleAttribute() const;
	QSharedPointer<DkBasicLoader> getLoader() const;
	QSharedPointer<DkMetaDataT> getMetaData() const;

	bool exists();
	bool setPageIdx(int skipIdx);

	QByteArray loadFileToBuffer(const QFileInfo fileInfo);
	bool loadImage();
	void setImage(const QImage& img, const QFileInfo& fileInfo);
	void saveMetaData();
	virtual void clear();

protected:
	QFileInfo fileInfo;
	QByteArray fileBuffer;
	QSharedPointer<DkBasicLoader> loader;
	QSharedPointer<DkThumbNailT> thumb;

	int loadState;
	bool edited;

	QSharedPointer<DkBasicLoader> loadImageIntern(const QFileInfo fileInfo, const QByteArray fileBuffer);
	QFileInfo saveImageIntern(const QFileInfo fileInfo, QImage saveImg, int compression);
	void saveMetaDataIntern(const QFileInfo& fileInfo, const QByteArray& fileBuffer);
	void init();
};

class DkImageContainerT : public QObject, public DkImageContainer {
	Q_OBJECT

public:
	DkImageContainerT(const QFileInfo& file);
	~DkImageContainerT();

	bool loadImageThreaded();
	void fetchFile();
	void cancel();
	void clear();

	bool saveImageThreaded(const QFileInfo& fileInfo, const QImage& saveImg, int compression = -1);
	bool saveImageThreaded(const QFileInfo& fileInfo, int compression = -1);
	void saveMetaDataThreaded();
	
	///**
	// * Returns whether the thumbnail was loaded, or does not exist.
	// * @return int a status (loaded | not loaded | exists not | loading)
	// **/ 
	//int hasImage() const;

signals:
	void fileLoadedSignal(bool loaded = true);
	void fileSavedSignal(QFileInfo fileInfo, bool saved = true);
	void showInfoSignal(QString msg, int time = 3000, int position = 0);
	void errorDialogSignal(const QString& msg);

protected slots:
	void bufferLoaded();
	void imageLoaded();
	void savingFinished();
	void loadingFinished();

protected:
	void fetchImage();
	
	QByteArray loadFileToBuffer(const QFileInfo fileInfo);
	QSharedPointer<DkBasicLoader> loadImageIntern(const QFileInfo fileInfo, const QByteArray fileBuffer);
	QFileInfo saveImageIntern(const QFileInfo fileInfo, QImage saveImg, int compression);
	void saveMetaDataIntern();

	QFutureWatcher<QByteArray> bufferWatcher;
	QFutureWatcher<QSharedPointer<DkBasicLoader> > imageWatcher;
	QFutureWatcher<QFileInfo> saveImageWatcher;
	QFutureWatcher<bool> saveMetaDataWatcher;

	bool fetchingImage;
	bool fetchingBuffer;
	//bool savingImage;
	//bool savingMetaData;
};



};