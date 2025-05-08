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

#include "DkActionManager.h"
#include "DkBasicLoader.h"
#include "DkDialog.h"
#include "DkImageContainer.h"
#include "DkImageStorage.h"
#include "DkMessageBox.h"
#include "DkMetaData.h"
#include "DkSaveDialog.h"
#include "DkSettings.h"
#include "DkStatusBar.h"
#include "DkThumbs.h"
#include "DkTimer.h"
#include "DkUtils.h"
#include <utility>

#pragma warning(push, 0) // no warnings from includes - begin
#include <QApplication>
#include <QBuffer>
#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QImageReader>
#include <QImageWriter>
#include <QMessageBox>
#include <QMovie>
#include <QMutex>
#include <QPainter>
#include <QPluginLoader>
#include <QProgressDialog>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QStringBuilder>
#include <QStringList>
#include <QThread>
#include <QTimer>
#include <QWidget>
#include <QWriteLocker>
#include <QtConcurrentRun>
#include <qmath.h>

// opencv
#ifdef WITH_OPENCV

#ifdef Q_OS_WIN
#pragma warning(disable : 4996)
#endif
#endif

#ifdef WITH_LIBRAW
#include <libraw/libraw.h>
#elif defined(Q_OS_WIN) && !defined(SOCK_STREAM)
#include <winsock2.h> // needed since libraw 0.16
#endif

#pragma warning(pop) // no warnings from includes - end

namespace nmc
{
DkImageLoader::DkImageLoader()
{
    qRegisterMetaType<QFileInfo>("QFileInfo");

    mDirWatcher = new QFileSystemWatcher(this);
    connect(mDirWatcher, &QFileSystemWatcher::directoryChanged, this, &DkImageLoader::directoryChanged);

    mSortingIsDirty = false;
    mSortingImages = false;

    connect(&mCreateImageWatcher, &QFutureWatcher<QVector<QSharedPointer<DkImageContainerT>>>::finished, this, &DkImageLoader::imagesSorted);

    mDelayedUpdateTimer.setSingleShot(true);
    connect(&mDelayedUpdateTimer, &QTimer::timeout, this, [this]() {
        directoryChanged();
    });

    connect(DkActionManager::instance().action(DkActionManager::menu_file_save_copy), &QAction::triggered, this, &DkImageLoader::copyUserFile);
    connect(DkActionManager::instance().action(DkActionManager::menu_edit_undo), &QAction::triggered, this, &DkImageLoader::undo);
    connect(DkActionManager::instance().action(DkActionManager::menu_edit_redo), &QAction::triggered, this, &DkImageLoader::redo);
    connect(DkActionManager::instance().action(DkActionManager::menu_view_gps_map), &QAction::triggered, this, &DkImageLoader::showOnMap);
    connect(DkActionManager::instance().action(DkActionManager::sc_delete_silent), &QAction::triggered, this, &DkImageLoader::deleteFile, Qt::UniqueConnection);
}

DkImageLoader::~DkImageLoader()
{
    if (mCreateImageWatcher.isRunning())
        mCreateImageWatcher.blockSignals(true);
}

/**
 * Clears the path.
 * Calling this method makes the loader forget
 * about the current directory. It also destroys
 * the currently loaded image.
 **/
void DkImageLoader::clearPath()
{
    // lastFileLoaded must exist
    if (mCurrentImage && mCurrentImage->exists()) {
        this->receiveUpdates(false);
        mLastImageLoaded = mCurrentImage;
        mImages.clear();

        // only clear the current image if it exists
        mCurrentImage.clear();
    }
}

/**
 * Loads a given directory.
 * @param newDir the directory to be loaded.
 **/
bool DkImageLoader::loadDir(const QString &newDirPath, bool scanRecursive)
{
    // if (creatingImages) {
    //	//emit showInfoSignal(tr("Indexing folder..."), 4000);	// stop showing
    //	return false;
    // }

    DkTimer dt;
    DkFileInfo info(newDirPath);

    // folder changed signal was emitted
    if (mFolderUpdated && newDirPath == mCurrentDir) {
        mFolderUpdated = false;
        DkFileInfoList files =
            DkFileInfo::readDirectory(newDirPath,
                                      mFolderFilterString); // this line takes seconds if you have lots of files and slow loading (e.g. network)

        // might get empty too (e.g. someone deletes all images)
        if (files.empty()) {
            emit showInfoSignal(tr("%1 \n does not contain any image").arg(newDirPath), 4000); // stop showing
            mImages.clear();
            emit updateDirSignal(mImages);
            return false;
        }

        // disabled threaded sorting - people didn't like it (#484 and #460)
        // if (files.size() > 2000) {
        //	createImages(files, false);
        //	sortImagesThreaded(images);
        //}
        // else
        createImages(files, true);

        qDebug() << "getting file list.....";
    }
    // new folder is loaded
    else if ((newDirPath != mCurrentDir || mImages.empty()) && !newDirPath.isEmpty() && info.isDir()) {
        DkFileInfoList files;

        // newDir.setNameFilters(DkSettingsManager::param().app().fileFilters);
        // newDir.setSorting(QDir::LocaleAware);		// TODO: extend

        // update save directory
        mCurrentDir = newDirPath;
        mFolderUpdated = false;

        mFolderFilterString.clear(); // delete key words -> otherwise user may be confused

        if (scanRecursive && DkSettingsManager::param().global().scanSubFolders)
            files = updateSubFolders(mCurrentDir);
        else
            files = DkFileInfo::readDirectory(mCurrentDir,
                                              mFolderFilterString); // this line takes seconds if you have lots of files and slow loading (e.g. network)

        // ok new folder, this should speed-up loading
        mImages.clear();

        //// TODO: creating ~120 000 images takes about 2 secs
        //// but sorting (just filenames) takes ages (on windows)
        //// so we should fix this using 2 strategies:
        //// - thread the image creation process
        //// - while loading (if the user wants to move in the folder) we could display some message (e.g. indexing dir)
        // if (files.size() > 2000) {
        //	createImages(files, false);
        //	sortImagesThreaded(mImages);
        // }
        // else
        createImages(files, true);

        qInfoClean() << newDirPath << " [" << mImages.size() << "] indexed in " << dt;
    }
    // else
    //	qDebug() << "ignoring... old dir: " << dir.absolutePath() << " newDir: " << newDir << " file size: " << images.size();

    return true;
}

void DkImageLoader::loadDirRecursive(const QString &newDirPath)
{
    this->loadDir(newDirPath, true);
}

void DkImageLoader::sortImagesThreaded(QVector<QSharedPointer<DkImageContainerT>> images)
{
    if (mSortingImages) {
        mSortingIsDirty = true;
        return;
    }

    mSortingIsDirty = false;
    mSortingImages = true;
    mCreateImageWatcher.setFuture(QtConcurrent::run([&, images] {
        return sortImages(images);
    }));

    qDebug() << "sorting images threaded...";
}

void DkImageLoader::imagesSorted()
{
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

void DkImageLoader::createImages(const DkFileInfoList &files, bool sort)
{
    // TODO: change files to QStringList
    DkTimer dt;
    QVector<QSharedPointer<DkImageContainerT>> oldImages = mImages;
    mImages.clear();

    QDate today = QDate::currentDate();

    for (const DkFileInfo &f : files) {
        const QString &fp = f.path();
        int oIdx = findFileIdx(fp, oldImages);

        // NOTE: we had this here: oIdx != -1 && QFileInfo(oldImages.at(oIdx)->filePath()).lastModified() == f.lastModified())
        // however, that did not detect file changes & slowed down the process - so I removed it...
        mImages << ((oIdx != -1) ? oldImages.at(oIdx) : QSharedPointer<DkImageContainerT>(new DkImageContainerT(f)));
    }
    qInfo() << "[DkImageLoader]" << mImages.size() << "containers created in" << dt;

    if (sort) {
        DkImageLoader::sort();
        qInfo() << "[DkImageLoader] after sorting: " << dt;

        if (mDirWatcher) {
            if (!mDirWatcher->directories().isEmpty())
                mDirWatcher->removePaths(mDirWatcher->directories());
            mDirWatcher->addPath(mCurrentDir);
        }
    }
}

QVector<QSharedPointer<DkImageContainerT>> DkImageLoader::sortImages(QVector<QSharedPointer<DkImageContainerT>> images) const
{
    // this is dead code and could crash, see sort() for a correct way to do it
    // std::sort(images.begin(), images.end(), imageContainerLessThanPtr);
    return images;
}

/**
 * Loads the ancesting or subsequent file.
 * @param skipIdx the number of files that should be skipped after/before the current file.
 * @param silent if true, no status information will be displayed.
 **/
void DkImageLoader::changeFile(int skipIdx)
{
    // if (!img.isNull() && !file.exists())
    //	return;
    // if (!file.exists() && !virtualFile.exists()) {
    //	qDebug() << virtualFile.absoluteFilePath() << "does not exist...!!!";
    //	return;
    // }

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
 * Find subsequent file at offset skipIdx
 * @param skipIdx the number of files to be skipped from the current file
 * @param recursive true if calling ourself
 * @param currFileIdx passed to recursive calls to adjust for subfolders
 * @note If scanFoldersRecursive, then all subfolders (mSubFolders) are checked,
 *       otherwise only the current folder is considered.
 * @return
 **/
QSharedPointer<DkImageContainerT> DkImageLoader::getSkippedImage(int skipIdx, bool recursive /* = false */, int currFileIdx /* = 0 */)
{
    QSharedPointer<DkImageContainerT> imgC;

    if (!mCurrentImage)
        return imgC;

    DkTimer dt;

    // load a page (e.g. within a tiff file)
    if (mCurrentImage->setPageIdx(skipIdx))
        return mCurrentImage;

    // we need the current file's index, non-trivial if there are file/folder deletions etc
    // note: the recursive calls are providing this for us
    if (!recursive) {
        // we need a valid current dir before we can proceed; it might have been modified
        // do not scan recursive here since getSkippedImage is called for every scroll and can be slow when recursing
        // note: the recursive call already did loadDir() so its fine to have this here
        // note: we don't care if this fails
        // note: loadDir will not update dir if we renamed or deleted a file, for that to happen,
        //       mFolderUpdated must also be set
        loadDir(mCurrentImage->dirPath(), false);

        // note: this makes no sense so I've disabled it for now
        // QString file = (mCurrentImage->exists()) ? mCurrentImage->filePath() : mCurrentDir;
        QString file = mCurrentImage->filePath();

        currFileIdx = findFileIdx(file, mImages);

        if (currFileIdx == -1) {
            // current file was deleted or renamed, externally or by ourself
            // locate where it would be in sorted order
            qDebug() << "missing file" << mCurrentImage->filePath();

            bool sortAscending = DkSettingsManager::param().global().sortDir == DkSettings::sort_ascending;
            const auto isLessThan = DkImageContainer::compareFunc();

            currFileIdx = 0;
            for (; currFileIdx < mImages.size(); currFileIdx++) {
                if (!sortAscending ^ isLessThan(mCurrentImage, mImages[currFileIdx]))
                    break;
            }

            if (skipIdx > 0)
                currFileIdx--; // -1 because the current file was dropped
            if (mImages.size() == currFileIdx) // last one is the closest
                currFileIdx = mImages.size() - 1;
        }
    }

    // new index in the current dir
    int newFileIdx = currFileIdx + skipIdx;

    // invalid index means we have to loop, search subfolder etc depending on settings
    bool validIndex = newFileIdx >= 0 && newFileIdx < mImages.size();

    // search subfolders
    if (!validIndex && DkSettingsManager::param().global().scanSubFolders && mSubFolders.size() > 1) {
        int currFolderIdx = mSubFolders.indexOf(mCurrentDir);
        int newFolderIdx = getSubFolderIdx(currFolderIdx, newFileIdx >= 0);

        qDebug() << mSubFolders;
        qDebug() << "new folder idx: " << newFolderIdx;

        if (newFolderIdx < 0) {
            // not a problem; the subfolder list could all be empty folders
        } else {
            int oldFileSize = mImages.size();

            qDebug() << "loading subfolder: " << mSubFolders[newFolderIdx];
            loadDir(mSubFolders.at(newFolderIdx), false); // don't scan recursive again

            if (newFileIdx >= oldFileSize) {
                skipIdx -= oldFileSize - currFileIdx; // subtract how many being skipped
                currFileIdx = 0; // restart at first image in dir
            } else {
                skipIdx += currFileIdx + 1; // add how many being skipped
                currFileIdx = mImages.size() - 1; // restart at last image in dir
            }

            qDebug() << "new skip idx: " << skipIdx << "cFileIdx: " << currFileIdx << " -----------------------------";
            return getSkippedImage(skipIdx, true, currFileIdx);
        }
    }

    // could happen if current dir was deleted; we
    // don't check status of loadDir() so we can try subfolders
    if (mImages.empty()) {
        qWarning() << "unexpected empty file list";
        return imgC;
    }

    // loop the directory
    if (DkSettingsManager::param().global().loop) {
        newFileIdx %= mImages.size();

        while (newFileIdx < 0) // should be hit once
            newFileIdx = mImages.size() + newFileIdx;
    }
    // clip to pos1 if skipIdx < -1
    else if (currFileIdx > 0 && newFileIdx < 0) {
        newFileIdx = 0;
    }
    // clip to end if skipIdx > 1
    else if (currFileIdx < mImages.size() - 1 && newFileIdx >= mImages.size()) {
        newFileIdx = mImages.size() - 1;
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

    if (newFileIdx >= 0 && newFileIdx < mImages.size())
        imgC = mImages.at(newFileIdx);

    // file requested becomes current file
    return imgC;
}

/**
 * Loads the file at index idx.
 * @param idx the file index of the file which should be loaded.
 **/
void DkImageLoader::loadFileAt(int idx)
{
    // if (basicLoader.hasImage() && !file.exists())
    //	return;

    if (mCurrentImage)
        qDebug() << "current image: " << mCurrentImage->filePath();
    else
        qDebug() << "current image is NULL";

    DkFileInfo cDir(mCurrentDir);
    Q_ASSERT(cDir.isDir());

    if (mCurrentImage && !cDir.exists())
        loadDir(mCurrentImage->dirPath());

    if (mImages.empty())
        return;

    if (cDir.exists()) {
        if (idx == -1) {
            idx = mImages.size() - 1;
        } else if (DkSettingsManager::param().global().loop) {
            idx %= mImages.size();

            while (idx < 0)
                idx = mImages.size() + idx;

        } else if (idx < 0 && !DkSettingsManager::param().global().loop) {
            QString msg = tr("You have reached the beginning");
            emit showInfoSignal(msg, 1000);
            return;
        } else if (idx >= mImages.size()) {
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

QSharedPointer<DkImageContainerT> DkImageLoader::findOrCreateFile(const QString &filePath) const
{
    QSharedPointer<DkImageContainerT> imgC = findFile(filePath);

    if (!imgC)
        imgC = QSharedPointer<DkImageContainerT>(new DkImageContainerT(DkFileInfo(filePath)));

    return imgC;
}

QSharedPointer<DkImageContainerT> DkImageLoader::findFile(const QString &filePath) const
{
    // if one image is from zip than all should be
    // for images in zip the "images[idx]->file() == file" comparison somahow does not work
    if (mImages.size() > 0) {
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

int DkImageLoader::findFileIdx(const QString &filePath, const QVector<QSharedPointer<DkImageContainerT>> &images) const
{
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

QStringList DkImageLoader::getFileNames() const
{
    QStringList fileNames;

    for (int idx = 0; idx < mImages.size(); idx++)
        fileNames.append(QFileInfo(mImages[idx]->filePath()).fileName());

    return fileNames;
}

QVector<QSharedPointer<DkImageContainerT>> DkImageLoader::getImages()
{
    loadDir(mCurrentDir);
    return mImages;
}

/**
 * Loads the first file of the current directory.
 **/
void DkImageLoader::firstFile()
{
    loadFileAt(0);
}

/**
 * Loads the last file of the current directory.
 **/
void DkImageLoader::lastFile()
{
    loadFileAt(-1);
}

bool DkImageLoader::unloadFile()
{
    if (!mCurrentImage)
        return true;

    // if we are either in rc or remote display mode & the directory does not exist - we received an image, so don't ask the user
    if (mCurrentImage->isEdited()) {
        DkMessageBox *msgBox = new DkMessageBox(QMessageBox::Question,
                                                tr("Save Image"),
                                                tr("Do you want to save changes to:\n%1").arg(QFileInfo(mCurrentImage->filePath()).fileName()),
                                                (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel),
                                                DkUtils::getMainWindow());

        msgBox->setDefaultButton(QMessageBox::No);
        msgBox->setObjectName("saveEditDialog");

        int answer = msgBox->exec();

        // Save image if pixmap edited (lastImageEdit); otherwise save only metadata if metadata edited
        bool imgEdited = mCurrentImage->getLoader()->isImageEdited();
        bool metaEdited = mCurrentImage->getLoader()->isMetaDataEdited();

        if (answer == QMessageBox::Accepted || answer == QMessageBox::Yes) {
            if (DkUtils::isSavable(mCurrentImage->fileInfo().fileName())) {
                if (imgEdited)
                    mCurrentImage->saveImageThreaded(mCurrentImage->filePath());
                else if (metaEdited)
                    mCurrentImage->saveMetaData();
            } else {
                saveUserFileAs(mCurrentImage->image(), false); // we loose all metadata here - right?
            }

        } else if (answer != QMessageBox::No) { // only 'No' will discard the changes
            return false;
        }
    }

    return true;
}

/**
 * Activates or deactivates the loader.
 * If activated, the directory is indexed & the current image is loaded.
 * If deactivated, the image list & the current image are deleted which
 * should save some memory. In addition, all signals are mBlocked.
 * @param isActive if true, the loader is activated
 **/
void DkImageLoader::activate(bool isActive /* = true */)
{
    if (!isActive) {
        // go to sleep - schlofand wöhlar ihr camölar
        blockSignals(true);
        clearPath();
    } else if (!mCurrentImage) {
        // wake up again
        blockSignals(false);
        setCurrentImage(mLastImageLoaded);
    } else
        emit updateDirSignal(mImages);
}

void DkImageLoader::setCurrentImage(QSharedPointer<DkImageContainerT> newImg)
{
    // force index folder if we dir out of the zip
    if (mCurrentImage && newImg && mCurrentImage->isFromZip() && !newImg->isFromZip())
        mFolderUpdated = true;

    if (signalsBlocked()) {
        mCurrentImage = newImg;
        return;
    }

    if (newImg)
        loadDir(newImg->dirPath());
    // else
    //	qDebug() << "empty image assigned"; // TODO

    // if the file stays the same, we just want to update the pointer
    bool updatePointer = newImg && mCurrentImage && newImg->filePath() == mCurrentImage->filePath();

    // cancel action if the image is currently loading
    if (DkSettingsManager::param().resources().waitForLastImg && mCurrentImage && mCurrentImage->getLoadState() == DkImageContainerT::loading && newImg
        && newImg->dirPath() == mCurrentImage->dirPath())
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
            // currentImage->saveMetaDataThreaded();

            if (!DkSettingsManager::param().resources().cacheMemory)
                mCurrentImage->clear();

            mCurrentImage->getLoader()->resetPageIdx();
        }
        this->receiveUpdates(false); // reset updates
    }

    mCurrentImage = newImg;

    if (mCurrentImage) {
        this->receiveUpdates(true);
    }
}

void DkImageLoader::reloadImage()
{
    if (!mCurrentImage)
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

void DkImageLoader::showOnMap()
{
    QSharedPointer<DkMetaDataT> metaData = getCurrentImage()->getMetaData();

    if (!DkMetaDataHelper::getInstance().hasGPS(metaData)) {
        emit showInfoSignal(tr("Sorry, I could not find the GPS coordinates..."));
        return;
    }

    QDesktopServices::openUrl(QUrl(DkMetaDataHelper::getInstance().getGpsCoordinates(metaData)));
}

void DkImageLoader::load(const DkFileInfo &info)
{
    Q_ASSERT(info.isFile());

    QSharedPointer<DkImageContainerT> newImg = findOrCreateFile(info.path());
    setCurrentImage(newImg);
    load(mCurrentImage);
    // loadDir(info.dirPath()); // does not seem to be needed
}

void DkImageLoader::load(QSharedPointer<DkImageContainerT> image /* = QSharedPointer<DkImageContainerT> */)
{
    if (!image)
        return;

    setCurrentImage(image);

    if (mCurrentImage && mCurrentImage->getLoadState() == DkImageContainerT::loading)
        return;

    emit updateSpinnerSignalDelayed(true);
    bool loaded = mCurrentImage->loadImageThreaded(); // loads file threaded

    if (!loaded)
        emit updateSpinnerSignalDelayed(false);

    // if loaded is false, we definitively know that the file does not exist -> early exception here?
}

void DkImageLoader::imageLoaded(bool loaded /* = false */)
{
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

    QApplication::sendPostedEvents(); // force an event post here

    updateCacher(mCurrentImage);
    updateHistory();

    if (mCurrentImage)
        emit imageHasGPSSignal(DkMetaDataHelper::getInstance().hasGPS(mCurrentImage->getMetaData()));

    // update status bar info
    if (mCurrentImage && !mImages.empty() && mImages.indexOf(mCurrentImage) >= 0)
        DkStatusBarManager::instance().setMessage(tr("%1 of %2").arg(mImages.indexOf(mCurrentImage) + 1).arg(mImages.size()),
                                                  DkStatusBar::status_filenumber_info);
    else
        DkStatusBarManager::instance().setMessage("", DkStatusBar::status_filenumber_info);
}

void DkImageLoader::downloadFile(const QUrl &url)
{
    QSharedPointer<DkImageContainerT> newImg = findOrCreateFile(QString());
    setCurrentImage(newImg);
    newImg->downloadFile(url);
    emit updateSpinnerSignalDelayed(true);
}

/**
 * Saves a temporary file to the folder specified in DkSettingsManager.
 * @param img the image (which was in most cases pasted to nomacs)
 **/
QString DkImageLoader::saveTempFile(const QImage &img, const QString &name, const QString &fileExt, bool force, bool threaded)
{
    QString filePath = DkSettingsManager::param().global().tmpPath;
    QFileInfo fInfo(filePath + QDir::separator());

    if (!force && (filePath.isEmpty() || !fInfo.exists())) {
        if (!filePath.isEmpty())
            qWarning() << filePath << "does not exist";
        return QString();
    } else if (filePath.isEmpty() || !fInfo.exists()) {
        fInfo = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));

        if (!fInfo.isDir()) {
            // load system default open dialog
            QString dirName = QFileDialog::getExistingDirectory(DkUtils::getMainWindow(),
                                                                tr("Save Directory"),
                                                                getDirPath(),
                                                                QFileDialog::ShowDirsOnly | DkDialog::fileDialogOptions());

            fInfo = QFileInfo(dirName + QDir::separator());

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

void DkImageLoader::saveFileWeb(const QImage &saveImg)
{
    QWidget *dialogParent = DkUtils::getMainWindow();
    QString saveName;
    QFileInfo saveFileInfo;

    if (hasFile()) {
        saveFileInfo = QFileInfo(mCurrentDir, fileName());
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

    QString fileName = QFileDialog::getSaveFileName(dialogParent,
                                                    tr("Save File %1").arg(saveName),
                                                    saveFileInfo.absoluteFilePath(),
                                                    saveFilterGui,
                                                    nullptr,
                                                    DkDialog::fileDialogOptions());

    if (fileName.isEmpty())
        return;

    DkCompressDialog *jpgDialog = new DkCompressDialog(dialogParent);
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

void DkImageLoader::copyUserFile()
{
    // the subsequent modals destroy the active window
    QWidget *dialogParent = DkUtils::getMainWindow();
    QString saveName;

    auto imgC = getCurrentImage();

    if (hasFile() && imgC) {
        int filterIdx = -1;

        QString extension = imgC->fileInfo().suffix();

        // retrieve the extension name (that's more user friendly)
        QStringList sF = DkSettingsManager::param().app().openFilters;
        QRegularExpression exp = QRegularExpression(extension, QRegularExpression::CaseInsensitiveOption);

        for (int idx = 1; idx < sF.size(); idx++) {
            if (exp.match(sF.at(idx)).hasMatch()) {
                extension = sF.at(idx);
                filterIdx = idx;
                break;
            }
        }

        QString saveName = QFileInfo(getCopyPath(), imgC->fileName()).absoluteFilePath();

        saveName = QFileDialog::getSaveFileName(dialogParent, tr("Save File %1").arg(saveName), saveName, extension, nullptr, DkDialog::fileDialogOptions());

        if (saveName.isEmpty())
            return;

        qDebug() << fileName() << "->" << saveName;

        if (QFile::copy(imgC->filePath(), saveName)) {
            mCopyDir = QFileInfo(saveName).absolutePath();
            qInfo() << fileName() << "copied to" << saveName;
        } else {
            emit showInfoSignal(tr("Sorry, I could not copy the image..."));
        }
    }
}

void DkImageLoader::saveUserFileAs(const QImage &saveImg, bool silent)
{
    // TODO glitch if (!mCurrentImage->getMetaData()->isLoaded()); see rotateImage()

    // the subsequent modals destroy the active window
    QWidget *dialogParent = DkUtils::getMainWindow();

    QString selectedFilter;
    QString saveName = fileName();
    QFileInfo saveFileInfo;

    if (hasFile()) {
        saveFileInfo = QFileInfo(mCurrentDir, fileName());

        int filterIdx = -1;

        QStringList sF = DkSettingsManager::param().app().saveFilters;
        // qDebug() << sF;

        QRegularExpression exp = QRegularExpression(saveFileInfo.suffix(), QRegularExpression::CaseInsensitiveOption);

        for (int idx = 0; idx < sF.size(); idx++) {
            if (exp.match(sF.at(idx)).hasMatch()) {
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
        DkMessageBox *msg = new DkMessageBox(QMessageBox::Question,
                                             tr("Overwrite File"),
                                             tr("Do you want to overwrite:\n%1?").arg(fileName),
                                             (QMessageBox::Yes | QMessageBox::No),
                                             dialogParent);
        msg->setObjectName("overwriteDialog");

        answer = msg->exec();
    }
    if (answer == QDialog::Rejected || answer == QMessageBox::No) {
        // note: basename removes the whole file name from the first dot...
        QString savePath = (!selectedFilter.isEmpty()) ? saveFileInfo.absoluteFilePath() : QFileInfo(saveFileInfo.absoluteDir(), saveName).absoluteFilePath();

        fileName = QFileDialog::getSaveFileName(dialogParent,
                                                tr("Save File %1").arg(saveName),
                                                savePath,
                                                DkSettingsManager::param().app().saveFilters.join(";;"),
                                                &selectedFilter,
                                                DkDialog::fileDialogOptions());
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
    int compression = -1; // default value

    // Save only metadata if image itself hasn't been edited (after exif rotation)
    bool sameFile = fileName == filePath();
    bool imgEdited = mCurrentImage->getLoader()->isImageEdited();
    bool metaEdited = mCurrentImage->getLoader()->isMetaDataEdited();
    if (!imgEdited && metaEdited && sameFile) {
        // Save metadata only
        mCurrentImage->saveMetaData(); // DkBasicLoader::saveMetaData() (otherwise called after saving)

        // Notify listeners about saved image
        setCurrentImage(mCurrentImage);
        mCurrentImage->setEdited(false);
        emit imageLoadedSignal(mCurrentImage, true);
        emit imageUpdatedSignal(mCurrentImage);

        // Skip the rest which is only relevant when re-encoding/saving the image
        return;
    }
    // Saving image normally, clear exif rotation flag to prevent double rotation
    mCurrentImage->getLoader()->getMetaData()->clearOrientation();
    // Below are the compress/encode routines; at the end of a long call chain (saveIntern internSave Threaded)
    // saveToBuffer() is responsible for adding the exif data to the image buffer soup
    // which is then written to the specified file.

    DkCompressDialog *jpgDialog = 0;
    QImage lSaveImg = saveImg;

    DkTifDialog *tifDialog = 0;

    if (selectedFilter.contains("jxl")) {
        // jxl has to be before old jpeg to avoid triggering jpg_dialog for .JXL format
        if (!jpgDialog)
            jpgDialog = new DkCompressDialog(dialogParent);

        jpgDialog->setDialogMode(DkCompressDialog::jxl_dialog);
        jpgDialog->setImage(saveImg);

        if (!jpgDialog->exec()) {
            jpgDialog->deleteLater();
            return;
        }

        compression = jpgDialog->getCompression();

    } else if (selectedFilter.contains(QRegularExpression("(j2k|jp2|jpf|jpx)", QRegularExpression::CaseInsensitiveOption))) {
        if (!jpgDialog)
            jpgDialog = new DkCompressDialog(dialogParent);

        jpgDialog->setDialogMode(DkCompressDialog::j2k_dialog);

        jpgDialog->setImage(saveImg);
        jpgDialog->setMinimumSize(450, 150);

        if (!jpgDialog->exec()) {
            jpgDialog->deleteLater();
            return;
        }

        compression = jpgDialog->getCompression();

    } else if (selectedFilter.contains(QRegularExpression("(jpg|jpeg)", QRegularExpression::CaseInsensitiveOption))) {
        if (!jpgDialog)
            jpgDialog = new DkCompressDialog(dialogParent);

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

    } else if (selectedFilter.contains("webp")) {
        if (!jpgDialog)
            jpgDialog = new DkCompressDialog(dialogParent);

        jpgDialog->setDialogMode(DkCompressDialog::webp_dialog);
        jpgDialog->setImage(saveImg);

        if (!jpgDialog->exec()) {
            jpgDialog->deleteLater();
            return;
        }

        compression = jpgDialog->getCompression();

    } else if (selectedFilter.contains("avif")) {
        if (!jpgDialog)
            jpgDialog = new DkCompressDialog(dialogParent);

        jpgDialog->setDialogMode(DkCompressDialog::avif_dialog);
        jpgDialog->setImage(saveImg);

        if (!jpgDialog->exec()) {
            jpgDialog->deleteLater();
            return;
        }

        compression = jpgDialog->getCompression();

    } else if (selectedFilter.contains("tif")) {
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
void DkImageLoader::saveFile(const QString &filePath, const QImage &saveImg, const QString &fileFilter, int compression, bool threaded)
{
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
            endSuffix = newSuffix.indexOf(" ");
        else if (newSuffix.indexOf(" ") == -1)
            endSuffix = newSuffix.indexOf(")");
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
    } else if (saveStarted && !threaded) {
        imageSaved(lFilePath);
    }
}

void DkImageLoader::imageSaved(const QString &filePath, bool saved, bool loadToTab)
{
    emit updateSpinnerSignalDelayed(false);
    mDirWatcher->blockSignals(false);

    QFileInfo fInfo(filePath);
    if (!fInfo.exists() || !fInfo.isFile() || !saved)
        return;

    if (DkSettingsManager::instance().param().resources().loadSavedImage == DkSettings::ls_load_to_tab && loadToTab) {
        emit loadImageToTab(filePath);
    } else if (DkSettingsManager::instance().param().resources().loadSavedImage == DkSettings::ls_load) {
        emit imageLoadedSignal(mCurrentImage, true);
        emit imageUpdatedSignal(mCurrentImage);
        qDebug() << "image updated: " << mCurrentImage->fileName();
    } else {
        mFolderUpdated = true;
        loadDir(mCurrentImage->dirPath());
    }
}

/**
 * Updates the file history.
 * The file history stores the last 10 folders.
 **/
void DkImageLoader::updateHistory()
{
    if (!DkSettingsManager::param().global().logRecentFiles || DkSettingsManager::param().app().privateMode)
        return;

    if (!mCurrentImage || mCurrentImage->hasImage() != DkImageContainer::loaded || !mCurrentImage->exists())
        return;

    QFileInfo file = QFileInfo(mCurrentImage->filePath());

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
    for (const QString &cFile : DkSettingsManager::param().global().recentFiles) {
        QFileInfo fi(cFile);

        if (fi.absolutePath() == file.absolutePath())
            tmpRecentFiles.append(cFile);
    }

    // maximum 5 most recent images from the same folder
    for (int idx = tmpRecentFiles.size() - 1; idx > 3; idx--) {
        rFiles.removeAll(tmpRecentFiles.at(idx));
    }

    rFiles.push_front(file.absoluteFilePath());
    rFolders.push_front(file.absolutePath());

    rFiles.removeDuplicates();
    rFolders.removeDuplicates();

    for (int idx = 0; idx < rFiles.size() - DkSettingsManager::param().global().numFiles - 10; idx++)
        rFiles.pop_back();

    for (int idx = 0; idx < rFolders.size() - DkSettingsManager::param().global().numFiles - 10; idx++)
        rFolders.pop_back();

    // sync with other instances
    settings.beginGroup("GlobalSettings");
    settings.setValue("recentFolders", rFolders);
    settings.setValue("recentFiles", rFiles);
    settings.endGroup();

    // update
    DkSettingsManager::param().global().recentFiles = rFiles;
    DkSettingsManager::param().global().recentFolders = rFolders;

    // DkSettings s = DkSettings();
    // s.save();
}

// image manipulation --------------------------------------------------------------------
/**
 * Deletes the currently loaded file.
 **/
bool DkImageLoader::deleteFile()
{
    if (mCurrentImage && mCurrentImage->exists()) {
        QString fileName = mCurrentImage->fileName();
        int currFileIdx = findFileIdx(mCurrentImage->filePath(), mImages);
        if (DkUtils::moveToTrash(mCurrentImage->filePath())) {
            mImages.removeAt(currFileIdx);
            QSharedPointer<DkImageContainerT> imgC = getSkippedImage(1);
            if (!imgC)
                imgC = getSkippedImage(0); // deleted from the end
            load(imgC);
            emit showInfoSignal(tr("%1 deleted...").arg(fileName));
            return true;
        } else
            emit showInfoSignal(tr("Sorry, I could not delete: %1").arg(fileName));
    }

    return false;
}

/**
 * @brief rotates the image.
 *
 * First, we try to set the rotation flag in the metadata.
 * That wouldn't work if the image has no meta data, like a bmp file for example.
 * Then, the image matrix is rotated.
 *
 * @param angle the rotation angle in degree.
 **/
void DkImageLoader::rotateImage(double angle)
{
    qDebug() << "rotating image...";

    if (!mCurrentImage || !mCurrentImage->hasImage()) {
        qDebug() << "sorry, loader has no image";
        return;
    }

    QImage img = DkImage::rotateImage(mCurrentImage->pixmap(), qRound(angle));

    QImage thumb = DkImage::createThumb(mCurrentImage->pixmap());

    QSharedPointer<DkMetaDataT> metaData = mCurrentImage->getMetaData(); // via ImageContainer, BasicLoader
    bool metaDataSet = false;

    if (metaData->hasMetaData() && DkSettingsManager::param().metaData().saveExifOrientation) {
        try {
            // Set orientation in exif data
            if (!metaData->isJpg())
                metaData->setThumbnail(thumb); // FIXME: creates wrong thumb for non-JPG formats supporting EXIF! (PNG)
            metaData->setOrientation(qRound(angle));
            metaDataSet = true;

        } catch (...) {
        }
    }

    if (metaDataSet) {
        // Add history item with edited metadata (exif rotation)
        mCurrentImage->setMetaData(metaData, img, tr("Rotated")); // new edit with modified metadata
        setImageUpdated();
    } else {
        // Update the image itself, along with the history and everything
        // In other words, the rotated image is saved to the history and the edit flag is set
        // the exif rotation flag will be reset when adding the new image to the history (BasicLoader)
        mCurrentImage->setImage(img, tr("Rotated")); // new edit with rotated pixmap (clears orientation)
        setImageUpdated();
        // TODO There's a glitch when rotating/changing the image after switching back from settings
        // which causes the containers to be reloaded. If we call the local setImage() overload,
        // the metadata object will be reset causing the image to be saved without modified metadata on Save.
        // With the call above, no metadata is lost, but when navigating away, confirming save on unload
        // and navigating back, the previous image may still appear (loaded while/before async save).
        // TODO a) prevent metadata reset without also resetting the gui; b) send signal after save
        // to reload the saved image (in the other container); c) don't load x while saving x ...
        // [2022-09, pse]
    }
}

/**
 * Reloads the file index if the directory was edited.
 * @param path the path to the current directory
 **/
void DkImageLoader::directoryChanged(const QString &path)
{
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
        } else
            mTimerBlockedUpdate = true;
    }
}

/**
 * Returns true if a file was specified.
 * @return bool true if a file name/path was specified
 **/
bool DkImageLoader::hasFile() const
{
    return mCurrentImage && mCurrentImage->exists();
}

bool DkImageLoader::hasMovie() const
{
    if (!mCurrentImage || !mCurrentImage->exists())
        return false;

    return mCurrentImage->hasMovie();
}

bool DkImageLoader::hasSvg() const
{
    if (!mCurrentImage || !mCurrentImage->exists())
        return false;

    return mCurrentImage->hasSvg();
}

/**
 * Returns the currently loaded file information.
 * @return QFileInfo the current file info
 **/
QString DkImageLoader::filePath() const
{
    if (!mCurrentImage)
        return QString();

    return mCurrentImage->filePath();
}

QSharedPointer<DkImageContainerT> DkImageLoader::getCurrentImage() const
{
    return mCurrentImage;
}

QSharedPointer<DkImageContainerT> DkImageLoader::getLastImage() const
{
    return mLastImageLoaded;
}

/**
 * Returns the currently loaded directory.
 * @return QDir the currently loaded directory.
 **/
QString DkImageLoader::getDirPath() const
{
    return mCurrentDir;
}

QStringList DkImageLoader::getFoldersRecursive(const QString &dirPath)
{
    // DkTimer dt;
    QStringList subFolders;
    // qDebug() << "scanning recursively: " << dir.absolutePath();

    if (DkSettingsManager::param().global().scanSubFolders) {
        QDirIterator dirs(dirPath, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);

        int nFolders = 0;
        while (dirs.hasNext()) {
            dirs.next();
            subFolders << dirs.filePath();
            nFolders++;

            if (nFolders > 100)
                break;

            // getFoldersRecursive(dirs.filePath(), subFolders);
            // qDebug() << "loop: " << dirs.filePath();
        }
    }

    subFolders << dirPath;

    std::sort(subFolders.begin(), subFolders.end(), DkUtils::compLogicQString);

    qDebug() << dirPath << "loaded recursively...";

    // qDebug() << "scanning folders recursively took me: " << QString::fromStdString(dt.getTotal());
    return subFolders;
}

DkFileInfoList DkImageLoader::updateSubFolders(const QString &rootDirPath)
{
    mSubFolders = getFoldersRecursive(rootDirPath);
    DkFileInfoList files;
    qDebug() << mSubFolders;

    // find the first subfolder that has images
    for (int idx = 0; idx < mSubFolders.size(); idx++) {
        mCurrentDir = mSubFolders[idx];
        files = DkFileInfo::readDirectory(mCurrentDir); // this line takes seconds if you have lots of files and slow loading (e.g. network)
        if (!files.empty())
            break;
    }

    return files;
}

/**
 * Find the first subfolder that has images
 * @param fromIdx where to start from
 * @param next true if check forward
 * @return -1 or found folder
 * @see getSkippedImage()
 */
int DkImageLoader::getSubFolderIdx(int fromIdx, bool forward) const
{
    int idx = -1;

    if (mSubFolders.empty())
        return idx;

    bool loop = DkSettingsManager::param().global().loop;

    for (int i = 1; i < mSubFolders.size(); i++) {
        int checkIdx = fromIdx + (forward ? i : -i);

        if (loop) {
            if (checkIdx < 0)
                checkIdx += mSubFolders.size();
            else
                checkIdx %= mSubFolders.size();
        }

        if (checkIdx < 0 || checkIdx >= mSubFolders.size())
            return -1;

        QDir cDir = mSubFolders[checkIdx];
        // FIXME: expensive call to read dir discards result
        DkFileInfoList cFiles = DkFileInfo::readDirectory(cDir.absolutePath());
        if (!cFiles.empty()) {
            idx = checkIdx;
            break;
        }
    }

    return idx;
}

void DkImageLoader::errorDialog(const QString &msg) const
{
    QMessageBox errorDialog(qApp->activeWindow());
    errorDialog.setWindowTitle(tr("Error"));
    errorDialog.setIcon(QMessageBox::Critical);
    errorDialog.setText(msg);
    errorDialog.show();

    errorDialog.exec();
}

void DkImageLoader::updateCacher(QSharedPointer<DkImageContainerT> imgC)
{
    if (!imgC || !DkSettingsManager::param().resources().cacheMemory)
        return;

    DkTimer dt;

    //// no caching? delete all
    // if (!DkSettingsManager::param().resources().cacheMemory) {
    //	for (int idx = 0; idx < images.size(); idx++) {
    //		images.at(idx)->clear();
    //	}
    //	return;
    // }

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
        if (idx > cIdx && idx <= cIdx + DkSettingsManager::param().resources().maxImagesCached)
            mem += cImg->getMemoryUsage();
        else if (abs(cIdx - idx) > 1) {
            cImg->clear();
            if (cImg->hasImage())
                qDebug() << "[Cacher]" << cImg->filePath() << "freed";

            continue;
        } else
            totalMem += cImg->getMemoryUsage();

        // ignore the last and current one
        if (idx == cIdx - 1 || idx == cIdx) {
            continue;
        }
        // fully load the next image
        else if (idx == cIdx + 1 && mem < DkSettingsManager::param().resources().cacheMemory
                 && mImages.at(idx)->getLoadState() == DkImageContainerT::not_loaded) {
            cImg->loadImageThreaded();
            qDebug() << "[Cacher] " << cImg->filePath() << " fully cached...";
        } else if (idx > cIdx && idx < cIdx + DkSettingsManager::param().resources().maxImagesCached - 2
                   && mem < DkSettingsManager::param().resources().cacheMemory && mImages.at(idx)->getLoadState() == DkImageContainerT::not_loaded) {
            // dt.getIvl();
            mImages.at(idx)->fetchFile(); // TODO: crash detected here
            qDebug() << "[Cacher] " << cImg->filePath() << " file fetched...";
        }
    }

    qDebug() << "[Cacher] created in" << dt << "(" << mem + totalMem << "MB)";
}

void DkImageLoader::sort()
{
    for (auto &img : std::as_const(mImages))
        if (!img) {
            qWarning() << "attempt to sort null image(s) averted";
            return;
        }

    bool ascending = DkSettingsManager::param().global().sortDir == DkSettings::sort_ascending;

    auto cmp = DkImageContainer::compareFunc();

    std::sort(mImages.begin(), mImages.end(), cmp);
    if (!ascending)
        std::reverse(mImages.begin(), mImages.end());

    emit updateDirSignal(mImages);
}

void DkImageLoader::currentImageUpdated() const
{
    if (mCurrentImage.isNull())
        return;

    emit imageUpdatedSignal(mCurrentImage);
}

/**
 * Returns the directory where files are copied to.
 * @return QDir the directory where the user copied the last file to.
 **/
QString DkImageLoader::getCopyPath() const
{
    if (mCopyDir.isEmpty() || !QDir(mCopyDir).exists())
        return mCurrentDir;
    else
        return mCopyDir;
}

/**
 * Returns if an image is loaded currently.
 * @return bool true if an image is loaded.
 **/
bool DkImageLoader::hasImage() const
{
    return mCurrentImage && mCurrentImage->hasImage();
}

bool DkImageLoader::isEdited() const
{
    return mCurrentImage && mCurrentImage->isEdited();
}

int DkImageLoader::numFiles() const
{
    return mImages.size();
}

void DkImageLoader::undo()
{
    if (!mCurrentImage)
        return;

    mCurrentImage->undo();
}

void DkImageLoader::redo()
{
    if (!mCurrentImage)
        return;

    mCurrentImage->redo();
}

/**
 * @brief Returns the currently loaded pixmap. May differ from the image returned by getImage()
 * in case the pixmap represents a meta modification, like after rotating it.
 * This is primarily meant to be displayed in the gui.
 *  When saving to disk, the actual image as returned by getImage() should be used.
 *
 * @return QImage
 */
QImage DkImageLoader::getPixmap()
{
    if (!mCurrentImage)
        return QImage();

    // return mCurrentImage->pixmap();
    return mCurrentImage->getLoader()->pixmap();
}

void DkImageLoader::setFolderFilter(const QString &filter)
{
    mFolderFilterString = filter;
    mFolderUpdated = true;
    loadDir(mCurrentDir); // simulate a folder update operation
}

/**
 * Sets the current directory to dir.
 * @param dir the directory to be loaded.
 **/
void DkImageLoader::setDir(const DkFileInfo &info)
{
    Q_ASSERT(info.isDir());

    if (loadDir(info.path()))
        firstFile();
}

/**
 * Sets the current image to a new image buffer (e.g. pasted image)
 * @param img the loader's new image.
 * @param editName the name in the edit history
 **/
QSharedPointer<DkImageContainerT> DkImageLoader::setImage(const QImage &img, const QString &editName)
{
    QSharedPointer<DkImageContainerT> newImg(new DkImageContainerT());
    newImg->setImage(img, editName);
    return setImage(newImg);
}

/**
 * Sets the current image to a new image container.
 * @param img the loader's new image.
 **/
QSharedPointer<DkImageContainerT> DkImageLoader::setImage(QSharedPointer<DkImageContainerT> img)
{
    setCurrentImage(img);
    emit imageUpdatedSignal(mCurrentImage);

    return img;
}

void DkImageLoader::setImageUpdated()
{
    mCurrentImage->setEdited();
    emit imageUpdatedSignal(mCurrentImage);
}

/**
 * Returns the current file name.
 * @return QString the file name of the currently loaded file.
 **/
QString DkImageLoader::fileName() const
{
    if (!mCurrentImage || !mCurrentImage->exists())
        return QString();

    return mCurrentImage->fileName();
}

/**
 * Connects or disconnects the signals of the current image to corresponding slots.
 *
 * @param[in] connectSignals true to connect or false to disconnect
 */
void DkImageLoader::receiveUpdates(bool connectSignals)
{
    if (!mCurrentImage) {
        return;
    }

    DkImageContainerT *currImage = mCurrentImage.data();

    if (currImage == nullptr) {
        return;
    }

    // !selected - do not connect twice
    if (connectSignals && !currImage->isSelected()) {
        connect(currImage, &DkImageContainerT::errorDialogSignal, this, &DkImageLoader::errorDialog, Qt::UniqueConnection);
        connect(currImage, &DkImageContainerT::fileLoadedSignal, this, &DkImageLoader::imageLoaded, Qt::UniqueConnection);
        connect(currImage, &DkImageContainerT::showInfoSignal, this, &DkImageLoader::showInfoSignal, Qt::UniqueConnection);
        connect(currImage, &DkImageContainerT::fileSavedSignal, this, &DkImageLoader::imageSaved, Qt::UniqueConnection);
        connect(currImage, &DkImageContainerT::imageUpdatedSignal, this, &DkImageLoader::currentImageUpdated, Qt::UniqueConnection);
    } else if (!connectSignals) {
        disconnect(currImage, &DkImageContainerT::errorDialogSignal, this, &DkImageLoader::errorDialog);
        disconnect(currImage, &DkImageContainerT::fileLoadedSignal, this, &DkImageLoader::imageLoaded);
        disconnect(currImage, &DkImageContainerT::showInfoSignal, this, &DkImageLoader::showInfoSignal);
        disconnect(currImage, &DkImageContainerT::fileSavedSignal, this, &DkImageLoader::imageSaved);
        disconnect(currImage, &DkImageContainerT::imageUpdatedSignal, this, &DkImageLoader::currentImageUpdated);
    }

    currImage->receiveUpdates(connectSignals);
}
}
