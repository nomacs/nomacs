/*******************************************************************************************************
 SbCompositePlugin.cpp

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2015 #YOUR_NAME

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

#include "SbCompositePlugin.h"

#include <QAction>

/*******************************************************************************************************
 * SbCompositePlugin	- enter the plugin class name (e.g. DkPageExtractionPlugin)
 * Simon Brenner		- your name/pseudonym whatever
 * #DATE				- today...
 * #DESCRIPTION		- describe your plugin in one sentence
 * #MENU_NAME			- a user friendly name (e.g. Flip Image)
 * #MENU_STATUS_TIPP	- status tip of your plugin
 * #RUN_ID_1			- generate an ID using: GUID without hyphens generated at http://www.guidgenerator.com/
 * ID_ACTION1			- your action name (e.g. id_flip_horizontally)
 * #ACTION_NAME1		- your action name (e.g. Flip Horizotally - user friendly!)
 * #ACTION_TIPP1		- your action status tip (e.g. Flips an image horizontally - user friendly!)
 *******************************************************************************************************/

namespace nmc
{

/**
 * Returns descriptive image for every ID
 * @param plugin ID
 **/
QImage SbCompositePlugin::image() const
{
    return QImage(":/CompositePlugin/img/description.png");
};

/**
 * Main function: runs plugin based on its ID
 * @param plugin ID
 * @param image to be processed
 **/
QSharedPointer<nmc::DkImageContainer> SbCompositePlugin::runPlugin(const QString &runID,
                                                                   QSharedPointer<nmc::DkImageContainer> imgC) const
{
    Q_UNUSED(runID);

    qDebug() << "called runPlugin";
    if (!imgC)
        qDebug() << "imgC was null";

    if (mViewport && imgC) {
        if (mApply) {
            imgC->setImage(buildComposite(), tr("composite")); // put input image names here?
        }
        mViewport->setVisible(false);
        mDockWidget->setVisible(false);
    }

    return imgC;
}

bool SbCompositePlugin::createViewPort(QWidget *parent)
{
    qDebug() << "called createViewPort";

    if (!mViewport) {
        mViewport = new SbViewPort(parent);
        connect(mViewport, SIGNAL(gotImage()), this, SLOT(onViewportGotImage()));
    }
    if (!mDockWidget) {
        buildUI();
    }
    setVisible(true);
    return false;
}

DkPluginViewPort *SbCompositePlugin::getViewPort()
{
    qDebug() << "called getViewPort";
    return mViewport;
}

void SbCompositePlugin::setVisible(bool visible)
{
    qDebug() << "called setVisible";
    if (mViewport)
        mViewport->setVisible(visible);
    if (mDockWidget)
        mDockWidget->setVisible(visible);

    if (!visible) {
        // cleanup
        for (SbChannelWidget *cw : mChannelWidgets) {
            cw->setImg();
        }
        for (int i = 0; i < 3; i++) {
            mChannels[i] = cv::Mat();
        }
    }
}

// ##############################################################

void SbCompositePlugin::buildUI()
{
    mMainWidget = new QWidget();

    mOuterLayout = new QBoxLayout(QBoxLayout::Direction::TopToBottom); // default
    mOuterLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    for (int i = 0; i < 3; i++) {
        auto *cw = new SbChannelWidget(static_cast<SbChannelWidget::Channel>(i), mMainWidget);
        mChannelWidgets.append(cw);
    }

    for (SbChannelWidget *s : mChannelWidgets) {
        connect(s, SIGNAL(imageChanged(int)), this, SLOT(onImageChanged(int)));
        connect(s, SIGNAL(newAlpha(QImage)), this, SLOT(onNewAlpha(QImage)));
        mOuterLayout->addWidget(s);
    }

    auto *buttonLayout = new QHBoxLayout();
    auto *applyButton = new QPushButton("apply");
    applyButton->setIcon(QIcon(":/CompositePlugin/img/description.png"));
    applyButton->setIconSize(QSize(24, 24));
    connect(applyButton, SIGNAL(released()), this, SLOT(onPushButtonApply()));
    auto *cancelButton = new QPushButton("cancel");
    cancelButton->setIcon(QIcon(":/CompositePlugin/img/close.svg"));
    cancelButton->setIconSize(QSize(24, 24));
    connect(cancelButton, SIGNAL(released()), this, SLOT(onPushButtonCancel()));
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->setAlignment(Qt::AlignBottom);
    mOuterLayout->addItem(buttonLayout);
    mOuterLayout->addStretch();

    mMainWidget->setLayout(mOuterLayout);

    // dock widget & scroll area setup
    mDockWidget = new SbCompositeDockWidget(tr("Composite Plugin"));
    QSettings settings;
    Qt::DockWidgetArea dockLocation = static_cast<Qt::DockWidgetArea>(
        settings.value("sbCompWidgetLocation", Qt::LeftDockWidgetArea).toInt());

    mScrollArea = new QScrollArea(mDockWidget);
    mScrollArea->setMinimumSize(SbChannelWidget::kThumbMaxSize + 50,
                                SbChannelWidget::kThumbMaxSize + 100); // very dirty
    mScrollArea->setWidget(mMainWidget);
    mScrollArea->setWidgetResizable(true);

    mDockWidget->setWidget(mScrollArea);

    connect(mDockWidget, SIGNAL(closed()), this, SLOT(onDockWidgetClose()));
    connect(mDockWidget,
            SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this,
            SLOT(onDockLocationChanged(Qt::DockWidgetArea)));

    QMainWindow *mainWindow = getMainWindow();
    if (mainWindow)
        mainWindow->addDockWidget(dockLocation, mDockWidget);
}

QImage SbCompositePlugin::buildComposite() const
{
    cv::Mat composite;
    if (mAlpha.empty()) {
        cv::merge(mChannels, 3, composite);
    } else {
        cv::Mat bgra[4] = {mChannels[2],
                           mChannels[1],
                           mChannels[0],
                           mAlpha}; // when merging 4 channels, blue and red are reversed again.. why..
        cv::merge(bgra, 4, composite);
    }
    return DkImage::mat2QImage(composite);
}

void SbCompositePlugin::onImageChanged(int c)
{
    qDebug() << "image changed in channel " << c;
    mChannels[c] = mChannelWidgets[c]->getImg();

    // set all channels with non-matching sizes to zeros of matching size
    for (int i = 0; i < 3; i++) {
        if (i != c) {
            if (!(mChannels[i].rows == mChannels[c].rows && mChannels[i].cols == mChannels[c].cols)) {
                mChannels[i] = cv::Mat::zeros(mChannels[c].rows, mChannels[c].cols, mChannels[c].type());
                mChannelWidgets[i]->setImg();
            }
        }
    }
    mViewport->loadImage(buildComposite());
}

void SbCompositePlugin::onNewAlpha(QImage _alpha)
{
    if (_alpha == QImage()) {
        qDebug() << "got empty alpha";
        mAlpha = cv::Mat();
    } else {
        qDebug() << "got full alpha";
        mAlpha = DkImage::qImage2Mat(_alpha);
        // currently it seems like qImage2Mat converts a single-channel QImage to a multi-channel Mat. so..
        if (mAlpha.channels() == 4)
            cv::cvtColor(mAlpha, mAlpha, CV_RGBA2GRAY);
        else if (mAlpha.channels() == 3)
            cv::cvtColor(mAlpha, mAlpha, CV_RGB2GRAY);
    }
}

void SbCompositePlugin::onViewportGotImage()
{
    // put that image into the three channels
    QSharedPointer<DkImageContainerT> imgC = mViewport->getImgC();
    QImage newImage = imgC->image();
    cv::Mat rgb = DkImage::qImage2Mat(newImage);
    if (rgb.channels() >= 3) {
        std::vector<cv::Mat> c;
        split(rgb, c);
        for (int i = 0; i < 3; i++) {
            mChannels[i] = c[2 - i]; // channels are BGR.. why?
            mChannelWidgets[i]->setImg(c[2 - i], imgC->fileName());
        }
        if (rgb.channels() >= 4) {
            mAlpha = c[3];
        }
    }
    // else? i don't think this can happen..
    emit mViewport->loadImage(buildComposite());
}

void SbCompositePlugin::onDockWidgetClose()
{
    emit mViewport->closePlugin(true);
}

void SbCompositePlugin::onDockLocationChanged(Qt::DockWidgetArea a)
{
    // vertical layout
    if (a == Qt::DockWidgetArea::LeftDockWidgetArea || a == Qt::DockWidgetArea::RightDockWidgetArea) {
        mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
        mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        mOuterLayout->setDirection(QBoxLayout::Direction::TopToBottom);
    }
    // horizontal layout
    else {
        mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
        mOuterLayout->setDirection(QBoxLayout::Direction::LeftToRight);
    }
}

void SbCompositePlugin::onPushButtonApply()
{
    mApply = true;
    emit mViewport->closePlugin(false);
}

void SbCompositePlugin::onPushButtonCancel()
{
    mApply = false;
    emit mViewport->closePlugin(false);
}

};
