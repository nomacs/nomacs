/*******************************************************************************************************
 DkPluginInterface.h
 Created on:	14.04.2013

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

#include "DkBaseWidgets.h"
#include "DkBatchInfo.h"
#include "DkImageContainer.h"
#include "DkSettings.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QApplication>
#include <QFileInfo>
#include <QGraphicsView>
#include <QImage>
#include <QMainWindow>
#include <QSettings>
#include <QString>
#include <QStringList>
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

namespace nmc
{

class DkPluginViewPort;

class DkPluginInterface
{
public:
    enum ifTypes {
        interface_basic = 0,
        interface_batch,
        interface_viewport,

        inteface_end,
    };

    virtual ~DkPluginInterface()
    {
    }

    virtual QImage image() const = 0;

    virtual QList<QAction *> createActions(QWidget *)
    {
        return QList<QAction *>();
    };
    virtual QList<QAction *> pluginActions() const
    {
        return QList<QAction *>();
    };
    virtual int interfaceType() const
    {
        return interface_basic;
    };
    virtual bool closesOnImageChange() const
    {
        return true;
    };

    /// <summary>
    /// The plugin's compute function.
    /// NOTE: it needs to be const for we run it with multiple threads.
    /// </summary>
    /// <param name="runID">The run identifier.</param>
    /// <param name="imgC">The image container to be processed.</param>
    /// <returns>A processed image container</returns>
    virtual QSharedPointer<DkImageContainer> runPlugin(const QString &runID = QString(),
                                                       QSharedPointer<DkImageContainer> imgC = QSharedPointer<DkImageContainer>()) const = 0;

    QMainWindow *getMainWindow() const
    {
        QWidgetList widgets = QApplication::topLevelWidgets();
        QMainWindow *win = 0;

        for (int idx = 0; idx < widgets.size(); idx++) {
            if (widgets.at(idx)->inherits("QMainWindow")) {
                win = qobject_cast<QMainWindow *>(widgets.at(idx));
                break;
            }
        }

        return win;
    };
};

class DkBatchPluginInterface : public DkPluginInterface
{
public:
    virtual int interfaceType() const
    {
        return interface_batch;
    };

    virtual QSharedPointer<DkImageContainer> runPlugin(const QString &runID = QString(),
                                                       QSharedPointer<DkImageContainer> imgC = QSharedPointer<DkImageContainer>()) const
    {
        QSharedPointer<DkBatchInfo> dummy;
        DkSaveInfo saveInfo;

        if (imgC) {
            saveInfo.setInputFilePath(imgC->filePath());
            saveInfo.setOutputFilePath(imgC->filePath());
            saveInfo.setInputDirIsOutputDir(true);
        }

        return runPlugin(runID, imgC, saveInfo, dummy);
    };

    virtual QSharedPointer<DkImageContainer>
    runPlugin(const QString &runID, QSharedPointer<DkImageContainer> imgC, const DkSaveInfo &saveInfo, QSharedPointer<DkBatchInfo> &batchInfo) const = 0;

    virtual void preLoadPlugin() const = 0; // is called before batch processing
    virtual void postLoadPlugin(const QVector<QSharedPointer<DkBatchInfo>> &batchInfo) const = 0; // is called after batch processing

    virtual QString name() const = 0; // is needed for settings
    virtual QString settingsFilePath() const
    {
        return DkSettingsManager::param().settingsPath();
    };

    void loadSettings(const QString &settingsPath = "")
    {
        QString sp = settingsPath.isEmpty() ? settingsFilePath() : settingsPath;
        QSettings settings(sp, QSettings::IniFormat);
        loadSettings(settings);
    };
    void saveSettings(const QString &settingsPath = "")
    {
        QString sp = settingsPath.isEmpty() ? settingsFilePath() : settingsPath;
        QSettings settings(sp, QSettings::IniFormat);
        saveSettings(settings);
    };

    virtual void loadSettings(QSettings &){}; // dummy
    virtual void saveSettings(QSettings &) const {}; // dummy
};

class DkViewPortInterface : public DkPluginInterface
{
public:
    virtual int interfaceType() const
    {
        return interface_viewport;
    };

    // return false here if you have a simple viewport (no children)
    // and you want the user to be able to e.g. scroll thumbs while your plugin is active
    virtual bool hideHUD() const
    {
        return true;
    };

    virtual bool createViewPort(QWidget *parent) = 0;
    virtual DkPluginViewPort *getViewPort() = 0;

    virtual void setVisible(bool visible) = 0;
};

class DllCoreExport DkPluginViewPort : public DkWidget
{
    Q_OBJECT

public:
    DkPluginViewPort(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags())
        : DkWidget(parent, flags)
    {
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        // setStyleSheet("QGraphicsView{background-color: QColor(100,0,0,20); border: 1px solid #FFFFFF;}");
    };

    void setWorldMatrix(QTransform *worldMatrix)
    {
        mWorldMatrix = worldMatrix;
    };

    void setImgMatrix(QTransform *imgMatrix)
    {
        mImgMatrix = imgMatrix;
    };

    virtual void updateImageContainer(QSharedPointer<DkImageContainerT> imgC){}; // dummy

signals:
    void closePlugin(bool askForSaving = false) const;
    void loadFile(const QString &filePath) const;
    void loadImage(const QImage &image) const;
    void showInfo(const QString &msg) const;

protected:
    virtual void closeEvent(QCloseEvent *event)
    {
        emit closePlugin();
        QWidget::closeEvent(event);
    };

    virtual QPointF mapToImage(const QPointF &pos) const
    {
        if (!mWorldMatrix || !mImgMatrix)
            return pos;

        QPointF imgPos = mWorldMatrix->inverted().map(pos);
        imgPos = mImgMatrix->inverted().map(imgPos);

        return imgPos;
    };

    virtual QPointF mapToViewport(const QPointF &pos) const
    {
        if (!mWorldMatrix)
            return pos;

        return mWorldMatrix->inverted().map(pos);
    };

    QTransform *mWorldMatrix = 0;
    QTransform *mImgMatrix = 0;
};

}

// Change this version number if DkPluginInterface is changed!
Q_DECLARE_INTERFACE(nmc::DkPluginInterface, "com.nomacs.ImageLounge.DkPluginInterface/3.6")
Q_DECLARE_INTERFACE(nmc::DkBatchPluginInterface, "com.nomacs.ImageLounge.DkBatchPluginInterface/3.6")
Q_DECLARE_INTERFACE(nmc::DkViewPortInterface, "com.nomacs.ImageLounge.DkViewPortInterface/3.8")
