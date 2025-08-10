/*******************************************************************************************************
 DkImgTransformationsPlugin.h
 Created on:	01.06.2014

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

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>
#include <QImage>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QMouseEvent>
#include <QObject>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QSpinBox>
#include <QString>
#include <QStringList>
#include <QToolBar>
#include <QVector4D>
#include <QtCore/qmath.h>
#include <QtPlugin>

#include "DkPluginInterface.h"
#include "DkSkewEstimator.h"

namespace nmp
{

class DkImgTransformationsViewPort;
class DkImgTransformationsToolBar;
class DkInteractionRects;

enum {
    mode_scale = 0,
    mode_rotate,
    mode_shear,

    mode_end,
};

enum {
    guide_no_guide = 0,
    guide_rule_of_thirds,
    guide_grid,

    guide_end,
};

class DkImgTransformationsPlugin : public QObject, nmc::DkViewPortInterface
{
    Q_OBJECT
    Q_INTERFACES(nmc::DkViewPortInterface)
    Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.DkAffineTransformationsPlugin/3.5" FILE
                          "DkAffineTransformationsPlugin.json")

public:
    DkImgTransformationsPlugin();
    ~DkImgTransformationsPlugin() override;

    QImage image() const override;
    bool hideHUD() const override;

    QSharedPointer<nmc::DkImageContainer> runPlugin(
        const QString &runID = QString(),
        QSharedPointer<nmc::DkImageContainer> image = QSharedPointer<nmc::DkImageContainer>()) const override;

    bool createViewPort(QWidget *parent) override;
    nmc::DkPluginViewPort *getViewPort() override;

    void setVisible(bool visible) override;

protected:
    nmc::DkPluginViewPort *mViewport = nullptr;
};

class DkImgTransformationsViewPort : public nmc::DkPluginViewPort
{
    Q_OBJECT

public:
    explicit DkImgTransformationsViewPort(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~DkImgTransformationsViewPort() override;

    bool isCanceled();
    QImage getTransformedImage();

public slots:
    void setPanning(bool checked);
    void applyChangesAndClose();
    void discardChangesAndClose();
    void setVisible(bool visible) override;
    void setScaleXValue(double val);
    void setScaleYValue(double val);
    void setShearXValue(double val);
    void setShearYValue(double val);
    void setRotationValue(double val);
    void calculateAutoRotation();
    void setCropEnabled(bool enabled);
    void setAngleLinesEnabled(bool enabled);
    void setGuideStyle(int guideMode);

protected slots:

    void setMode(int mode);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    QPoint map(const QPointF &pos);
    void init();
    void drawGuide(QPainter *painter, const QPolygonF &p, int paintMode);

    bool cancelTriggered;
    bool panning;
    DkImgTransformationsToolBar *imgTransformationsToolbar;
    QCursor defaultCursor;
    DkInteractionRects *intrRect;
    QPointF scaleValues;
    QPointF shearValues;
    QPointF shearValuesTemp;
    QPointF shearValuesDir;
    bool insideIntrRect;
    int intrIdx;
    int selectedMode;
    int defaultMode;
    double rotationValue;
    double rotationValueTemp;
    QPoint referencePoint;
    QPoint rotationCenter;
    double imgRatioAngle;
    QCursor rotatingCursor;
    bool rotCropEnabled;
    DkSkewEstimator skewEstimator;
    bool angleLinesEnabled;
    int mGuideMode;
};

class DkImgTransformationsToolBar : public QToolBar
{
    Q_OBJECT

public:
    enum {
        apply_icon = 0,
        cancel_icon,
        pan_icon,
        scale_icon,
        rotate_icon,
        shear_icon,

        icons_end,
    };

    enum {
        settings_mode = 0,
        settings_guide,
        settings_crop,
        settings_lines,

        settings_end,
    };

    DkImgTransformationsToolBar(const QString &title, int defaultMode, QWidget *parent = nullptr);
    ~DkImgTransformationsToolBar() override;

    void setRotationValue(double val);
    void setScaleValue(QPointF val);
    void setShearValue(QPointF val);
    void setCropState(int val);
    void setGuideLineState(int val);
    void setAngleLineState(int val);

public slots:
    void on_applyAction_triggered();
    void on_cancelAction_triggered();
    void on_panAction_toggled(bool checked);
    void on_scaleAction_toggled(bool checked);
    void on_rotateAction_toggled(bool checked);
    void on_shearAction_toggled(bool checked);
    void on_scaleXBox_valueChanged(double val);
    void on_scaleYBox_valueChanged(double val);
    void on_shearXBox_valueChanged(double val);
    void on_shearYBox_valueChanged(double val);
    void on_rotationBox_valueChanged(double val);
    void on_cropEnabledBox_stateChanged(int val);
    void on_showLinesBox_stateChanged(int val);
    void on_autoRotateButton_clicked();
    void on_guideBox_currentIndexChanged(int val);
    void setVisible(bool visible) override;

signals:
    void applySignal();
    void cancelSignal();
    void scaleXValSignal(double val);
    void scaleYValSignal(double val);
    void shearXValSignal(double val);
    void shearYValSignal(double val);
    void rotationValSignal(double val);
    void calculateAutoRotationSignal();
    void cropEnabledSignal(bool enabled);
    void showLinesSignal(bool enabled);
    void panSignal(bool checked);
    void modeChangedSignal(int mode);
    void guideStyleSignal(int guideMode);

protected:
    void createLayout(int defaultMode);
    void createIcons();
    void modifyLayout(int mode);
    void updateAffineTransformPluginSettings(int val, int type);

    QDoubleSpinBox *scaleXBox;
    QDoubleSpinBox *scaleYBox;
    QDoubleSpinBox *shearXBox;
    QDoubleSpinBox *shearYBox;
    QDoubleSpinBox *rotationBox;
    QCheckBox *cropEnabledBox;
    QPushButton *autoRotateButton;
    QCheckBox *showLinesBox;
    QMap<QString, QAction *> toolbarWidgetList;
    QComboBox *guideBox;

    QAction *panAction;
    QAction *scaleAction;
    QAction *rotateAction;
    QAction *shearAction;
    QVector<QIcon> icons; // needed for colorizing
};

class DkInteractionRects : public QWidget
{
    Q_OBJECT

public:
    explicit DkInteractionRects(QRect imgRect, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::Widget);
    explicit DkInteractionRects(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::Widget);
    ~DkInteractionRects() override;

    void draw(QPainter *painter);
    void updateRects(QRect imgRect);
    QCursor getCursorShape(int idx);
    QVector<QRect> getInteractionRects();
    void setInitialValues(QRect rect);
    QSize getInitialSize();
    QPointF getInitialPoint(int idx);
    QPoint getCenter();

protected:
    void init();

    QVector<QRect> intrRect;
    QVector<QCursor> intrCursors;
    QVector<QPointF> initialPoints;
    QSize initialSize;

    QSize size;
};

};
