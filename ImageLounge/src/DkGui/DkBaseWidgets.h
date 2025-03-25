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
    DkWidget(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
};

// non-specialized functions for the mixin
class DllCoreExport DkFadeHelper
{
    Q_DISABLE_COPY_MOVE(DkFadeHelper);

public:
    DkFadeHelper() = delete;
    DkFadeHelper(QWidget *w);

    // binds action checked state to widget visibility
    void registerAction(QAction *action)
    {
        mAction = action;
    }

    // binds a per-appmode setting to visibility changes
    void setDisplaySettings(QBitArray *displayBits)
    {
        mDisplayBits = displayBits;
    }

    // returns the setting value for current app mode
    bool getCurrentDisplaySetting();

protected:
    // if true, overrides of of setVisible() should directly call QWidget::setVisible()
    bool mSetWidgetVisible = false;

    // fade in or fade out, set QWidget visibility, action checkstate, settings
    void fade(bool show, bool saveSetting = true);

private:
    void setWidgetVisible(bool visible);
    void animateOpacity();

    QWidget *mWidget; // widget we are showing/hiding
    QGraphicsOpacityEffect *mOpacityEffect;

    bool mBlocked = false;
    bool mShowing = false;
    bool mHiding = false;

    QAction *mAction = nullptr; // hide/show action for widget
    QBitArray *mDisplayBits = nullptr; // pointer to DkSettings visiblity bits for widget
};

// mixin pattern to allow QWidget::setVisible to
// fade a widget in/out. Also can manage visiblity settings and actions
template<class QWidgetBase>
class DkFadeMixin : public QWidgetBase, public DkFadeHelper
{
    Q_DISABLE_COPY_MOVE(DkFadeMixin<QWidgetBase>)
public:
    DkFadeMixin() = delete;

    // we need a few constructors for different widget types
    DkFadeMixin(QWidget *parent, Qt::WindowFlags flags)
        : QWidgetBase(parent, flags)
        , DkFadeHelper(this)
    {
    }

    DkFadeMixin(QWidget *parent)
        : QWidgetBase(parent)
        , DkFadeHelper(this)
    {
    }

    // overload for text widgets
    DkFadeMixin(const QString &text, QWidget *parent, Qt::WindowFlags flags)
        : QWidgetBase(text, parent, flags)
        , DkFadeHelper(this)
    {
    }

    DkFadeMixin(const QString &text, QWidget *parent)
        : QWidgetBase(text, parent)
        , DkFadeHelper(this)
    {
    }

    //
    // QWidget::hide() and QWidget::show() are not overriden
    // as they are not virtual and go through QWidget::setVisible()
    //

    // if saveSetting=true then set the bound setting
    virtual void setVisible(bool visible, bool saveSetting)
    {
        if (mSetWidgetVisible)
            QWidgetBase::setVisible(visible);
        else
            fade(visible, saveSetting);
    }

    // always sets the bound setting, if there is one
    void setVisible(bool visible) override
    {
        setVisible(visible, true);
    }

    void show(bool saveSetting = true)
    {
        setVisible(true, saveSetting);
    }

    void hide(bool saveSetting = true)
    {
        setVisible(false, saveSetting);
    }
};

class DllCoreExport DkFadeWidget : public DkFadeMixin<DkWidget>
{
    Q_OBJECT

public:
    DkFadeWidget(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

    enum {
        pos_west,
        pos_north,
        pos_east,
        pos_south,
        pos_dock_hor,
        pos_dock_ver,

        pos_end,
    };

protected:
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
    DkLabel(const QString &text = QString(), QWidget *parent = 0);
    virtual ~DkLabel();

    virtual void showTimed(int time = 3000);
    virtual void setText(const QString &msg, int time = 3000);
    QString getText();
    // void setFontSize(int fontSize);
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

    void setVisible(bool) override;

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
    virtual void draw(QPainter *painter);

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
    DkLabelBg(const QString &text = QString(), QWidget *parent = 0);
    virtual ~DkLabelBg(){};
};

/**
 * This label fakes the DkFadeWidget behavior.
 * (allows for registering actions + fades in and out)
 * we need this class too, since we cannot derive from DkLabel & DkFadeWidget
 * at the same time -> both have QObject as common base class.
 **/
class DkFadeLabel : public DkFadeMixin<DkLabel>
{
    Q_OBJECT

public:
    DkFadeLabel(const QString &text = QString(), QWidget *parent = 0);
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
