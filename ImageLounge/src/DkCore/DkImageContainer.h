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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QFutureWatcher>
#include <QTimer>
#include <QSharedPointer>
#pragma warning(pop)		// no warnings from includes - end

#pragma warning(disable: 4251)	// TODO: remove

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

#include "DkThumbs.h"

namespace nmc {

// nomacs defines
class DkBasicLoader;
class DkMetaDataT;
class DkZipContainer;
class FileDownloader;
class DkRotatingRect;

class DllCoreExport DkImageContainer {

public:
	enum {
		loading_canceled = -3,
		loading = -2,
		exists_not = -1,
		not_loaded,
		loaded,
	};

	DkImageContainer(const QString& filePath);
	virtual ~DkImageContainer();
	bool operator==(const DkImageContainer& ric) const;
	bool operator< (const DkImageContainer& o) const;
	bool operator<= (const DkImageContainer& o) const;
	bool operator> (const DkImageContainer& o) const;
	bool operator>= (const DkImageContainer& o) const;

	QImage image();
	QImage pixmap();
	QImage imageScaledToHeight(int height);
	QImage imageScaledToWidth(int width);

	bool hasImage() const;
	bool hasSvg() const;
	bool hasMovie() const;

	int getLoadState() const;
	QFileInfo fileInfo() const;
	QString filePath() const;
	QString dirPath() const;
	QString fileName() const;
	bool isFromZip();
	bool isEdited() const;
	bool isSelected() const;
	void setEdited(bool edited = true);
	QString getTitleAttribute() const;
	float getMemoryUsage() const;
	float getFileSize() const;

	virtual QSharedPointer<DkBasicLoader> getLoader();
	virtual QSharedPointer<DkMetaDataT> getMetaData();
	virtual QSharedPointer<DkThumbNailT> getThumb();
	virtual QSharedPointer<QByteArray> getFileBuffer();
#ifdef WITH_QUAZIP
	QSharedPointer<DkZipContainer> getZipData();
#endif
#ifdef Q_OS_WIN
	std::wstring getFileNameWStr() const;
#endif

	bool exists();
	bool setPageIdx(int skipIdx);

	QSharedPointer<QByteArray> loadFileToBuffer(const QString& filePath);
	bool loadImage();
	void setImage(const QImage& img, const QString& editName);
	void setImage(const QImage& img, const QString& editName, const QString& filePath);
	void setMetaData(QSharedPointer<DkMetaDataT> editedMetaData, const QImage& img, const QString& editName);
	void setMetaData(QSharedPointer<DkMetaDataT> editedMetaData, const QString& editName);
	void setMetaData(const QString& editName);
	bool saveImage(const QString& filePath, const QImage saveImg, int compression = -1);
	bool saveImage(const QString& filePath, int compression = -1);
	void saveMetaData();
	virtual void clear();
	virtual void undo();
	virtual void redo();
	virtual void setHistoryIndex(int idx);
	void cropImage(const DkRotatingRect & rect, const QColor & col, bool cropToMetadata);
	DkRotatingRect cropRect();

protected:
	QSharedPointer<DkBasicLoader> loadImageIntern(const QString& filePath, QSharedPointer<DkBasicLoader> loader, const QSharedPointer<QByteArray> fileBuffer);
	void saveMetaDataIntern(const QString& filePath, QSharedPointer<DkBasicLoader> loader, QSharedPointer<QByteArray> fileBuffer = QSharedPointer<QByteArray>());
	QString saveImageIntern(const QString& filePath, QSharedPointer<DkBasicLoader> loader, QImage saveImg, int compression);
	void setFilePath(const QString& filePath);
	void init();

	QSharedPointer<QByteArray> mFileBuffer;
	QSharedPointer<DkBasicLoader> mLoader;
	QSharedPointer<DkThumbNailT> mThumb;

	int mLoadState	= not_loaded;
	bool mEdited	= false;
	bool mSelected	= false;

	QFileInfo mFileInfo;
	QVector<QImage> scaledImages;

#ifdef WITH_QUAZIP	
	QSharedPointer<DkZipContainer> mZipData;
#endif
#ifdef Q_OS_WIN
	std::wstring mFileNameStr;	// speeds up sorting of filenames on windows
#endif

private:
	QString mFilePath;

};

bool imageContainerLessThan(const DkImageContainer& l, const DkImageContainer& r);
bool imageContainerLessThanPtr(const QSharedPointer<DkImageContainer> l, const QSharedPointer<DkImageContainer> r);

class DllCoreExport DkImageContainerT : public QObject, public DkImageContainer {
	Q_OBJECT

public:
	DkImageContainerT(const QString& filePath);
	virtual ~DkImageContainerT();

	void fetchFile();
	void cancel();
	void clear() override;
	void receiveUpdates(QObject* obj, bool connectSignals = true);
	void downloadFile(const QUrl& url);

	bool loadImageThreaded(bool force = false);
	bool saveImageThreaded(const QString& filePath, const QImage saveImg, int compression = -1);
	bool saveImageThreaded(const QString& filePath, int compression = -1);
	void saveMetaDataThreaded(const QString& filePath);
	void saveMetaDataThreaded();
	bool isFileDownloaded() const;

	virtual QSharedPointer<DkBasicLoader> getLoader() override;
	virtual QSharedPointer<DkThumbNailT> getThumb() override;
	static QSharedPointer<DkImageContainerT> fromImageContainer(QSharedPointer<DkImageContainer> imgC);

	virtual void undo() override;
	virtual void redo() override;
	virtual void setHistoryIndex(int idx) override;

signals:
	void fileLoadedSignal(bool loaded = true) const;
	void fileSavedSignal(const QString& fileInfo, bool saved = true, bool loadToTab = true) const;
	void showInfoSignal(const QString& msg, int time = 3000, int position = 0) const;
	void errorDialogSignal(const QString& msg) const;
	void thumbLoadedSignal(bool loaded = true) const;
	void imageUpdatedSignal() const;

public slots:
	void checkForFileUpdates(); 

protected slots:
	void bufferLoaded();
	void imageLoaded();
	void savingFinished();
	void loadingFinished();
	void fileDownloaded(const QString& filePath);

protected:
	void fetchImage();
	
	QSharedPointer<QByteArray> loadFileToBuffer(const QString& filePath);
	QSharedPointer<DkBasicLoader> loadImageIntern(const QString& filePath, QSharedPointer<DkBasicLoader> loader, const QSharedPointer<QByteArray> fileBuffer);
	QString saveImageIntern(const QString& filePath, QSharedPointer<DkBasicLoader> loader, QImage saveImg, int compression);
	void saveMetaDataIntern(const QString& filePath, QSharedPointer<DkBasicLoader> loader, QSharedPointer<QByteArray> fileBuffer);
	
	QFutureWatcher<QSharedPointer<QByteArray> > mBufferWatcher;
	QFutureWatcher<QSharedPointer<DkBasicLoader> > mImageWatcher;
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
};

}
