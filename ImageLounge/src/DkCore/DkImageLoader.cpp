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
#include "DkDialog.h"
#include "DkImageStorage.h"
#include "DkMessageBox.h"
#include "DkMetaData.h"
#include "DkSaveDialog.h"
#include "DkSettings.h"
#include "DkStatusBar.h"
#include "DkTimer.h"
#include "DkUtils.h"

#include <QApplication>
#include <QCheckBox>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QMessageBox>
#include <QPainter>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QStringList>
#include <QTimer>
#include <QtConcurrentRun>
#include <qmath.h>

#ifdef WITH_LIBRAW
#include <libraw/libraw.h>
#elif defined(Q_OS_WIN) && !defined(SOCK_STREAM)
#include <winsock2.h> // needed since libraw 0.16
#endif

#include <utility>

namespace nmc
{
DkImageLoader::DkImageLoader()
{
    qRegisterMetaType<QFileInfo>("QFileInfo");

    mDirWatcher = new QFileSystemWatcher(this);
    connect(mDirWatcher, &QFileSystemWatcher::directoryChanged, this, &DkImageLoader::directoryChanged);
    connect(mDirWatcher, &QFileSystemWatcher::fileChanged, this, &DkImageLoader::directoryChanged); // for containers

    mSortingIsDirty = false;
    mSortingImages = false;

    connect(&mCreateImageWatcher,
            &QFutureWatcher<QVector<QSharedPointer<DkImageContainerT>>>::finished,
            this,
            &DkImageLoader::imagesSorted);

    mDelayedUpdateTimer.setSingleShot(true);
    connect(&mDelayedUpdateTimer, &QTimer::timeout, this, [this]() {
        directoryChanged();
    });

    connect(DkActionManager::instance().action(DkActionManager::menu_file_save_copy),
            &QAction::triggered,
            this,
            &DkImageLoader::copyUserFile);
    connect(DkActionManager::instance().action(DkActionManager::menu_edit_undo),
            &QAction::triggered,
            this,
            &DkImageLoader::undo);
    connect(DkActionManager::instance().action(DkActionManager::menu_edit_redo),
            &QAction::triggered,
            this,
            &DkImageLoader::redo);
    connect(DkActionManager::instance().action(DkActionManager::menu_view_gps_map),
            &QAction::triggered,
            this,
            &DkImageLoader::showOnMap);
    connect(DkActionManager::instance().action(DkActionManager::sc_delete_silent),
            &QAction::triggered,
            this,
            &DkImageLoader::deleteFile,
            Qt::UniqueConnection);
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
        DkFileInfoList
            files = DkFileInfo::readDirectory(newDirPath,
                                              mFolderFilterString); // this line takes seconds if you have lots of files
                                                                    // and slow loading (e.g. network)

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
                                              mFolderFilterString); // this line takes seconds if you have lots of files
                                                                    // and slow loading (e.g. network)

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
    //	qDebug() << "ignoring... old dir: " << dir.absolutePath() << " newDir: " << newDir << " file size: " <<
    // images.size();

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

    // this is duplicate code; it is also dead code since threaded sort is disabled
    const QStringList watched[] = {mDirWatcher->directories(), mDirWatcher->files()};
    for (auto &w : watched) {
        if (!w.isEmpty()) // Qt gives a warning if we pass empty list
            mDirWatcher->removePaths(w);
    }
    mDirWatcher->addPath(mCurrentDir);

    qDebug() << "images sorted...";
}

void DkImageLoader::createImages(const DkFileInfoList &files, bool sort)
{
    DkTimer dt;

    // preserve current images when updating
    const QVector<QSharedPointer<DkImageContainerT>> oldImages = mImages;

    mImages.clear();
    mImages.reserve(files.count());

    // use a temporary hashmap or this loop is n^2 !!
    QHash<QString, int> oldFileIndex;
    for (int idx = 0; idx < oldImages.count(); ++idx)
        oldFileIndex.insert(oldImages.at(idx)->filePath(), idx);

    for (const DkFileInfo &f : files) {
        int oldIdx = -1;
        auto it = oldFileIndex.constFind(f.path());
        if (it != oldFileIndex.constEnd())
            oldIdx = it.value();

        // NOTE: we had this here: oIdx != -1 && QFileInfo(oldImages.at(oIdx)->filePath()).lastModified() ==
        // f.lastModified()) however, that did not detect file changes & slowed down the process - so I removed it...
        mImages << ((oldIdx != -1) ? oldImages.at(oldIdx)
                                   : QSharedPointer<DkImageContainerT>(new DkImageContainerT(f)));
    }
    qInfo() << "[DkImageLoader]" << mImages.size() << "containers created in" << dt;

    if (sort) {
        DkImageLoader::sort();
        qInfo() << "[DkImageLoader] after sorting: " << dt;
    }

    // the watched dir didn't necessarily change but we'll do this anyways
    const QStringList watched[] = {mDirWatcher->directories(), mDirWatcher->files()};
    for (auto &w : watched) {
        if (!w.isEmpty()) // Qt gives a warning if we pass empty list
            mDirWatcher->removePaths(w);
    }
    mDirWatcher->addPath(mCurrentDir);
}

QVector<QSharedPointer<DkImageContainerT>> DkImageLoader::sortImages(
    QVector<QSharedPointer<DkImageContainerT>> images) const
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
QSharedPointer<DkImageContainerT> DkImageLoader::getSkippedImage(int skipIdx,
                                                                 bool recursive /* = false */,
                                                                 int currFileIdx /* = 0 */)
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

    // FIXME: somehow mCurrentDir gets messed up, so we have this recovery workaround
    DkFileInfo cDir(mCurrentDir);
    if (!cDir.isDir() || !cDir.exists()) {
        qWarning() << "[Loader] current dir is now invalid:" << mCurrentDir;
        if (mCurrentImage) {
            qWarning() << "[Loader] attempting to reload from:" << mCurrentImage->dirPath();
            loadDir(mCurrentImage->dirPath());
            cDir = DkFileInfo(mCurrentDir);
        }
    }

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
    for (auto &imgC : mImages) {
        if (imgC->filePath() == filePath)
            return imgC;
    }

    return {};
}

int DkImageLoader::findFileIdx(const QString &filePath, const QVector<QSharedPointer<DkImageContainerT>> &images) const
{
    // TODO: this could be removed if we pass fileInfo
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

bool DkImageLoader::promptSaveBeforeUnload()
{
    if (!mCurrentImage || !mCurrentImage->isEdited()) {
        return true;
    }

    auto *msgBox = new DkMessageBox(QMessageBox::Question,
                                    tr("Save Image"),
                                    tr("Do you want to save changes to:\n%1")
                                        .arg(QFileInfo(mCurrentImage->filePath()).fileName()),
                                    (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel),
                                    DkUtils::getMainWindow());

    msgBox->setDefaultButton(QMessageBox::No);
    msgBox->setObjectName("saveEditDialog");

    const int answer = msgBox->exec();

    if (answer == QMessageBox::Accepted || answer == QMessageBox::Yes) {
        // Save image if pixmap edited (lastImageEdit); otherwise save only metadata if metadata edited
        const bool imgEdited = mCurrentImage->getLoader()->isImageEdited();
        const bool metaEdited = mCurrentImage->getLoader()->isMetaDataEdited();

        if (DkUtils::isSavable(mCurrentImage->fileInfo().fileName())) {
            if (imgEdited)
                mCurrentImage->saveImageThreaded(mCurrentImage->filePath());
            else if (metaEdited)
                mCurrentImage->saveMetaData();
        } else {
            saveUserFile(mCurrentImage->image(), false); // we loose all metadata here - right?
        }

        // Clear the image container to force reload so we get correct state.
        mCurrentImage->clear();
        return true;
    }

    if (answer == QMessageBox::No) {
        // Clear the image container to discard all edited changes.
        mCurrentImage->clear();
        return true;
    }

    // Cancel is pressed
    return false;
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
    if (DkSettingsManager::param().resources().waitForLastImg && mCurrentImage
        && mCurrentImage->getLoadState() == DkImageContainerT::loading && newImg
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
    QSharedPointer<DkImageContainerT> newImg = findOrCreateFile(info.path());
    setCurrentImage(newImg);
    load(mCurrentImage);
    // loadDir(info.dirPath()); // does not seem to be needed
}

void DkImageLoader::load(QSharedPointer<DkImageContainerT> image /* = QSharedPointer<DkImageContainerT> */)
{
    mOrientationWarningShown = false;
    mSaveOrientationWarningShown = false;

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
        DkStatusBarManager::instance()
            .setMessage(tr("%1 of %2").arg(mImages.indexOf(mCurrentImage) + 1).arg(mImages.size()),
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
QString DkImageLoader::saveTempFile(const QImage &img, const QString &name, const QString &fileExt, bool threaded)
{
    QString filePath = DkUtils::getTemporaryFilePath(name, fileExt);
    if (filePath.isEmpty())
        return {};

    if (QFile::exists(filePath)) {
        qWarning() << "File exists, will not overwrite:" << filePath;
        return {};
    }

    qInfo() << "Saving file to: " << filePath;
    saveFile(filePath, img, "", -1, threaded);
    return filePath;
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

    auto *jpgDialog = new DkCompressDialog(dialogParent);
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

    auto imgC = getCurrentImage();

    if (hasFile() && imgC) {
        QString extension = imgC->fileInfo().suffix();

        // retrieve the extension name (that's more user friendly)
        QStringList sF = DkSettingsManager::param().app().openFilters;
        QRegularExpression exp = QRegularExpression(extension, QRegularExpression::CaseInsensitiveOption);

        for (int idx = 1; idx < sF.size(); idx++) {
            if (exp.match(sF.at(idx)).hasMatch()) {
                extension = sF.at(idx);
                break;
            }
        }

        QString saveName = QFileInfo(getCopyPath(), imgC->fileName()).absoluteFilePath();

        saveName = QFileDialog::getSaveFileName(dialogParent,
                                                tr("Save File %1").arg(saveName),
                                                saveName,
                                                extension,
                                                nullptr,
                                                DkDialog::fileDialogOptions());

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

void DkImageLoader::saveUserFile(const QImage &saveImg, bool silent)
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

    bool overwriteFile = false;
    QString fileName;

    // don't ask the user if save was hit & the file format is supported for saving
    if (silent && !selectedFilter.isEmpty() && isEdited()) {
        fileName = filePath();
        auto *msg = new DkMessageBox(QMessageBox::Question,
                                     tr("Overwrite File"),
                                     tr("Do you want to overwrite:\n%1?").arg(fileName),
                                     (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel),
                                     dialogParent);
        msg->setObjectName("overwriteDialog");

        int answer = msg->exec();
        overwriteFile = answer == QMessageBox::Yes;
        if (answer == QDialog::Rejected || answer == QMessageBox::Cancel)
            return;
    }

    if (!overwriteFile) {
        // note: basename removes the whole file name from the first dot...
        QString savePath = (!selectedFilter.isEmpty())
            ? saveFileInfo.absoluteFilePath()
            : QFileInfo(saveFileInfo.absoluteDir(), saveName).absoluteFilePath();

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

        // Skip the rest which is only relevant when re-encoding/saving the image
        return;
    }
    // Saving image normally, clear exif rotation flag to prevent double rotation
    mCurrentImage->getLoader()->getMetaData()->clearOrientation();
    // Below are the compress/encode routines; at the end of a long call chain (saveIntern internSave Threaded)
    // saveToBuffer() is responsible for adding the exif data to the image buffer soup
    // which is then written to the specified file.

    DkCompressDialog *jpgDialog = nullptr;
    QImage lSaveImg = saveImg;

    DkTifDialog *tifDialog = nullptr;

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

    } else if (selectedFilter.contains(
                   QRegularExpression("(j2k|jp2|jpf|jpx)", QRegularExpression::CaseInsensitiveOption))) {
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
void DkImageLoader::saveFile(const QString &filePath,
                             const QImage &saveImg,
                             const QString &fileFilter,
                             int compression,
                             bool threaded)
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
    bool saveStarted = (threaded) ? imgC->saveImageThreaded(lFilePath, sImg, compression)
                                  : imgC->saveImage(lFilePath, sImg, compression);

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
    QStringList rFolders = settings.value("recentFolders", DkSettingsManager::param().global().recentFolders)
                               .toStringList();
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
    if (!mCurrentImage || !mCurrentImage->exists())
        return false;

    if (mCurrentImage->fileInfo().isFromZip()) {
        emit showInfoSignal(tr("Sorry, deleting archived files is unsupported."));
        return false;
    }

    QString fileName = mCurrentImage->fileName();
    int currFileIdx = findFileIdx(mCurrentImage->filePath(), mImages);
    if (!DkUtils::moveToTrash({mCurrentImage->filePath()})) {
        emit showInfoSignal(tr("Sorry, I could not delete: %1").arg(fileName));
        return false;
    }

    mImages.removeAt(currFileIdx);
    QSharedPointer<DkImageContainerT> imgC = getSkippedImage(1);
    if (!imgC)
        imgC = getSkippedImage(0); // deleted from the end
    load(imgC);
    emit showInfoSignal(tr("%1 deleted...").arg(fileName));
    return true;
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
    if (!mCurrentImage || !mCurrentImage->hasImage()) {
        return;
    }

    QImage img = DkImage::rotateImage(mCurrentImage->pixmap(), qRound(angle));

    QSharedPointer<DkMetaDataT> metaData = mCurrentImage->getMetaData(); // via ImageContainer, BasicLoader

    const auto loader = mCurrentImage->getLoader();
    DkBasicLoader::Flags flags = loader->flags();

    // if we touched image pixels, we cannot use metadata rotation
    bool modifiedPixels = loader->isImageEdited();

    bool saveMetaData = DkSettingsManager::param().metaData().saveExifOrientation;

    // TODO: if format supports EXIF, here we may offer to create default EXIF metadata
    // if (!modifiedPixels && saveMetaData) { }

    if (metaData->hasMetaData() && saveMetaData && !modifiedPixels && metaData->isWriteable()) {
        // We may have ignored orientation metadata when loading the image (HEIC/AVIF or user enabled it).
        // Warn if this may result in invalid metadata
        // This is not a problem if there is no write support, "Save As" will clear orientation metadata
        if (!mOrientationWarningShown && (flags & DkBasicLoader::Flag::ignored_orientation)) {
            auto *msgBox = new DkMessageBox(QMessageBox::Warning,
                                            tr("Creating Inconsistent Metadata"),
                                            tr("Orientation metadata is disabled or ignored, you must use\n"
                                               "\"Save As\" for correct metadata."),
                                            QMessageBox::Ok | QMessageBox::Cancel,
                                            DkUtils::getMainWindow());
            msgBox->setDefaultButton(QMessageBox::Ok);
            msgBox->setCheckBoxText(tr("&Do not warn me again"));
            msgBox->setObjectName("rotateOrientationIgnored");
            int result = msgBox->exec();
            if (result == QMessageBox::Rejected || result == QMessageBox::Cancel) {
                return;
            }
            mOrientationWarningShown = true;
        }

        try {
            // NOTE: we cannot set a thumbnail reliably here. We cannot even know what the raw
            // untransformed image is for formats that have internal/intrinsic transform like HEIC,
            // maybe this is an exiv2 limitation, or just a format problem in general.
            // However, this should not be too big of an issue because:
            // - if there is a thumb, it rotates (correctly) along with the image
            // - we were not asked to add one
            // - save-as will add/update thumb if metadata-only save is not possible
            metaData->setOrientation(qRound(angle));
            mCurrentImage->setMetaData(metaData, img, tr("Rotated (EXIF)")); // new edit with modified metadata

            // we are done; setMetaData()->setEdited()->imageUpdated()
            return;

        } catch (const Exiv2::Error &e) {
            qWarning() << "[Exiv2] rotate metadata failed" << static_cast<int>(e.code()) << e.what();
        } catch (...) {
            qWarning() << "[Exiv2] rotate metadata failed";
        }

        emit tr("Sorry, metadata rotation failed (check log).");
    }

    // User may not be aware they disabled this
    if (!mSaveOrientationWarningShown && metaData->hasMetaData() && !saveMetaData && !modifiedPixels
        && metaData->isWriteable()) {
        auto *msgBox = new DkMessageBox(QMessageBox::Warning,
                                        tr("Lossless Rotation Disabled"),
                                        tr("This file could be rotated losslessly with EXIF metadata,\n"
                                           "but saving orientation metadata has been disabled in settings."),
                                        QMessageBox::Ok | QMessageBox::Cancel,
                                        DkUtils::getMainWindow());
        msgBox->setDefaultButton(QMessageBox::Ok);
        msgBox->setCheckBoxText(tr("&Do not warn me again"));
        msgBox->setObjectName("rotateSaveOrientationDisabled");
        int result = msgBox->exec();
        if (result == QMessageBox::Rejected || result == QMessageBox::Cancel) {
            return;
        }
        mSaveOrientationWarningShown = true;
    }

    // Add image to edit history with a copy of current metadata (if any).
    // This forces "Save As" which will clear the orientation when saved; nothing else is needed here.
    mCurrentImage->setImage(img, tr("Rotated"));

    // We need this; setImage() does not call setEdited() like setMetaData().
    mCurrentImage->setEdited();

    // TODO There's a glitch when rotating/changing the image after switching back from settings
    // which causes the containers to be reloaded. If we call the local setImage() overload,
    // the metadata object will be reset causing the image to be saved without modified metadata on Save.
    // With the call above, no metadata is lost, but when navigating away, confirming save on unload
    // and navigating back, the previous image may still appear (loaded while/before async save).
    // TODO a) prevent metadata reset without also resetting the gui; b) send signal after save
    // to reload the saved image (in the other container); c) don't load x while saving x ...
    // [2022-09, pse]
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
        QDirIterator dirs(dirPath,
                          QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks,
                          QDirIterator::Subdirectories);

        int nFolders = 0;
        while (dirs.hasNext()) {
            dirs.next();
            DkFileInfo fileInfo(dirs.filePath());
            if (fileInfo.isDir()) {
                subFolders << fileInfo.path();
                nFolders++;

                if (nFolders > 100)
                    break;

                // getFoldersRecursive(dirs.filePath(), subFolders);
                // qDebug() << "loop: " << dirs.filePath();
            }
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
        files = DkFileInfo::readDirectory(
            mCurrentDir); // this line takes seconds if you have lots of files and slow loading (e.g. network)
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
                   && mem < DkSettingsManager::param().resources().cacheMemory
                   && mImages.at(idx)->getLoadState() == DkImageContainerT::not_loaded) {
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
void DkImageLoader::setImage(const QImage &img, const QString &editName)
{
    QSharedPointer<DkImageContainerT> newImg(new DkImageContainerT());
    newImg->setImage(img, editName);
    setImage(newImg);
}

/**
 * Sets the current image to a new image container.
 * @param img the loader's new image.
 **/
void DkImageLoader::setImage(QSharedPointer<DkImageContainerT> img)
{
    setCurrentImage(img);
    emit imageUpdatedSignal(mCurrentImage);
}

void DkImageLoader::setImageUpdated()
{
    mCurrentImage->setEdited();
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
        connect(currImage,
                &DkImageContainerT::errorDialogSignal,
                this,
                &DkImageLoader::errorDialog,
                Qt::UniqueConnection);
        connect(currImage,
                &DkImageContainerT::fileLoadedSignal,
                this,
                &DkImageLoader::imageLoaded,
                Qt::UniqueConnection);
        connect(currImage,
                &DkImageContainerT::showInfoSignal,
                this,
                &DkImageLoader::showInfoSignal,
                Qt::UniqueConnection);
        connect(currImage, &DkImageContainerT::fileSavedSignal, this, &DkImageLoader::imageSaved, Qt::UniqueConnection);
        connect(currImage,
                &DkImageContainerT::imageUpdatedSignal,
                this,
                &DkImageLoader::currentImageUpdated,
                Qt::UniqueConnection);
        connect(currImage,
                &DkImageContainerT::zipFileDownloadedSignal,
                this,
                &DkImageLoader::setDir,
                Qt::UniqueConnection);
    } else if (!connectSignals) {
        disconnect(currImage, &DkImageContainerT::errorDialogSignal, this, &DkImageLoader::errorDialog);
        disconnect(currImage, &DkImageContainerT::fileLoadedSignal, this, &DkImageLoader::imageLoaded);
        disconnect(currImage, &DkImageContainerT::showInfoSignal, this, &DkImageLoader::showInfoSignal);
        disconnect(currImage, &DkImageContainerT::fileSavedSignal, this, &DkImageLoader::imageSaved);
        disconnect(currImage, &DkImageContainerT::imageUpdatedSignal, this, &DkImageLoader::currentImageUpdated);
        disconnect(currImage, &DkImageContainerT::zipFileDownloadedSignal, this, &DkImageLoader::setDir);
    }

    currImage->receiveUpdates(connectSignals);
}
}
