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
	#ifdef DK_DLL
	#define DllExport Q_DECL_EXPORT
	#else
	#define DllExport
	#endif
#endif

namespace nmc {

class DllExport DkMetaData {

public:
	DkMetaData(QFileInfo file = QFileInfo());

	DkMetaData(const DkMetaData& metaData);

	~DkMetaData() {};

	DkMetaData& operator=(const DkMetaData& metadata) {

		if (this == &metadata)
			return *this;

		this->file = metadata.file;
		this->mdata = false;
		this->hasMetaData = metadata.hasMetaData;
		this->dirty = metadata.dirty;

		return *this;
	};

	void setFileName(QFileInfo file) {

		// do nothing if the same file is set
		if (this->file == file)
			return;

		this->file = file;
		mdata = false;
		hasMetaData = true;
		dirty = false;
	};

	QFileInfo getFile() const {
		return file;
	};

	bool isDirty() {
		return dirty;
	};

	void reloadImg();

	void saveMetaDataToFile(QFileInfo fileN = QFileInfo(), int orientation = 0);

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

	bool mdata;
	bool hasMetaData;
	bool dirty;

};

};
