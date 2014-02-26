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

	saveMetaData();

	loader->release();
	fileBuffer.clear();
	init();
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

	if (fileBuffer.isNull())
		fileBuffer = loadFileToBuffer(fileInfo);

	loader = loadImageIntern(fileInfo, fileBuffer);

	return loader->hasImage();
}

QByteArray DkImageContainer::loadFileToBuffer(const QFileInfo fileInfo) {

	QFile file(fileInfo.absoluteFilePath());
	file.open(QIODevice::ReadOnly);

	QByteArray ba;
	ba = file.readAll();

	return ba;
}


QSharedPointer<DkBasicLoader> DkImageContainer::loadImageIntern(const QFileInfo fileInfo, const QByteArray fileBuffer) {

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


void DkImageContainer::saveMetaDataIntern(const QFileInfo& fileInfo, const QByteArray& fileBuffer) {

	QByteArray ba = fileBuffer;

	if (fileBuffer.isEmpty())
		ba = loadFileToBuffer(fileInfo);
		
	loader->saveMetaData(fileInfo, ba);
}

bool DkImageContainer::isEdited() const {

	return edited;
}

bool DkImageContainer::setPageIdx(int skipIdx) {

	return loader->setPageIdx(skipIdx);
}


// DkImageContainerT --------------------------------------------------------------------
DkImageContainerT::DkImageContainerT(const QFileInfo& file) : DkImageContainer(file) {

	fetchingBuffer = false;
	fetchingImage = false;
	connect(&saveImageWatcher, SIGNAL(finished()), this, SLOT(savingFinished()));
	connect(&bufferWatcher, SIGNAL(finished()), this, SLOT(bufferLoaded()));
	connect(&imageWatcher, SIGNAL(finished()), this, SLOT(imageLoaded()));
	connect(&imageWatcher, SIGNAL(canceled()), this, SLOT(cancelFinished()));
	connect(&imageWatcher, SIGNAL(canceled()), this, SLOT(cancelFinished()));
	connect(loader.data(), SIGNAL(errorDialogSignal(const QString&)), this, SIGNAL(errorDialogSignal(const QString&)));

	//connect(&metaDataWatcher, SIGNAL(finished()), this, SLOT(metaDataLoaded()));
}

DkImageContainerT::~DkImageContainerT() {
	bufferWatcher.blockSignals(true);
	bufferWatcher.cancel();
	imageWatcher.blockSignals(true);
	imageWatcher.cancel();
	//metaDataWatcher.blockSignals(true);
	//metaDataWatcher.cancel();

	saveImageWatcher.waitForFinished();
}

bool DkImageContainerT::loadImageThreaded() {

	QDateTime modifiedBefore = fileInfo.lastModified();
	fileInfo.refresh();

	if (fileInfo.lastModified() != modifiedBefore)
		fileBuffer.clear();

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

void DkImageContainerT::fetchFile() {

	// ignore doubled calls
	if (fetchingBuffer)
		return;
	else if (!fileBuffer.isEmpty()) {
		bufferLoaded();
		return;
	}

	// we have to do our own bool here
	// watcher.isRunning() returns false if the thread is waiting in the pool
	fetchingBuffer = true;

	QFuture<QByteArray> future = QtConcurrent::run(this, 
		&nmc::DkImageContainerT::loadFileToBuffer, fileInfo);

	bufferWatcher.setFuture(future);
}

void DkImageContainerT::bufferLoaded() {

	if (bufferWatcher.isCanceled())
		return;

	fileBuffer = bufferWatcher.result();

	if (imgLoaded() == loading)
		fetchImage();

	fetchingBuffer = false;
}

void DkImageContainerT::fetchImage() {

	if (loader->hasImage() || fileBuffer.isNull() || loadState == exists_not) {
		loadingFinished();
		return;
	}
	
	// we have to do our own bool here
	// watcher.isRunning() returns false if the thread is waiting in the pool
	fetchingImage = true;

	QFuture<QSharedPointer<DkBasicLoader> > future = QtConcurrent::run(this, 
		&nmc::DkImageContainerT::loadImageIntern, fileInfo, fileBuffer);

	imageWatcher.setFuture(future);
}

void DkImageContainerT::imageLoaded() {

	if (imageWatcher.isCanceled())
		return;

	// deliver image
	loader = imageWatcher.result();

	fetchingImage = false;

	//if (img.isNull())
		loadingFinished();
	//else
	//	fetchMetaData();
}

void DkImageContainerT::loadingFinished() {

	DkTimer dt;

	if (!loader->hasImage()) {
		QString msg = tr("Sorry, I could not load: %1").arg(fileInfo.fileName());
		emit showInfoSignal(msg);
		emit fileLoadedSignal(false);
		loadState = exists_not;
		return;
	}

	loadState = loaded;
	emit fileLoadedSignal(true);
	
	qDebug() << "metadata loaded and image rotated in: " << QString::fromStdString(dt.getTotal());
}

void DkImageContainerT::cancel() {

	if (loadState != loading)
		return;
		//cancelFinished();

	bufferWatcher.cancel();
	imageWatcher.cancel();
}

void DkImageContainerT::cancelFinished() {

	if (bufferWatcher.isCanceled() && imageWatcher.isCanceled())
		clear();
}

QByteArray DkImageContainerT::loadFileToBuffer(const QFileInfo fileInfo) {

	return DkImageContainer::loadFileToBuffer(fileInfo);
}

QSharedPointer<DkBasicLoader> DkImageContainerT::loadImageIntern(const QFileInfo fileInfo, const QByteArray fileBuffer) {

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