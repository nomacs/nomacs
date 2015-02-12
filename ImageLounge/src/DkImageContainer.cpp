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
#include "DkImageStorage.h"
#include "DkMetaData.h"
#include "DkThumbs.h"
#include "DkBasicLoader.h"
#include "DkSettings.h"
#include "DkUtils.h"
#include "DkTimer.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QObject>
#include <QImage>
#include <QtConcurrentRun>

// quazip
#ifdef WITH_QUAZIP
#include <quazip/JlCompress.h>
#endif
#pragma warning(pop)		// no warnings from includes - end

#pragma warning(disable: 4251)	// TODO: remove


namespace nmc {

#ifdef WITH_QUAZIP
QString DkZipContainer::mZipMarker = "dIrChAr";
#endif

// DkImageContainer --------------------------------------------------------------------
/**
 * Creates a DkImageContainer.
 * This class is the basic image management class.
 * @param fileInfo the file of the given 
 **/ 
DkImageContainer::DkImageContainer(const QFileInfo& fileInfo) {
	
	this->fileInfo = fileInfo;
	init();
}

DkImageContainer::~DkImageContainer() {

}

void DkImageContainer::init() {

	edited = false;
	selected = false;
	
	// always keep in mind that a file does not exist
	if (!edited && loadState != exists_not)
		loadState = not_loaded;

}

bool DkImageContainer::operator==(const DkImageContainer& ric) const {
	return fileInfo.absoluteFilePath() == ric.file().absoluteFilePath();
}

bool DkImageContainer::operator<=(const DkImageContainer& o) const {

	if (*this == o)
		return true;

	return *this < o;
}

bool DkImageContainer::operator<(const DkImageContainer& o) const {

	return imageContainerLessThan(*this, o);
}

bool DkImageContainer::operator>(const DkImageContainer& o) const {

	return !imageContainerLessThan(*this, o);
}

bool DkImageContainer::operator>=(const DkImageContainer& o) const {

	if (*this == o)
		return true;

	return !imageContainerLessThan(*this, o);
}


void DkImageContainer::clear() {

	if (loader)
		loader->release();
	if (fileBuffer)
		fileBuffer->clear();
	init();
}

QFileInfo DkImageContainer::file() const {

	return fileInfo;
}

bool DkImageContainer::isFromZip() {

#ifdef WITH_QUAZIP
	return getZipData() && getZipData()->isZip();
#else
	return false;
#endif
}

bool DkImageContainer::exists() {

#ifdef WITH_QUAZIP

	if (isFromZip())
		return true;
#endif

	fileInfo.refresh();
	return fileInfo.exists();
}

QString DkImageContainer::getTitleAttribute() const {

	if (!loader || loader->getNumPages() <= 1)
		return QString();

	QString attr = "[" + QString::number(loader->getPageIdx()) + "/" + 
		QString::number(loader->getNumPages()) + "]";

	return attr;
}

QSharedPointer<DkBasicLoader> DkImageContainer::getLoader() {

	if (!loader) {
		this->loader = QSharedPointer<DkBasicLoader>(new DkBasicLoader());
	}

	return loader;
}

QSharedPointer<DkMetaDataT> DkImageContainer::getMetaData() {

	return getLoader()->getMetaData();
}

QSharedPointer<DkThumbNailT> DkImageContainer::getThumb() {

	if (!thumb) {
#ifdef WITH_QUAZIP	
		if(isFromZip()) 
			thumb = QSharedPointer<DkThumbNailT>(new DkThumbNailT(getZipData()->getEncodedFileInfo()));
		else
			thumb = QSharedPointer<DkThumbNailT>(new DkThumbNailT(fileInfo));
#else
		thumb = QSharedPointer<DkThumbNailT>(new DkThumbNailT(fileInfo));
#endif	
	}

	return thumb;
}

QSharedPointer<QByteArray> DkImageContainer::getFileBuffer() {

	if (!fileBuffer) {
		fileBuffer = QSharedPointer<QByteArray>(new QByteArray());
	}

	return fileBuffer;
}

float DkImageContainer::getMemoryUsage() const {

	if (!loader)
		return 0;

	float memSize = fileBuffer ? fileBuffer->size()/(1024.0f*1024.0f) : 0;
	memSize += DkImage::getBufferSizeFloat(loader->image().size(), loader->image().depth());

	return memSize;
}

float DkImageContainer::getFileSize() const {

	return fileInfo.size()/(1024.0f*1024.0f);
}


QImage DkImageContainer::image() {

	if (getLoader()->image().isNull() && getLoadState() == not_loaded)
		loadImage();

	return loader->image();
}

void DkImageContainer::setImage(const QImage& img) {

	setImage(img, fileInfo);
}

void DkImageContainer::setImage(const QImage& img, const QFileInfo& fileInfo) {

	this->fileInfo = fileInfo;
	getLoader()->setImage(img, fileInfo);
	edited = true;
}

bool DkImageContainer::hasImage() const {

	if (!loader)
		return false;

	return loader->hasImage();
}

int DkImageContainer::getLoadState() const {

	return loadState;
}

bool DkImageContainer::loadImage() {

	if (getFileBuffer()->isEmpty())
		fileBuffer = loadFileToBuffer(fileInfo);

	loader = loadImageIntern(fileInfo, getLoader(), fileBuffer);

	return loader->hasImage();
}

bool DkImageContainer::saveImage(const QFileInfo fileInfo, int compression /* = -1 */) {
	return saveImage(fileInfo, getLoader()->image(), compression);
}

bool DkImageContainer::saveImage(const QFileInfo fileInfo, const QImage saveImg, int compression /* = -1 */) {

	QFileInfo saveFile = saveImageIntern(fileInfo, loader, saveImg, compression);

	saveFile.refresh();
	qDebug() << "save file: " << saveFile.absoluteFilePath();

	return !saveFile.exists() || !saveFile.isFile();
}

QSharedPointer<QByteArray> DkImageContainer::loadFileToBuffer(const QFileInfo fileInfo) {

	QFileInfo fInfo = fileInfo.isSymLink() ? fileInfo.symLinkTarget() : fileInfo;

#ifdef WITH_QUAZIP
	if (isFromZip()) 
		return getZipData()->extractImage(getZipData()->getZipFileInfo(), getZipData()->getImageFileInfo());
#endif

	if (fInfo.suffix().contains("psd")) {	// for now just psd's are not cached because their file might be way larger than the part we need to read
		return QSharedPointer<QByteArray>(new QByteArray());
	}

	QFile file(fInfo.absoluteFilePath());
	file.open(QIODevice::ReadOnly);

	QSharedPointer<QByteArray> ba(new QByteArray(file.readAll()));
	file.close();

	return ba;
}


QSharedPointer<DkBasicLoader> DkImageContainer::loadImageIntern(const QFileInfo fileInfo, QSharedPointer<DkBasicLoader> loader, const QSharedPointer<QByteArray> fileBuffer) {

	try {
		loader->loadGeneral(fileInfo, fileBuffer, true);
	} catch(...) {}

	return loader;
}

QFileInfo DkImageContainer::saveImageIntern(const QFileInfo fileInfo, QSharedPointer<DkBasicLoader> loader, QImage saveImg, int compression) {

	return loader->save(fileInfo, saveImg, compression);
}

void DkImageContainer::saveMetaData() {

	if (!loader)
		return;

	saveMetaDataIntern(fileInfo, loader, fileBuffer);
}

void DkImageContainer::saveMetaDataIntern(const QFileInfo fileInfo, QSharedPointer<DkBasicLoader> loader, QSharedPointer<QByteArray> fileBuffer) {

	loader->saveMetaData(fileInfo, fileBuffer);
}

void DkImageContainer::setEdited(bool edited) {
	this->edited = edited;
}

bool DkImageContainer::isEdited() const {

	return edited;
}

bool DkImageContainer::isSelected() const {

	return selected;
}

bool DkImageContainer::setPageIdx(int skipIdx) {

	return getLoader()->setPageIdx(skipIdx);
}


#ifdef WITH_QUAZIP
QSharedPointer<DkZipContainer> DkImageContainer::getZipData() {

	if (!zipData) {
		this->zipData = QSharedPointer<DkZipContainer>(new DkZipContainer(fileInfo));
		if (zipData->isZip())
			this->fileInfo = zipData->getImageFileInfo();
	}

	return zipData;
}
#endif

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

// DkImageContainerT --------------------------------------------------------------------
DkImageContainerT::DkImageContainerT(const QFileInfo& file) : DkImageContainer(file) {
	
	fetchingImage = false;
	fetchingBuffer = false;
	
	// our file watcher
	fileUpdateTimer.setSingleShot(false);
	fileUpdateTimer.setInterval(500);
	waitForUpdate = false;
	downloaded = false;

	connect(&fileUpdateTimer, SIGNAL(timeout()), this, SLOT(checkForFileUpdates()));
	//connect(&metaDataWatcher, SIGNAL(finished()), this, SLOT(metaDataLoaded()));
}

DkImageContainerT::~DkImageContainerT() {
	
	bufferWatcher.blockSignals(true);
	bufferWatcher.cancel();
	imageWatcher.blockSignals(true);
	imageWatcher.cancel();

	saveMetaData();

	// we have to wait here
	saveMetaDataWatcher.blockSignals(true);
	saveImageWatcher.blockSignals(true);
}

void DkImageContainerT::clear() {

	cancel();

	if (fetchingImage || fetchingBuffer)
		return;

	DkImageContainer::clear();
}

void DkImageContainerT::checkForFileUpdates() {

#ifdef WITH_QUAZIP
	if(isFromZip()) fileInfo = getZipData()->getZipFileInfo();
#endif

	QDateTime modifiedBefore = fileInfo.lastModified();
	fileInfo.refresh();
	
	bool changed = false;

	// if image exists_not don't do this
	if (!fileInfo.exists() && loadState == loaded) {
		changed = true;
	}

	if (fileInfo.lastModified() != modifiedBefore)
		waitForUpdate = true;

#ifdef WITH_QUAZIP
	if(isFromZip()) fileInfo = getZipData()->getImageFileInfo();
#endif

	if (changed) {
		fileUpdateTimer.stop();
		if (DkSettings::global.askToSaveDeletedFiles) {
			edited = changed;
			emit fileLoadedSignal(true);
		}
		return;
	}

	// we use our own file watcher, since the qt watcher
	// uses locks to check for updates. this might
	// be more accurate. however, the locks are pretty nasty
	// if the user e.g. wants to delete the file while watching
	// it in nomacs
	if (waitForUpdate && fileInfo.isReadable()) {
		waitForUpdate = false;
		getThumb()->setImage(QImage());
		loadImageThreaded(true);
	}

}

bool DkImageContainerT::loadImageThreaded(bool force) {

#ifdef WITH_QUAZIP
	//zip archives: get zip file fileInfo for checks
	if(isFromZip()) fileInfo = getZipData()->getZipFileInfo();
#endif
	
	// check file for updates
	QDateTime modifiedBefore = fileInfo.lastModified();
	fileInfo.refresh();

	if (force || fileInfo.lastModified() != modifiedBefore || getLoader()->isDirty()) {
		qDebug() << "updating image...";
		getThumb()->setImage(QImage());
		clear();
	}

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

#ifdef WITH_QUAZIP
	//zip archives: use the image file info from now on
	if(isFromZip()) fileInfo = getZipData()->getImageFileInfo();
#endif
	
	loadState = loading;
	fetchFile();
	return true;
}

void DkImageContainerT::fetchFile() {
	
	if (fetchingBuffer && getLoadState() == loading_canceled) {
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
	connect(&bufferWatcher, SIGNAL(finished()), this, SLOT(bufferLoaded()));

	bufferWatcher.setFuture(QtConcurrent::run(this, 
		&nmc::DkImageContainerT::loadFileToBuffer, fileInfo));
}

void DkImageContainerT::bufferLoaded() {

	fetchingBuffer = false;
	fileBuffer = bufferWatcher.result();

	if (getLoadState() == loading)
		fetchImage();
	else if (getLoadState() == loading_canceled) {
		loadState = not_loaded;
		clear();
		return;
	}
}

void DkImageContainerT::fetchImage() {

	if (fetchingBuffer)
		bufferWatcher.waitForFinished();

	if (fetchingImage) {
		loadState = loading;
		return;
	}

	if (getLoader()->hasImage() || /*!fileBuffer || fileBuffer->isEmpty() ||*/ loadState == exists_not) {
		loadingFinished();
		return;
	}
	
	qDebug() << "fetching: " << fileInfo.absoluteFilePath();
	fetchingImage = true;

	connect(&imageWatcher, SIGNAL(finished()), this, SLOT(imageLoaded()));

	imageWatcher.setFuture(QtConcurrent::run(this, 
		&nmc::DkImageContainerT::loadImageIntern, fileInfo, loader, fileBuffer));
}

void DkImageContainerT::imageLoaded() {

	fetchingImage = false;

	if (getLoadState() == loading_canceled) {
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

	if (getLoadState() == loading_canceled) {
		loadState = not_loaded;
		clear();
		return;
	}

	if (!getLoader()->hasImage()) {
		fileUpdateTimer.stop();
		edited = false;
		QString msg = tr("Sorry, I could not load: %1").arg(fileInfo.fileName());
		emit showInfoSignal(msg);
		emit fileLoadedSignal(false);
		loadState = exists_not;
		return;
	}
	else if (!getThumb()->hasImage()) {
		getThumb()->setImage(getLoader()->image());
	}

	// clear file buffer if it exceeds a certain size?! e.g. psd files
	if (fileBuffer && fileBuffer->size()/(1024.0f*1024.0f) > DkSettings::resources.cacheMemory*0.5f)
		fileBuffer->clear();
	
	loadState = loaded;
	emit fileLoadedSignal(true);
}

void DkImageContainerT::downloadFile(const QUrl& url) {

	if (!fileDownloader) {
		fileDownloader = QSharedPointer<FileDownloader>(new FileDownloader(url, this));
		connect(fileDownloader.data(), SIGNAL(downloaded()), this, SLOT(fileDownloaded()));
		qDebug() << "trying to download: " << url;
	}
	else
		fileDownloader->downloadFile(url);
}

void DkImageContainerT::fileDownloaded() {

	if (!fileDownloader) {
		qDebug() << "empty fileDownloader, where it should not be";
		emit fileLoadedSignal(false);
		return;
	}

	fileBuffer = fileDownloader->downloadedData();

	if (!fileBuffer || fileBuffer->isEmpty()) {
		qDebug() << fileDownloader->getUrl() << " not downloaded...";
		emit showInfoSignal(tr("Sorry, I could not download:\n%1").arg(fileDownloader->getUrl().toString()));
		emit fileLoadedSignal(false);
		return;
	}

	downloaded = true;
	fetchImage();
}

void DkImageContainerT::cancel() {

	if (loadState != loading)
		return;

	loadState = loading_canceled;
}

void DkImageContainerT::receiveUpdates(QObject* obj, bool connectSignals /* = true */) {

	// !selected - do not connect twice
	if (connectSignals && !selected) {
		connect(this, SIGNAL(errorDialogSignal(const QString&)), obj, SIGNAL(errorDialogSignal(const QString&)));
		connect(this, SIGNAL(fileLoadedSignal(bool)), obj, SLOT(imageLoaded(bool)));
		connect(this, SIGNAL(showInfoSignal(QString, int, int)), obj, SIGNAL(showInfoSignal(QString, int, int)));
		connect(this, SIGNAL(fileSavedSignal(QFileInfo, bool)), obj, SLOT(imageSaved(QFileInfo, bool)));
		fileUpdateTimer.start();
	}
	else if (!connectSignals) {
		disconnect(this, SIGNAL(errorDialogSignal(const QString&)), obj, SIGNAL(errorDialogSignal(const QString&)));
		disconnect(this, SIGNAL(fileLoadedSignal(bool)), obj, SLOT(imageLoaded(bool)));
		disconnect(this, SIGNAL(showInfoSignal(QString, int, int)), obj, SIGNAL(showInfoSignal(QString, int, int)));
		disconnect(this, SIGNAL(fileSavedSignal(QFileInfo, bool)), obj, SLOT(imageSaved(QFileInfo, bool)));
		fileUpdateTimer.stop();
	}

	selected = connectSignals;

}

void DkImageContainerT::saveMetaDataThreaded() {

	if (!exists() || getLoader()->getMetaData() && !getLoader()->getMetaData()->isDirty())
		return;

	fileUpdateTimer.stop();
	QFuture<void> future = QtConcurrent::run(this, 
		&nmc::DkImageContainerT::saveMetaDataIntern, fileInfo, getLoader(), getFileBuffer());

}

bool DkImageContainerT::saveImageThreaded(const QFileInfo fileInfo, int compression /* = -1 */) {

	return saveImageThreaded(fileInfo, getLoader()->image(), compression);
}


bool DkImageContainerT::saveImageThreaded(const QFileInfo fileInfo, const QImage saveImg, int compression /* = -1 */) {

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

	fileUpdateTimer.stop();
	connect(&saveImageWatcher, SIGNAL(finished()), this, SLOT(savingFinished()));

	saveImageWatcher.setFuture(QtConcurrent::run(this, 
		&nmc::DkImageContainerT::saveImageIntern, fileInfo, loader, saveImg, compression));

	return true;
}

void DkImageContainerT::savingFinished() {

	QFileInfo saveFile = saveImageWatcher.result();
	saveFile.refresh();
	qDebug() << "save file: " << saveFile.absoluteFilePath();
	
	if (!saveFile.exists() || !saveFile.isFile())
		emit fileSavedSignal(saveFile, false);
	else {
		//// reset thumb - loadImageThreaded should do it anyway
		//thumb = QSharedPointer<DkThumbNailT>(new DkThumbNailT(saveFile, loader->image()));

		if (fileBuffer)
			fileBuffer->clear();	// do a complete clear?
		fileInfo = saveFile;
		edited = false;
		downloaded = false;
		if (selected) {
			loadImageThreaded(true);	// force a reload
			fileUpdateTimer.start();
		}
		emit fileSavedSignal(saveFile);
	}
}

QSharedPointer<QByteArray> DkImageContainerT::loadFileToBuffer(const QFileInfo fileInfo) {

	return DkImageContainer::loadFileToBuffer(fileInfo);
}

QSharedPointer<DkBasicLoader> DkImageContainerT::loadImageIntern(const QFileInfo fileInfo, QSharedPointer<DkBasicLoader> loader, const QSharedPointer<QByteArray> fileBuffer) {

	return DkImageContainer::loadImageIntern(fileInfo, loader, fileBuffer);
}

QFileInfo DkImageContainerT::saveImageIntern(const QFileInfo fileInfo, QSharedPointer<DkBasicLoader> loader, QImage saveImg, int compression) {

	qDebug() << "saveImage in T: " << fileInfo.absoluteFilePath();

	return DkImageContainer::saveImageIntern(fileInfo, loader, saveImg, compression);
}

void DkImageContainerT::saveMetaDataIntern(QFileInfo fileInfo, QSharedPointer<DkBasicLoader> loader, QSharedPointer<QByteArray> fileBuffer) {

	return DkImageContainer::saveMetaDataIntern(fileInfo, loader, fileBuffer);
}

QSharedPointer<DkBasicLoader> DkImageContainerT::getLoader() {

	if (!loader) {
		DkImageContainer::getLoader();
		connect(loader.data(), SIGNAL(errorDialogSignal(const QString&)), this, SIGNAL(errorDialogSignal(const QString&)));
	}

	return loader;
}

QSharedPointer<DkThumbNailT> DkImageContainerT::getThumb() {

	if (!thumb) {
		DkImageContainer::getThumb();
		connect(thumb.data(), SIGNAL(thumbLoadedSignal(bool)), this, SIGNAL(thumbLoadedSignal(bool)));
	}

	return thumb;
}

bool DkImageContainerT::isFileDownloaded() const {

	return downloaded;
}

};