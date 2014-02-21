/*******************************************************************************************************
 DkMetaData.h
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
#include <QDebug>
#include <QBuffer>


#ifdef HAVE_EXIV2_HPP
#include <exiv2/exiv2.hpp>
#else
#include <exiv2/image.hpp>
#include <iomanip>
#endif


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

class DllExport DkMetaDataT {

public:
	DkMetaDataT();

	void readMetaData(const QFileInfo& fileInfo);
	void readMetaData(const QFileInfo& fileInfo, const QByteArray& ba);

	bool saveMetaData(const QFileInfo& fileInfo);
	bool saveMetaData(QByteArray& ba);

	int getOrientation() const;
	int getRating() const;
	QString getNativeExifValue(const QString& key) const;
	QString getExifValue(const QString& key) const;
	QString getIptcValue(const QString& key) const;
	QImage getThumbnail() const;
	QStringList getExifKeys() const;
	QStringList getExifValues() const;
	QStringList getIptcKeys() const;
	QStringList getIptcValues() const;

	void setOrientation(int o);
	void setRating(int r);
	void setExifValue(QString key, QString taginfo);
	void setThumbnail(QImage thumb);

	bool hasMetaData() const;
	bool isLoaded() const;
	bool isTiff() const;
	bool isJpg() const;
	bool isRaw() const;
	void printMetaData() const; //only for debug

protected:
	Exiv2::Image::AutoPtr exifImg;
	QFileInfo file;

	enum {
		not_loaded,
		no_data,
		loaded,
		dirty,
	};

	int exifState;
};

class DllExport DkMetaData {

public:
	DkMetaData(QFileInfo file = QFileInfo());
	DkMetaData(const QByteArray& ba);

	DkMetaData(const DkMetaData& metaData);

	~DkMetaData() {};

	DkMetaData& operator=(const DkMetaData& metadata) {

		if (this == &metadata)
			return *this;

		this->file = metadata.file;
		this->buffer = metadata.buffer;
		this->mdata = false;
		this->hasMetaData = metadata.hasMetaData;
		this->dirty = metadata.dirty;

		return *this;
	};

	bool isLoaded() const;

	// TODO: remove!
	void setFileName(QFileInfo file) {

		// do nothing if the same file is set
		if (this->file == file)
			return;

		this->file = file;
		mdata = false;
		hasMetaData = true;
		dirty = false;
	};

	void setBuffer(const QByteArray& ba) {
		buffer = ba;
	};

	QFileInfo getFile() const {
		return file;
	};

	bool isDirty() {
		return dirty;
	};

	//void reloadImg();

	void saveMetaDataToFile(QFileInfo fileN = QFileInfo(), int orientation = 0);
	void saveMetaDataToBuffer(QByteArray& ba, int orientation = 0);

	std::string getNativeExifValue(std::string key);
	std::string getExifValue(std::string key);
	bool setExifValue(std::string key, std::string taginfo);
	std::string getIptcValue(std::string key);
	int getOrientation();
	QImage getThumbnail();
	void saveThumbnail(QImage thumb, QFileInfo saveFile);
	void saveOrientation(int o);
	int getHorizontalFlipped();
	void saveHorizontalFlipped(int f);
	float getRating();
	void saveRating(int r);
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
	QByteArray buffer;

	bool mdata;
	bool hasMetaData;
	bool dirty;
};

};
