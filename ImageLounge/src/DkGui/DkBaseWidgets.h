/*******************************************************************************************************
 DkBaseWidgets.h
 Created on:	11.02.2015

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2015 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2015 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2015 Florian Kleber <florian@nomacs.org>

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
#include <QDockWidget>
#include <QLabel>
#include <QScrollArea>
#include <QTimer>
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

// Qt defines
class QBitArray;
class QGraphicsOpacityEffect;
class QPushButton;
class QComboBox;

namespace nmc
{

class DllCoreExport DkWidget : public QWidget
{
    Q_OBJECT

public:
	DkWidget(QWidget* parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
};

class DllCoreExport DkFadeWidget : public DkWidget
{
    Q_OBJECT

public:
    DkFadeWidget(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

    void registerAction(QAction *action);
    void block(bool blocked);
    void setDisplaySettings(QBitArray *displayBits);
    bool getCurrentDisplaySetting();
    bool isHiding() const;

    enum {
        pos_west,
        pos_north,
        pos_east,
        pos_south,
        pos_dock_hor,
        pos_dock_ver,

        pos_end,
    };

public slots:
    virtual void show(bool saveSetting = true);
    virtual void hide(bool saveSetting = true);
    virtual void setVisible(bool visible, bool saveSetting = true);

    void animateOpacityUp();
    void animateOpacityDown();

protected:
    bool mBlocked;
    bool mHiding;
    bool mShowing;

    QGraphicsOpacityEffect *mOpacityEffect;
    QBitArray *mDisplaySettingsBits;
    QAction *mAction = 0;

    // functions
    void init();
	void paintEvent(QPaintEvent *event) override;
};

class DllCoreExport DkNamedWidget : public DkFadeWidget
{
    Q_OBJECT

public:
    DkNamedWidget(const QString &name, QWidget *parent);

    QString name() const;

protected:
    QString mName;
};

class DllCoreExport DkLabel : public QLabel
{
    Q_OBJECT

public:
    DkLabel(QWidget *parent = 0, const QString &text = QString());
    virtual ~DkLabel();

    virtual void showTimed(int time = 3000);
    virtual void setText(const QString &msg, int time = 3000);
    QString getText();
    void setFontSize(int fontSize);
    void stop();
    void setFixedWidth(int fixedWidth);

    void block(bool blocked)
    {
        mBlocked = blocked;
    };

    void setMargin(const QPoint &margin)
    {
        mMargin = margin;
        updateStyleSheet();
    };

public slots:
    virtual void hide();

protected:
    QWidget *mParent;
    QTimer mTimer;
    QString mText;
    QColor mTextCol;
    int mDefaultTime;
    int mFontSize;
    int mTime;
    int mFixedWidth;
    QPoint mMargin;
    bool mBlocked;

    // functions
    virtual void init();
	virtual void paintEvent(QPaintEvent *event) override;
	virtual void draw(QPainter* painter);

    // for my children...
    virtual void drawBackground(QPainter *){};
    virtual void setTextToLabel();
    virtual void updateStyleSheet();
};

class DllCoreExport DkElidedLabel : public QLabel
{
    Q_OBJECT

public:
    DkElidedLabel(QWidget *parent = 0, const QString &text = QString());

    void setText(QString text);
    QString text() const
    {
        return content;
    }
    QSize minimumSizeHint();
    QSize minimumSize();
    int minimumWidth();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void updateElision();
    QString content;
};

class DkLabelBg : public DkLabel
{
    Q_OBJECT

public:
    DkLabelBg(QWidget *parent = 0, const QString &text = QString());
    virtual ~DkLabelBg(){};
};

/**
 * This label fakes the DkFadeWidget behavior.
 * (allows for registering actions + fades in and out)
 * we need this class too, since we cannot derive from DkLabel & DkFadeWidget
 * at the same time -> both have QObject as common base class.
 **/
class DkFadeLabel : public DkLabel
{
    Q_OBJECT

public:
    DkFadeLabel(QWidget *parent = 0, const QString &text = QString());

    void block(bool blocked);
    void registerAction(QAction *action);
    void setDisplaySettings(QBitArray *displayBits);
    bool getCurrentDisplaySetting();

public slots:
    virtual void show(bool saveSetting = true);
    virtual void hide(bool saveSetting = true);
    virtual void setVisible(bool visible, bool saveSetting = true);

protected slots:
    void animateOpacityUp();
    void animateOpacityDown();

protected:
    bool hiding;
    bool showing;
    QBitArray *displaySettingsBits;

    QAction *mAction = 0;

    QGraphicsOpacityEffect *opacityEffect;

    // functions
    void init();
};

class DllCoreExport DkDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    DkDockWidget(const QString &title, QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
    ~DkDockWidget();

    void registerAction(QAction *action);
    void setDisplaySettings(QBitArray *displayBits);
    bool getCurrentDisplaySetting() const;
    static bool testDisplaySettings(const QBitArray &displaySettingsBits);
    Qt::DockWidgetArea getDockLocationSettings(const Qt::DockWidgetArea &defaultArea) const;

public slots:
    virtual void setVisible(bool visible, bool saveSetting = true);

protected:
    virtual void closeEvent(QCloseEvent *event) override;

    QBitArray *displaySettingsBits;
    QAction *mAction = 0;
};

class DllCoreExport DkResizableScrollArea : public QScrollArea
{
    Q_OBJECT

public:
    DkResizableScrollArea(QWidget *parent = 0);

    void updateSize();

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

protected:
    bool eventFilter(QObject *o, QEvent *e);
};

}
