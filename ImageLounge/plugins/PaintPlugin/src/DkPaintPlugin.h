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

#include <QAction>
#include <QColorDialog>
#include <QGraphicsBlurEffect>
#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>
#include <QImage>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QMouseEvent>
#include <QObject>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QStringList>
#include <QToolBar>
#include <QtPlugin>

#include "DkBaseViewPort.h"
#include "DkImageStorage.h"
#include "DkNoMacs.h"
#include "DkPluginInterface.h"
#include "DkSettings.h"
#include "DkUtils.h"

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

    DkPaintViewPort *getPaintViewPort();

protected:
    nmc::DkPluginViewPort *viewport;
};

class DkPaintViewPort : public nmc::DkPluginViewPort
{
    Q_OBJECT

public:
    DkPaintViewPort(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
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
    void setVisible(bool visible) override;
    void undoLastPaint();

signals:
    void editShowSignal(bool show);

protected slots:
    void setMode(int mode);
    void textChange(const QString &text);
    void textEditFinsh();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    virtual void init();

    void loadSettings();
    void saveSettings() const;

    QVector<QPainterPath> paths;
    QVector<QPen> pathsPen;
    QVector<int> pathsMode;
    QPointF begin;
    QString sbuffer;

    int selectedMode;
    bool textinputenable;
    QPainterPath ArrowHead;

    bool cancelTriggered;
    bool isOutside;
    QBrush mBrush;
    QPen mPen;
    QPointF lastPoint;
    bool panning;
    DkPaintToolBar *paintToolbar;
    QCursor defaultCursor;
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

    DkPaintToolBar(const QString &title, QWidget *parent = nullptr);
    ~DkPaintToolBar() override;

    void setPenColor(const QColor &col);
    void setPenWidth(int width);

public slots:
    void on_applyAction_triggered();
    void on_cancelAction_triggered();
    void on_panAction_toggled(bool checked);
    void on_pencilAction_toggled(bool checked);
    void on_lineAction_toggled(bool checked);
    void on_arrowAction_toggled(bool checked);
    void on_circleAction_toggled(bool checked);
    void on_squareAction_toggled(bool checked);
    void on_squarefillAction_toggled(bool checked);
    void on_blurAction_toggled(bool checked);
    void on_textAction_toggled(bool checked);
    void on_penColButton_clicked();
    void on_widthBox_valueChanged(int val);
    void on_alphaBox_valueChanged(int val);
    void on_textInput_textChanged(const QString &text);
    void on_textInput_editingFinished();
    void on_undoAction_triggered();
    void showLineEdit(bool show);
    void setVisible(bool visible) override;

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

    QPushButton *penColButton;
    QColorDialog *colorDialog;
    QSpinBox *widthBox;
    QSpinBox *alphaBox;
    QColor penCol;
    int penAlpha;
    QMap<QString, QAction *> toolbarWidgetList;
    QAction *panAction;
    QAction *undoAction;

    QAction *pencilAction;
    QAction *lineAction;
    QAction *arrowAction;
    QAction *circleAction;
    QAction *squareAction;
    QAction *squarefillAction;
    QAction *blurAction;
    QAction *textAction;

    QLineEdit *textInput;

    QVector<QIcon> icons; // needed for colorizing
};

};
