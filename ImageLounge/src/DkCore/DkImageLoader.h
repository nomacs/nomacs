/*******************************************************************************************************
 DkImageLoader.h
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

#pragma once

#pragma warning(push, 0) // no warnings from includes - begin
#include <QImage>
#include <QTimer>
#pragma warning(pop) // no warnings from includes - end

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

// my classes
#include "DkImageContainer.h"

#ifdef Q_OS_LINUX
typedef unsigned char byte;
#endif

// Qt defines
class QFileSystemWatcher;
class QUrl;

namespace nmc
{

/**
 * This class is a basic image loader class.
 * It takes care of the file watches for the current folder,
 * holds the currently displayed image,
 * calls the load routines
 * and saves the image or the image metadata.
 **/
class DllCoreExport DkImageLoader : public QObject
{
    Q_OBJECT

public:
    DkImageLoader(const QString &filePath = QString());
    virtual ~DkImageLoader();

    QFileInfoList updateSubFolders(const QString &rootDirPath);

    void rotateImage(double angle);
    QSharedPointer<DkImageContainerT> getCurrentImage() const;
    QSharedPointer<DkImageContainerT> getLastImage() const;
    QString filePath() const;
    QStringList getFileNames() const;

    QVector<QSharedPointer<DkImageContainerT>> getImages();
    QSharedPointer<DkImageContainerT> setImage(const QImage &img, const QString &editName, const QString &editFilePath = QString());
    QSharedPointer<DkImageContainerT> setImage(QSharedPointer<DkImageContainerT> img);
    void setImageUpdated();
    void setCurrentImage(QSharedPointer<DkImageContainerT> newImg);
    void sort();

    // file selection
    void firstFile();
    void lastFile();
    QSharedPointer<DkImageContainerT> getSkippedImage(int skipIdx, bool recursive = false, int currFileIdx = 0);

    QString getDirPath() const;
    void setDir(const QString &dir);

    bool hasMovie() const;
    bool hasSvg() const;

    void activate(bool isActive = true);
    bool hasImage() const;
    bool isEdited() const;
    int numFiles() const;
    QImage getPixmap();

#ifdef WITH_QUAZIP
    bool loadZipArchive(const QString &zipPath);
#endif

signals:
    void updateSpinnerSignalDelayed(bool start = false, int timeDelayed = 700) const;
    void setPlayer(bool play) const;

    // new signals
    void imageUpdatedSignal(QSharedPointer<DkImageContainerT> image) const;
    void imageUpdatedSignal(int idx) const; // folder scrollbar needs that
    void imageLoadedSignal(QSharedPointer<DkImageContainerT> image, bool loaded = true) const;
    void showInfoSignal(const QString &msg, int time = 3000, int position = 0) const;
    void updateDirSignal(QVector<QSharedPointer<DkImageContainerT>> images) const;
    void imageHasGPSSignal(bool hasGPS) const;
    void loadImageToTab(const QString &filePath) const;

public slots:
    void undo();
    void redo();
    void changeFile(int skipIdx);
    void directoryChanged(const QString &path = QString());
    void saveFileWeb(const QImage &saveImg);
    void saveUserFileAs(const QImage &saveImg, bool silent);
    void copyUserFile();
    void saveFile(const QString &filename, const QImage &saveImg = QImage(), const QString &fileFilter = "", int compression = -1, bool threaded = true);
    void load(QSharedPointer<DkImageContainerT> image = QSharedPointer<DkImageContainerT>());
    void load(const QString &filePath);
    void downloadFile(const QUrl &url);
    bool deleteFile();
    QString saveTempFile(const QImage &img, const QString &name = "img", const QString &fileExt = ".png", bool force = false, bool threaded = true);
    void setFolderFilter(const QString &filter);
    bool loadDir(const QString &newDirPath, bool scanRecursive = true);
    void loadDirRecursive(const QString &newDirPath);
    void errorDialog(const QString &msg) const;
    void loadFileAt(int idx);

    // new slots
    void currentImageUpdated() const;
    void imageLoaded(bool loaded = false);
    void imageSaved(const QString &file, bool saved = true, bool loadToTab = true);
    void imagesSorted();
    bool unloadFile();
    void reloadImage();
    void showOnMap();

protected:
    // functions
    void updateCacher(QSharedPointer<DkImageContainerT> imgC);
    int getSubFolderIdx(int fromIdx, bool forward) const;
    void updateHistory();
    void sortImagesThreaded(QVector<QSharedPointer<DkImageContainerT>> images);
    void createImages(const QFileInfoList &files, bool sort = true);
    QVector<QSharedPointer<DkImageContainerT>> sortImages(QVector<QSharedPointer<DkImageContainerT>> images) const;
    void receiveUpdates(bool connectSignals);

    static QStringList getFoldersRecursive(const QString &dirPath);
    QFileInfoList getFilteredFileInfoList(const QString &dirPath, QString folderKeywords = QString()) const;

    void clearPath();

    QString getSavePath() const;
    QString getCopyPath() const;

    QSharedPointer<DkImageContainerT> findOrCreateFile(const QString &filePath) const;
    QSharedPointer<DkImageContainerT> findFile(const QString &filePath) const;
    int findFileIdx(const QString &filePath, const QVector<QSharedPointer<DkImageContainerT>> &images) const;

    bool hasFile() const;
    QString fileName() const;

    QString mFolderFilterString; // are deleted if a new folder is opened

    QTimer mDelayedUpdateTimer;
    bool mTimerBlockedUpdate = false;
    QString mCurrentDir;
    QString mSaveDir;
    QString mCopyDir;
    QFileSystemWatcher *mDirWatcher = 0;
    QStringList mSubFolders;
    QVector<QSharedPointer<DkImageContainerT>> mImages;
    QSharedPointer<DkImageContainerT> mCurrentImage;
    QSharedPointer<DkImageContainerT> mLastImageLoaded;
    bool mFolderUpdated = false;
    bool mSortingImages = false;
    bool mSortingIsDirty = false;
    QFutureWatcher<QVector<QSharedPointer<DkImageContainerT>>> mCreateImageWatcher;
};

}
