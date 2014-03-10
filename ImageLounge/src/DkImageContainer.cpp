/*******************************************************************************************************
 DkImageContainer.cpp
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

#include "DkImageContainer.h"

namespace nmc {

// DkImageContainer --------------------------------------------------------------------
DkImageContainer::DkImageContainer(const QFileInfo& fileInfo) {

	this->fileInfo = fileInfo;
	this->loader = QSharedPointer<DkBasicLoader>(new DkBasicLoader());
	this->fileBuffer = QSharedPointer<QByteArray>(new QByteArray());

	init();
}

void DkImageContainer::init() {

	
	// always keep in mind that a file does not exist
	if (!edited && loadState != exists_not)
		loadState = not_loaded;

	edited = false;

}

bool DkImageContainer::operator ==(const DkImageContainer& ric) const {
	return fileInfo.absoluteFilePath() == ric.file().absoluteFilePath();
}

//bool operator==(const DkImageContainer& lic, const DkImageContainer& ric) {
//
//	return lic.file().absoluteFilePath() == ric.file().absoluteFilePath();
//}

bool DkImageContainer::operator<=(const DkImageContainer& o) const {

	if (*this == o)
		return true;

	return *this < o;
}

bool DkImageContainer::operator<(const DkImageContainer& o) const {

	return imageContainerLessThan(*this, o);
}

bool imageContainerLessThanPtr(const QSharedPointer<DkImageContainer> l, const QSharedPointer<DkImageContainer> r) {

	if (!l || !r)
		return false;

	return imageContainerLessThan(*l, *r);
}

bool imageContainerLessThan(const DkImageContainer& l, const DkImageContainer& r) {

	switch(DkSettings::global.sortMode) {

	case DkSettings::sort_filename:

		if (DkSettings::global.sortDir == DkSettings::sort_ascending)
			return DkUtils::compFilename(l.file(), r.file());
		else
			return DkUtils::compFilenameInv(l.file(), r.file());
		break;

	case DkSettings::sort_date_created:
		if (DkSettings::global.sortDir == DkSettings::sort_ascending)
			return DkUtils::compDateCreated(l.file(), r.file());
		else
			return DkUtils::compDateCreatedInv(l.file(), r.file());
		break;

	case DkSettings::sort_date_modified:
		if (DkSettings::global.sortDir == DkSettings::sort_ascending)
			return DkUtils::compDateModified(l.file(), r.file());
		else
			return DkUtils::compDateModifiedInv(l.file(), r.file());

	case DkSettings::sort_random:
		return DkUtils::compRandom(l.file(), r.file());

	default:
		// filename
		return DkUtils::compFilename(l.file(), r.file());
	}
	
}

void DkImageContainer::clear() {

	//if (edited) // trigger gui question

	if (imgLoaded() == loading || imgLoaded() == loading_canceled) {
		qDebug() << "[DkImageContainer] " << fileInfo.fileName() << " cleared...";
		return;
	}

	//saveMetaData();
	loader->release();
	fileBuffer->clear();
	init();
	qDebug() << "[DkImageContainer] " << fileInfo.fileName() << " cleared...";
}

QFileInfo DkImageContainer::file() const {

	return fileInfo;
}

bool DkImageContainer::exists() {

	fileInfo.refresh();
	return fileInfo.exists();
}

QString DkImageContainer::getTitleAttribute() const {

	if (loader->getNumPages() <= 1)
		return QString();

	QString attr = "[" + QString::number(loader->getPageIdx()) + "/" + 
		QString::number(loader->getNumPages()) + "]";

	return attr;
}

QSharedPointer<DkBasicLoader> DkImageContainer::getLoader() const {

	return loader;
}

QSharedPointer<DkMetaDataT> DkImageContainer::getMetaData() const {

	return loader->getMetaData();
}

QSharedPointer<DkThumbNailT> DkImageContainer::getThumb() const {

	return thumb;
}

float DkImageContainer::getMemoryUsage() const {

	float memSize = fileBuffer->size()/(1024.0f*1024.0f);
	memSize += DkImage::getBufferSizeFloat(loader->image().size(), loader->image().depth());

	return memSize;
}

float DkImageContainer::getFileSize() const {

	return fileInfo.size()/(1024.0f*1024.0f);
}


QImage DkImageContainer::image() {

	if (loader->image().isNull() && hasImage() == not_loaded)
		loadImage();

	return loader->image();
}

void DkImageContainer::setImage(const QImage& img, const QFileInfo& fileInfo) {

	this->fileInfo = fileInfo;
	loader->setImage(img, fileInfo);
	edited = true;
}

bool DkImageContainer::hasImage() const {

	return loader->hasImage();
}

int DkImageContainer::imgLoaded() const {

	return loadState;
}

bool DkImageContainer::loadImage() {


	if (fileBuffer->isEmpty())
		fileBuffer = loadFileToBuffer(fileInfo);

	loader = loadImageIntern(fileInfo, fileBuffer);

	return loader->hasImage();
}

QSharedPointer<QByteArray> DkImageContainer::loadFileToBuffer(const QFileInfo fileInfo) {

	QFile file(fileInfo.absoluteFilePath());
	file.open(QIODevice::ReadOnly);

	QSharedPointer<QByteArray> ba(new QByteArray(file.readAll()));
	file.close();

	return ba;
}


QSharedPointer<DkBasicLoader> DkImageContainer::loadImageIntern(const QFileInfo fileInfo, const QSharedPointer<QByteArray> fileBuffer) {

	// checks performed so load the file
	//QSharedPointer<DkBasicLoader> basicLoader(new DkBasicLoader());
	
	try {
		loader->loadGeneral(fileInfo, fileBuffer, true);
	} catch(...) {}

	return loader;
}

QFileInfo DkImageContainer::saveImageIntern(const QFileInfo fileInfo, QImage saveImg, int compression) {

	return loader->save(fileInfo, saveImg, compression);
}

void DkImageContainer::saveMetaData() {

	saveMetaDataIntern(fileInfo, fileBuffer);
}


void DkImageContainer::saveMetaDataIntern(const QFileInfo& fileInfo, QSharedPointer<QByteArray> fileBuffer) {

	loader->saveMetaData(fileInfo, fileBuffer);
}

bool DkImageContainer::isEdited() const {

	return edited;
}

bool DkImageContainer::setPageIdx(int skipIdx) {

	return loader->setPageIdx(skipIdx);
}


// DkImageContainerT --------------------------------------------------------------------
DkImageContainerT::DkImageContainerT(const QFileInfo& file) : DkImageContainer(file) {

	thumb = QSharedPointer<DkThumbNailT>(new DkThumbNailT(file));
	fetchingImage = false;
	fetchingBuffer = false;
	connect(&saveImageWatcher, SIGNAL(finished()), this, SLOT(savingFinished()));
	connect(&bufferWatcher, SIGNAL(finished()), this, SLOT(bufferLoaded()));
	connect(&imageWatcher, SIGNAL(finished()), this, SLOT(imageLoaded()));
	connect(loader.data(), SIGNAL(errorDialogSignal(const QString&)), this, SIGNAL(errorDialogSignal(const QString&)));
	connect(thumb.data(), SIGNAL(thumbUpdated()), this, SIGNAL(thumbUpdated()));
	//connect(&metaDataWatcher, SIGNAL(finished()), this, SLOT(metaDataLoaded()));
}

DkImageContainerT::~DkImageContainerT() {
	bufferWatcher.blockSignals(true);
	bufferWatcher.cancel();
	imageWatcher.blockSignals(true);
	imageWatcher.cancel();

	//metaDataWatcher.blockSignals(true);
	//metaDataWatcher.cancel();

	// we have to wait here
	bufferWatcher.waitForFinished();
	imageWatcher.waitForFinished();
	saveImageWatcher.waitForFinished();
	saveMetaDataWatcher.waitForFinished();
}

void DkImageContainerT::clear() {

	cancel();

	if (fetchingImage || fetchingBuffer)
		return;

	qDebug() << "clearing...";

	DkImageContainer::clear();
}

bool DkImageContainerT::loadImageThreaded() {

	QDateTime modifiedBefore = fileInfo.lastModified();
	fileInfo.refresh();

	if (fileInfo.lastModified() != modifiedBefore)
		fileBuffer->clear();

	// null file?
	if (fileInfo.fileName().isEmpty() || !fileInfo.exists()) {

		QString msg = tr("Sorry, the file: %1 does not exist... ").arg(fileInfo.fileName());
		emit showInfoSignal(msg);
		loadState = exists_not;
		return false;
	}
	else if (!fileInfo.permission(QFile::ReadUser)) {

		QString msg = tr("Sorry, you are not allowed to read: %1").arg(fileInfo.fileName());
		emit showInfoSignal(msg);
		loadState = exists_not;
		return false;
	}

	loadState = loading;
	fetchFile();
	return true;
}

void DkImageContainerT::fetchFile() {
	
	if (fetchingBuffer && imgLoaded() == loading_canceled) {
		loadState = loading;
		return;
	}
	if (fetchingImage)
		imageWatcher.waitForFinished();

	// ignore doubled calls
	if (fileBuffer && !fileBuffer->isEmpty()) {
		bufferLoaded();
		return;
	}

	fetchingBuffer = true;
	QFuture<QSharedPointer<QByteArray> > future = QtConcurrent::run(this, 
		&nmc::DkImageContainerT::loadFileToBuffer, fileInfo);

	bufferWatcher.setFuture(future);
}

void DkImageContainerT::bufferLoaded() {

	qDebug() << "buffer loaded by: " << QObject::sender();
	
	fetchingBuffer = false;
	fileBuffer = bufferWatcher.result();

	if (imgLoaded() == loading)
		fetchImage();
	else if (imgLoaded() == loading_canceled)
		clear();
}

void DkImageContainerT::fetchImage() {

	if (fetchingBuffer)
		bufferWatcher.waitForFinished();

	if (fetchingImage) {
		loadState = loading;
		return;
	}

	if (loader->hasImage() || !fileBuffer || fileBuffer->isEmpty() || loadState == exists_not) {
		loadingFinished();
		return;
	}
	
	qDebug() << "fetching: " << fileInfo.absoluteFilePath();
	fetchingImage = true;

	if (DkSettings::resources.fastThumbnailPreview)
		thumb->fetchThumb(DkThumbNailT::force_exif_thumb, fileBuffer);

	QFuture<QSharedPointer<DkBasicLoader> > future = QtConcurrent::run(this, 
		&nmc::DkImageContainerT::loadImageIntern, fileInfo, fileBuffer);

	imageWatcher.setFuture(future);
}

void DkImageContainerT::imageLoaded() {

	fetchingImage = false;

	if (imgLoaded() == loading_canceled) {
		loadState = not_loaded;
		clear();
		return;
	}

	// deliver image
	loader = imageWatcher.result();

	loadingFinished();
}

void DkImageContainerT::loadingFinished() {

	DkTimer dt;

	if (imgLoaded() == loading_canceled) {
		clear();
		loadState = not_loaded;
		return;
	}

	if (!loader->hasImage()) {
		QString msg = tr("Sorry, I could not load: %1").arg(fileInfo.fileName());
		emit showInfoSignal(msg);
		emit fileLoadedSignal(false);
		loadState = exists_not;
		return;
	}

	// clear file buffer it it exceeds a certain size?! e.g. psd files
	if (fileBuffer->size()/(1024.0f*1024.0f) > DkSettings::resources.cacheMemory*0.5f)
		fileBuffer->clear();
	
	loadState = loaded;
	emit fileLoadedSignal(true);
	
}

void DkImageContainerT::cancel() {

	if (loadState != loading)
		return;

	loadState = loading_canceled;
}

void DkImageContainerT::saveMetaDataThreaded() {

	if (!exists())
		return;

	QFuture<void> future = QtConcurrent::run(this, 
		&nmc::DkImageContainerT::saveMetaDataIntern);

}

bool DkImageContainerT::saveImageThreaded(const QFileInfo& fileInfo, int compression /* = -1 */) {

	return saveImageThreaded(fileInfo, loader->image(), compression);
}


bool DkImageContainerT::saveImageThreaded(const QFileInfo& fileInfo, const QImage& saveImg, int compression /* = -1 */) {

	saveImageWatcher.waitForFinished();

	if (saveImg.isNull()) {
		QString msg = tr("I can't save an empty file, sorry...\n");
		emit errorDialogSignal(msg);
		return false;
	}
	if (!fileInfo.absoluteDir().exists()) {
		QString msg = tr("Sorry, the directory: %1  does not exist\n").arg(fileInfo.absolutePath());
		emit errorDialogSignal(msg);
		return false;
	}
	if (fileInfo.exists() && !fileInfo.isWritable()) {
		QString msg = tr("Sorry, I can't write to the file: %1").arg(fileInfo.fileName());
		emit errorDialogSignal(msg);
		return false;
	}

	qDebug() << "attempting to save: " << fileInfo.absoluteFilePath();

	// TODO: add thumbnail?!
	QFuture<QFileInfo> future = QtConcurrent::run(this, 
		&nmc::DkImageContainerT::saveImageIntern, fileInfo, saveImg, compression);

	saveImageWatcher.setFuture(future);

	return true;
}

void DkImageContainerT::savingFinished() {

	QFileInfo saveFile = saveImageWatcher.result();
	saveFile.refresh();
	qDebug() << "save file: " << saveFile.absoluteFilePath();

	if (!saveFile.exists()) {
		emit errorDialogSignal(tr("Sorry, I could not save the image"));
		emit fileSavedSignal(saveFile, false);
	}
	else {
		fileBuffer.clear();
		fileInfo = saveFile;
		edited = false;
		emit fileSavedSignal(saveFile);
	}
}

QSharedPointer<QByteArray> DkImageContainerT::loadFileToBuffer(const QFileInfo fileInfo) {

	return DkImageContainer::loadFileToBuffer(fileInfo);
}

QSharedPointer<DkBasicLoader> DkImageContainerT::loadImageIntern(const QFileInfo fileInfo, const QSharedPointer<QByteArray> fileBuffer) {

	return DkImageContainer::loadImageIntern(fileInfo, fileBuffer);
}

QFileInfo DkImageContainerT::saveImageIntern(const QFileInfo fileInfo, QImage saveImg, int compression) {

	qDebug() << "saveImage in T: " << fileInfo.absoluteFilePath();

	return DkImageContainer::saveImageIntern(fileInfo, saveImg, compression);
}

void DkImageContainerT::saveMetaDataIntern() {

	return DkImageContainer::saveMetaDataIntern(fileInfo, fileBuffer);
}

};