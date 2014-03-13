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
#include <QVector2D>
#include <QSharedPointer>


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

	void readMetaData(const QFileInfo& fileInfo, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>());

	bool saveMetaData(const QFileInfo& fileInfo, bool force = false);
	bool saveMetaData(QSharedPointer<QByteArray>& ba, bool force = false);

	int getOrientation() const;
	int getRating() const;
	QVector2D getResolution() const;
	QString getNativeExifValue(const QString& key) const;
	QString getExifValue(const QString& key) const;
	QString getIptcValue(const QString& key) const;
	QImage getThumbnail() const;
	QStringList getExifKeys() const;
	QStringList getExifValues() const;
	QStringList getIptcKeys() const;
	QStringList getIptcValues() const;

	void setResolution(const QVector2D& res);
	void clearOrientation();
	void setOrientation(int o);
	void setRating(int r);
	void setExifValue(QString key, QString taginfo);
	void setThumbnail(QImage thumb);

	bool hasMetaData() const;
	bool isLoaded() const;
	bool isTiff() const;
	bool isJpg() const;
	bool isRaw() const;
	bool isDirty() const;
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

};
