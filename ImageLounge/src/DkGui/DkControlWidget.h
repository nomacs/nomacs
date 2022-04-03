/*******************************************************************************************************
 DkControlWidget.h
 Created on:	31.08.2015

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
#include "DkImageContainer.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QSharedPointer>
#include <QWidget>
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

#ifdef Q_OS_WIN
#pragma warning(disable : 4251) // TODO: remove
#endif

class QStackedLayout;
class QGridLayout;
class QLabel;

namespace nmc
{

// some dummies
class DkFilePreview;
class DkThumbScrollWidget;
class DkMetaDataHUD;
class DkCommentWidget;
class DkViewPort;
class DkCropWidget;
class DkZoomWidget;
class DkPlayer;
class DkFolderScrollBar;
class DkRatingLabelBg;
class DkDelayedMessage;
class DkFileInfoLabel;
class DkHistogram;
class DkLabelBg;
class DkPluginViewPort;
class DkMetaDataT;
class DkDelayedInfo;
class DkOverview;
class DkViewPortInterface;

class DllCoreExport DkControlWidget : public DkWidget
{
    Q_OBJECT

public:
    enum VerPos { top_scroll = 0, top_thumbs, top_metadata, top_info, ver_center, bottom_info, bottom, bottom_metadata, bottom_thumbs, ver_pos_end };
    enum HorPos { left_thumbs = 0, left_metadata, left, hor_center, right, right_metadata, right_thumbs, hor_pos_end };

    enum InfoPos { bottom_left_label, bottom_right_label, top_left_label };

    enum Widgets {
        last_widget = -1,
        hud_widget,
		crop_widget,

        widget_end
    };

    DkControlWidget(DkViewPort *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~DkControlWidget(){};

    void setFullScreen(bool fullscreen);

	DkFilePreview* getFilePreview() const;
	DkFolderScrollBar* getScroller() const;
	DkMetaDataHUD* getMetaDataWidget() const;
	DkCommentWidget* getCommentWidget() const;
	DkOverview* getOverview() const;
	DkZoomWidget* getZoomWidget() const;
	DkPlayer* getPlayer() const;
	DkFileInfoLabel* getFileInfoLabel() const;
	DkHistogram* getHistogram() const;
	DkCropWidget* getCropWidget() const;

    void stopLabels();
    void showWidgetsSettings();
    void setWidgetsVisible(bool visible, bool saveSettings = false);

    void settingsChanged();

public slots:
    void toggleHUD(bool hide);
    void showPreview(bool visible);
    void showMetaData(bool visible);
    void showFileInfo(bool visible);
    void showPlayer(bool visible);
    void startSlideshow(bool start = true);
	void hideCrop(bool hide = true);
	void showCrop(bool visible);
    void showOverview(bool visible);
    void showHistogram(bool visible);
    void showCommentWidget(bool visible);
    void switchWidget(QWidget *widget = 0);
    void changeMetaDataPosition(int pos);
    void changeThumbNailPosition(int pos);
    void showScroller(bool visible);
    void setPluginWidget(DkViewPortInterface *pluginWidget, bool removeWidget);

    bool closePlugin(bool askForSaving, bool force = false);
    bool applyPluginChanges(bool askForSaving);

    void updateImage(QSharedPointer<DkImageContainerT> imgC);
    void setInfo(const QString &msg, int time = 3000, int location = bottom_left_label);
    virtual void setInfoDelayed(const QString &msg, bool start = false, int delayTime = 1000);
    void updateRating(int rating);

    void imageLoaded(bool loaded);

    void update();

signals:
    void imageUpdatedSignal();

protected slots:
    void setCommentSaved();

protected:
    // events
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    // functions
    void init();
    void connectWidgets();

    // layout (switching of HUD contexts)
    QVector<QWidget *> mWidgets;
    QStackedLayout *mLayout;
    QGridLayout *mHudLayout;

	DkViewPort* mViewport;
	DkCropWidget* mCropWidget;

    DkFilePreview *mFilePreview;
    DkMetaDataHUD *mMetaDataInfo;
    DkCommentWidget *mCommentWidget;
    DkZoomWidget *mZoomWidget;
    DkPlayer *mPlayer;
    DkHistogram *mHistogram;

    DkFolderScrollBar *mFolderScroll;
    DkFileInfoLabel *mFileInfoLabel;
    DkRatingLabelBg *mRatingLabel;

    DkDelayedMessage *mDelayedInfo;

    DkLabelBg *mBottomLabel;
    DkLabelBg *mBottomLeftLabel;

    DkPluginViewPort *mPluginViewport = 0;

    QSharedPointer<DkImageContainerT> mImgC;

    QLabel *mWheelButton;

    QPointF mEnterPos;
};

}
