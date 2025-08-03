/*******************************************************************************************************
 SbCompositePlugin.h

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2015 Markus Diem <markus@nomacs.org>

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

#include "DkPluginInterface.h"
#include "SbChannelWidget.h"
#include <QDockWidget>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QStyle>
#include <QVBoxLayout>
#include <QVector>
#include <opencv2/opencv.hpp>

namespace nmc
{

// subclassed to access its close event
class SbCompositeDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit SbCompositeDockWidget(const QString &title,
                                   QWidget *parent = nullptr,
                                   Qt::WindowFlags flags = Qt::WindowFlags())
        : QDockWidget(title, parent, flags)
    {
        setObjectName("CompositeDockWidget"); // fixes saving...
    }
    ~SbCompositeDockWidget() override = default;

protected:
    void closeEvent(QCloseEvent *) override
    {
        emit closed();
    }
signals:
    void closed();
};

// subclassed to access the image container
class SbViewPort : public DkPluginViewPort
{
    Q_OBJECT
public:
    explicit SbViewPort(QWidget *parent = nullptr)
        : DkPluginViewPort(parent){};
    void updateImageContainer(QSharedPointer<DkImageContainerT> imgC) override
    {
        if (!imgC)
            return;
        mImgC = imgC;
        emit gotImage();
    }
    QSharedPointer<DkImageContainerT> getImgC()
    {
        return mImgC;
    }

private:
    QSharedPointer<DkImageContainerT> mImgC;
signals:
    void gotImage();
};

// the main thing
class SbCompositePlugin : public QObject, DkViewPortInterface
{
    Q_OBJECT
    Q_INTERFACES(nmc::DkViewPortInterface)
    Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.SbCompositePlugin/0.1" FILE "SbCompositePlugin.json")

public:
    explicit SbCompositePlugin(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    ~SbCompositePlugin() override = default;

    // DkPluginInterface
    QImage image() const override;
    QSharedPointer<nmc::DkImageContainer> runPlugin(
        const QString &runID = QString(),
        QSharedPointer<nmc::DkImageContainer> imgC = QSharedPointer<nmc::DkImageContainer>()) const override;
    virtual bool closesOnImageChange()
    {
        return false;
    } // actually I think this has no effect...

    // DkViewPortInterface
    bool createViewPort(QWidget *parent) override;
    DkPluginViewPort *getViewPort() override;
    void setVisible(bool visible) override;

protected:
    SbCompositeDockWidget *dockWidget = nullptr;
    QScrollArea *scrollArea = nullptr;
    QWidget *mainWidget = nullptr;
    QBoxLayout *outerLayout = nullptr;
    QVector<SbChannelWidget *> channelWidgets;
    SbViewPort *viewport = nullptr;
    cv::Mat channels[3];
    cv::Mat alpha;
    bool apply = false;

    void buildUI(); // initialize UI and connect
    QImage buildComposite() const; // merge channels (and alpha if present) to a rgb(a) QImage

public slots:
    void onImageChanged(int channel); // fetch new image from respective channel
    void onNewAlpha(QImage _alpha); // update alpha (don't trigger buildComposite())
    void onViewportGotImage(); // get image from the viewport, split it into channels, assign them to the channel
                               // widgets
    void onDockWidgetClose(); // close plugin, ask for apply/cancel (this is buggy)
    void onDockLocationChanged(
        Qt::DockWidgetArea a); // switches between vertical / horizontal layout depending on the current dock area
    void onPushButtonApply(); // close plugin and apply
    void onPushButtonCancel(); // close plugin and cancel
};

};
