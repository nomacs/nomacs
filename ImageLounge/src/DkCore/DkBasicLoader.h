/*******************************************************************************************************
 DkBasicLoader.h
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

#pragma warning(push, 0)
#include <QFutureWatcher>
#include <QImageReader>
#include <QNetworkAccessManager>
#include <QSharedPointer>
#include <QUrl>
#pragma warning(pop)

#pragma warning(disable : 4251) // TODO: remove
// #include "DkImageStorage.h"

#ifndef Q_OS_WIN
#include "qpsdhandler.h"
#endif

// opencv
#ifdef WITH_OPENCV

#ifdef Q_OS_WIN
#pragma warning(disable : 4996)
#endif

#include <opencv2/imgproc/imgproc.hpp>
#endif

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

// Qt defines
class QNetworkReply;
class LibRaw;

namespace nmc
{
class DkMetaDataT;

#ifdef WITH_QUAZIP
class DllCoreExport DkZipContainer
{
public:
    DkZipContainer(const QString &fileName);

    bool isZip() const;
    QString getZipFilePath() const;
    QString getImageFileName() const;
    QString getEncodedFilePath() const;
    static QString zipMarker();
    static QSharedPointer<QByteArray> extractImage(const QString &zipFile, const QString &imageFile);
    static void extractImage(const QString &zipFile, const QString &imageFile, QByteArray &ba);
    static QString decodeZipFile(const QString &encodedFileInfo);
    static QString decodeImageFile(const QString &encodedFileInfo);
    static QString encodeZipFile(const QString &zipFile, const QString &imageFile);

protected:
    QString mEncodedFilePath;
    QString mZipFilePath;
    QString mImageFileName;
    bool mImageInZip;
    static QString mZipMarker;
};
#endif

class DllCoreExport DkEditImage
{
public:
    DkEditImage();
    DkEditImage(const QImage &img, const QSharedPointer<DkMetaDataT> &metaData, const QString &editName = "");
    DkEditImage(const QSharedPointer<DkMetaDataT> &metaData, const QImage &img, const QString &editName = "");

    void setImage(const QImage &img);
    QString editName() const;
    QImage image() const;
    bool hasImage() const;
    bool hasMetaData() const;
    bool hasNewImage() const;
    bool hasNewMetaData() const;
    QSharedPointer<DkMetaDataT> metaData() const;
    int size() const;

protected:
    QString mEditName;
    QImage mImg;
    bool mNewImg;
    bool mNewMetaData;
    QSharedPointer<DkMetaDataT> mMetaData;
};

class DllCoreExport DkRawLoader
{
public:
    DkRawLoader(const QString &filePath, const QSharedPointer<DkMetaDataT> &metaData);

    bool isEmpty() const;
    void setLoadFast(bool fast);

    bool load(const QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>());

    QImage image() const;

protected:
    QString mFilePath;
    QSharedPointer<DkMetaDataT> mMetaData;

    QImage mImg;

    enum Cam {
        camera_unknown = 0,
        camera_iiq,
        camera_canon,

        camera_end
    };

    bool mLoadFast = false;
    bool mIsChromatic = true;
    Cam mCamType = camera_unknown;

    bool loadPreview(const QSharedPointer<QByteArray> &ba);

#ifdef WITH_LIBRAW
    cv::Mat mColorMat;
    cv::Mat mGammaTable;

    QImage loadPreviewRaw(LibRaw &iProcessor) const;
    bool openBuffer(const QSharedPointer<QByteArray> &ba, LibRaw &iProcessor) const;
    void detectSpecialCamera(const LibRaw &iProcessor);

    cv::Mat demosaic(LibRaw &iProcessor) const;
    cv::Mat prepareImg(const LibRaw &iProcessor) const;

    cv::Mat whiteMultipliers(const LibRaw &iProcessor) const;
    cv::Mat gammaTable(const LibRaw &iProcessor) const;

    void whiteBalance(const LibRaw &iProcessor, cv::Mat &img) const;

    void gammaCorrection(const LibRaw &iProcessor, cv::Mat &img) const;

    void reduceColorNoise(const LibRaw &iProcessor, cv::Mat &img) const;

    QImage raw2Img(const LibRaw &iProcessor, cv::Mat &img) const;

    template<typename num>
    num clip(double val) const
    {
        int vr = qRound(val);

        // trust me I'm an engineer @ -2
        // with -2 we do not get pink in oversaturated areas
        if (vr > std::numeric_limits<num>::max())
            vr = std::numeric_limits<num>::max() - 2;
        if (vr < 0)
            vr = 0;

        return static_cast<num>(vr);
    }
#endif
};

/**
 * This class provides image loading and editing capabilities.
 * It additionally stores the currently loaded image.
 **/
class DllCoreExport DkBasicLoader : public QObject
{
    Q_OBJECT

public:
    DkBasicLoader();

    ~DkBasicLoader()
    {
        release();
    };

    /**
     * Get rotation value
     * @return see DkMetaData::getOrientationDegrees()
     */
    static int getOrientationDegrees(const QImageIOHandler::Transformations transform);

    /**
     * Get mirror/horizontal flip
     * @return see DkMetadata::isOrientationMirrored()
     */
    static bool isOrientationMirrored(const QImageIOHandler::Transformations transform);

    /**
     * Load image from file.
     **/
    bool loadGeneral(const QString &filePath, bool loadMetaData = false, bool fast = true);

    /**
     * Loads the image for the given file
     * @param filePath path to the file; provides suffix to find the right loader
     * @param ba byteArray of the file contents; always used first
     * @param loadMetadata load exif metadata
     * @param fast use fast-but-less-accurate loader (e.g. RAW preview JPG)
     * @note if the image is multipage, the next page is loaded
     * @return true if image was loaded and assigned to the edit history
     **/
    bool loadGeneral(const QString &filePath, const QSharedPointer<QByteArray> ba, bool loadMetaData = false, bool fast = true);

    /**
     * Loads the page requested (with respect to the current page)
     * @param skipIdx number of pages to skip (+/- current page index)
     * @return true if image was loaded
     **/
    bool loadPage(int skipIdx = 0);

    /**
     * Loads the absolute page number
     * @return true if image was loaded
     */
    bool loadPageAt(int pageIdx = 0);

    int getNumPages() const
    {
        return mNumPages;
    };

    int getPageIdx() const
    {
        return mPageIdx;
    };

    /**
     * Set page index but do not load anything (loadGeneral() is required)
     */
    bool setPageIdx(int skipIdx);

    /**
     * Set page index to 1 but do not set the dirty flag (loadGeneral() will not call loadPage())
     * FIXME: it is not clear why this should exist, it can be removed with no apparent effect,
     *        it could be attempt to correct navigation errors when multiple multi-page tiffs are
     *        in the same directory (see TIFF in formats_testset)
     */
    void resetPageIdx();

    QString save(const QString &filePath, const QImage &img, int compression = -1);
    bool saveToBuffer(const QString &filePath, const QImage &img, QSharedPointer<QByteArray> &ba, int compression = -1) const;
    void saveThumbToMetaData(const QString &filePath, QSharedPointer<QByteArray> &ba);
    void saveMetaData(const QString &filePath, QSharedPointer<QByteArray> &ba);
    void saveThumbToMetaData(const QString &filePath);
    void saveMetaData(const QString &filePath);

    /**
     * Check if file suffix should be treated as a container/folder
     */
    static bool isContainer(const QString &filePath);

    /**
     * Sets a new image (if edited outside the basicLoader class)
     * @param img the new image
     * @param file assigns the current file name
     **/
    void setImage(const QImage &img, const QString &editName, const QString &file);
    void pruneEditHistory();
    void setEditImage(const QImage &img, const QString &editName = "");
    void setEditMetaData(const QSharedPointer<DkMetaDataT> &metaData, const QImage &img, const QString &editName = "");
    void setEditMetaData(const QSharedPointer<DkMetaDataT> &metaData, const QString &editName = "");
    void setEditMetaData(const QString &editName);

    // void setTraining(bool training)
    // {
    //     mTraining = true;
    // };

    // bool getTraining()
    // {
    //     return mTraining;
    // };

    QSharedPointer<DkMetaDataT> getMetaData() const;

    /**
     * Return the last edit image (most recent edit), excluding metadata edits
     */
    QImage lastImage() const;

    /**
     * Return the current edit image
     */
    QImage pixmap() const;

    QImage image() const
    {
        return pixmap();
    }

    QSharedPointer<DkMetaDataT> lastMetaDataEdit(bool return_nullptr = true, bool return_orig = false) const;

    bool isImageEdited();
    bool isMetaDataEdited();

    // QString getFile() const
    // {
    //     return mFile;
    // };

    /**
     * Get if loadGeneral() should be called again to fetch the image.
     * FIXME: this should be renamed since it doesn't have anything to do with file being modified
     */
    bool isDirty() const
    {
        return mPageIdxDirty;
    };

    /**
     * Returns the current image size.
     * @return QSize the image size.
     **/
    QSize size()
    {
        return image().size();
    };

    /**
     * Returns true if an image is currently loaded.
     * @return bool true if an image is loaded.
     **/
    bool hasImage()
    {
        return !image().isNull();
    };

    void undo();
    void redo();
    QVector<DkEditImage> *history();
    DkEditImage lastEdit() const;

    void setMinHistorySize(int size);
    void setHistoryIndex(int idx);
    int historyIndex() const;

    QSharedPointer<QByteArray> loadFileToBuffer(const QString &filePath) const;
    bool writeBufferToFile(const QString &fileInfo, const QSharedPointer<QByteArray> ba) const;

    void release();

    // TODO: return this type from all load* functions instead of bool
    struct LoaderResult {
        bool ok = false;
        // QString error;
        // QString name;
        QImage img;
        bool supportsTransform = false;
        QImageIOHandler::Transformations transform = QImageIOHandler::TransformationNone;
    };

#ifdef WITH_OPENCV
    // cv::Mat getImageCv();
    bool loadOpenCVVecFile(const QString &filePath, QImage &img, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>(), QSize s = QSize()) const;
    cv::Mat getPatch(const unsigned char **dataPtr, QSize patchSize) const;
    int mergeVecFiles(const QStringList &vecFilePaths, QString &saveFileInfo) const;
    bool readHeader(const unsigned char **dataPtr, int &fileCount, int &vecSize) const;
    void getPatchSizeFromFileName(const QString &fileName, int &width, int &height) const;
#else
    bool loadOpenCVVecFile(const QString &, QImage &, QSharedPointer<QByteArray> = QSharedPointer<QByteArray>(), QSize = QSize())
    {
        return false;
    };
    int mergeVecFiles(const QStringList &, QString &) const
    {
        return 0;
    };
    bool readHeader(const unsigned char **, int &, int &) const
    {
        return false;
    };
    void getPatchSizeFromFileName(const QString &, int &, int &) const {};

#endif
    LoaderResult loadQt(const QString &filePath, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>(), const QByteArray &format = QByteArray());

    bool loadPSD(const QString &filePath, QImage &img, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>()) const;
    bool loadTIFF(const QString &filePath, QImage &img, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>()) const;
    bool loadDRIF(const QString &filePath, QImage &img, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>()) const;

#ifdef Q_OS_WIN
    // bool saveWindowsIcon(const QString &filePath, const QImage &img) const;
    bool saveWindowsIcon(const QImage &img, QSharedPointer<QByteArray> &ba) const;
#endif

signals:
    void errorDialogSignal(const QString &msg) const;

    void undoSignal();
    void redoSignal();
    void resetMetaDataSignal();

protected:
    /**
     * Loads special RAW files that are generated by the Hamamatsu scientific camera.
     */
    bool loadROH(const QString &filePath, QImage &img, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>()) const;

    /**
     * TGA image loader for Qt unsupported variants
     */
    bool loadTGA(const QString &filePath, QImage &img, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>()) const;

    /**
     * LibRAW image loader
     */
    bool loadRAW(const QString &filePath, QImage &img, QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>(), bool fast = false) const;

    /**
     * Get page count for multi-page files (currently TIFF)
     */
    void indexPages(const QString &filePath, const QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>());

    /**
     * Convert ARGB buffer to ABGR
     */
    void convert32BitOrder(void *buffer, int width) const;

    // bool mTraining;

    QString mFile;
    int mNumPages;
    int mPageIdx;
    bool mPageIdxDirty;
    QSharedPointer<DkMetaDataT> mMetaData;
    QVector<DkEditImage> mImages;
    int mMinHistorySize = 2;
    int mImageIndex = 0;
};

namespace tga
{
typedef struct {
    unsigned char r, g, b, a;
} Pixel;

typedef struct {
    char idlength;
    char colourmaptype;
    char datatypecode;
    short colourmaporigin;
    short colourmaplength;
    char colourmapdepth;
    short x_origin;
    short y_origin;
    short width;
    short height;
    char bitsperpixel;
    char imagedescriptor;
} Header;

class DkTgaLoader
{
public:
    DkTgaLoader(QSharedPointer<QByteArray> ba = QSharedPointer<QByteArray>());

    QImage image() const;
    bool load();

private:
    void mergeBytes(Pixel *pixel, unsigned char *p, int bytes) const;
    bool load(QSharedPointer<QByteArray> ba);

    QImage mImg;
    QSharedPointer<QByteArray> mBa;
};
};

// file downloader from: http://qt-project.org/wiki/Download_Data_from_URL
class FileDownloader : public QObject
{
    Q_OBJECT

public:
    explicit FileDownloader(const QUrl &imageUrl, const QString &filePath = "", QObject *parent = 0);

    virtual ~FileDownloader();

    QSharedPointer<QByteArray> downloadedData() const;
    QUrl getUrl() const;
    void downloadFile(const QUrl &url);

signals:
    void downloaded(const QString &filePath = "");

private slots:
    void fileDownloaded(QNetworkReply *pReply);
    void saved();

private:
    QNetworkAccessManager mWebCtrl;
    QSharedPointer<QByteArray> mDownloadedData;
    QUrl mUrl;
    QString mFilePath;

    QFutureWatcher<bool> mSaveWatcher;

    static bool save(const QString &filePath, const QSharedPointer<QByteArray> data);
};

}
