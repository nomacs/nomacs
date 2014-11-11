/*******************************************************************************************************
 DkMetaData.cpp
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

#include <QTranslator>
#include <QObject>

#include "DkMetaData.h"
#include "DkImage.h"

namespace nmc {

// DkMetaDataT --------------------------------------------------------------------
DkMetaDataT::DkMetaDataT() {

	exifState = not_loaded;
}

void DkMetaDataT::readMetaData(const QFileInfo& fileInfo, QSharedPointer<QByteArray> ba) {

	this->file = fileInfo;

	try {
		if (!ba || ba->isEmpty()) {
#ifdef EXV_UNICODE_PATH
#if QT_VERSION < 0x050000
			// it was crashing here - if the thumbnail is fetched in the constructor of a label
			// seems that the QFileInfo was corrupted?!
			std::wstring filePath = (file.isSymLink()) ? file.symLinkTarget().toStdWString() : file.absoluteFilePath().toStdWString();
			exifImg = Exiv2::ImageFactory::open(filePath);
#else
			std::wstring filePath = (file.isSymLink()) ? (wchar_t*)file.symLinkTarget().utf16() : (wchar_t*)file.absoluteFilePath().utf16();
			exifImg = Exiv2::ImageFactory::open(filePath);
#endif
#else
			std::string filePath = (file.isSymLink()) ? file.symLinkTarget().toStdString() : file.absoluteFilePath().toStdString();
			exifImg = Exiv2::ImageFactory::open(filePath);
#endif
		}
		else {
			Exiv2::MemIo::AutoPtr exifBuffer(new Exiv2::MemIo((const byte*)ba->constData(), ba->size()));
			exifImg = Exiv2::ImageFactory::open(exifBuffer);
		}
	} 
	catch (...) {
		exifState = no_data;
		qDebug() << "[Exiv2] could not open file for exif data";
		return;
	}

	if (exifImg.get() == 0) {
		exifState = no_data;
		qDebug() << "[Exiv2] image could not be opened for exif data extraction";
		return;
	}

	try {
		exifImg->readMetadata();

		if (!exifImg->good()) {
			qDebug() << "[Exiv2] metadata could not be read";
			exifState = no_data;
			return;
		}

	}catch (...) {
		exifState = no_data;
		qDebug() << "[Exiv2] could not read metadata (exception)";
		return;
	}
	
	//qDebug() << "[Exiv2] metadata loaded";
	exifState = loaded;

	//printMetaData();

}

bool DkMetaDataT::saveMetaData(const QFileInfo& fileInfo, bool force) {

	if (exifState != loaded && exifState != dirty)
		return false;

	QFile file(fileInfo.absoluteFilePath());
	file.open(QFile::ReadOnly);
	
	QSharedPointer<QByteArray> ba(new QByteArray(file.readAll()));
	file.close();
	bool saved = saveMetaData(ba, force);
	if (!saved) {
		qDebug() << "[DkMetaDataT] could not save: " << fileInfo.fileName();
		return saved;
	}
	else if (ba->isEmpty()) {
		qDebug() << "[DkMetaDataT] could not save: " << fileInfo.fileName() << " empty Buffer!";
		return false;
	}

	file.open(QFile::WriteOnly);
	file.write(ba->data(), ba->size());
	file.close();

	qDebug() << "[DkMetaDataT] I saved: " << ba->size() << " bytes";

	return true;
}

bool DkMetaDataT::saveMetaData(QSharedPointer<QByteArray>& ba, bool force) {

	if (!ba)
		return false;

	if (!force && exifState != dirty)
		return false;
	else if (exifState == not_loaded || exifState == no_data)
		return false;

	Exiv2::ExifData &exifData = exifImg->exifData();
	Exiv2::XmpData &xmpData = exifImg->xmpData();
	Exiv2::IptcData &iptcData = exifImg->iptcData();

	Exiv2::Image::AutoPtr exifImgN;
	Exiv2::MemIo::AutoPtr exifMem;

	try {

		exifMem = Exiv2::MemIo::AutoPtr(new Exiv2::MemIo((byte*)ba->data(), ba->size()));
		exifImgN = Exiv2::ImageFactory::open(exifMem);
	} 
	catch (...) {

		qDebug() << "could not open image for exif data";
		return false;
	}

	if (exifImgN.get() == 0) {
		qDebug() << "image could not be opened for exif data extraction";
		return false;
	}

	exifImgN->readMetadata();

	qDebug() << "orientation for saving: " << getOrientation();

	exifImgN->setExifData(exifData);
	exifImgN->setXmpData(xmpData);
	exifImgN->setIptcData(iptcData);
	
	exifImgN->writeMetadata();

	// now get the data again
	Exiv2::DataBuf exifBuf = exifImgN->io().read(exifImgN->io().size());
	if (exifBuf.pData_) {
		QSharedPointer<QByteArray> tmp = QSharedPointer<QByteArray>(new QByteArray((const char*)exifBuf.pData_, exifBuf.size_));

		if (tmp->size() > qRound(ba->size()*0.5f))
			ba = tmp;
		else
			return false;	// catch exif bug - observed e.g. for hasselblad RAW (3fr) files - see: Bug #995 (http://dev.exiv2.org/issues/995)
	} else
		return false;

	exifImg = exifImgN;
	exifState = loaded;

	return true;
}

QString DkMetaDataT::getDescription() const {

	QString description;

	if (exifState != loaded && exifState != dirty)
		return description;

	try {
		Exiv2::ExifData &exifData = exifImg->exifData();

		if (!exifData.empty()) {

			Exiv2::ExifKey key = Exiv2::ExifKey("Exif.Image.ImageDescription");
			Exiv2::ExifData::iterator pos = exifData.findKey(key);

			if (pos != exifData.end() && pos->count() != 0) {

				Exiv2::Value::AutoPtr v = pos->getValue();

				description = QString::fromStdString(pos->toString());
			}
		}
	}
	catch (...) {

		qDebug() << "[DkMetaDataT] Error: could not load description";
		return description;
	}

	return description;

}

int DkMetaDataT::getOrientation() const {

	if (exifState != loaded && exifState != dirty)
		return -1;

	int orientation = -1;

	try {
		Exiv2::ExifData &exifData = exifImg->exifData();

		if (!exifData.empty()) {

			Exiv2::ExifKey key = Exiv2::ExifKey("Exif.Image.Orientation");
			Exiv2::ExifData::iterator pos = exifData.findKey(key);

			if (pos != exifData.end() && pos->count() != 0) {
			
				Exiv2::Value::AutoPtr v = pos->getValue();

				orientation = (int)pos->toFloat();

				switch (orientation) {
				case 6: orientation = 90;
					break;
				case 7: orientation = 90;
					break;
				case 3: orientation = 180;
					break;
				case 4: orientation = 180;
					break;
				case 8: orientation = -90;
					break;
				case 5: orientation = -90;
					break;
				default: orientation = 0;
					break;
				}	
			}
		}
	}
	catch(...) {
		return 0;
	}

	return orientation;
}

int DkMetaDataT::getRating() const {
	
	if (exifState != loaded && exifState != dirty)
		return -1;

	float exifRating = -1;
	float xmpRating = -1;
	float fRating = 0;

	Exiv2::ExifData &exifData = exifImg->exifData();		//Exif.Image.Rating  - short
	Exiv2::XmpData &xmpData = exifImg->xmpData();			//Xmp.xmp.Rating - text

	//get Rating of Exif Tag
	if (!exifData.empty()) {
		Exiv2::ExifKey key = Exiv2::ExifKey("Exif.Image.Rating");
		Exiv2::ExifData::iterator pos = exifData.findKey(key);

		if (pos != exifData.end() && pos->count() != 0) {
			Exiv2::Value::AutoPtr v = pos->getValue();
			exifRating = v->toFloat();
		}
	}

	//get Rating of Xmp Tag
	if (!xmpData.empty()) {
		Exiv2::XmpKey key = Exiv2::XmpKey("Xmp.xmp.Rating");
		Exiv2::XmpData::iterator pos = xmpData.findKey(key);

		//xmp Rating tag
		if (pos != xmpData.end() && pos->count() != 0) {
			Exiv2::Value::AutoPtr v = pos->getValue();
			xmpRating = v->toFloat();
		}

		//if xmpRating not found, try to find MicrosoftPhoto Rating tag
		if (xmpRating == -1) {
			key = Exiv2::XmpKey("Xmp.MicrosoftPhoto.Rating");
			pos = xmpData.findKey(key);
			if (pos != xmpData.end() && pos->count() != 0) {
				Exiv2::Value::AutoPtr v = pos->getValue();
				xmpRating = v->toFloat();
			}
		}
	}

	if (xmpRating == -1.0f && exifRating != -1.0f)
		fRating = exifRating;
	else if (xmpRating != -1.0f && exifRating == -1.0f)
		fRating = xmpRating;
	else
		fRating = exifRating;

	return fRating;
}

QString DkMetaDataT::getNativeExifValue(const QString& key) const {

	QString info;

	if (exifState != loaded && exifState != dirty)
		return info;

	Exiv2::ExifData &exifData = exifImg->exifData();

	if (!exifData.empty()) {

		Exiv2::ExifData::iterator pos;

		try {
			Exiv2::ExifKey ekey = Exiv2::ExifKey(key.toStdString());
			pos = exifData.findKey(ekey);

		} catch(...) {
			return info;
		}

		if (pos != exifData.end() && pos->count() != 0) {
			
			if (pos->count () < 2000) {	// diem: this is about performance - adobe obviously embeds whole images into tiff exiv data 

				qDebug() << "pos count: " << pos->count();
				Exiv2::Value::AutoPtr v = pos->getValue();
			
				info = QString::fromStdString(pos->toString());
			}
			else {
				info = QObject::tr("<data too large to display>");
			}
		}
			
	}

	return info;

}

QString DkMetaDataT::getXmpValue(const QString& key) const {

	QString info;

	if (exifState != loaded && exifState != dirty)
		return info;

	Exiv2::XmpData &xmpData = exifImg->xmpData();

	if (!xmpData.empty()) {

		Exiv2::XmpData::iterator pos;

		try {
			Exiv2::XmpKey ekey = Exiv2::XmpKey(key.toStdString());
			pos = xmpData.findKey(ekey);

		} catch(...) {
			return info;
		}

		if (pos != xmpData.end() && pos->count() != 0) {
			Exiv2::Value::AutoPtr v = pos->getValue();
			info = QString::fromStdString(pos->toString());
		}
	}

	return info;
}


QString DkMetaDataT::getExifValue(const QString& key) const {

	QString info;

	if (exifState != loaded && exifState != dirty)
		return info;

	Exiv2::ExifData &exifData = exifImg->exifData();
	std::string sKey = key.toStdString();

	if (!exifData.empty()) {

		Exiv2::ExifData::iterator pos;

		try {
			Exiv2::ExifKey ekey = Exiv2::ExifKey("Exif.Image." + sKey);
			pos = exifData.findKey(ekey);

			if (pos == exifData.end() || pos->count() == 0) {
				Exiv2::ExifKey ekey = Exiv2::ExifKey("Exif.Photo." + sKey);	
				pos = exifData.findKey(ekey);
			}
		} catch(...) {
			try {
				sKey = "Exif.Photo." + sKey;
				Exiv2::ExifKey ekey = Exiv2::ExifKey(sKey);	
				pos = exifData.findKey(ekey);
			} catch (... ) {
				return "";
			}
		}

		if (pos != exifData.end() && pos->count() != 0) {
			Exiv2::Value::AutoPtr v = pos->getValue();
			info = QString::fromStdString(pos->toString());
		}
	}

	return info;
}

QString DkMetaDataT::getIptcValue(const QString& key) const {

	QString info;

	if (exifState != loaded && exifState != dirty)
		return info;

	Exiv2::IptcData &iptcData = exifImg->iptcData();

	if (!iptcData.empty()) {

		Exiv2::IptcData::iterator pos;

		try {
			Exiv2::IptcKey ekey = Exiv2::IptcKey(key.toStdString());
			pos = iptcData.findKey(ekey);
		} catch (...) {
			return info;
		}

		if (pos != iptcData.end() && pos->count() != 0) {
			Exiv2::Value::AutoPtr v = pos->getValue();
			info = QString::fromStdString(pos->toString());
		}
	}

	return info;
}

void DkMetaDataT::getFileMetaData(QStringList& fileKeys, QStringList& fileValues) const {

	fileKeys.append(QObject::tr("Filename"));
	fileValues.append(file.fileName());

	fileKeys.append(QObject::tr("Path"));
	fileValues.append(file.absolutePath());

	if (file.isSymLink()) {
		fileKeys.append(QObject::tr("Target"));
		fileValues.append(file.symLinkTarget());
	}

	fileKeys.append(QObject::tr("Size"));
	fileValues.append(DkUtils::readableByte(file.size()));

	// date group
	fileKeys.append(QObject::tr("Date") + "." + QObject::tr("Created"));
	fileValues.append(file.created().toString(Qt::SystemLocaleDate));

	fileKeys.append(QObject::tr("Date") + "." + QObject::tr("Last Modified"));
	fileValues.append(file.lastModified().toString(Qt::SystemLocaleDate));

	fileKeys.append(QObject::tr("Date") + "." + QObject::tr("Last Read"));
	fileValues.append(file.lastRead().toString(Qt::SystemLocaleDate));

	if (!file.owner().isEmpty()) {
		fileKeys.append(QObject::tr("Owner"));
		fileValues.append(file.owner());
	}

	fileKeys.append(QObject::tr("OwnerID"));
	fileValues.append(QString::number(file.ownerId()));

	if (!file.group().isEmpty()) {
		fileKeys.append(QObject::tr("Group"));
		fileValues.append(file.group());
	}

	QString permissionString;
	fileKeys.append(QObject::tr("Permissions") + "." + QObject::tr("Owner"));
	permissionString += file.permissions() & QFile::ReadOwner	? "r" : "-";
	permissionString += file.permissions() & QFile::WriteOwner	? "w" : "-";
	permissionString += file.permissions() & QFile::ExeOwner	? "x" : "-";
	fileValues.append(permissionString);

	permissionString = "";
	fileKeys.append(QObject::tr("Permissions") + "." + QObject::tr("User"));
	permissionString += file.permissions() & QFile::ReadUser	? "r" : "-";
	permissionString += file.permissions() & QFile::WriteUser	? "w" : "-";
	permissionString += file.permissions() & QFile::ExeUser		? "x" : "-";
	fileValues.append(permissionString);

	permissionString = "";
	fileKeys.append(QObject::tr("Permissions") + "." + QObject::tr("Group"));
	permissionString += file.permissions() & QFile::ReadGroup	? "r" : "-";
	permissionString += file.permissions() & QFile::WriteGroup	? "w" : "-";
	permissionString += file.permissions() & QFile::ExeGroup	? "x" : "-";
	fileValues.append(permissionString);

	permissionString = "";
	fileKeys.append(QObject::tr("Permissions") + "." + QObject::tr("Other"));
	permissionString += file.permissions() & QFile::ReadOther	? "r" : "-";
	permissionString += file.permissions() & QFile::WriteOther	? "w" : "-";
	permissionString += file.permissions() & QFile::ExeOther	? "x" : "-";
	fileValues.append(permissionString);

	QStringList tmpKeys;

	// full file keys are needed to create the hierarchy
	for (int idx = 0; idx < fileKeys.size(); idx++) {
		tmpKeys.append(QObject::tr("File") + "." + fileKeys.at(idx));
	}

	fileKeys = tmpKeys;
}

QImage DkMetaDataT::getThumbnail() const {

	QImage qThumb;

	if (exifState != loaded && exifState != dirty)
		return qThumb;

	Exiv2::ExifData &exifData = exifImg->exifData();

	if (exifData.empty())
		return qThumb;

	try {
		Exiv2::ExifThumb thumb(exifData);
		Exiv2::DataBuf buffer = thumb.copy();
		// ok, get the buffer...
		std::pair<Exiv2::byte*, long> stdBuf = buffer.release();
		QByteArray ba = QByteArray((char*)stdBuf.first, (int)stdBuf.second);
		qThumb.loadFromData(ba);

		delete stdBuf.first;
	}
	catch (...) {
		qDebug() << "Sorry, I could not load the thumb from the exif data...";
	}

	return qThumb;
}

QImage DkMetaDataT::getPreviewImage(int minPreviewWidth) const {

	QImage qImg;

	if (exifState != loaded && exifState != dirty)
		return qImg;

	Exiv2::ExifData &exifData = exifImg->exifData();

	if (exifData.empty())
		return qImg;

	try {

		Exiv2::PreviewManager loader(*exifImg);
		Exiv2::PreviewPropertiesList pList = loader.getPreviewProperties();

		int maxWidth = 0;
		int mIdx = -1;

		// select the largest preview image
		for (size_t idx = 0; idx < pList.size(); idx++) {
			
			if (pList[idx].width_ > (uint32_t)maxWidth && pList[idx].width_ > (uint32_t)minPreviewWidth) {
				mIdx = idx;
				maxWidth = pList[idx].width_;
			}
		}

		if (mIdx == -1)
			return qImg;
		
		// Get the selected preview image
		Exiv2::PreviewImage preview = loader.getPreviewImage(pList[mIdx]);

		QByteArray ba((const char*)preview.pData(), preview.size());
		if (!qImg.loadFromData(ba))
			return QImage();
	}
	catch (...) {
		qDebug() << "Sorry, I could not load the thumb from the exif data...";
	}

	return qImg;
}


bool DkMetaDataT::hasMetaData() const {

	return !(exifState == no_data || exifState == not_loaded);
}

bool DkMetaDataT::isLoaded() const {

	return exifState == loaded || exifState == dirty;
}

bool DkMetaDataT::isTiff() const {

	QString newSuffix = file.suffix();
	return newSuffix.contains(QRegExp("(tif|tiff)", Qt::CaseInsensitive));
}

bool DkMetaDataT::isJpg() const {

	QString newSuffix = file.suffix();
	return newSuffix.contains(QRegExp("(jpg|jpeg)", Qt::CaseInsensitive));
}

bool DkMetaDataT::isRaw() const {

	QString newSuffix = file.suffix();
	return newSuffix.contains(QRegExp("(nef|crw|cr2|arw)", Qt::CaseInsensitive));
}

bool DkMetaDataT::isDirty() const {

	return exifState == dirty;
}

QStringList DkMetaDataT::getExifKeys() const {

	QStringList exifKeys;

	if (exifState != loaded && exifState != dirty)
		return exifKeys;

	Exiv2::ExifData &exifData = exifImg->exifData();
	Exiv2::ExifData::const_iterator end = exifData.end();

	if (exifData.empty()) {
		return exifKeys;

	} else {

		for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {

			std::string tmp = i->key();
			exifKeys << QString::fromStdString(tmp);

			//qDebug() << QString::fromStdString(tmp);
		}
	}

	return exifKeys;
}

QStringList DkMetaDataT::getXmpKeys() const {

	QStringList xmpKeys;

	if (exifState != loaded && exifState != dirty)
		return xmpKeys;

	Exiv2::XmpData &xmpData = exifImg->xmpData();
	Exiv2::XmpData::const_iterator end = xmpData.end();

	if (xmpData.empty()) {
		return xmpKeys;

	} else {

		for (Exiv2::XmpData::const_iterator i = xmpData.begin(); i != end; ++i) {

			std::string tmp = i->key();
			xmpKeys << QString::fromStdString(tmp);
		}
	}

	return xmpKeys;
}


QStringList DkMetaDataT::getExifValues() const {

	QStringList exifValues;

	if (exifState != loaded && exifState != dirty)
		return QStringList();

	Exiv2::ExifData &exifData = exifImg->exifData();
	Exiv2::ExifData::const_iterator end = exifData.end();

	if (exifData.empty())
		return exifValues;

	for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {

		std::string tmp = i->value().toString();
		exifValues << QString::fromStdString(tmp);
	}

	return exifValues;
}

QStringList DkMetaDataT::getIptcKeys() const {

	QStringList iptcKeys;
	
	if (exifState != loaded && exifState != dirty)
		return iptcKeys;

	Exiv2::IptcData &iptcData = exifImg->iptcData();
	Exiv2::IptcData::iterator endI = iptcData.end();

	if (iptcData.empty())
		return iptcKeys;

	for (Exiv2::IptcData::iterator md = iptcData.begin(); md != endI; ++md) {

		std::string tmp = md->key();
		iptcKeys << QString::fromStdString(tmp);
	}

	return iptcKeys;
}

QStringList DkMetaDataT::getIptcValues() const {
	
	QStringList iptcValues;

	if (exifState != loaded && exifState != dirty)
		return iptcValues;

	Exiv2::IptcData &iptcData = exifImg->iptcData();
	Exiv2::IptcData::iterator endI = iptcData.end();

	if (iptcData.empty())
		return iptcValues;
	for (Exiv2::IptcData::iterator md = iptcData.begin(); md != endI; ++md) {

		std::string tmp = md->value().toString();
		iptcValues << QString::fromStdString(tmp);
	}

	return iptcValues;
}

void DkMetaDataT::setThumbnail(QImage thumb) {

	if (exifState == not_loaded || exifState == no_data) 
		return;

	try {
		Exiv2::ExifData exifData = exifImg->exifData();

		if (exifData.empty())
			exifData = Exiv2::ExifData();

		// ok, let's try to save the thumbnail...
		Exiv2::ExifThumb eThumb(exifData);

		QByteArray data;
		QBuffer buffer(&data);
		buffer.open(QIODevice::WriteOnly);
		thumb.save(&buffer, "JPEG");	// here we destroy the alpha channel of thumbnails

		try {
			// whipe all exif data of the thumbnail
			Exiv2::MemIo::AutoPtr exifBufferThumb(new Exiv2::MemIo((const byte*)data.constData(), data.size()));
			Exiv2::Image::AutoPtr exifImgThumb = Exiv2::ImageFactory::open(exifBufferThumb);

			if (exifImgThumb.get() != 0 && exifImgThumb->good())
				exifImgThumb->clearExifData();
		}
		catch (...) {
			qDebug() << "could not clear the thumbnail exif info";
		}

		eThumb.erase();	// erase all thumbnails
		eThumb.setJpegThumbnail((Exiv2::byte *)data.data(), data.size());

		exifImg->setExifData(exifData);
		exifState = dirty;

	} catch (...) {
		qDebug() << "I could not save the thumbnail...";
	}
}

QVector2D DkMetaDataT::getResolution() const {


	QVector2D resV = QVector2D(72,72);
	QString xRes, yRes;

	try {

		if (hasMetaData()) {
			//metaData = DkImageLoader::imgMetaData;
			xRes = getExifValue("XResolution");
			QStringList res;
			res = xRes.split("/");
			if (res.size() != 2) {
				//throw DkException("no x resolution found\n");
				return resV;
			}
			resV.setX(res.at(1).toFloat() != 0 ? res.at(0).toFloat()/res.at(1).toFloat() : 72);

			yRes = getExifValue("YResolution");
			res = yRes.split("/");

			qDebug() << "Resolution"  << xRes << " " << yRes;
			if (res.size() != 2)
				return resV;
				//throw DkException("no y resolution found\n");
			resV.setY(res.at(1).toFloat() != 0 ? res.at(0).toFloat()/res.at(1).toFloat() : 72);
		}
	} catch (...) {
		qDebug() << "could not load Exif resolution, set to 72dpi";
	}

	return resV;
}

void DkMetaDataT::setResolution(const QVector2D& res) {

	QString x,y;
	x.setNum(res.x());
	y.setNum(res.y());
	x=x+"/1";
	y=y+"/1";

	setExifValue("Exif.Image.XResolution",x);
	setExifValue("Exif.Image.YResolution",y);
}

void DkMetaDataT::clearOrientation() {

	if (exifState == not_loaded || exifState == no_data)
		return;

	setExifValue("Exif.Image.Orientation", "0");
}

void DkMetaDataT::setOrientation(int o) {

	if (exifState == not_loaded || exifState == no_data)
		return;

	if (o!=90 && o!=-90 && o!=180 && o!=0 && o!=270)
		return;

	if (o==-180) o=180;
	if (o== 270) o=-90;

	int orientation;

	Exiv2::ExifData& exifData = exifImg->exifData();
	Exiv2::ExifKey key = Exiv2::ExifKey("Exif.Image.Orientation");

	// this does not really work -> *.bmp images
	if (exifData.empty())
		exifData["Exif.Image.Orientation"] = uint16_t(1);

	Exiv2::ExifData::iterator pos = exifData.findKey(key);

	if (pos == exifData.end() || pos->count() == 0) {
		exifData["Exif.Image.Orientation"] = uint16_t(1);

		pos = exifData.findKey(key);
	}

	Exiv2::Value::AutoPtr v = pos->getValue();
	Exiv2::UShortValue* prv = dynamic_cast<Exiv2::UShortValue*>(v.release());
	if (!prv)	return;

	Exiv2::UShortValue::AutoPtr rv = Exiv2::UShortValue::AutoPtr(prv);
	if (rv->value_.empty())	return;

	orientation = (int) rv->value_[0];
	if (orientation <= 0 || orientation > 8) orientation = 1;

	switch (orientation) {
	case 1: if (o!=0) orientation = (o == -90) ? 8 : (o==90 ? 6 : 3);
		break;
	case 2: if (o!=0) orientation = (o == -90) ? 5 : (o==90 ? 7 : 4);
		break;
	case 3: if (o!=0) orientation = (o == -90) ? 6 : (o==90 ? 8 : 1);
		break;
	case 4: if (o!=0) orientation = (o == -90) ? 7 : (o==90 ? 5 : 2);
		break;
	case 5: if (o!=0) orientation = (o == -90) ? 4 : (o==90 ? 2 : 7);
		break;
	case 6: if (o!=0) orientation = (o == -90) ? 1 : (o==90 ? 3 : 8);
		break;
	case 7: if (o!=0) orientation = (o == -90) ? 2 : (o==90 ? 4 : 5);
		break;
	case 8: if (o!=0) orientation = (o == -90) ? 3 : (o==90 ? 1 : 6);
		break;
	}
	rv->value_[0] = (unsigned short) orientation;
	pos->setValue(rv.get());

	exifImg->setExifData(exifData);

	exifState = dirty;
}

bool DkMetaDataT::setDescription(const QString& description) {

	if (exifState == not_loaded || exifState == no_data)
		return false;

	return setExifValue("Exif.Image.ImageDescription", description);
}

void DkMetaDataT::setRating(int r) {

	if (exifState == not_loaded || exifState == no_data || getRating() == r)
		return;

	unsigned short percentRating = 0;
	std::string sRating, sRatingPercent;

	if (r == 5)  { percentRating = 99; sRating = "5"; sRatingPercent = "99";}
	else if (r==4) { percentRating = 75; sRating = "4"; sRatingPercent = "75";}
	else if (r==3) { percentRating = 50; sRating = "3"; sRatingPercent = "50";}
	else if (r==2) { percentRating = 25; sRating = "2"; sRatingPercent = "25";}
	else if (r==1) {percentRating = 1; sRating = "1"; sRatingPercent = "1";}
	else {r=0;}

	Exiv2::ExifData &exifData = exifImg->exifData();		//Exif.Image.Rating  - short
	Exiv2::XmpData &xmpData = exifImg->xmpData();			//Xmp.xmp.Rating - text

	if (r>0) {
		exifData["Exif.Image.Rating"] = uint16_t(r);
		exifData["Exif.Image.RatingPercent"] = uint16_t(r);

		Exiv2::Value::AutoPtr v = Exiv2::Value::create(Exiv2::xmpText);
		v->read(sRating);
		xmpData.add(Exiv2::XmpKey("Xmp.xmp.Rating"), v.get());
		v->read(sRatingPercent);
		xmpData.add(Exiv2::XmpKey("Xmp.MicrosoftPhoto.Rating"), v.get());
	} 
	else {

		Exiv2::ExifKey key = Exiv2::ExifKey("Exif.Image.Rating");
		Exiv2::ExifData::iterator pos = exifData.findKey(key);
		if (pos != exifData.end()) exifData.erase(pos);

		key = Exiv2::ExifKey("Exif.Image.RatingPercent");
		pos = exifData.findKey(key);
		if (pos != exifData.end()) exifData.erase(pos);

		Exiv2::XmpKey key2 = Exiv2::XmpKey("Xmp.xmp.Rating");
		Exiv2::XmpData::iterator pos2 = xmpData.findKey(key2);
		if (pos2 != xmpData.end()) xmpData.erase(pos2);

		key2 = Exiv2::XmpKey("Xmp.MicrosoftPhoto.Rating");
		pos2 = xmpData.findKey(key2);
		if (pos2 != xmpData.end()) xmpData.erase(pos2);
	}

	exifImg->setExifData(exifData);
	exifImg->setXmpData(xmpData);

	exifState = dirty;
}

bool DkMetaDataT::setExifValue(QString key, QString taginfo) {

	if (exifState == not_loaded || exifState == no_data)
		return false;

	Exiv2::ExifData &exifData = exifImg->exifData();

	bool setExifSuccessfull = false;

	if (!exifData.empty() && getExifKeys().contains(key)) {

		Exiv2::Exifdatum& tag = exifData[key.toStdString()];

		if (!tag.setValue(taginfo.toStdString())) {
			exifState = dirty;
			setExifSuccessfull = true;
		}
	}
	else if (!exifData.empty()) {

		Exiv2::ExifKey exivKey(key.toStdString());
		Exiv2::Exifdatum tag(exivKey);
		if (!tag.setValue(taginfo.toStdString())) {
			exifState = dirty;
			setExifSuccessfull = true;
		}

		exifData.add(tag);
	}

	return setExifSuccessfull;
}

void DkMetaDataT::printMetaData() const {

	if (exifState != loaded && exifState != dirty)
		return;

	Exiv2::ExifData &exifData = exifImg->exifData();
	Exiv2::IptcData &iptcData = exifImg->iptcData();
	Exiv2::XmpData &xmpData = exifImg->xmpData();

	qDebug() << "Exif------------------------------------------------------------------";

	QStringList exifKeys = getExifKeys();

	for (int idx = 0; idx < exifKeys.size(); idx++)
		qDebug() << exifKeys.at(idx) << " is " << getNativeExifValue(exifKeys.at(idx));

	qDebug() << "IPTC------------------------------------------------------------------";

	Exiv2::IptcData::iterator endI2 = iptcData.end();
	QStringList iptcKeys = getIptcKeys();

	for (int idx = 0; idx < iptcKeys.size(); idx++)
		qDebug() << iptcKeys.at(idx) << " is " << getIptcValue(iptcKeys.at(idx));

	qDebug() << "XMP------------------------------------------------------------------";

	Exiv2::XmpData::iterator endI3 = xmpData.end();
	for (Exiv2::XmpData::iterator md = xmpData.begin(); md != endI3; ++md) {
		std::cout << std::setw(44) << std::setfill(' ') << std::left
			<< md->key() << " "
			<< "0x" << std::setw(4) << std::setfill('0') << std::right
			<< std::hex << md->tag() << " "
			<< std::setw(9) << std::setfill(' ') << std::left
			<< md->typeName() << " "
			<< std::dec << std::setw(3)
			<< std::setfill(' ') << std::right
			<< md->count() << "  "
			<< std::dec << md->value()
			<< std::endl;
	}

}

// DkMetaDataHelper --------------------------------------------------------------------

void DkMetaDataHelper::init() {

	camSearchTags.append("ImageSize");
	camSearchTags.append("Orientation");
	camSearchTags.append("Make");
	camSearchTags.append("Model");
	camSearchTags.append("ApertureValue");
	camSearchTags.append("ISOSpeedRatings");
	camSearchTags.append("Flash");
	camSearchTags.append("FocalLength");
	camSearchTags.append("ExposureMode");
	camSearchTags.append("ExposureTime");

	descSearchTags.append("Rating");
	descSearchTags.append("UserComment");
	descSearchTags.append("DateTime");
	descSearchTags.append("DateTimeOriginal");
	descSearchTags.append("ImageDescription");
	descSearchTags.append("Byline");
	descSearchTags.append("BylineTitle");
	descSearchTags.append("City");
	descSearchTags.append("Country");
	descSearchTags.append("Headline");
	descSearchTags.append("Caption");
	descSearchTags.append("CopyRight");
	descSearchTags.append("Keywords");
	descSearchTags.append("Path");
	descSearchTags.append("FileSize");


	for (int i = 0; i  < DkSettings::scamDataDesc.size(); i++) 
		translatedCamTags << qApp->translate("nmc::DkMetaData", DkSettings::scamDataDesc.at(i).toLatin1());

	for (int i = 0; i  < DkSettings::sdescriptionDesc.size(); i++)
		translatedDescTags << qApp->translate("nmc::DkMetaData", DkSettings::sdescriptionDesc.at(i).toLatin1());

	if (translatedCamTags.size() != DkSettings::camData_end)
		qDebug() << "wrong definition of Camera Data (Exif). Size of CamData tags is different from enum";
	if (translatedDescTags.size() != DkSettings::desc_end - DkSettings::camData_end)
		qDebug() << "wrong definition of Description Data (Exif). Size of Descriptions tags is different from enum";

	exposureModes.append(QObject::tr("not defined"));
	exposureModes.append(QObject::tr("manual"));
	exposureModes.append(QObject::tr("normal"));
	exposureModes.append(QObject::tr("aperture priority"));
	exposureModes.append(QObject::tr("shutter priority"));
	exposureModes.append(QObject::tr("program creative"));
	exposureModes.append(QObject::tr("high-speed program"));
	exposureModes.append(QObject::tr("porQObject::trait mode"));
	exposureModes.append(QObject::tr("landscape mode"));

	// flash mapping is taken from: http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/EXIF.html#Flash
	flashModes.insert(0x0, QObject::tr("No Flash"));
	flashModes.insert(0x1, QObject::tr("Fired"));
	flashModes.insert(0x5, QObject::tr("Fired, Return not detected"));
	flashModes.insert(0x7, QObject::tr("Fired, Return detected"));
	flashModes.insert(0x8, QObject::tr("On, Did not fire"));
	flashModes.insert(0x9, QObject::tr("On, Fired"));
	flashModes.insert(0xd, QObject::tr("On, Return not detected"));
	flashModes.insert(0xf, QObject::tr("On, Return detected"));
	flashModes.insert(0x10, QObject::tr("Off, Did not fire"));
	flashModes.insert(0x14, QObject::tr("Off, Did not fire, Return not detected"));
	flashModes.insert(0x18, QObject::tr("Auto, Did not fire"));
	flashModes.insert(0x19, QObject::tr("Auto, Fired"));
	flashModes.insert(0x1d, QObject::tr("Auto, Fired, Return not detected"));
	flashModes.insert(0x1f, QObject::tr("Auto, Fired, Return detected"));
	flashModes.insert(0x20, QObject::tr("No flash function"));
	flashModes.insert(0x30, QObject::tr("Off, No flash function"));
	flashModes.insert(0x41, QObject::tr("Fired, Red-eye reduction"));
	flashModes.insert(0x45, QObject::tr("Fired, Red-eye reduction, Return not detected"));
	flashModes.insert(0x47, QObject::tr("Fired, Red-eye reduction, Return detected"));
	flashModes.insert(0x49, QObject::tr("On, Red-eye reduction"));
	flashModes.insert(0x4d, QObject::tr("On, Red-eye reduction, Return not detected"));
	flashModes.insert(0x4f, QObject::tr("On, Red-eye reduction, Return detected"));
	flashModes.insert(0x50, QObject::tr("Off, Red-eye reduction"));
	flashModes.insert(0x58, QObject::tr("Auto, Did not fire, Red-eye reduction"));
	flashModes.insert(0x59, QObject::tr("Auto, Fired, Red-eye reduction"));
	flashModes.insert(0x5d, QObject::tr("Auto, Fired, Red-eye reduction, Return not detected"));
	flashModes.insert(0x5f, QObject::tr("Auto, Fired, Red-eye reduction, Return detected"));
}

QString DkMetaDataHelper::getApertureValue(QSharedPointer<DkMetaDataT> metaData) const {

	QString key = camSearchTags.at(DkSettings::camData_aperture); 

	QString value = metaData->getExifValue(key);
	if (value.isEmpty()) value = metaData->getExifValue("FNumber");	// try alternative tag

	QStringList sList = value.split('/');

	if (sList.size() == 2) {
		double val = std::pow(1.4142, sList[0].toDouble()/sList[1].toDouble());	// see the exif documentation (e.g. http://www.media.mit.edu/pia/Research/deepview/exif.html)
		value = QString::fromStdString(DkUtils::stringify(val,1));
	}

	return value;
}

QString DkMetaDataHelper::getFocalLength(QSharedPointer<DkMetaDataT> metaData) const {

	// focal length
	QString key = camSearchTags.at(DkSettings::camData_focallength);

	QString value = metaData->getExifValue(key);
	QStringList sList = value.split('/');

	if (sList.size() == 2) {
		double val = sList[0].toDouble()/sList[1].toDouble();
		value = QString::fromStdString(DkUtils::stringify(val,1)) + " mm";
	}

	return value;
}

QString DkMetaDataHelper::getExposureTime(QSharedPointer<DkMetaDataT> metaData) const {

	QString key = camSearchTags.at(DkSettings::camData_exposuretime);
	QString value = metaData->getExifValue(key);
	QStringList sList = value.split('/');

	if (sList.size() == 2) {
		int nom = sList[0].toInt();		// nominator
		int denom = sList[1].toInt();	// denominator

		// if exposure time is less than a second -> compute the gcd for nice values (1/500 instead of 2/1000)
		if (nom <= denom) {
			int gcd = DkMath::gcd(denom, nom);
			value = QString::number(nom/gcd) % QString("/") % QString::number(denom/gcd);
		}
		else
			value = QString::fromStdString(DkUtils::stringify((float)nom/(float)denom,1));

		value += " sec";
	}

	return value;
}

QString DkMetaDataHelper::getExposureMode(QSharedPointer<DkMetaDataT> metaData) const {

	QString key = camSearchTags.at(DkSettings::camData_exposuremode);
	QString value = metaData->getExifValue(key);
	int mode = value.toInt();

	if (mode >= 0 && mode < exposureModes.size())
		value = exposureModes[mode];

	return value;
}

QString DkMetaDataHelper::getFlashMode(QSharedPointer<DkMetaDataT> metaData) const {

	QString key = camSearchTags.at(DkSettings::camData_exposuremode);
	QString value = metaData->getExifValue(key);
	unsigned int mode = value.toUInt();
	value = flashModes[mode];

	return value;
}

QString DkMetaDataHelper::getGpsCoordinates(QSharedPointer<DkMetaDataT> metaData) const {

	QString Lat, LatRef, Lon, LonRef, gpsInfo;
	QStringList help;

	try {

		if (metaData->hasMetaData()) {
			//metaData = DkImageLoader::imgMetaData;
			Lat = metaData->getNativeExifValue("Exif.GPSInfo.GPSLatitude");
			LatRef = metaData->getNativeExifValue("Exif.GPSInfo.GPSLatitudeRef");
			Lon = metaData->getNativeExifValue("Exif.GPSInfo.GPSLongitude");
			LonRef = metaData->getNativeExifValue("Exif.GPSInfo.GPSLongitudeRef");
			//example url
			//http://maps.google.at/maps?q=N+48°+8'+31.940001''+E16°+15'+35.009998''

			gpsInfo = "http://maps.google.at/maps?q=" + LatRef + "+";

			help = Lat.split(" ");
			for (int i=0; i<help.size(); ++i) {
				float val1, val2;
				QString valS;
				QStringList coordP;
				valS = help.at(i);
				coordP = valS.split("/");
				if (coordP.size() != 2)
					throw DkException(QObject::tr("could not parse GPS Data").toStdString());

				val1 = coordP.at(0).toFloat();
				val2 = coordP.at(1).toFloat();
				val1 = val2 != 0 ? val1/val2 : val1;

				if (i==0) {
					valS.setNum((int)val1);
					gpsInfo += valS + "°";
				}
				if (i==1) {
					if (val2 > 1)							
						valS.setNum(val1, 'f', 6);
					else
						valS.setNum((int)val1);
					gpsInfo += "+" + valS + "'";
				}
				if (i==2) {
					if (val1 != 0) {
						valS.setNum(val1, 'f', 6);
						gpsInfo += "+" + valS + "''";
					}
				}
			}

			gpsInfo += "+" + LonRef;
			help = Lon.split(" ");
			for (int i=0; i<help.size(); ++i) {
				float val1, val2;
				QString valS;
				QStringList coordP;
				valS = help.at(i);
				coordP = valS.split("/");
				if (coordP.size() != 2)
					throw DkException(QObject::tr("could not parse GPS Data").toStdString());

				val1 = coordP.at(0).toFloat();
				val2 = coordP.at(1).toFloat();
				val1 = val2 != 0 ? val1/val2 : val1;

				if (i==0) {
					valS.setNum((int)val1);
					gpsInfo += valS + "°";
					//gpsInfo += valS + QString::fromUtf16((ushort*)"0xb0");//QChar('°');
					//gpsInfo += valS + QString::setUnicode("0xb0");//QChar('°');
				}
				if (i==1) {
					if (val2 > 1)							
						valS.setNum(val1, 'f', 6);
					else
						valS.setNum((int)val1);
					gpsInfo += "+" + valS + "'";
				}
				if (i==2) {
					if (val1 != 0) {
						valS.setNum(val1, 'f', 6);
						gpsInfo += "+" + valS + "''";
					}
				}
			}

		}

	} catch (...) {
		gpsInfo = "";
		//qDebug() << "could not load Exif GPS information";
	}

	return gpsInfo;
}

QString DkMetaDataHelper::translateKey(const QString& key) const {

	QString translatedKey = key;

	int keyIdx = camSearchTags.indexOf(key);
	if (keyIdx != -1)
		translatedKey = translatedCamTags.at(keyIdx);

	keyIdx = descSearchTags.indexOf(key);
	if (keyIdx != -1)
		translatedKey = translatedDescTags.at(keyIdx);

	return translatedKey;
}

QString DkMetaDataHelper::resolveSpecialValue(QSharedPointer<DkMetaDataT> metaData, const QString& key, const QString& value) const {

	QString rValue = value;

	if (key == camSearchTags[DkSettings::camData_aperture]) {
		rValue = DkMetaDataHelper::getInstance().getApertureValue(metaData);
	}
	else if (key == camSearchTags[DkSettings::camData_focallength]) {
		rValue = DkMetaDataHelper::getInstance().getFocalLength(metaData);
	}
	else if (key == camSearchTags[DkSettings::camData_exposuretime]) {
		rValue = DkMetaDataHelper::getInstance().getExposureTime(metaData);
	}
	else if (key == camSearchTags[DkSettings::camData_exposuremode]) {
		rValue = DkMetaDataHelper::getInstance().getExposureMode(metaData);						
	} 
	else if (key == camSearchTags[DkSettings::camData_flash]) {
		rValue = DkMetaDataHelper::getInstance().getFlashMode(metaData);
	}

	return rValue;
}

bool DkMetaDataHelper::hasGPS(QSharedPointer<DkMetaDataT> metaData) const {

	return !getGpsCoordinates(metaData).isEmpty();
}

QStringList DkMetaDataHelper::getCamSearchTags() const {

	return camSearchTags;
}

QStringList DkMetaDataHelper::getDescSearchTags() const {

	return descSearchTags;
}

QStringList DkMetaDataHelper::getTranslatedCamTags() const {

	return translatedCamTags;
}

QStringList DkMetaDataHelper::getTranslatedDescTags() const {

	return translatedDescTags;
}

QStringList DkMetaDataHelper::getAllExposureModes() const {

	return exposureModes;
}

QMap<int, QString> DkMetaDataHelper::getAllFlashModes() const {

	return flashModes;
}

}
