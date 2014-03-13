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
			// TODO: for now we don't support unicode filenames for exif data
			// however we could if: we load the file as a buffer and provide this buffer as *byte to exif
			// this is more work and should be done when updating the cacher as we should definitely
			// not load the image twice...
#ifdef EXV_UNICODE_PATH
#if QT_VERSION < 0x050000
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
	bool saved = saveMetaData(ba, force);
	if (!saved)
		return saved;
	
	file.open(QFile::WriteOnly);
	file.write(ba->data());
	file.close();

	qDebug() << "[Exiv2] old save metadata used!";

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
	if (exifBuf.pData_)
		ba = QSharedPointer<QByteArray>(new QByteArray((const char*)exifBuf.pData_, exifBuf.size_));
	else
		return false;

	exifImg = exifImgN;
	exifState = loaded;

	return true;
}

int DkMetaDataT::getOrientation() const {

	if (exifState != loaded && exifState != dirty)
		return -1;

	int orientation = -1;

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
	}
	catch (...) {
		qDebug() << "Sorry, I could not load the thumb from the exif data...";
	}

	return qThumb;
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

	Exiv2::ExifData exifData = exifImg->exifData();

	if (exifData.empty())
		exifData = Exiv2::ExifData();

	// ok, let's try to save the thumbnail...
	try {
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

	setExifValue("orientation", "0");
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

	// this try is a fast fix -> if the image does not support exiv data -> an exception is raised here -> tell the loader to save the orientated matrix
	exifImg->setExifData(exifData);
	exifState = dirty;
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

void DkMetaDataT::setExifValue(QString key, QString taginfo) {

	if (exifState == not_loaded || exifState == no_data)
		return;

	Exiv2::ExifData &exifData = exifImg->exifData();

	if (!exifData.empty() || !getExifKeys().contains(key)) {

		Exiv2::Exifdatum& tag = exifData[key.toStdString()];

		if (tag.setValue(taginfo.toStdString()))
			exifState = dirty;
	}
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

}
