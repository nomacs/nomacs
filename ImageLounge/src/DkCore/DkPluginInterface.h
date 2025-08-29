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

#include <QApplication>
#include <QImage>
#include <QMainWindow>
#include <QSettings>
#include <QString>

#include "DkBaseWidgets.h"
#include "DkBatchInfo.h"
#include "DkImageContainer.h"
#include "DkSettings.h"

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

    virtual ~DkPluginInterface() = default;

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
    virtual QSharedPointer<DkImageContainer> runPlugin(
        const QString &runID = QString(),
        QSharedPointer<DkImageContainer> imgC = QSharedPointer<DkImageContainer>()) const = 0;

    QMainWindow *getMainWindow() const
    {
        QWidgetList widgets = QApplication::topLevelWidgets();
        QMainWindow *win = nullptr;

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
    int interfaceType() const override
    {
        return interface_batch;
    };

    QSharedPointer<DkImageContainer> runPlugin(
        const QString &runID = QString(),
        QSharedPointer<DkImageContainer> imgC = QSharedPointer<DkImageContainer>()) const override
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

    virtual QSharedPointer<DkImageContainer> runPlugin(const QString &runID,
                                                       QSharedPointer<DkImageContainer> imgC,
                                                       const DkSaveInfo &saveInfo,
                                                       QSharedPointer<DkBatchInfo> &batchInfo) const = 0;

    virtual void preLoadPlugin() const = 0; // is called before batch processing
    virtual void postLoadPlugin(
        const QVector<QSharedPointer<DkBatchInfo>> &batchInfo) const = 0; // is called after batch processing

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

    virtual void loadSettings(QSettings &) {}; // dummy
    virtual void saveSettings(QSettings &) const {}; // dummy
};

class DkViewPortInterface : public DkPluginInterface
{
public:
    int interfaceType() const override
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
    explicit DkPluginViewPort(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags())
        : DkWidget(parent, flags)
    {
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    };

    virtual void updateImageContainer(QSharedPointer<DkImageContainerT> imgC)
    {
        Q_UNUSED(imgC);
    }

signals:
    void closePlugin(bool askForSaving = false) const;
    void loadFile(const QString &filePath) const;
    void loadImage(const QImage &image) const;
    void showInfo(const QString &msg) const;

protected:
    void closeEvent(QCloseEvent *event) override
    {
        emit closePlugin();
        QWidget::closeEvent(event);
    };
};

}

// Change this version number if DkPluginInterface is changed!
Q_DECLARE_INTERFACE(nmc::DkPluginInterface, "com.nomacs.ImageLounge.DkPluginInterface/3.6")
Q_DECLARE_INTERFACE(nmc::DkBatchPluginInterface, "com.nomacs.ImageLounge.DkBatchPluginInterface/3.6")
Q_DECLARE_INTERFACE(nmc::DkViewPortInterface, "com.nomacs.ImageLounge.DkViewPortInterface/3.8")
