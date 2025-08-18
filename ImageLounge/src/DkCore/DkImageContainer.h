/*******************************************************************************************************
 DkImageContainer.h
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

#pragma once

#include <QFutureWatcher>
#include <QTimer>

#include "DkFileInfo.h"

namespace nmc
{
class DkBasicLoader;
class DkMetaDataT;
class FileDownloader;
class DkRotatingRect;

class DllCoreExport DkImageContainer
{
public:
    enum {
        loading_canceled = -3,
        loading = -2,
        exists_not = -1,
        not_loaded,
        loaded,
    };

    explicit DkImageContainer(const DkFileInfo &fileInfo);
    virtual ~DkImageContainer();

    QImage image();
    QImage pixmap();
    QImage imageScaledToHeight(int height);

    bool hasImage() const;
    bool hasSvg() const;
    bool hasMovie() const;

    int getLoadState() const;
    DkFileInfo fileInfo() const;
    QString filePath() const;
    QString dirPath() const;
    QString fileName() const;
    bool isEdited() const;
    bool isSelected() const;
    QString getTitleAttribute() const;
    float getMemoryUsage() const;
    float getFileSize() const;

    // file info when container was constructed
    DkFileInfo originalFileInfo() const
    {
        return mOriginalFileInfo;
    };

    virtual QSharedPointer<DkBasicLoader> getLoader();
    virtual QSharedPointer<DkMetaDataT> getMetaData();
    virtual QSharedPointer<QByteArray> getFileBuffer();
#ifdef Q_OS_WIN
    std::wstring getFileNameWStr() const;
#endif

    bool exists();
    bool setPageIdx(int skipIdx);

    static QSharedPointer<QByteArray> loadFileToBuffer(const DkFileInfo &fileInfo);

    bool loadImage();
    void setImage(const QImage &img, const QString &editName);
    bool saveImage(const QString &filePath, const QImage saveImg, int compression = -1);
    bool saveImage(const QString &filePath, int compression = -1);
    void saveMetaData();
    virtual void clear();
    virtual void undo();
    virtual void redo();
    virtual void setHistoryIndex(int idx);
    void cropImage(const DkRotatingRect &rect, const QColor &col, bool cropToMetadata);
    DkRotatingRect cropRect();

    /**
     * Get a less-than function based on the global sort mode, suitable for std::sort et al
     * @note does not incorporate the ascending/descending mode (always ascending),
     *       to sort descending, reverse the array after sorting.
     */
    static std::function<bool(const QSharedPointer<DkImageContainer> &, const QSharedPointer<DkImageContainer> &)>
    compareFunc();

protected:
    QSharedPointer<DkBasicLoader> loadImageIntern(const QString &filePath,
                                                  QSharedPointer<DkBasicLoader> loader,
                                                  const QSharedPointer<QByteArray> fileBuffer);
    void saveMetaDataIntern(const QString &filePath,
                            QSharedPointer<DkBasicLoader> loader,
                            QSharedPointer<QByteArray> fileBuffer = QSharedPointer<QByteArray>());
    QString saveImageIntern(const QString &filePath,
                            QSharedPointer<DkBasicLoader> loader,
                            QImage saveImg,
                            int compression);
    void setFile(const DkFileInfo &fileInfo);
    void init();

    QSharedPointer<QByteArray> mFileBuffer;
    QSharedPointer<DkBasicLoader> mLoader;

    int mLoadState = not_loaded;
    bool mEdited = false;
    bool mSelected = false;

    DkFileInfo mFileInfo;
    QVector<QImage> scaledImages;

#ifdef Q_OS_WIN
    std::wstring mFileNameStr; // speeds up sorting of filenames on windows
#endif

private:
    const DkFileInfo mOriginalFileInfo;
};

class DllCoreExport DkImageContainerT : public QObject, public DkImageContainer
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DkImageContainerT)

public:
    explicit DkImageContainerT(const DkFileInfo &fileInfo = {});
    ~DkImageContainerT() override;

    void fetchFile();
    void cancel();
    void clear() override;
    void receiveUpdates(bool connectSignals);
    void downloadFile(const QUrl &url);

    bool loadImageThreaded(bool force = false);
    bool saveImageThreaded(const QString &filePath, const QImage saveImg, int compression = -1);
    bool saveImageThreaded(const QString &filePath, int compression = -1);
    void saveMetaDataThreaded(const QString &filePath);
    void saveMetaDataThreaded();
    bool isFileDownloaded() const;

    QSharedPointer<DkBasicLoader> getLoader() override;
    static QSharedPointer<DkImageContainerT> fromImageContainer(QSharedPointer<DkImageContainer> imgC);

    void undo() override;
    void redo() override;
    void setHistoryIndex(int idx) override;
    void setMetaData(QSharedPointer<DkMetaDataT> editedMetaData, const QImage &img, const QString &editName);
    void setMetaData(const QString &editName);
    void setEdited(bool edited = true);
    void setRating(int rating);

signals:
    void fileLoadedSignal(bool loaded = true) const;
    void fileSavedSignal(const QString &fileInfo, bool saved = true, bool loadToTab = true) const;
    void showInfoSignal(const QString &msg, int time = 3000, int position = 0) const;
    void errorDialogSignal(const QString &msg) const;
    void thumbLoadedSignal(bool loaded = true) const;
    void imageUpdatedSignal() const;
    void zipFileDownloadedSignal(const DkFileInfo &file) const;

public slots:
    void checkForFileUpdates();

protected slots:
    void bufferLoaded();
    void imageLoaded();
    void savingFinished();
    void loadingFinished();
    void fileDownloaded(const QString &filePath);

protected:
    void fetchImage();

    QFutureWatcher<QSharedPointer<QByteArray>> mBufferWatcher;
    QFutureWatcher<QSharedPointer<DkBasicLoader>> mImageWatcher;
    QFutureWatcher<QString> mSaveImageWatcher;
    QFutureWatcher<bool> mSaveMetaDataWatcher;

    QSharedPointer<FileDownloader> mFileDownloader;

    enum UpdateStates {
        update_idle,
        update_pending,
        update_loading,

        update_end
    };

    int mWaitForUpdate = false;

    bool mFetchingImage = false;
    bool mFetchingBuffer = false;
    bool mDownloaded = false;

    QTimer mFileUpdateTimer;

private:
    void setMetaData(QSharedPointer<DkMetaDataT> editedMetaData, const QString &editName);
};

}
