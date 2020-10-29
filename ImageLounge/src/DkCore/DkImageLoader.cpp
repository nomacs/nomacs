/*******************************************************************************************************
 DkImage.cpp
 Created on:	21.04.2011
 
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

#include "DkImageLoader.h"

#include "DkMessageBox.h"
#include "DkSettings.h"
#include "DkTimer.h"
#include "DkThumbs.h"
#include "DkImageStorage.h"
#include "DkBasicLoader.h"
#include "DkMetaData.h"
#include "DkImageContainer.h"
#include "DkMessageBox.h"
#include "DkSaveDialog.h"
#include "DkUtils.h"
#include "DkStatusBar.h"
#include "DkActionManager.h"
#include "DkDialog.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QWidget>
#include <QImageWriter>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QFile>
#include <QSettings>
#include <QImageReader>
#include <QDir>
#include <QThread>
#include <QBuffer>
#include <QStringBuilder>
#include <QDebug>
#include <QMutex>
#include <QFileIconProvider>
#include <QStringList>
#include <QMessageBox>
#include <QDirIterator>
#include <QProgressDialog>
#include <QReadLocker>
#include <QWriteLocker>
#include <QReadWriteLock>
#include <QTimer>
#include <QMovie>
#include <QByteArray>
#include <QCoreApplication>
#include <QApplication>
#include <QPluginLoader>
#include <QFileDialog>
#include <QPainter>
#include <qmath.h>
#include <QtConcurrentRun>
#include <QStandardPaths>
#include <QDesktopServices>

// quazip
#ifdef WITH_QUAZIP
#ifdef WITH_QUAZIP1
#include <quazip/JlCompress.h>
#else
#include <quazip5/JlCompress.h>
#endif
#endif

// opencv
#ifdef WITH_OPENCV

#ifdef Q_OS_WIN
#pragma warning(disable: 4996)
#endif
#endif 

#ifdef WITH_LIBRAW
#include <libraw/libraw.h>
#elif defined(Q_OS_WIN) && !defined(SOCK_STREAM)
#include <winsock2.h>	// needed since libraw 0.16
#endif

#pragma warning(pop)	// no warnings from includes - end

namespace nmc {

// DkImageLoader -> is nomacs file handling routine --------------------------------------------------------------------
/**
 * Default constructor.
 * Creates a DkImageLoader instance with a given file.
 * @param file the file to be loaded.
 **/ 
DkImageLoader::DkImageLoader(const QString& filePath) {

	qRegisterMetaType<QFileInfo>("QFileInfo");

	mDirWatcher = new QFileSystemWatcher(this);
	connect(mDirWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(directoryChanged(QString)));

	mSortingIsDirty = false;
	mSortingImages = false;

	connect(&mCreateImageWatcher, SIGNAL(finished()), this, SLOT(imagesSorted()));

	mDelayedUpdateTimer.setSingleShot(true);
	connect(&mDelayedUpdateTimer, SIGNAL(timeout()), this, SLOT(directoryChanged()));

    connect(DkActionManager::instance().action(DkActionManager::menu_file_save_copy), SIGNAL(triggered()), this, SLOT(copyUserFile()));
    connect(DkActionManager::instance().action(DkActionManager::menu_edit_undo), SIGNAL(triggered()), this, SLOT(undo()));
	connect(DkActionManager::instance().action(DkActionManager::menu_edit_redo), SIGNAL(triggered()), this, SLOT(redo()));
	connect(DkActionManager::instance().action(DkActionManager::menu_view_gps_map), SIGNAL(triggered()), this, SLOT(showOnMap()));
	connect(DkActionManager::instance().action(DkActionManager::sc_delete_silent), SIGNAL(triggered()), this, SLOT(deleteFile()), Qt::UniqueConnection);

	//saveDir = DkSettingsManager::param().global().lastSaveDir;	// loading save dir is obsolete ?!
	 
	QFileInfo fInfo(filePath);

	if (fInfo.exists())
		loadDir(fInfo.absolutePath());
	else
		mCurrentDir = DkSettingsManager::param().global().lastDir;
}

/**
 * Default destructor.
 **/ 
DkImageLoader::~DkImageLoader() {
	
	if (mCreateImageWatcher.isRunning())
		mCreateImageWatcher.blockSignals(true);
}

/**
 * Clears the path.
 * Calling this method makes the loader forget
 * about the current directory. It also destroys
 * the currently loaded image.
 **/ 
void DkImageLoader::clearPath() {

	// lastFileLoaded must exist
	if (mCurrentImage && mCurrentImage->exists()) {
		mCurrentImage->receiveUpdates(this, false);
		mLastImageLoaded = mCurrentImage;
		mImages.clear();

		// only clear the current image if it exists
		mCurrentImage.clear();
	}
}

#ifdef WITH_QUAZIP
/**
 * Loads a given zip archive and the first image in it.
 * @param zipFile the archive to be loaded.
 **/ 
bool DkImageLoader::loadZipArchive(const QString& zipPath) {

	QStringList fileNameList = JlCompress::getFileList(zipPath);
	
	// remove the * in fileFilters
	QStringList fileFiltersClean = DkSettingsManager::param().app().browseFilters;
	for (int idx = 0; idx < fileFiltersClean.size(); idx++)
		fileFiltersClean[idx].replace("*", "");

	QStringList fileList;
	for (int idx = 0; idx < fileNameList.size(); idx++) {
		
		for (int idxFilter = 0; idxFilter < fileFiltersClean.size(); idxFilter++) {

			if (fileNameList.at(idx).contains(fileFiltersClean[idxFilter], Qt::CaseInsensitive)) {
				fileList.append(fileNameList.at(idx));
				break;
			}
		}
	}

	QFileInfoList fileInfoList;
	//encode both the input zip file and the output image into a single fileInfo
	for (const QString& filePath : fileList)
		fileInfoList.append(DkZipContainer::encodeZipFile(zipPath, filePath));

	QFileInfo zipInfo(zipPath);

	// zip archives could not contain known image formats
	if (fileInfoList.empty()) {
		emit showInfoSignal(tr("%1 \n does not contain any image").arg(zipInfo.fileName()), 4000);	// stop mShowing
		return false;
	}

	createImages(fileInfoList);

	emit updateDirSignal(mImages);
	mCurrentDir = zipInfo.absolutePath();

	qDebug() << "ZIP FOLDER set: " << mCurrentDir;

	return true;
}
#endif

/**
 * Loads a given directory.
 * @param newDir the directory to be loaded.
 **/ 
bool DkImageLoader::loadDir(const QString& newDirPath, bool scanRecursive) {

	//if (creatingImages) {
	//	//emit showInfoSignal(tr("Indexing folder..."), 4000);	// stop showing
	//	return false;
	//}

	DkTimer dt;
	
	// folder changed signal was emitted
	if (mFolderUpdated && newDirPath == mCurrentDir) {
		
		mFolderUpdated = false;
		QFileInfoList files = getFilteredFileInfoList(newDirPath, mIgnoreKeywords, mKeywords, mFolderFilterString);		// this line takes seconds if you have lots of files and slow loading (e.g. network)

		// might get empty too (e.g. someone deletes all images)
 		if (files.empty()) {
			emit showInfoSignal(tr("%1 \n does not contain any image").arg(newDirPath), 4000);	// stop showing
			mImages.clear();
			emit updateDirSignal(mImages);
			return false;
		}

		// disabled threaded sorting - people didn't like it (#484 and #460)
		//if (files.size() > 2000) {
		//	createImages(files, false);
		//	sortImagesThreaded(images);
		//}
		//else
			createImages(files, true);

		qDebug() << "getting file list.....";
	}
	// new folder is loaded
	else if ((newDirPath != mCurrentDir || mImages.empty()) && !newDirPath.isEmpty() && QDir(newDirPath).exists()) {

		QFileInfoList files;

		//newDir.setNameFilters(DkSettingsManager::param().app().fileFilters);
		//newDir.setSorting(QDir::LocaleAware);		// TODO: extend

		// update save directory
		mCurrentDir = newDirPath;
		mFolderUpdated = false;

		mFolderFilterString.clear();	// delete key words -> otherwise user may be confused

		if (scanRecursive && DkSettingsManager::param().global().scanSubFolders)
			files = updateSubFolders(mCurrentDir);
		else 
			files = getFilteredFileInfoList(mCurrentDir, mIgnoreKeywords, mKeywords, mFolderFilterString);		// this line takes seconds if you have lots of files and slow loading (e.g. network)

		if (files.empty()) {
			emit showInfoSignal(tr("%1 \n does not contain any image").arg(mCurrentDir), 4000);	// stop showing
			return false;
		}

		// ok new folder, this should speed-up loading
		mImages.clear();
		
		//// TODO: creating ~120 000 images takes about 2 secs
		//// but sorting (just filenames) takes ages (on windows)
		//// so we should fix this using 2 strategies: 
		//// - thread the image creation process
		//// - while loading (if the user wants to move in the folder) we could display some message (e.g. indexing dir)
		//if (files.size() > 2000) {
		//	createImages(files, false);
		//	sortImagesThreaded(mImages);
		//}
		//else
			createImages(files, true);

		qInfoClean() << newDirPath << " [" << mImages.size() << "] indexed in " << dt;
	}
	//else
	//	qDebug() << "ignoring... old dir: " << dir.absolutePath() << " newDir: " << newDir << " file size: " << images.size();

	return true;
}

void DkImageLoader::sortImagesThreaded(QVector<QSharedPointer<DkImageContainerT > > images) {

	if (mSortingImages) {
		mSortingIsDirty = true;
		return;
	}

	mSortingIsDirty = false;
	mSortingImages = true;
	mCreateImageWatcher.setFuture(QtConcurrent::run(this, 
		&nmc::DkImageLoader::sortImages, images));

	qDebug() << "sorting images threaded...";
}

void DkImageLoader::imagesSorted() {

	mSortingImages = false;
	mImages = mCreateImageWatcher.result();

	if (mSortingIsDirty) {
		qDebug() << "re-sorting because it's dirty...";
		sortImagesThreaded(mImages);
		return;
	}

	emit updateDirSignal(mImages);

	if (mDirWatcher) {
		if (!mDirWatcher->directories().isEmpty())
			mDirWatcher->removePaths(mDirWatcher->directories());
		mDirWatcher->addPath(mCurrentDir);
	}

	qDebug() << "images sorted...";
}

void DkImageLoader::createImages(const QFileInfoList& files, bool sort) {

	// TODO: change files to QStringList
	DkTimer dt;
	QVector<QSharedPointer<DkImageContainerT > > oldImages = mImages;
	mImages.clear();

	QDate today = QDate::currentDate();

	for (const QFileInfo& f : files) {

		const QString& fp = f.absoluteFilePath();
		int oIdx = findFileIdx(fp, oldImages);

		// NOTE: we had this here: oIdx != -1 && QFileInfo(oldImages.at(oIdx)->filePath()).lastModified() == f.lastModified())
		// however, that did not detect file changes & slowed down the process - so I removed it...
		mImages << ((oIdx != -1) ? oldImages.at(oIdx) : QSharedPointer<DkImageContainerT >(new DkImageContainerT(fp)));
	}
	qInfo() << "[DkImageLoader]" << mImages.size() << "containers created in" << dt;

	if (sort) {
		qSort(mImages.begin(), mImages.end(), imageContainerLessThanPtr);
		qInfo() << "[DkImageLoader] after sorting: " << dt;

		emit updateDirSignal(mImages);

		if (mDirWatcher) {
			if (!mDirWatcher->directories().isEmpty())
				mDirWatcher->removePaths(mDirWatcher->directories());
			mDirWatcher->addPath(mCurrentDir);
		}
	}

}

QVector<QSharedPointer<DkImageContainerT > > DkImageLoader::sortImages(QVector<QSharedPointer<DkImageContainerT > > images) const {

	qSort(images.begin(), images.end(), imageContainerLessThanPtr);
	return images;
}

/**
 * Loads the ancesting or subsequent file.
 * @param skipIdx the number of files that should be skipped after/before the current file.
 * @param silent if true, no status information will be displayed.
 **/ 
void DkImageLoader::changeFile(int skipIdx) {

	//if (!img.isNull() && !file.exists())
	//	return;
	//if (!file.exists() && !virtualFile.exists()) {
	//	qDebug() << virtualFile.absoluteFilePath() << "does not exist...!!!";
	//	return;
	//}

	if (skipIdx == 0) {
		reloadImage();
		return;
	}

	// update dir
	loadDir(mCurrentDir);

	QSharedPointer<DkImageContainerT> imgC = getSkippedImage(skipIdx);

	load(imgC);
}

/**
 * Returns the file info of the ancesting/subsequent file + skipIdx.
 * @param skipIdx the number of files to be skipped from the current file.
 * @param silent if true, no status information will be displayed.
 * @return QFileInfo the file info of the demanded file
 **/ 
QSharedPointer<DkImageContainerT> DkImageLoader::getSkippedImage(int skipIdx, bool searchFile, bool recursive) {

	QSharedPointer<DkImageContainerT> imgC;

	if (!mCurrentImage)
		return imgC;

	DkTimer dt;


	// load a page (e.g. within a tiff file)
	if (mCurrentImage->setPageIdx(skipIdx))
		return mCurrentImage;

	//if (searchFile && currentImage->file().absoluteDir() != dir.absolutePath()) {
	//	qDebug() << "loading new dir: " << currentImage->file().absolutePath();
	//	qDebug() << "old dir: " << dir.absolutePath();
	
	if (!recursive)
		loadDir(mCurrentImage->dirPath(), false);

	// locate the current file
	int newFileIdx = 0;
	
	if (searchFile) mTmpFileIdx = 0;

	//qDebug() << "virtual file " << virtualFile.absoluteFilePath();
	//qDebug() << "file" << file.absoluteFilePath();

	//if (virtualExists || file.exists()) {

	if (searchFile) {

		QString file = (mCurrentImage->exists()) ? mCurrentImage->filePath() : mCurrentDir;

		mTmpFileIdx = findFileIdx(file, mImages);

		// could not locate the file -> it was deleted?!
		if (mTmpFileIdx == -1) {

			mTmpFileIdx = 0;
			for (; mTmpFileIdx < mImages.size(); mTmpFileIdx++) {

				if (*(mCurrentImage.data()) < *(mImages[mTmpFileIdx].data()))
					break;
			}

			if (skipIdx > 0)
				mTmpFileIdx--;	// -1 because the current file does not exist
			if (mImages.size() == mTmpFileIdx)	// could not locate file - resize
				mTmpFileIdx = 0;

		}		

	}
	newFileIdx = mTmpFileIdx + skipIdx;

	//qDebug() << "subfolders: " << DkSettingsManager::param().global().scanSubFolders << "subfolder size: " << (subFolders.size() > 1);

	if (DkSettingsManager::param().global().scanSubFolders && mSubFolders.size() > 1 && (newFileIdx < 0 || newFileIdx >= mImages.size())) {

		int folderIdx = 0;

		// locate folder
		for (int idx = 0; idx < mSubFolders.size(); idx++) {
			if (mSubFolders[idx] == mCurrentDir) {
				folderIdx = idx;
				break;
			}
		}

		if (newFileIdx < 0)
			folderIdx = getPrevFolderIdx(folderIdx);
		else
			folderIdx = getNextFolderIdx(folderIdx);

		qDebug() << "new folder idx: " << folderIdx;
			
		//if (DkSettingsManager::param().global().loop)
		//	folderIdx %= subFolders.size();

		if (folderIdx >= 0 && folderIdx < mSubFolders.size()) {
				
			int oldFileSize = mImages.size();
			loadDir(mSubFolders[folderIdx], false);	// don't scan recursive again
			qDebug() << "loading new folder: " << mSubFolders[folderIdx];

			if (newFileIdx >= oldFileSize) {
				newFileIdx -= oldFileSize;
				mTmpFileIdx = 0;
				qDebug() << "new skip idx: " << newFileIdx << "cFileIdx: " << mTmpFileIdx << " -----------------------------";
				getSkippedImage(newFileIdx, false, true);
			}
			else if (newFileIdx < 0) {
				newFileIdx += mTmpFileIdx;
				mTmpFileIdx = mImages.size()-1;
				qDebug() << "new skip idx: " << newFileIdx << "cFileIdx: " << mTmpFileIdx << " -----------------------------";
				getSkippedImage(newFileIdx, false, true);
			}
		}
		//// dir up
		//else if (folderIdx == subFolders.size()) {

		//	qDebug() << "going one up";
		//	dir.cd("..");
		//	loadDir(dir, false);	// don't scan recursive again
		//	newFileIdx += cFileIdx;
		//	cFileIdx = 0;
		//	getChangedFileInfo(newFileIdx, silent, false);
		//}
		//// get root files
		//else if (folderIdx < 0) {
		//	loadDir(dir, false);
		//}

	}

#ifdef WITH_QUAZIP
	if (mCurrentImage && (newFileIdx < 0 || newFileIdx >= mImages.size()) && mCurrentImage->isFromZip() && mCurrentImage->getZipData()) {

		// load the zip again and go on from there
		setCurrentImage(QSharedPointer<DkImageContainerT>(new DkImageContainerT(mCurrentImage->getZipData()->getZipFilePath())));

		if (newFileIdx >= mImages.size())
			newFileIdx -= mImages.size() - 1;

		return getSkippedImage(newFileIdx);
	}
#endif

	// this should never happen!
	if (mImages.empty()) {
		qDebug() << "file list is empty, where it should not be";
		return imgC;
	}

	// loop the directory
	if (DkSettingsManager::param().global().loop) {
		newFileIdx %= mImages.size();

		while (newFileIdx < 0)	// should be hit once
			newFileIdx = mImages.size() + newFileIdx;

	}
	// clip to pos1 if skipIdx < -1
	else if (mTmpFileIdx > 0 && newFileIdx < 0) {
		newFileIdx = 0;
	}
	// clip to end if skipIdx > 1
	else if (mTmpFileIdx < mImages.size()-1 && newFileIdx >= mImages.size()) {
		newFileIdx = mImages.size()-1;
	}
	// tell user that there is nothing left to display
	else if (newFileIdx < 0) {

		// TODO: find an elegant way to switch back to the zip folder
		QString msg = tr("You have reached the beginning");
		showInfoSignal(msg, 1000);
		return imgC;
	}
	// tell user that there is nothing left to display
	else if (newFileIdx >= mImages.size()) {
		QString msg = tr("You have reached the end");
			
		if (!DkSettingsManager::param().global().loop)
			emit setPlayer(false);

		showInfoSignal(msg, 1000);
		return imgC;
	}

	mTmpFileIdx = newFileIdx;

	if (newFileIdx >= 0 && newFileIdx < mImages.size())
		imgC = mImages.at(newFileIdx);


	// file requested becomes current file
	return imgC;
	
}


/**
* Loads the file at index idx.
* @param idx the file index of the file which should be loaded.
**/ 
void DkImageLoader::loadFileAt(int idx) {

	//if (basicLoader.hasImage() && !file.exists())
	//	return;

	if (mCurrentImage)
		qDebug() << "current image: " << mCurrentImage->filePath();
	else
		qDebug() << "current image is NULL";

	QDir cDir(mCurrentDir);

	if (mCurrentImage && !cDir.exists())
		loadDir(mCurrentImage->dirPath());

	if(mImages.empty())
		return;

	if (cDir.exists()) {

		if (idx == -1) {
			idx = mImages.size()-1;
		}
		else if (DkSettingsManager::param().global().loop) {
			idx %= mImages.size();

			while (idx < 0)
				idx = mImages.size() + idx;

		}
		else if (idx < 0 && !DkSettingsManager::param().global().loop) {
			QString msg = tr("You have reached the beginning");
			emit showInfoSignal(msg, 1000);
			return;
		}
		else if (idx >= mImages.size()) {
			QString msg = tr("You have reached the end");
			if (!DkSettingsManager::param().global().loop)
				emit(setPlayer(false));
			emit showInfoSignal(msg, 1000);
			return;
		}
	}

	// file requested becomes current file
	setCurrentImage(mImages.at(idx));

	load(mCurrentImage);
}

QSharedPointer<DkImageContainerT> DkImageLoader::findOrCreateFile(const QString& filePath) const {

	QSharedPointer<DkImageContainerT> imgC = findFile(filePath);

	if (!imgC)
		imgC = QSharedPointer<DkImageContainerT>(new DkImageContainerT(filePath));

	return imgC;
}

QSharedPointer<DkImageContainerT> DkImageLoader::findFile(const QString& filePath) const {

	// if one image is from zip than all should be
	// for images in zip the "images[idx]->file() == file" comparison somahow does not work
	if(mImages.size() > 0) {

		if (mImages[0]->isFromZip()) {
			int idx = findFileIdx(filePath, mImages);
			if (idx < 0) 
				return QSharedPointer<DkImageContainerT>();
			else 
				return mImages[idx];
		}
	}

	for (int idx = 0; idx < mImages.size(); idx++) {

		if (mImages[idx]->filePath() == filePath)
			return mImages[idx];
	}

	return QSharedPointer<DkImageContainerT>();
}

int DkImageLoader::findFileIdx(const QString& filePath, const QVector<QSharedPointer<DkImageContainerT> >& images) const {

	// this seems a bit bizare...
	// however, in converting the string from a fileInfo - we quarantee that the separators are the same (/ vs \)
	QString lFilePath = filePath;
	lFilePath.replace("\\", QDir::separator());

	for (int idx = 0; idx < images.size(); idx++) {

		if (images[idx]->filePath() == lFilePath)
			return idx;
	}

	return -1;
}

QStringList DkImageLoader::getFileNames() const {

	QStringList fileNames;

	for (int idx = 0; idx < mImages.size(); idx++)
		fileNames.append(QFileInfo(mImages[idx]->filePath()).fileName());

	return fileNames;
}

QVector<QSharedPointer<DkImageContainerT> > DkImageLoader::getImages() {

	loadDir(mCurrentDir);
	return mImages;
}

void DkImageLoader::setImages(QVector<QSharedPointer<DkImageContainerT> > images) {

	mImages = images;
	emit updateDirSignal(images);
}

/**
 * Loads the first file of the current directory.
 **/ 
void DkImageLoader::firstFile() {

	loadFileAt(0);
}

/**
 * Loads the last file of the current directory.
 **/ 
void DkImageLoader::lastFile() {
	
	loadFileAt(-1);
}

bool DkImageLoader::unloadFile() {

	if (!mCurrentImage)
		return true;

	// if we are either in rc or remote display mode & the directory does not exist - we received an image, so don't ask the user
	if (mCurrentImage->isEdited()) {
		DkMessageBox* msgBox = new DkMessageBox(
			QMessageBox::Question, 
			tr("Save Image"), 
			tr("Do you want to save changes to:\n%1").arg(QFileInfo(mCurrentImage->filePath()).fileName()), 
			(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel), 
			DkUtils::getMainWindow());

		msgBox->setDefaultButton(QMessageBox::No);
		msgBox->setObjectName("saveEditDialog");

		int answer = msgBox->exec();

		if (answer == QMessageBox::Accepted || answer == QMessageBox::Yes) {
			
			if (DkUtils::isSavable(mCurrentImage->fileInfo().fileName()))
				mCurrentImage->saveImageThreaded(mCurrentImage->filePath());
			else
				saveUserFileAs(mCurrentImage->image(), false);	// we loose all metadata here - right?
		}
		else if (answer != QMessageBox::No) {	// only 'No' will discard the changes
			return false;
		}
	}

	return true;
}

/**
 * Convenience function see @activate.
 **/ 
void DkImageLoader::deactivate() {

	activate(false);
}


/**
 * Activates or deactivates the loader.
 * If activated, the directory is indexed & the current image is loaded.
 * If deactivated, the image list & the current image are deleted which
 * should save some memory. In addition, all signals are mBlocked.
 * @param isActive if true, the loader is activated
 **/ 
void DkImageLoader::activate(bool isActive /* = true */) {

	if (!isActive) {
		// go to sleep - schlofand wöhlar ihr camölar
		blockSignals(true);
		clearPath();
	}
	else if (!mCurrentImage) {
		// wake up again
		blockSignals(false);
		setCurrentImage(mLastImageLoaded);
	}
	else
		emit updateDirSignal(mImages);
}

void DkImageLoader::setCurrentImage(QSharedPointer<DkImageContainerT> newImg) {

	// force index folder if we dir out of the zip
	if (mCurrentImage && newImg && mCurrentImage->isFromZip() && !newImg->isFromZip())
		mFolderUpdated = true;

	if (signalsBlocked()) {
		mCurrentImage = newImg;
		return;
	}

	if (newImg)
		loadDir(newImg->dirPath());
	//else
	//	qDebug() << "empty image assigned";
	
	// if the file stays the same, we just want to update the pointer
	bool updatePointer = newImg && mCurrentImage && newImg->filePath() == mCurrentImage->filePath();

	// cancel action if the image is currently loading
	if (DkSettingsManager::param().resources().waitForLastImg &&
		mCurrentImage && mCurrentImage->getLoadState() == DkImageContainerT::loading && 
		newImg && newImg->dirPath() == mCurrentImage->dirPath())
		return;

	if (mCurrentImage) {

		// do we load a new image?
		if (!updatePointer) {
			mCurrentImage->cancel();

			if (mCurrentImage->getLoadState() == DkImageContainer::loading_canceled)
				emit showInfoSignal(newImg->fileName(), 3000, 1);

			// this causes a crash 
			// because the release will trigger the saving of
			// metadata (in a different thread) - and
			// the auto_ptr does not like that at all
			// anyhow we don't need to save the metadata twice
			//currentImage->saveMetaDataThreaded();

			if (!DkSettingsManager::param().resources().cacheMemory)
				mCurrentImage->clear();

			mCurrentImage->getLoader()->resetPageIdx();
		}
		mCurrentImage->receiveUpdates(this, false);	// reset updates
	}

	mCurrentImage = newImg;

	if (mCurrentImage)
		mCurrentImage->receiveUpdates(this);
}

void DkImageLoader::reloadImage() {

	if(!mCurrentImage)
		return;

	if (!mCurrentImage->exists()) {
		// message when reloaded
		QString msg = tr("sorry, %1 does not exist anymore...").arg(mCurrentImage->fileName());
		emit showInfoSignal(msg, 4000);
		return;
	}

	mCurrentDir = "";
	mImages.clear();
	mCurrentImage->clear();
	setCurrentImage(mCurrentImage);
	loadDir(mCurrentImage->dirPath());
	load(mCurrentImage);
}

void DkImageLoader::showOnMap() {

	QSharedPointer<DkMetaDataT> metaData = getCurrentImage()->getMetaData();

	if (!DkMetaDataHelper::getInstance().hasGPS(metaData)) {
		emit showInfoSignal(tr("Sorry, I could not find the GPS coordinates..."));
		return;
	}

	QDesktopServices::openUrl(QUrl(DkMetaDataHelper::getInstance().getGpsCoordinates(metaData)));
}

void DkImageLoader::load(const QString& filePath) {

	bool hasZipMarker = false;

#ifdef WITH_QUAZIP
	hasZipMarker = filePath.contains(DkZipContainer::zipMarker()) != 0;
#endif

	if (QFileInfo(filePath).isFile() || hasZipMarker) {
		QSharedPointer<DkImageContainerT> newImg = findOrCreateFile(filePath);
		setCurrentImage(newImg);
		load(mCurrentImage);
	}
	else
		firstFile();
	
	// if here is a folder upate bug - this was before -- if (QFileInfo(filePath).isFile() || hasZipMarker) { 
	loadDir(QFileInfo(filePath).absolutePath());
}

void DkImageLoader::load(QSharedPointer<DkImageContainerT> image /* = QSharedPointer<DkImageContainerT> */) {

	if (!image)
		return;

#ifdef WITH_QUAZIP
	bool isZipArchive = DkBasicLoader::isContainer(image->filePath());

	if (isZipArchive) {
		loadZipArchive(image->filePath());
		firstFile();
		return;
	}
#endif

	setCurrentImage(image);

	if (mCurrentImage && mCurrentImage->getLoadState() == DkImageContainerT::loading)
		return;

	emit updateSpinnerSignalDelayed(true);
	bool loaded = mCurrentImage->loadImageThreaded();	// loads file threaded
	
	if (!loaded)
		emit updateSpinnerSignalDelayed(false);

	// if loaded is false, we definitively know that the file does not exist -> early exception here?
}

void DkImageLoader::imageLoaded(bool loaded /* = false */) {
	
	emit updateSpinnerSignalDelayed(false);

	if (!mCurrentImage) {
		return;
	}

	emit imageLoadedSignal(mCurrentImage, loaded);

	if (!loaded)
		return;

	emit imageUpdatedSignal(mCurrentImage);

	if (mCurrentImage) {
		// this signal is needed by the folder scrollbar
		int idx = findFileIdx(mCurrentImage->filePath(), mImages);
		emit imageUpdatedSignal(idx);
	}

	QApplication::sendPostedEvents();	// force an event post here

	updateCacher(mCurrentImage);
	updateHistory();

	if (mCurrentImage)
		emit imageHasGPSSignal(DkMetaDataHelper::getInstance().hasGPS(mCurrentImage->getMetaData()));

	// update status bar info
	if (mCurrentImage && !mImages.empty() && mImages.indexOf(mCurrentImage) >= 0)
		DkStatusBarManager::instance().setMessage(tr("%1 of %2").arg(mImages.indexOf(mCurrentImage)+1).arg(mImages.size()), DkStatusBar::status_filenumber_info);
	else
		DkStatusBarManager::instance().setMessage("", DkStatusBar::status_filenumber_info);

}

void DkImageLoader::downloadFile(const QUrl& url) {

	QSharedPointer<DkImageContainerT> newImg = findOrCreateFile(QString());
	setCurrentImage(newImg);
	newImg->downloadFile(url);
	emit updateSpinnerSignalDelayed(true);
}

/**
 * Saves a temporary file to the folder specified in DkSettingsManager.
 * @param img the image (which was in most cases pasted to nomacs)
 **/ 
QString DkImageLoader::saveTempFile(const QImage& img, const QString& name, const QString& fileExt, bool force, bool threaded) {

	QString filePath = DkSettingsManager::param().global().tmpPath;
	QFileInfo fInfo(filePath + QDir::separator());

	if (!force && (filePath.isEmpty() || !fInfo.exists())) {
		
		if (!filePath.isEmpty())
			qWarning() << filePath << "does not exist";
		return QString();
	}
	else if (filePath.isEmpty() || !fInfo.exists()) {

		fInfo = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

		if (!fInfo.isDir()) {
			// load system default open dialog
			QString dirName = QFileDialog::getExistingDirectory(
				DkUtils::getMainWindow(), 
				tr("Save Directory"), 
				getDirPath(),
				QFileDialog::ShowDirsOnly | DkDialog::fileDialogOptions()
			);

			fInfo = dirName + QDir::separator();

			if (!fInfo.exists())
				return QString();
		}
	}

	qInfo() << "saving to: " << fInfo.absolutePath();
	
	QString fileName = name + "-" + DkUtils::nowString() + fileExt;
	fInfo = QFileInfo(fInfo.absolutePath(), fileName);

	if (!fInfo.exists()) {
			
		saveFile(fInfo.absoluteFilePath(), img, "", -1, threaded);
		return fInfo.absoluteFilePath();
	}

	return QString();
}

void DkImageLoader::saveFileWeb(const QImage& saveImg) {
	
	QWidget* dialogParent = DkUtils::getMainWindow();
	QString saveName;
	QFileInfo saveFileInfo;

	if (hasFile()) {
		saveFileInfo = QFileInfo(getSavePath(), fileName());
		qDebug() << "save path: " << saveFileInfo.absoluteFilePath();
	}

	bool imgHasAlpha = DkImage::alphaChannelUsed(saveImg);

	QString suffix = imgHasAlpha ? ".png" : ".jpg";
	QString saveFilterGui;

	for (int idx = 0; idx < DkSettingsManager::param().app().saveFilters.size(); idx++) {

		if (DkSettingsManager::param().app().saveFilters.at(idx).contains(suffix)) {
			saveFilterGui = DkSettingsManager::param().app().saveFilters.at(idx);
			break;
		}
	}

	if (saveFileInfo.exists())
		saveFileInfo = QFileInfo(saveFileInfo.absolutePath(), saveFileInfo.baseName() + suffix);

	QString fileName = QFileDialog::getSaveFileName(
		dialogParent, 
		tr("Save File %1").arg(saveName),
		saveFileInfo.absoluteFilePath(), 
		saveFilterGui,
		nullptr,
		DkDialog::fileDialogOptions()
	);

	if (fileName.isEmpty())
		return;

	DkCompressDialog* jpgDialog = new DkCompressDialog(dialogParent);
	jpgDialog->setDialogMode(DkCompressDialog::web_dialog);
	jpgDialog->imageHasAlpha(imgHasAlpha);
	jpgDialog->setImage(saveImg);

	if (!jpgDialog->exec())
		return;

	QImage rImg = saveImg;
	float factor = jpgDialog->getResizeFactor();
	if (factor != -1)
		rImg = DkImage::resizeImage(rImg, QSize(), factor, DkImage::ipl_area);

	saveFile(fileName, rImg, suffix, jpgDialog->getCompression());

	jpgDialog->deleteLater();
}

void DkImageLoader::copyUserFile() {
    
	// the subsequent modals destroy the active window
    QWidget* dialogParent = DkUtils::getMainWindow();
	QString saveName;

	auto imgC = getCurrentImage();

    if (hasFile() && imgC) {

        int filterIdx = -1;

		QString extension = imgC->fileInfo().suffix();

		// retrieve the extension name (that's more user friendly)
		QStringList sF = DkSettingsManager::param().app().openFilters;
		QRegExp exp = QRegExp("*." + extension + "*", Qt::CaseInsensitive);
		exp.setPatternSyntax(QRegExp::Wildcard);

		for (int idx = 1; idx < sF.size(); idx++) {

			if (exp.exactMatch(sF.at(idx))) {
				extension = sF.at(idx);
				filterIdx = idx;
				break;
			}
		}

		QString saveName = QFileInfo(getCopyPath(), imgC->fileName()).absoluteFilePath();

		saveName = QFileDialog::getSaveFileName(
			dialogParent,
			tr("Save File %1").arg(saveName),
			saveName,
			extension,
			nullptr,
			DkDialog::fileDialogOptions()
		);

        if (saveName.isEmpty())
			return;

		qDebug() << fileName() << "->" << saveName;

		if (QFile::copy(imgC->filePath(), saveName)) {
			mCopyDir = QFileInfo(saveName).absolutePath();
            qInfo() << fileName() << "copied to" << saveName;
        }
		else {
            emit showInfoSignal(tr("Sorry, I could not copy the image..."));
        }
		
    }
}

void DkImageLoader::saveUserFileAs(const QImage& saveImg, bool silent) {

	// the subsequent modals destroy the active window
	QWidget* dialogParent = DkUtils::getMainWindow();

	QString selectedFilter;
	QString saveName = fileName();
	QFileInfo saveFileInfo;

	if (hasFile()) {

		saveFileInfo = QFileInfo(getSavePath(), fileName());

		int filterIdx = -1;

		QStringList sF = DkSettingsManager::param().app().saveFilters;
		//qDebug() << sF;

		QRegExp exp = QRegExp("*." + saveFileInfo.suffix() + "*", Qt::CaseInsensitive);
		exp.setPatternSyntax(QRegExp::Wildcard);

		for (int idx = 0; idx < sF.size(); idx++) {

			if (exp.exactMatch(sF.at(idx))) {
				selectedFilter = sF.at(idx);
				filterIdx = idx;
				break;
			}
		}

		if (filterIdx == -1)
			saveName.remove("." + saveFileInfo.suffix());
	}

	QString fileName;

	int answer = QDialog::Rejected;

	// don't ask the user if save was hit & the file format is supported for saving
	if (silent && !selectedFilter.isEmpty() && isEdited()) {
		fileName = filePath();
		DkMessageBox* msg = new DkMessageBox(QMessageBox::Question, tr("Overwrite File"), 
			tr("Do you want to overwrite:\n%1?").arg(fileName), 
			(QMessageBox::Yes | QMessageBox::No), dialogParent);
		msg->setObjectName("overwriteDialog");

		answer = msg->exec();

	}
	if (answer == QDialog::Rejected || answer == QMessageBox::No) {
		// note: basename removes the whole file name from the first dot...
		QString savePath = (!selectedFilter.isEmpty()) ? saveFileInfo.absoluteFilePath() : QFileInfo(saveFileInfo.absoluteDir(), saveName).absoluteFilePath();

		fileName = QFileDialog::getSaveFileName(
			dialogParent, 
			tr("Save File %1").arg(saveName),
			savePath, 
			DkSettingsManager::param().app().saveFilters.join(";;"), 
			&selectedFilter,
			DkDialog::fileDialogOptions()
		);
	}

	if (fileName.isEmpty())
		return;

	QString ext = QFileInfo(fileName).suffix();

	if (!ext.isEmpty() && !selectedFilter.contains(ext)) {

		QStringList sF = DkSettingsManager::param().app().saveFilters;

		for (int idx = 0; idx < sF.size(); idx++) {

			if (sF.at(idx).contains(ext)) {
				selectedFilter = sF.at(idx);
				break;
			}
		}
	}

	QFileInfo sFile = QFileInfo(fileName);
	int compression = -1;	// default value

	DkCompressDialog* jpgDialog = 0;
	QImage lSaveImg = saveImg;

	if (selectedFilter.contains(QRegExp("(jpg|jpeg|j2k|jp2|jpf|jpx)", Qt::CaseInsensitive))) {
		
		if (!jpgDialog)
			jpgDialog = new DkCompressDialog(dialogParent);

		if (selectedFilter.contains(QRegExp("(j2k|jp2|jpf|jpx)")))
			jpgDialog->setDialogMode(DkCompressDialog::j2k_dialog);
		else
			jpgDialog->setDialogMode(DkCompressDialog::jpg_dialog);

		jpgDialog->imageHasAlpha(saveImg.hasAlphaChannel());
		jpgDialog->setImage(saveImg);
		jpgDialog->setMinimumSize(450, 150);

		if (!jpgDialog->exec()) {
			jpgDialog->deleteLater();
			return;
		}

		compression = jpgDialog->getCompression();

		if (saveImg.hasAlphaChannel()) {

			QRect imgRect = QRect(QPoint(), saveImg.size());
			QImage tmpImg = QImage(saveImg.size(), QImage::Format_RGB32);
			QPainter painter(&tmpImg);
			painter.fillRect(imgRect, jpgDialog->getBackgroundColor());
			painter.drawImage(imgRect, saveImg, imgRect);

			lSaveImg = tmpImg;
		}
	}

	if (selectedFilter.contains("webp")) {

		if (!jpgDialog)
			jpgDialog = new DkCompressDialog(dialogParent);

		jpgDialog->setDialogMode(DkCompressDialog::webp_dialog);
		jpgDialog->setImage(saveImg);

		if (!jpgDialog->exec()) {
			jpgDialog->deleteLater();
			return;
		}

		compression = jpgDialog->getCompression();
	}

	if (selectedFilter.contains("avif")) {

		if (!jpgDialog)
			jpgDialog = new DkCompressDialog(dialogParent);

		jpgDialog->setDialogMode(DkCompressDialog::avif_dialog);
		jpgDialog->setImage(saveImg);

		if (!jpgDialog->exec()) {
			jpgDialog->deleteLater();
			return;
		}

		compression = jpgDialog->getCompression();
	}

	DkTifDialog* tifDialog = 0;

	if (selectedFilter.contains("tif")) {

		if (!tifDialog)
			tifDialog = new DkTifDialog(dialogParent);

		if (!tifDialog->exec()) {
			tifDialog->deleteLater();
			return;
		}

		compression = tifDialog->getCompression();
	}

	saveFile(sFile.absoluteFilePath(), lSaveImg, selectedFilter, compression);

	if (tifDialog)
		tifDialog->deleteLater();
	if (jpgDialog)
		jpgDialog->deleteLater();

}

/**
 * Saves a file (not threaded!)
 * If the file already exists, it will be replaced.
 * @param file the file name/path
 * @param fileFilter the file extension (e.g. *.jpg)
 * @param saveImg the image to be saved
 * @param compression the compression method (for jpg, tif)
 **/ 
void DkImageLoader::saveFile(const QString& filePath, const QImage& saveImg, const QString& fileFilter, int compression, bool threaded) {
	
	QSharedPointer<DkImageContainerT> imgC = (mCurrentImage) ? mCurrentImage : findOrCreateFile(filePath);
	setCurrentImage(imgC);

	if (saveImg.isNull() && (!mCurrentImage || !mCurrentImage->hasImage()))
		emit showInfoSignal(tr("Sorry, I cannot save an empty image..."));

	// if the user did not specify the suffix - append the suffix of the file filter
	QString newSuffix = QFileInfo(filePath).suffix();
	QString lFilePath = filePath;
	if (newSuffix == "") {
		
		QString lFileFilter = fileFilter;
		newSuffix = lFileFilter.remove(0, fileFilter.indexOf("."));
		printf("new suffix: %s\n", newSuffix.toStdString().c_str());

		int endSuffix = -1;
		if (newSuffix.indexOf(")") == -1)
			endSuffix =  newSuffix.indexOf(" ");
		else if (newSuffix.indexOf(" ") == -1)
			endSuffix =  newSuffix.indexOf(")");
		else
			endSuffix = qMin(newSuffix.indexOf(")"), newSuffix.indexOf(" "));

		lFilePath.append(newSuffix.left(endSuffix));
	}

	emit updateSpinnerSignalDelayed(true);
	QImage sImg = (saveImg.isNull()) ? imgC->image() : saveImg;

	mDirWatcher->blockSignals(true);
	bool saveStarted = (threaded) ? imgC->saveImageThreaded(lFilePath, sImg, compression) : imgC->saveImage(lFilePath, sImg, compression);

	if (!saveStarted) {
		imageSaved(QString(), false);
	}
	else if (saveStarted && !threaded) {
		imageSaved(lFilePath);
	}
}

void DkImageLoader::imageSaved(const QString& filePath, bool saved, bool loadToTab) {

	emit updateSpinnerSignalDelayed(false);
	mDirWatcher->blockSignals(false);

	QFileInfo fInfo(filePath);
	if (!fInfo.exists() || !fInfo.isFile() || !saved)
		return;

	if (DkSettingsManager::instance().param().resources().loadSavedImage == DkSettings::ls_load_to_tab && loadToTab) {
		emit loadImageToTab(filePath);
	}
	else if (DkSettingsManager::instance().param().resources().loadSavedImage == DkSettings::ls_load) {
		emit imageLoadedSignal(mCurrentImage, true);
		emit imageUpdatedSignal(mCurrentImage);
		qDebug() << "image updated: " << mCurrentImage->fileName();
	}
	else {
		mFolderUpdated = true;
		loadDir(mCurrentImage->dirPath());
	}
}

/**
 * Updates the file history.
 * The file history stores the last 10 folders.
 **/ 
void DkImageLoader::updateHistory() {

	if (!DkSettingsManager::param().global().logRecentFiles || DkSettingsManager::param().app().privateMode)
		return;

	if (!mCurrentImage || mCurrentImage->hasImage() != DkImageContainer::loaded || !mCurrentImage->exists())
		return;

	QFileInfo file = mCurrentImage->filePath();

	// sync with other instances
	DefaultSettings settings;
	settings.beginGroup("GlobalSettings");
	QStringList rFolders = settings.value("recentFolders", DkSettingsManager::param().global().recentFolders).toStringList();
	QStringList rFiles = settings.value("recentFiles", DkSettingsManager::param().global().recentFiles).toStringList();
	settings.endGroup();

	rFiles.removeAll(file.absoluteFilePath());
	rFolders.removeAll(file.absolutePath());

	QStringList tmpRecentFiles;

	// try to collect images from different folders
	for (const QString& cFile : DkSettingsManager::param().global().recentFiles) {
		
		QFileInfo fi(cFile);

		if (fi.absolutePath() == file.absolutePath())
			tmpRecentFiles.append(cFile);
	}

	// maximum 5 most recent images from the same folder
	for (int idx = tmpRecentFiles.size()-1; idx > 3; idx--) {
		rFiles.removeAll(tmpRecentFiles.at(idx));
	}

	rFiles.push_front(file.absoluteFilePath());
	rFolders.push_front(file.absolutePath());

	rFiles.removeDuplicates();
	rFolders.removeDuplicates();

	for (int idx = 0; idx < rFiles.size()-DkSettingsManager::param().global().numFiles-10; idx++)
		rFiles.pop_back();

	for (int idx = 0; idx < rFolders.size()-DkSettingsManager::param().global().numFiles-10; idx++)
		rFolders.pop_back();

	// sync with other instances
	settings.beginGroup("GlobalSettings");
	settings.setValue("recentFolders", rFolders);
	settings.setValue("recentFiles", rFiles);
	settings.endGroup();

	// update
	DkSettingsManager::param().global().lastDir = file.absolutePath();
	DkSettingsManager::param().global().recentFiles = rFiles;
	DkSettingsManager::param().global().recentFolders = rFolders;


	//DkSettings s = DkSettings();
	//s.save();
}

// image manipulation --------------------------------------------------------------------
/**
 * Deletes the currently loaded file.
 **/ 
bool DkImageLoader::deleteFile() {
	
	if (mCurrentImage && mCurrentImage->exists()) {

		QString fileName = mCurrentImage->fileName();

		if (DkUtils::moveToTrash(mCurrentImage->filePath())) {
			QSharedPointer<DkImageContainerT> imgC = getSkippedImage(1);
			load(imgC);
			emit showInfoSignal(tr("%1 deleted...").arg(fileName));
			return true;
		}
		else
			emit showInfoSignal(tr("Sorry, I could not delete: %1").arg(fileName));
	}

	return false;
}

/**
 * Rotates the image.
 * First, we try to set the rotation flag in the metadata
 * (this is the fastest way to rotate an image).
 * If this does not work, the image matrix is rotated.
 * @param angle the rotation angle in degree.
 **/ 
void DkImageLoader::rotateImage(double angle) {

	qDebug() << "rotating image...";

	if (!mCurrentImage || !mCurrentImage->hasImage()) {
		qDebug() << "sorry, loader has no image";
		return;
	}

	QImage img = DkImage::rotate(mCurrentImage->image(), qRound(angle));

	QImage thumb = DkImage::createThumb(mCurrentImage->image());
	mCurrentImage->getThumb()->setImage(thumb);

	QSharedPointer<DkMetaDataT> metaData = mCurrentImage->getMetaData();
	bool metaDataSet = false;

	if (metaData->hasMetaData() && DkSettingsManager::param().metaData().saveExifOrientation) {
		try {
			if (!metaData->isJpg())
				metaData->setThumbnail(thumb);
			metaData->setOrientation(qRound(angle));
			metaDataSet = true;

			// if that is working out, we need to set the image without changing the history
			QVector<DkEditImage>* imgs = mCurrentImage->getLoader()->history();

			if (!imgs->isEmpty()) {
				imgs->last().setImage(img);
			}

		}
		catch (...) {
		}
	}

	if (!metaDataSet) {
		setImage(img, tr("Rotated"), mCurrentImage->filePath());
	}

	emit imageUpdatedSignal(mCurrentImage);
}

/**
 * Restores files that were destroyed by the Exiv2 lib.
 * If a watch (or some other read lock) is on a file, the
 * Exiv2 lib is known do damage the files on Windows.
 * This function restores these files.
 * @param fileInfo the file to be restored.
 * @return bool true if the file could be restored.
 **/ 
bool DkImageLoader::restoreFile(const QString& filePath) {

	QFileInfo fInfo(filePath);
	QStringList files = fInfo.dir().entryList();
	QString fileName = fInfo.fileName();
	QRegExp filePattern(fileName + "[0-9]+");
	QString backupFileName;

	// if exif crashed it saved a backup file with the format: filename.png1232
	for (int idx = 0; idx < files.size(); idx++) {

		if (filePattern.exactMatch(files[idx])) {
			backupFileName = files[idx];
			break;
		}
	}

	if (backupFileName.isEmpty()) {
		qDebug() << "I could not locate the backup file...";
		return true;
	}

	// delete the destroyed file
	QFile file(filePath);
	QFile backupFile(fInfo.absolutePath() + QDir::separator() + backupFileName);

	if (file.size() == 0 || file.size() <= backupFile.size()) {
		
		if (!file.remove()) {

			// ok I did not destroy the original file - so delete the back-up
			// -> this reverts the file - but otherwise we spam to the disk 
			// actions reverted here include meta data saving
			if (file.size() != 0)
				return backupFile.remove();

			qDebug() << "I could not remove the file...";
			return false;
		}
	}
	else {
		
		qDebug() << "non-empty file: " << fileName << " I won't delete it...";
		qDebug() << "file size: " << file.size() << " back-up file size: " << backupFile.size();
		return false;
	}

	// now 
	return backupFile.rename(fInfo.absoluteFilePath());
}

/**
 * Reloads the file index if the directory was edited.
 * @param path the path to the current directory
 **/ 
void DkImageLoader::directoryChanged(const QString& path) {

	if (path.isEmpty() || path == mCurrentDir) {

		mFolderUpdated = true;
		
		// guarantee, that only every XX seconds a folder update occurs
		// think of a folder where 100s of files are written to...
		// as this could be pretty fast, the thumbsloader (& whoever) would create a 
		// greater offset and slow down the system
		if ((path.isEmpty() && mTimerBlockedUpdate) || (!path.isEmpty() && !mDelayedUpdateTimer.isActive())) {

			loadDir(mCurrentDir, false);
			mTimerBlockedUpdate = false;

			if (!path.isEmpty())
				mDelayedUpdateTimer.start(1000);
		}
		else
			mTimerBlockedUpdate = true;
	}
	
}

/**
 * Returns true if a file was specified.
 * @return bool true if a file name/path was specified
 **/ 
bool DkImageLoader::hasFile() const {

	return mCurrentImage && mCurrentImage->exists();
}

bool DkImageLoader::hasMovie() const {

	if (!mCurrentImage || !mCurrentImage->exists())
		return false;

	return mCurrentImage->hasMovie();
}

bool DkImageLoader::hasSvg() const {

	if (!mCurrentImage || !mCurrentImage->exists())
		return false;

	return mCurrentImage->hasSvg();
}

/**
 * Returns the currently loaded file information.
 * @return QFileInfo the current file info
 **/ 
QString DkImageLoader::filePath() const {

	if (!mCurrentImage)
		return QString();

	
	return mCurrentImage->filePath();
}

QSharedPointer<DkImageContainerT> DkImageLoader::getCurrentImage() const {

	return mCurrentImage;
}

QSharedPointer<DkImageContainerT> DkImageLoader::getLastImage() const {

	return mLastImageLoaded;
}

/**
 * Returns the currently loaded directory.
 * @return QDir the currently loaded directory.
 **/ 
QString DkImageLoader::getDirPath() const {

	return mCurrentDir;
}

QStringList DkImageLoader::getFoldersRecursive(const QString& dirPath) {

	//DkTimer dt;
	QStringList subFolders;
	//qDebug() << "scanning recursively: " << dir.absolutePath();

	if (DkSettingsManager::param().global().scanSubFolders) {

		QDirIterator dirs(dirPath, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
	
		int nFolders = 0;
		while (dirs.hasNext()) {
			dirs.next();
			subFolders << dirs.filePath();
			nFolders++;

			if (nFolders > 100)
				break;
			
			//getFoldersRecursive(dirs.filePath(), subFolders);
			//qDebug() << "loop: " << dirs.filePath();
		}
	}	

	subFolders << dirPath;

	qSort(subFolders.begin(), subFolders.end(), DkUtils::compLogicQString);

	qDebug() << dirPath << "loaded recursively...";
	
	//qDebug() << "scanning folders recursively took me: " << QString::fromStdString(dt.getTotal());
	return subFolders;
}

QFileInfoList DkImageLoader::updateSubFolders(const QString& rootDirPath) {
	
	mSubFolders = getFoldersRecursive(rootDirPath);
	QFileInfoList files;
	qDebug() << mSubFolders;

	// find the first subfolder that has images
	for (int idx = 0; idx < mSubFolders.size(); idx++) {
		mCurrentDir = mSubFolders[idx];
		files = getFilteredFileInfoList(mCurrentDir, mIgnoreKeywords, mKeywords);		// this line takes seconds if you have lots of files and slow loading (e.g. network)
		if (!files.empty())
			break;
	}

	return files;
}

int DkImageLoader::getNextFolderIdx(int folderIdx) {
	
	int nextIdx = -1;

	if (mSubFolders.empty())
		return nextIdx;

	// find the first sub folder that has images
	for (int idx = 1; idx < mSubFolders.size(); idx++) {
		
		int tmpNextIdx = folderIdx + idx;

		if (DkSettingsManager::param().global().loop)
			tmpNextIdx %= mSubFolders.size();
		else if (tmpNextIdx >= mSubFolders.size())
			return -1;

		QDir cDir = mSubFolders[tmpNextIdx];
		QFileInfoList cFiles = getFilteredFileInfoList(cDir.absolutePath(), mIgnoreKeywords, mKeywords);		// this line takes seconds if you have lots of files and slow loading (e.g. network)
		if (!cFiles.empty()) {
			nextIdx = tmpNextIdx;
			break;
		}
	}

	return nextIdx;
}

int DkImageLoader::getPrevFolderIdx(int folderIdx) {
	
	int prevIdx = -1;

	if (mSubFolders.empty())
		return prevIdx;

	// find the first sub folder that has images
	for (int idx = 1; idx < mSubFolders.size(); idx++) {

		int tmpPrevIdx = folderIdx - idx;

		if (DkSettingsManager::param().global().loop && tmpPrevIdx < 0)
			tmpPrevIdx += mSubFolders.size();
		else if (tmpPrevIdx < 0)
			return -1;

		QDir cDir = mSubFolders[tmpPrevIdx];
		QFileInfoList cFiles = getFilteredFileInfoList(cDir.absolutePath(), mIgnoreKeywords, mKeywords);		// this line takes seconds if you have lots of files and slow loading (e.g. network)
		if (!cFiles.empty()) {
			prevIdx = tmpPrevIdx;
			break;
		}
	}

	return prevIdx;
}

void DkImageLoader::errorDialog(const QString& msg) const {

	QMessageBox errorDialog(qApp->activeWindow());
	errorDialog.setWindowTitle(tr("Error"));
	errorDialog.setIcon(QMessageBox::Critical);
	errorDialog.setText(msg);
	errorDialog.show();

	errorDialog.exec();
}

void DkImageLoader::updateCacher(QSharedPointer<DkImageContainerT> imgC) {

	if (!imgC || !DkSettingsManager::param().resources().cacheMemory)
		return;

	DkTimer dt;

	//// no caching? delete all
	//if (!DkSettingsManager::param().resources().cacheMemory) {
	//	for (int idx = 0; idx < images.size(); idx++) {
	//		images.at(idx)->clear();
	//	}
	//	return;
	//}

	int cIdx = findFileIdx(imgC->filePath(), mImages);
	double mem = 0;
	double totalMem = 0;

	if (cIdx == -1) {
		qWarning() << "WARNING: image not found for caching!";
		return;
	}

	for (int idx = 0; idx < mImages.size(); idx++) {

		auto cImg = mImages.at(idx);

		// clear images if they are edited
		if (idx != cIdx && cImg->isEdited()) {
			cImg->clear();
			continue;
		}

		// do not count the last & the current image
		if (idx > cIdx && idx <= cIdx+DkSettingsManager::param().resources().maxImagesCached)
			mem += cImg->getMemoryUsage();
		else if (abs(cIdx-idx) > 1) {
			cImg->clear();
			if (cImg->hasImage())
				qDebug() << "[Cacher]" << cImg->filePath() << "freed";

			continue;
		}
		else
			totalMem += cImg->getMemoryUsage();

		// ignore the last and current one
		if (idx == cIdx-1 || idx == cIdx) {
			continue;
		}
		// fully load the next image
		else if (idx == cIdx+1 && mem < DkSettingsManager::param().resources().cacheMemory && mImages.at(idx)->getLoadState() == DkImageContainerT::not_loaded) {
			cImg->loadImageThreaded();
			qDebug() << "[Cacher] " <<cImg->filePath() << " fully cached...";
		}
		else if (idx > cIdx && idx < cIdx+DkSettingsManager::param().resources().maxImagesCached-2 && mem < DkSettingsManager::param().resources().cacheMemory && mImages.at(idx)->getLoadState() == DkImageContainerT::not_loaded) {
			//dt.getIvl();
			mImages.at(idx)->fetchFile();		// TODO: crash detected here
			qDebug() << "[Cacher] " << cImg->filePath() << " file fetched...";
		}
	}

	qDebug() << "[Cacher] created in" << dt << "(" << mem + totalMem << "MB)";
}

/**
 * Returns the file list of the directory dir.
 * Note: this function might get slow if lots of files (> 10000) are in the
 * directory or if the directory is in the net.
 * Currently the file list is sorted according to the system specification.
 * @param dir the directory to load the file list from.
 * @param ignoreKeywords if one of these keywords is in the file name, the file will be ignored.
 * @param keywords if one of these keywords is not in the file name, the file will be ignored.
 * @return QStringList all filtered files of the current directory.
 **/ 
QFileInfoList DkImageLoader::getFilteredFileInfoList(const QString& dirPath, QStringList ignoreKeywords, QStringList keywords, QString folderKeywords) {

	DkTimer dt;

	if (dirPath.isEmpty())
		return QFileInfoList();

#ifdef Q_OS_WIN

	QString winPath = QDir::toNativeSeparators(dirPath) + "\\*.*";

	const wchar_t* fname = reinterpret_cast<const wchar_t *>(winPath.utf16());

	WIN32_FIND_DATAW findFileData;
	HANDLE MyHandle = FindFirstFileW(fname, &findFileData);

	std::vector<std::wstring> fileNameList;
	std::wstring fileName;

	if( MyHandle != INVALID_HANDLE_VALUE) {
		
		do {

			fileName = findFileData.cFileName;
			fileNameList.push_back(fileName);	// TODO: sort correct according to numbers
		} while(FindNextFileW(MyHandle, &findFileData) != 0);
	}

	FindClose(MyHandle);
	
	// remove the * in fileFilters
	QStringList fileFiltersClean = DkSettingsManager::param().app().browseFilters;
	for (QString& filter : fileFiltersClean)
		filter.replace("*", "");
	
	//qDebug() << "browse filters: " << DkSettingsManager::param().app().browseFilters;

	QStringList fileList;
	std::vector<std::wstring>::iterator lIter = fileNameList.begin();

	// convert to QStringList
	for (unsigned int idx = 0; idx < fileNameList.size(); idx++, lIter++) {
		
		QString qFilename = DkUtils::stdWStringToQString(*lIter);

		// believe it or not, but this is 10 times faster than QRegExp
		// drawback: we also get files that contain *.jpg*
		for (int i = 0; i < fileFiltersClean.size(); i++) {

			if (qFilename.contains(fileFiltersClean[i], Qt::CaseInsensitive)) {
				fileList.append(qFilename);
				break;
			}
		}
	}

	qInfoClean() << "WinAPI, indexed (" << fileList.size() <<") files in: " << dt;
#else

	// true file list
	QDir tmpDir(dirPath);
	tmpDir.setSorting(QDir::LocaleAware);
	QStringList fileList = tmpDir.entryList(DkSettingsManager::param().app().browseFilters);

#endif

	// append files with no suffix
	QDir cDir(dirPath);
	QStringList allFiles = cDir.entryList();
	QStringList noSuffixFiles;

	for (const QString& name : allFiles) {
		
		if (!name.contains(".") && DkUtils::isValid(QFileInfo(dirPath, name))) {
			fileList << name;
		}
	}

	// remove files that contain ignore keywords
	for (int idx = 0; idx < ignoreKeywords.size(); idx++) {
		QRegExp exp = QRegExp("^((?!" + ignoreKeywords[idx] + ").)*$");
		exp.setCaseSensitivity(Qt::CaseInsensitive);
		fileList = fileList.filter(exp);
	}

	for (int idx = 0; idx < keywords.size(); idx++) {
		fileList = fileList.filter(keywords[idx], Qt::CaseInsensitive);
	}

	if (folderKeywords != "") {
		QStringList filterList = fileList;
		fileList = DkUtils::filterStringList(folderKeywords, filterList);
	}

	if (DkSettingsManager::param().resources().filterDuplicats) {

		QString preferredExtension = DkSettingsManager::param().resources().preferredExtension;
		preferredExtension = preferredExtension.replace("*.", "");
		qDebug() << "preferred extension: " << preferredExtension;

		QStringList resultList = fileList;
		fileList.clear();
		
		for (int idx = 0; idx < resultList.size(); idx++) {
			
			QFileInfo cFName = QFileInfo(resultList.at(idx));

			if (preferredExtension.compare(cFName.suffix(), Qt::CaseInsensitive) == 0) {
				fileList.append(resultList.at(idx));
				continue;
			}

			QString cFBase = cFName.baseName();
			bool remove = false;

			for (int cIdx = 0; cIdx < resultList.size(); cIdx++) {

				QString ccBase = QFileInfo(resultList.at(cIdx)).baseName();

				if (cIdx != idx && ccBase == cFBase && resultList.at(cIdx).contains(preferredExtension, Qt::CaseInsensitive)) {
					remove = true;
					break;
				}
			}
			
			if (!remove)
				fileList.append(resultList.at(idx));
		}
	}

	//fileList = sort(fileList, dir);

	QFileInfoList fileInfoList;
	
	for (int idx = 0; idx < fileList.size(); idx++)
		fileInfoList.append(QFileInfo(mCurrentDir, fileList.at(idx)));

	return fileInfoList;
}

void DkImageLoader::sort() {
	
	qSort(mImages.begin(), mImages.end(), imageContainerLessThanPtr);
	emit updateDirSignal(mImages);
}

void DkImageLoader::currentImageUpdated() const {

	if (mCurrentImage.isNull())
		return;

	emit imageUpdatedSignal(mCurrentImage);
}

/**
 * Returns the directory where files are copied to.
 * @return QDir the directory where the user copied the last file to.
 **/
QString DkImageLoader::getCopyPath() const {
    if (mCopyDir.isEmpty() || !QDir(mCopyDir).exists())
        return mCurrentDir;
    else
        return mCopyDir;
}

/**
 * Returns the directory where files are saved to.
 * @return QDir the directory where the user saved the last file to.
 **/ 
QString DkImageLoader::getSavePath() const {

	if (mSaveDir.isEmpty() || !QDir(mSaveDir).exists())
		return mCurrentDir;
	else
		return mSaveDir;
}

/**
* Returns if an image is loaded currently.
* @return bool true if an image is loaded.
**/ 
bool DkImageLoader::hasImage() const {
		
	return mCurrentImage && mCurrentImage->hasImage();
}

bool DkImageLoader::isEdited() const {
	return mCurrentImage && mCurrentImage->isEdited();
}

int DkImageLoader::numFiles() const {
	return mImages.size();
}

void DkImageLoader::undo() {

	if (!mCurrentImage)
		return;

	mCurrentImage->undo();
}

void DkImageLoader::redo() {

	if (!mCurrentImage)
		return;

	mCurrentImage->redo();
}

/**
	* Returns the currently loaded image.
	* @return QImage the current image
	**/ 
QImage DkImageLoader::getImage() {
		
	if (!mCurrentImage)
		return QImage();

	return mCurrentImage->image();
}

bool DkImageLoader::dirtyTiff() {

	if (!mCurrentImage)
		return false;

	return mCurrentImage->getLoader()->isDirty();
}

QStringList DkImageLoader::ignoreKeywords() const {
	return mIgnoreKeywords;
}

void DkImageLoader::setIgnoreKeywords(const QStringList& ignoreKeywords) {
	mIgnoreKeywords = ignoreKeywords;
}

void DkImageLoader::appendIgnoreKeyword(const QString& keyword) {
	mIgnoreKeywords.append(keyword);
}

QStringList DkImageLoader::keywords() const {
	return mKeywords;
}

void DkImageLoader::setKeywords(const QStringList& keywords) {
	mKeywords = keywords;
}

void DkImageLoader::appendKeyword(const QString& keyword) {
	mKeywords.append(keyword);
}

void DkImageLoader::loadLastDir() {

	if (DkSettingsManager::param().global().recentFolders.empty())
		return;

	setDir(DkSettingsManager::param().global().recentFolders[0]);
}

void DkImageLoader::setFolderFilters(const QStringList& filter) {

	setFolderFilter(filter.join(" "));
}

void DkImageLoader::setFolderFilter(const QString& filter) {

	mFolderFilterString = filter;
	mFolderUpdated = true;
	loadDir(mCurrentDir);	// simulate a folder update operation
}

QString DkImageLoader::getFolderFilter() {
	return mFolderFilterString;
}

QStringList DkImageLoader::getFolderFilters() {
	return mFolderFilterString.split(" ");
}

/**
 * Sets the current directory to dir.
 * @param dir the directory to be loaded.
 **/ 
void DkImageLoader::setDir(const QString& dir) {

	//QDir oldDir = file.absoluteDir();
	
	bool valid = loadDir(dir);

	if (valid)
		firstFile();
}

/**
 * Sets a new save directory.
 * @param dir the new save directory.
 **/ 
void DkImageLoader::setSaveDir(const QString& dirPath) {
	mSaveDir = dirPath;
}

/**
 * Sets the current image to img.
 * @param img the loader's new image.
 **/ 
QSharedPointer<DkImageContainerT> DkImageLoader::setImage(const QImage& img, const QString& editName, const QString& editFilePath) {
	
	qDebug() << "edited file: " << editFilePath;

	QSharedPointer<DkImageContainerT> newImg = findOrCreateFile(editFilePath);
	newImg->setImage(img, editName, editFilePath);
	
	setCurrentImage(newImg);
	emit imageUpdatedSignal(mCurrentImage);

	return newImg;
}
 
QSharedPointer<DkImageContainerT> DkImageLoader::setImage(QSharedPointer<DkImageContainerT> img) {

	setCurrentImage(img);
	emit imageUpdatedSignal(mCurrentImage);

	return img;
}

/**
 * Returns the current file name.
 * @return QString the file name of the currently loaded file.
 **/ 
QString DkImageLoader::fileName() const {
	
	if (!mCurrentImage || !mCurrentImage->exists())
		return QString();
	
	return mCurrentImage->fileName();
}

}
