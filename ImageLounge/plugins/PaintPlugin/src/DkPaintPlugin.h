/*******************************************************************************************************
 DkPaintPlugin.h
 Created on:	14.07.2013

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

#include <QImage>
#include <QObject>
#include <QPainterPath>
#include <QPen>
#include <QString>
#include <QToolBar>

#include "DkPluginInterface.h"

class QSpinBox;
class QLineEdit;
class QColorDialog;
class QPushButton;

namespace nmc
{
class DkBaseViewPort;
}

namespace nmp
{

class DkPaintViewPort;
class DkPaintToolBar;

enum {
    mode_pencil = 0,
    mode_line,
    mode_arrow,
    mode_circle,
    mode_square,
    mode_square_fill,
    mode_blur,
    mode_text,
};

class DkPaintPlugin : public QObject, nmc::DkViewPortInterface
{
    Q_OBJECT
    Q_INTERFACES(nmc::DkViewPortInterface)
    Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.DkPaintPlugin/3.4" FILE "DkPaintPlugin.json")

public:
    DkPaintPlugin();
    ~DkPaintPlugin() override;

    QImage image() const override;
    bool hideHUD() const override;

    QPainterPath getArrowHead(QPainterPath line, const int thickness);
    QLineF getShorterLine(QPainterPath line, const int thickness);
    void getBlur(QPainterPath rect, QPainter *painter, QImage &img, int radius);

    QSharedPointer<nmc::DkImageContainer> runPlugin(
        const QString &runID = QString(),
        QSharedPointer<nmc::DkImageContainer> image = QSharedPointer<nmc::DkImageContainer>()) const override;
    nmc::DkPluginViewPort *getViewPort() override;
    bool createViewPort(QWidget *parent) override;

    void setVisible(bool visible) override;

protected:
    DkPaintViewPort *mViewPort;
    DkPaintToolBar *mToolBar;
};

class DkPaintViewPort : public nmc::DkPluginViewPort
{
    Q_OBJECT

public:
    explicit DkPaintViewPort(QWidget *parent, Qt::WindowFlags flags = Qt::WindowFlags());
    ~DkPaintViewPort() override;

    QBrush getBrush() const;
    QPen getPen() const;
    bool isCanceled();
    QImage getPaintedImage();
    void clear();

public slots:
    void setBrush(const QBrush &brush);
    void setPen(const QPen &pen);
    void setPenWidth(int width);
    void setPenColor(QColor color);
    void setPanning(bool checked);
    void applyChangesAndClose();
    void discardChangesAndClose();
    void undoLastPaint();
    void setMode(int mode);
    void textChange(const QString &text);
    void textEditFinsh();

signals:
    void editShowSignal(bool show);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void init();
    void loadSettings();
    void saveSettings() const;

    void drawPaths(QPainter &painter, nmc::DkBaseViewPort *viewport, bool toImage) const;

    QVector<QPainterPath> mPaths; // list of paths, one per mouse down-drag-up cycle
    QVector<QPen> mPathsPen; // corresponding pen that was used for each path
    QVector<int> mPathsMode; // corresponding mode that was used for each path

    QPointF mBeginPos; // starting position of a new painter path (mouse down location mapped to image)
    bool mHasTextInput;

    int mCurrentMode; // current editing tool, except panning
    bool mTextInputActive; // true if text input box has input focus

    bool mCanceledEditing;
    QBrush mBrush;
    QPen mPen;
    bool mPanningToolActive; // true if panning tool is active (this is not a mode for some reason)
    bool mMouseDown; // true if mouse button is down
    QPointF mLastMousePos; // last mouse position in viewport coordinates

    QCursor mCurrentCursor; // cursor for the current mode/tool
};

class DkPaintToolBar : public QToolBar
{
    Q_OBJECT

public:
    enum {
        apply_icon = 0,
        cancel_icon,
        pan_icon,
        undo_icon,

        pencil_icon,
        line_icon,
        arrow_icon,
        circle_icon,
        square_icon,
        square_fill_icon,
        blur_icon,
        text_icon,

        icons_end,
    };

    explicit DkPaintToolBar(const QString &title, QWidget *parent = nullptr);
    ~DkPaintToolBar() override;

    void setPenColor(const QColor &col);
    void setPenWidth(int width);

public slots:
    void choosePenColor();
    void showLineEdit(bool show);

signals:
    void applySignal();
    void cancelSignal();
    void colorSignal(QColor color);
    void widthSignal(int width);
    void paintHint(int paintMode);
    void shadingHint(bool invert);
    void panSignal(bool checked);
    void undoSignal();
    void modeChangeSignal(int mode);
    void textChangeSignal(const QString &text);
    void editFinishSignal();

protected:
    void createLayout();
    void createIcons();

    QPushButton *mPenColorButton;
    QColorDialog *mColorDialog;
    QSpinBox *mWidthBox;
    QSpinBox *mAlphaBox;
    QColor mPenColor;
    int mPenAlpha;
    QMap<QString, QAction *> mToolbarWidgetList;
    QAction *mPanAction;
    QLineEdit *mTextInput;
    QVector<QIcon> mIcons;
};
};
