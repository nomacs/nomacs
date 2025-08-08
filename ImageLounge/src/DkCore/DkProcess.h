/*******************************************************************************************************
 DkProcess.h
 Created on:	27.12.2014

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

#include <QDir>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QSharedPointer>
#include <QStringList>
#include <QUrl>

#include "DkBatchInfo.h"
#include "DkManipulators.h"

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
class QImage;
class QSettings;

namespace nmc
{

// nomacs defines
class DkImageContainer;
class DkPluginContainer;
class DkBaseManipulator;
class DkMetaDataT;

class DllCoreExport DkAbstractBatch
{
public:
    DkAbstractBatch() = default;
    virtual ~DkAbstractBatch() = default;

    // ok, this is important:
    // we are using the abstract class to process specialized items
    // if we allow copy operations - we get slicing issues
    // so we just allow pointers to the batch processing functions.
    // but pointers result in threading issues - so we just use
    // QSharedPointers -> problem solved : )
    DkAbstractBatch(const DkAbstractBatch &o) = delete;
    DkAbstractBatch &operator=(const DkAbstractBatch &o) = delete;

    void setProperties() {};
    virtual void saveSettings(QSettings &) const {};
    virtual void loadSettings(QSettings &) {};
    virtual bool compute(QSharedPointer<DkImageContainer> container,
                         const DkSaveInfo &saveInfo,
                         QStringList &logStrings,
                         QVector<QSharedPointer<DkBatchInfo>> &batchInfos) const;
    virtual bool compute(QSharedPointer<DkImageContainer> container, QStringList &logStrings) const;
    virtual bool compute(QImage &, QStringList &) const
    {
        return true;
    };
    virtual bool isActive() const
    {
        return false;
    };
    virtual void postLoad(const QVector<QSharedPointer<DkBatchInfo>> &) const {};

    virtual QString name() const
    {
        return "Abstract Batch";
    };
    QString settingsName() const;

    static QSharedPointer<DkAbstractBatch> createFromName(const QString &settingsName);
};

#ifdef WITH_PLUGINS
class DllCoreExport DkPluginBatch : public DkAbstractBatch
{
public:
    DkPluginBatch();

    void saveSettings(QSettings &settings) const override;
    void loadSettings(QSettings &settings) override;

    virtual void preLoad();
    void postLoad(const QVector<QSharedPointer<DkBatchInfo>> &batchInfo) const override;
    void setProperties(const QStringList &pluginList);
    bool compute(QSharedPointer<DkImageContainer> container,
                 const DkSaveInfo &saveInfo,
                 QStringList &logStrings,
                 QVector<QSharedPointer<DkBatchInfo>> &batchInfos) const override;
    QString name() const override;
    bool isActive() const override;
    virtual QStringList pluginList() const;

protected:
    void loadAllPlugins();
    void loadPlugin(const QString &pluginString, QSharedPointer<DkPluginContainer> &plugin, QString &runID) const;

    QVector<QSharedPointer<DkPluginContainer>> mPlugins;
    QStringList mRunIDs;
    QStringList mPluginList;
};
#endif

class DllCoreExport DkManipulatorBatch : public DkAbstractBatch
{
public:
    DkManipulatorBatch();

    void saveSettings(QSettings &settings) const override;
    void loadSettings(QSettings &settings) override;

    void setProperties(const DkManipulatorManager &manager);
    bool compute(QSharedPointer<DkImageContainer> container, QStringList &logStrings) const override;
    QString name() const override;
    bool isActive() const override;

    DkManipulatorManager manager() const;

protected:
    DkManipulatorManager mManager;
};

class DllCoreExport DkBatchTransform : public DkAbstractBatch
{
public:
    DkBatchTransform();

    enum ResizeMode {
        resize_mode_default,
        resize_mode_long_side,
        resize_mode_short_side,
        resize_mode_width,
        resize_mode_height,
        resize_mode_zoom,

        resize_mode_end
    };

    enum ResizeProperty {
        resize_prop_default,
        resize_prop_decrease_only,
        resize_prop_increase_only,

        resize_prop_end
    };

    void saveSettings(QSettings &settings) const override;
    void loadSettings(QSettings &settings) override;

    void setProperties(int angle,
                       bool cropFromMetadata,
                       QRect cropRect,
                       bool cropRectCenter,
                       float scaleFactor,
                       float zoomHeight,
                       const ResizeMode &mode = resize_mode_default,
                       const ResizeProperty &prop = resize_prop_default,
                       int iplMethod = 1 /*DkImage::ipl_area*/,
                       bool correctGamma = false);

    bool compute(QSharedPointer<DkImageContainer> container, QStringList &logStrings) const override;
    QString name() const override;
    bool isActive() const override;

    int angle() const;
    bool cropMetatdata() const;
    bool cropFromRectangle() const;
    QRect cropRectangle() const;
    bool cropRectCenter() const;

    // resize
    ResizeMode mode() const;
    ResizeProperty prop() const;
    int iplMethod() const;
    float scaleFactor() const;
    float zoomHeight() const;
    bool correctGamma() const;

protected:
    bool prepareProperties(const QSize &imgSize, QSize &size, float &scaleFactor, QStringList &logStrings) const;
    bool isResizeActive() const;
    QString rectToString(const QRect &r) const;
    QRect stringToRect(const QString &s) const;

    int mAngle = 0;
    bool mCropFromMetadata = false;
    bool mCropRectCenter = false;

    ResizeMode mResizeMode = resize_mode_default;
    ResizeProperty mResizeProperty = resize_prop_default;
    float mResizeScaleFactor = 1.0f;
    float mResizeZoomHeight = 0;
    int mResizeIplMethod = 0;
    bool mResizeCorrectGamma = false;

    QRect mCropRect;
};

class DllCoreExport DkBatchProcess
{
public:
    explicit DkBatchProcess(const DkSaveInfo &saveInfo = DkSaveInfo());

    void setProcessChain(const QVector<QSharedPointer<DkAbstractBatch>> processes);
    bool compute(); // do the work
    QStringList getLog() const;
    bool hasFailed() const;
    bool wasProcessed() const;
    QString inputFile() const;
    QString outputFile() const;

    QVector<QSharedPointer<DkBatchInfo>> batchInfo() const;

protected:
    bool process();
    bool prepareDeleteExisting();
    bool deleteOrRestoreExisting();
    bool deleteOriginalFile();
    bool copyFile();
    bool renameFile();
    bool updateMetaData(DkMetaDataT *md);

    DkSaveInfo mSaveInfo;
    int mFailure = 0;
    bool mIsProcessed = false;

    QVector<QSharedPointer<DkBatchInfo>> mInfos;
    QVector<QSharedPointer<DkAbstractBatch>> mProcessFunctions;
    QStringList mLogStrings;
};

class DllCoreExport DkBatchConfig
{
public:
    DkBatchConfig() = default;
    DkBatchConfig(const QStringList &fileList, const QString &outputDir, const QString &fileNamePattern);
    virtual ~DkBatchConfig() = default;

    virtual void saveSettings(QSettings &settings) const;
    virtual void loadSettings(QSettings &settings);

    bool isOk() const;

    void setFileList(const QStringList &fileList)
    {
        mFileList = fileList;
    };
    void setOutputDir(const QString &outputDir)
    {
        mOutputDirPath = outputDir;
    };
    void setFileNamePattern(const QString &pattern)
    {
        mFileNamePattern = pattern;
    };
    void setProcessFunctions(const QVector<QSharedPointer<DkAbstractBatch>> &processFunctions)
    {
        mProcessFunctions = processFunctions;
    };
    void setSaveInfo(const DkSaveInfo &saveInfo)
    {
        mSaveInfo = saveInfo;
    };

    QStringList getFileList() const
    {
        return mFileList;
    };
    QString getOutputDirPath() const
    {
        return mOutputDirPath;
    };
    QString getFileNamePattern() const
    {
        return mFileNamePattern;
    };
    QVector<QSharedPointer<DkAbstractBatch>> getProcessFunctions() const
    {
        return mProcessFunctions;
    };
    DkSaveInfo saveInfo() const
    {
        return mSaveInfo;
    };

protected:
    DkSaveInfo mSaveInfo;

    QStringList mFileList;
    QString mOutputDirPath;
    QString mFileNamePattern;

    QVector<QSharedPointer<DkAbstractBatch>> mProcessFunctions;
};

class DllCoreExport DkBatchProcessing : public QObject
{
    Q_OBJECT

public:
    enum {
        batch_item_failed,
        batch_item_succeeded,
        batch_item_not_computed,

        batch_item_end
    };

    explicit DkBatchProcessing(const DkBatchConfig &config = DkBatchConfig(), QWidget *parent = nullptr);

    void compute();
    static bool computeItem(DkBatchProcess &item);

    QStringList getLog() const;
    int getNumFailures() const;
    int getNumItems() const;
    int getNumProcessed() const;

    bool isComputing() const;
    QList<int> getCurrentResults();
    QStringList getResultList() const;
    QString getBatchSummary(const DkBatchProcess &batch) const;
    void waitForFinished();

    // getter, setter
    void setBatchConfig(const DkBatchConfig &config)
    {
        mBatchConfig = config;
    };
    DkBatchConfig getBatchConfig() const
    {
        return mBatchConfig;
    };

    void postLoad();

    static void computeBatch(const QString &settingsPath, const QString &logPath);

public slots:
    // user interaction
    void cancel();

signals:
    void progressValueChanged(int idx);
    void finished();

protected:
    DkBatchConfig mBatchConfig;
    QVector<DkBatchProcess> mBatchItems;
    QList<int> mResList;

    // threading
    QFutureWatcher<void> mBatchWatcher;

    void init();
};

class DllCoreExport DkBatchProfile
{
public:
    explicit DkBatchProfile(const QString &profileDir = QString());

    static DkBatchConfig loadProfile(const QString &profilePath);
    static bool saveProfile(const QString &profilePath, const DkBatchConfig &batchConfig);
    static QString defaultProfilePath();
    static QString profileNameToPath(const QString &profileName);
    static QString makeUserFriendly(const QString &profilePath);
    static QString fileSuffix();

    QStringList profileNames();

protected:
    QStringList index(const QString &profileDir) const;

    QString mProfileDir;
    QStringList mProfilePaths;
};
}
