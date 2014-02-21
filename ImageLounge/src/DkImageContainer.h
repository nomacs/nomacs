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

namespace nmc {

class DkImageContainer {

public:
	enum {
		loading = -2,
		exists_not = -1,
		not_loaded,
		loaded,
	};

	DkImageContainer(const QFileInfo& fileInfo);
	bool operator==(const DkImageContainer& o);

	QImage image();

	int hasImage() const;
	QFileInfo file() const;

	QByteArray loadFileToBuffer(const QFileInfo fileInfo);

protected:
	QFileInfo fileInfo;
	DkMetaData metaData;
	QImage img;
	QByteArray fileBuffer;

	int loadState;

	void loadFile();
	QImage loadImage(const QFileInfo fileInfo, const QByteArray fileBuffer);
};

class DkImageContainerT : public QObject, DkImageContainer {
	Q_OBJECT

public:
	DkImageContainerT(const QFileInfo& file);
	~DkImageContainerT();

	bool loadImage();
	void fetchFile();

	///**
	// * Returns whether the thumbnail was loaded, or does not exist.
	// * @return int a status (loaded | not loaded | exists not | loading)
	// **/ 
	//int hasImage() const;

signals:
	void fileLoadedSignal(const QFileInfo& fileInfo, bool loaded = true);
	void showInfoSignal(QString msg, int time = 3000, int position = 0);

protected slots:
	void bufferLoaded();
	void imageLoaded();

protected:
	void fetchImage();
	void loadingFinished();

	QByteArray loadFileToBuffer(const QFileInfo fileInfo);
	QImage loadImage(const QFileInfo fileInfo, const QByteArray fileBuffer);

	QFutureWatcher<QByteArray> bufferWatcher;
	QFutureWatcher<QImage> imageWatcher;

	bool fetchingBuffer;
	bool fetchingImage;
};



};