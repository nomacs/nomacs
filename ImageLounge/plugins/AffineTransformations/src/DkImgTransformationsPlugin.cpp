/*******************************************************************************************************
 DkImgTransformationsPlugin.cpp
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

#include "DkImgTransformationsPlugin.h"

#include "DkBaseViewPort.h"
#include "DkMath.h"
#include "DkSettings.h"
#include "DkToolbars.h"
#include "DkUtils.h"

#include <QActionGroup>
#include <QMouseEvent>

#define PI 3.14159265

namespace nmp
{

/*-----------------------------------DkImgTransformationsPlugin ---------------------------------------------*/

/**
 *	Constructor
 **/
DkImgTransformationsPlugin::DkImgTransformationsPlugin()
{
}

/**
 *	Destructor
 **/
DkImgTransformationsPlugin::~DkImgTransformationsPlugin()
{
}

/**
 * Returns descriptive image
 **/
QImage DkImgTransformationsPlugin::image() const
{
    return QImage(":/nomacsPluginImgTrans/img/description.png");
}
bool DkImgTransformationsPlugin::hideHUD() const
{
    return false;
}

/**
 * Main function: runs plugin based on its ID
 * @param run ID
 * @param current image in the Nomacs mViewport
 **/
QSharedPointer<nmc::DkImageContainer> DkImgTransformationsPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC) const
{
    Q_UNUSED(runID);

    // for a mViewport plugin runID and image are null
    if (mViewport && imgC) {
        DkImgTransformationsViewPort *transformVp = qobject_cast<DkImgTransformationsViewPort *>(mViewport);

        QImage retImg = QImage();
        if (!transformVp->isCanceled())
            retImg = transformVp->getTransformedImage();

        mViewport->setVisible(false);
        imgC->setImage(retImg, tr("Transformed")); // TODO: specify which transform?!

        return imgC;
    }

    return imgC;
};

bool DkImgTransformationsPlugin::createViewPort(QWidget *parent)
{
    Q_UNUSED(parent);
    mViewport = new DkImgTransformationsViewPort();

    return true;
}

/**
 * returns ImgTransformationsViewPort
 **/
nmc::DkPluginViewPort *DkImgTransformationsPlugin::getViewPort()
{
    return mViewport;
}

void DkImgTransformationsPlugin::setVisible(bool visible)
{
    mViewport->setVisible(visible);
}

/*-----------------------------------DkImgTransformationsViewPort ---------------------------------------------*/

DkImgTransformationsViewPort::DkImgTransformationsViewPort(QWidget *parent, Qt::WindowFlags flags)
    : nmc::DkPluginViewPort(parent, flags)
{
    init();
}

DkImgTransformationsViewPort::~DkImgTransformationsViewPort()
{
    // active deletion since the MainWindow takes ownership...
    // if we have issues with this, we could disconnect all signals between mViewport and toolbar too
    // however, then we have lot's of toolbars in memory if the user opens the plugin again and again
    if (imgTransformationsToolbar) {
        delete imgTransformationsToolbar;
        imgTransformationsToolbar = 0;
    }
}

void DkImgTransformationsViewPort::init()
{
    defaultMode = mode_scale;
    QSettings settings;
    settings.beginGroup("affineTransformPlugin");
    defaultMode = settings.value("mode", defaultMode).toInt();
    mGuideMode = settings.value("guideMode", guide_no_guide).toInt();
    rotCropEnabled = (settings.value("cropEnabled", Qt::Unchecked).toInt() == Qt::Checked);
    angleLinesEnabled = (settings.value("angleLines", Qt::Checked).toInt() == Qt::Checked);
    settings.endGroup();

    selectedMode = defaultMode;
    panning = false;
    cancelTriggered = false;
    defaultCursor = Qt::ArrowCursor;
    rotatingCursor = QCursor(QPixmap(":/nomacs/img/rotating-cursor.png"));
    setCursor(defaultCursor);
    setMouseTracking(true);
    scaleValues = QPointF(1, 1);
    shearValues = QPointF(0, 0);
    rotationValue = 0;
    insideIntrRect = false;
    intrIdx = 100;
    rotationCenter = QPoint();

    intrRect = new DkInteractionRects(this);
    skewEstimator = DkSkewEstimator(this);

    imgTransformationsToolbar = new DkImgTransformationsToolBar(tr("ImgTransformations Toolbar"), defaultMode, this);

    imgTransformationsToolbar->setCropState((rotCropEnabled) ? Qt::Checked : Qt::Unchecked);
    imgTransformationsToolbar->setGuideLineState(mGuideMode);
    imgTransformationsToolbar->setAngleLineState((angleLinesEnabled) ? Qt::Checked : Qt::Unchecked);

    connect(imgTransformationsToolbar, SIGNAL(scaleXValSignal(double)), this, SLOT(setScaleXValue(double)));
    connect(imgTransformationsToolbar, SIGNAL(scaleYValSignal(double)), this, SLOT(setScaleYValue(double)));
    connect(imgTransformationsToolbar, SIGNAL(shearXValSignal(double)), this, SLOT(setShearXValue(double)));
    connect(imgTransformationsToolbar, SIGNAL(shearYValSignal(double)), this, SLOT(setShearYValue(double)));
    connect(imgTransformationsToolbar, SIGNAL(rotationValSignal(double)), this, SLOT(setRotationValue(double)));
    connect(imgTransformationsToolbar, SIGNAL(calculateAutoRotationSignal()), this, SLOT(calculateAutoRotation()));
    connect(imgTransformationsToolbar, SIGNAL(cropEnabledSignal(bool)), this, SLOT(setCropEnabled(bool)));
    connect(imgTransformationsToolbar, SIGNAL(showLinesSignal(bool)), this, SLOT(setAngleLinesEnabled(bool)));
    connect(imgTransformationsToolbar, SIGNAL(modeChangedSignal(int)), this, SLOT(setMode(int)));
    connect(imgTransformationsToolbar, SIGNAL(guideStyleSignal(int)), this, SLOT(setGuideStyle(int)));
    connect(imgTransformationsToolbar, SIGNAL(panSignal(bool)), this, SLOT(setPanning(bool)));
    connect(imgTransformationsToolbar, SIGNAL(cancelSignal()), this, SLOT(discardChangesAndClose()));
    connect(imgTransformationsToolbar, SIGNAL(applySignal()), this, SLOT(applyChangesAndClose()));
}

QPoint DkImgTransformationsViewPort::map(const QPointF &pos)
{
    QPoint posM = pos.toPoint();
    if (mWorldMatrix)
        posM = mWorldMatrix->inverted().map(posM);
    if (mImgMatrix)
        posM = mImgMatrix->inverted().map(posM);

    return posM;
}

void DkImgTransformationsViewPort::mousePressEvent(QMouseEvent *event)
{
    // panning -> redirect to mViewport
    if (event->buttons() == Qt::LeftButton && (event->modifiers() == nmc::DkSettingsManager::param().global().altMod || panning)) {
        setCursor(Qt::ClosedHandCursor);
        event->setModifiers(Qt::NoModifier); // we want a 'normal' action in the mViewport
        event->ignore();
        return;
    }

    if (selectedMode == mode_scale) {
        QVector<QRect> rects = intrRect->getInteractionRects();
        int currIdx;
        for (currIdx = 0; currIdx < rects.size(); currIdx++) {
            if (rects.at(currIdx).contains(map(event->pos()))) {
                intrIdx = currIdx;
                insideIntrRect = true;
                break;
            }
        }

        if (currIdx >= rects.size())
            intrIdx = 100;
    } else if (selectedMode == mode_rotate) {
        if (event->buttons() == Qt::LeftButton) {
            referencePoint = map(event->pos());
            rotationValueTemp = rotationValue;
        }
    } else if (selectedMode == mode_shear) {
        if (event->buttons() == Qt::LeftButton) {
            shearValuesDir = QPointF(1, 0);

            nmc::DkVector c(rotationCenter);
            nmc::DkVector xn(map(event->pos()));
            xn = c - xn;

            if ((xn.angle() > imgRatioAngle && xn.angle() < PI - imgRatioAngle) || (xn.angle() < -imgRatioAngle && xn.angle() > -(PI - imgRatioAngle))) {
                setCursor(Qt::SizeVerCursor);
                shearValuesDir = QPointF(0, 1);
            } else
                setCursor(Qt::SizeHorCursor);

            referencePoint = map(event->pos());
            shearValuesTemp = shearValues;
        }
    }
    // no propagation
}

void DkImgTransformationsViewPort::mouseMoveEvent(QMouseEvent *event)
{
    // panning -> redirect to mViewport
    if (event->modifiers() == nmc::DkSettingsManager::param().global().altMod || panning) {
        event->setModifiers(Qt::NoModifier);
        event->ignore();
        update();
        return;
    }

    if (selectedMode == mode_scale) {
        QVector<QRect> rects = intrRect->getInteractionRects();

        if (insideIntrRect) {
            if (intrIdx < rects.size()) {
                setCursor(intrRect->getCursorShape(intrIdx));

                QSize initSize = intrRect->getInitialSize();
                QPointF initPoint = intrRect->getInitialPoint(intrIdx);
                int sign = 1;

                if (intrIdx < 6) {
                    if (intrIdx == 2 || intrIdx == 3 || intrIdx == 5)
                        sign = -1;
                    scaleValues.setY(
                        qMin(2.5, qMax(0.1, (initSize.height() * 0.5 + sign * (initPoint.y() - map(event->pos()).y())) / (initSize.height() * 0.5))));
                }

                sign = 1;
                if ((intrIdx >= 6) || (intrIdx < 4)) {
                    if (intrIdx == 2 || intrIdx == 1 || intrIdx == 7)
                        sign = -1;
                    scaleValues.setX(
                        qMin(2.5, qMax(0.1, (initSize.width() * 0.5 + sign * (initPoint.x() - map(event->pos()).x())) / (initSize.width() * 0.5))));
                }

                imgTransformationsToolbar->setScaleValue(scaleValues);
                this->repaint();
            }

        } else {
            int currIdx;

            for (currIdx = 0; currIdx < rects.size(); currIdx++) {
                if (rects.at(currIdx).contains(map(event->pos()))) {
                    setCursor(intrRect->getCursorShape(currIdx));
                    break;
                }
            }

            if (currIdx >= rects.size())
                setCursor(defaultCursor);
        }
    } else if (selectedMode == mode_rotate) {
        if (event->buttons() == Qt::LeftButton) {
            nmc::DkVector c(rotationCenter);
            nmc::DkVector xt(referencePoint);
            nmc::DkVector xn(map(event->pos()));

            // compute the direction vector;
            xt = c - xt;
            xn = c - xn;
            double angle = xn.angle() - xt.angle();

            rotationValue = rotationValueTemp + angle / PI * 180;
            if (rotationValue >= 360)
                rotationValue -= 360;
            if (rotationValue < 0)
                rotationValue += 360;

            imgTransformationsToolbar->setRotationValue(rotationValue);
            // this->repaint();
        }
    } else if (selectedMode == mode_shear) {
        if (event->buttons() != Qt::LeftButton) {
            nmc::DkVector c(rotationCenter);
            nmc::DkVector xn(map(event->pos()));
            xn = c - xn;

            if ((xn.angle() > imgRatioAngle && xn.angle() < PI - imgRatioAngle) || (xn.angle() < -imgRatioAngle && xn.angle() > -(PI - imgRatioAngle)))
                setCursor(Qt::SizeVerCursor);
            else
                setCursor(Qt::SizeHorCursor);
        } else if (event->buttons() == Qt::LeftButton) {
            QPoint currPoint = map(event->pos());

            shearValues.setX(shearValuesTemp.x() + shearValuesDir.x() * (currPoint.x() - referencePoint.x()) * 0.001);
            shearValues.setY(shearValuesTemp.y() + shearValuesDir.y() * (currPoint.y() - referencePoint.y()) * 0.001);

            imgTransformationsToolbar->setShearValue(shearValues);
        }
    }
}

void DkImgTransformationsViewPort::mouseReleaseEvent(QMouseEvent *event)
{
    insideIntrRect = false;
    intrIdx = 100;

    // panning -> redirect to mViewport
    if (event->modifiers() == nmc::DkSettingsManager::param().global().altMod || panning) {
        setCursor(defaultCursor);
        event->setModifiers(Qt::NoModifier);
        event->ignore();
        return;
    }
}

void DkImgTransformationsViewPort::paintEvent(QPaintEvent *event)
{
    QImage inImage = QImage();
    QRect imgRect = QRect();

    if (parent()) {
        nmc::DkBaseViewPort *mViewport = dynamic_cast<nmc::DkBaseViewPort *>(parent());
        if (mViewport) {
            imgRect = mViewport->getImage().rect();
            inImage = QImage(mViewport->getImage());
        }
    }

    QRect imgRectT = imgRect;
    QTransform affineTransform = QTransform();

    QPainter painter(this);

    painter.fillRect(this->rect(), nmc::DkSettingsManager::param().display().bgColor);

    // This is true 16:9 code (sorry : )
    if (mWorldMatrix)
        painter.setWorldTransform((*mImgMatrix) * (*mWorldMatrix));

    if (selectedMode == mode_scale) {
        painter.save();

        imgRectT.setSize(QSizeF(imgRectT.width() * scaleValues.x(), imgRectT.height() * scaleValues.y()).toSize());
        imgRectT.translate(QPointF(imgRectT.width() * 0.5 * (1 - scaleValues.x()) * (1 / scaleValues.x()),
                                   imgRectT.height() * 0.5 * (1 - scaleValues.y()) * (1 / scaleValues.y()))
                               .toPoint());

        affineTransform.scale(scaleValues.x(), scaleValues.y());
        affineTransform.translate(inImage.width() * 0.5 * (1 - scaleValues.x()) * (1 / scaleValues.x()),
                                  inImage.height() * 0.5 * (1 - scaleValues.y()) * (1 / scaleValues.y()));
    } else if (selectedMode == mode_rotate) {
        painter.save();

        double diag = qSqrt(inImage.height() * inImage.height() + inImage.width() * inImage.width());
        double initAngle = qAcos(inImage.width() / diag) * 180 / PI;
        affineTransform.translate(0.5 * inImage.width() - diag * 0.5 * qCos((initAngle + rotationValue) * PI / 180.0),
                                  0.5 * inImage.height() - diag * 0.5 * qSin((initAngle + rotationValue) * PI / 180.0));
        affineTransform.rotate(rotationValue);

        painter.fillRect(affineTransform.mapRect(inImage.rect()), Qt::white);
        imgRectT = affineTransform.mapRect(inImage.rect());
    } else if (selectedMode == mode_shear) {
        affineTransform.shear(shearValues.x(), shearValues.y());

        QRect transfRect = affineTransform.mapRect(inImage.rect());
        int signX = (shearValues.x() < 0) ? -1 : 1;
        int signY = (shearValues.y() < 0) ? -1 : 1;

        affineTransform.reset();
        affineTransform.translate(signX * (inImage.width() / 2 - transfRect.width() / 2), signY * (inImage.height() / 2 - transfRect.height() / 2));
        affineTransform.shear(shearValues.x(), shearValues.y());

        painter.fillRect(affineTransform.mapRect(inImage.rect()), Qt::white);
    }

    affineTransform *= painter.transform();

    painter.setTransform(affineTransform);

    painter.drawImage(inImage.rect(), inImage);

    drawGuide(&painter, QPolygonF(QRectF(imgRect)), mGuideMode);
    painter.drawRect(imgRect);

    if (selectedMode == mode_scale) {
        intrRect->updateRects(imgRectT);
        painter.restore();
        intrRect->draw(&painter);
    } else if (selectedMode == mode_rotate) {
        if (angleLinesEnabled) {
            QPen linePen(nmc::DkSettingsManager::param().display().highlightColor, qCeil(2.0 * imgRect.width() / 1000.0), Qt::SolidLine);
            QColor hCAlpha(50, 50, 50);
            hCAlpha.setAlpha(200);

            // QPen linePen(Qt::red, qCeil(3.0 * imgRect.width() / 1000.0), Qt::SolidLine);
            QVector<QVector4D> lines = skewEstimator.getLines();
            QVector<int> lineTypes = skewEstimator.getLineTypes();
            for (int i = 0; i < lines.size(); i++) {
                (lineTypes.at(i)) ? linePen.setColor(nmc::DkSettingsManager::param().display().highlightColor) : linePen.setColor(hCAlpha);
                painter.setPen(linePen);
                painter.drawLine(QPointF(lines.at(i).x(), lines.at(i).y()), QPointF(lines.at(i).z(), lines.at(i).w()));
            }
        }

        painter.restore();
        if (rotCropEnabled) {
            double newHeight = -((double)(inImage.height()) - (double)(inImage.width()) * qAbs(qTan(rotationValue * PI / 180)))
                / (qAbs(qTan(rotationValue * PI / 180)) * qAbs(qSin(rotationValue * PI / 180)) - qAbs(qCos(rotationValue * PI / 180)));
            QSize cropSize =
                QSize(qRound(((double)(inImage.width()) - newHeight * qAbs(qSin(rotationValue * PI / 180))) / qAbs(qCos(rotationValue * PI / 180))),
                      qRound(newHeight));
            QRect cropRect = QRect(QPointF(rotationCenter.x() - 0.5 * cropSize.width(), rotationCenter.y() - 0.5 * cropSize.height()).toPoint(), cropSize);

            if (cropSize.width() <= qSqrt(inImage.height() * inImage.height() + inImage.width() * inImage.width())
                && cropSize.height() <= qSqrt(inImage.height() * inImage.height() + inImage.width() * inImage.width())) {
                QBrush cropBrush = QBrush(QColor(128, 128, 128, 200));
                painter.fillRect(imgRectT.left(), imgRectT.top(), imgRectT.width(), -imgRectT.top() + cropRect.top(), cropBrush);
                painter.fillRect(imgRectT.left(), cropRect.bottom() + 1, imgRectT.width(), -cropRect.bottom() + imgRectT.bottom(), cropBrush);
                painter.fillRect(imgRectT.left(), cropRect.top(), cropRect.left() - imgRectT.left(), cropRect.height(), cropBrush);
                painter.fillRect(cropRect.right() + 1, cropRect.top(), -cropRect.right() + imgRectT.right(), cropRect.height(), cropBrush);

                painter.drawRect(cropRect);
            }
        }
    }

    painter.end();

    DkPluginViewPort::paintEvent(event);
}

void DkImgTransformationsViewPort::drawGuide(QPainter *painter, const QPolygonF &p, int paintMode)
{
    if (p.isEmpty() || paintMode == guide_no_guide)
        return;

    QColor col = painter->pen().color();
    col.setAlpha(150);
    QPen pen = painter->pen();
    QPen cPen = pen;
    cPen.setColor(col);
    painter->setPen(cPen);

    // vertical
    nmc::DkVector lp = p[1] - p[0]; // parallel to drawing
    nmc::DkVector l9 = p[3] - p[0]; // perpendicular to drawing

    int nLines = (paintMode == guide_rule_of_thirds) ? 3 : qRound(l9.norm() / 20);
    nmc::DkVector offset = l9;
    offset.normalize();
    offset *= l9.norm() / nLines;

    nmc::DkVector offsetVec = offset;

    for (int idx = 0; idx < (nLines - 1); idx++) {
        // step through & paint
        QLineF l = QLineF(nmc::DkVector(p[1] + offsetVec).toQPointF(), nmc::DkVector(p[0] + offsetVec).toQPointF());
        painter->drawLine(l);
        offsetVec += offset;
    }

    // horizontal
    lp = p[3] - p[0]; // parallel to drawing
    l9 = p[1] - p[0]; // perpendicular to drawing

    nLines = (paintMode == guide_rule_of_thirds) ? 3 : qRound(l9.norm() / 20);
    offset = l9;
    offset.normalize();
    offset *= l9.norm() / nLines;

    offsetVec = offset;

    for (int idx = 0; idx < (nLines - 1); idx++) {
        // step through & paint
        QLineF l = QLineF(nmc::DkVector(p[3] + offsetVec).toQPointF(), nmc::DkVector(p[0] + offsetVec).toQPointF());
        painter->drawLine(l);
        offsetVec += offset;
    }

    painter->setPen(pen); // revert painter
}

QImage DkImgTransformationsViewPort::getTransformedImage()
{
    if (parent()) {
        nmc::DkBaseViewPort *mViewport = dynamic_cast<nmc::DkBaseViewPort *>(parent());
        if (mViewport) {
            QImage inImage = mViewport->getImage();
            QTransform affineTransform = QTransform();

            if (selectedMode == mode_scale) {
                affineTransform.scale(scaleValues.x(), scaleValues.y());

                QImage paintedImage = QImage(affineTransform.mapRect(inImage.rect()).size(), inImage.format());
                QPainter imagePainter(&paintedImage);
                imagePainter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
                imagePainter.setTransform(affineTransform);
                imagePainter.drawImage(QPoint(0, 0), inImage);
                imagePainter.end();

                return paintedImage;
            } else if (selectedMode == mode_rotate) {
                double diag = qSqrt(inImage.height() * inImage.height() + inImage.width() * inImage.width());
                double initAngle = qAcos(inImage.width() / diag) * 180 / PI;
                affineTransform.translate(0.5 * inImage.width() - diag * 0.5 * qCos((initAngle + rotationValue) * PI / 180.0),
                                          0.5 * inImage.height() - diag * 0.5 * qSin((initAngle + rotationValue) * PI / 180.0));
                affineTransform.rotate(rotationValue);
                affineTransform.translate(-inImage.width() / 2, -inImage.height() / 2);

                QImage paintedImage = QImage(affineTransform.mapRect(inImage.rect()).size(), inImage.format());
                QPainter imagePainter(&paintedImage);
                imagePainter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
                imagePainter.fillRect(paintedImage.rect(), Qt::white);
                affineTransform.reset();
                affineTransform.translate(paintedImage.width() / 2, paintedImage.height() / 2);
                affineTransform.rotate(rotationValue);
                affineTransform.translate(-inImage.width() / 2, -inImage.height() / 2);
                imagePainter.setTransform(affineTransform);
                imagePainter.drawImage(QPoint(0, 0), inImage);
                imagePainter.end();

                if (rotCropEnabled) {
                    QRect croppedImageRect = paintedImage.rect();

                    QSize cropSize = QSize();
                    double newHeight = -((double)(inImage.height()) - (double)(inImage.width()) * qAbs(qTan(rotationValue * PI / 180)))
                        / (qAbs(qTan(rotationValue * PI / 180)) * qAbs(qSin(rotationValue * PI / 180)) - qAbs(qCos(rotationValue * PI / 180)));
                    cropSize =
                        QSize(qRound(((double)(inImage.width()) - newHeight * qAbs(qSin(rotationValue * PI / 180))) / qAbs(qCos(rotationValue * PI / 180))),
                              qRound(newHeight));
                    if (cropSize.width() <= qSqrt(inImage.height() * inImage.height() + inImage.width() * inImage.width())
                        && cropSize.height() <= qSqrt(inImage.height() * inImage.height() + inImage.width() * inImage.width()))
                        croppedImageRect =
                            QRect(QPointF(0.5 * paintedImage.width() - 0.5 * cropSize.width(), 0.5 * paintedImage.height() - 0.5 * cropSize.height()).toPoint(),
                                  cropSize);
                    QImage croppedImage = paintedImage.copy(croppedImageRect);

                    return croppedImage;
                }
                return paintedImage;
            } else if (selectedMode == mode_shear) {
                affineTransform.shear(shearValues.x(), shearValues.y());
                /*
                QRect transfRect = affineTransform.mapRect(inImage.rect());
                int signX = (shearValues.x() < 0) ? -1 : 1;
                int signY = (shearValues.y() < 0) ? -1 : 1;

                affineTransform.reset();
                affineTransform.translate(signX*(inImage.width()/2-transfRect.width()/2), signY*(inImage.height()/2-transfRect.height()/2));
                affineTransform.shear(shearValues.x(),shearValues.y());
                */
                QImage paintedImage = QImage(affineTransform.mapRect(inImage.rect()).size(), inImage.format());
                QPainter imagePainter(&paintedImage);
                imagePainter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
                imagePainter.fillRect(paintedImage.rect(), Qt::white);
                affineTransform.reset();
                affineTransform.translate(paintedImage.width() / 2, paintedImage.height() / 2);
                affineTransform.shear(shearValues.x(), shearValues.y());
                affineTransform.translate(-inImage.width() / 2, -inImage.height() / 2);
                imagePainter.setTransform(affineTransform);
                imagePainter.drawImage(QPoint(0, 0), inImage);
                imagePainter.end();

                return paintedImage;
            }
        }
    }

    return QImage();
}

void DkImgTransformationsViewPort::setMode(int mode)
{
    selectedMode = mode;
    setCursor(defaultCursor);

    if (mode == mode_rotate)
        setCursor(rotatingCursor);
    else if (mode == mode_shear)
        setCursor(Qt::SizeVerCursor);

    this->repaint();
}

void DkImgTransformationsViewPort::setScaleXValue(double val)
{
    this->scaleValues.setX(val);
    this->repaint();
}

void DkImgTransformationsViewPort::setScaleYValue(double val)
{
    this->scaleValues.setY(val);
    this->repaint();
}

void DkImgTransformationsViewPort::setShearXValue(double val)
{
    this->shearValues.setX(val);
    this->repaint();
}

void DkImgTransformationsViewPort::setShearYValue(double val)
{
    this->shearValues.setY(val);
    this->repaint();
}

void DkImgTransformationsViewPort::setRotationValue(double val)
{
    if (val < 0)
        val += 360;
    this->rotationValue = val;
    this->repaint();
}

void DkImgTransformationsViewPort::setCropEnabled(bool enabled)
{
    this->rotCropEnabled = enabled;
    this->repaint();
}

void DkImgTransformationsViewPort::setAngleLinesEnabled(bool enabled)
{
    this->angleLinesEnabled = enabled;
    this->repaint();
}

void DkImgTransformationsViewPort::calculateAutoRotation()
{
    if (parent()) {
        nmc::DkBaseViewPort *mViewport = dynamic_cast<nmc::DkBaseViewPort *>(parent());
        if (mViewport) {
            QImage img = mViewport->getImage();

            if (img.width() > 10 && img.height() > 10) {
                skewEstimator.setImage(img);
                rotationValue = skewEstimator.getSkewAngle();
                if (rotationValue < 0)
                    rotationValue += 360;
                imgTransformationsToolbar->setRotationValue(rotationValue);
                this->repaint();
                return;
            }
        }
    }

    rotationValue = 0;
    imgTransformationsToolbar->setRotationValue(rotationValue);
}

void DkImgTransformationsViewPort::setPanning(bool checked)
{
    this->panning = checked;
    if (checked)
        defaultCursor = Qt::OpenHandCursor;
    else
        defaultCursor = Qt::CrossCursor;
    setCursor(defaultCursor);
}

void DkImgTransformationsViewPort::applyChangesAndClose()
{
    cancelTriggered = false;
    emit closePlugin();
}

void DkImgTransformationsViewPort::discardChangesAndClose()
{
    cancelTriggered = true;
    emit closePlugin();
}

bool DkImgTransformationsViewPort::isCanceled()
{
    return cancelTriggered;
}

void DkImgTransformationsViewPort::setGuideStyle(int guideMode)
{
    this->mGuideMode = guideMode;
    this->repaint();
}

void DkImgTransformationsViewPort::setVisible(bool visible)
{
    if (parent()) {
        nmc::DkBaseViewPort *mViewport = dynamic_cast<nmc::DkBaseViewPort *>(parent());
        if (mViewport) {
            intrRect->setInitialValues(mViewport->getImage().rect());
            rotationCenter = QPoint(mViewport->getImage().width() / 2, mViewport->getImage().height() / 2);

            imgRatioAngle = atan2(mViewport->getImage().height(), mViewport->getImage().width());
        }
    }

    if (imgTransformationsToolbar)
        nmc::DkToolBarManager::inst().showToolBar(imgTransformationsToolbar, visible);

    setMode(defaultMode);
    DkPluginViewPort::setVisible(visible);
}

/*-----------------------------------DkImgTransformationsToolBar ---------------------------------------------*/
DkImgTransformationsToolBar::DkImgTransformationsToolBar(const QString &title, int defaultMode, QWidget *parent /* = 0 */)
    : QToolBar(title, parent)
{
    createIcons();
    createLayout(defaultMode);
    QMetaObject::connectSlotsByName(this);
}

DkImgTransformationsToolBar::~DkImgTransformationsToolBar()
{
}

void DkImgTransformationsToolBar::createIcons()
{
    // create icons
    icons.resize(icons_end);

    icons[apply_icon] = nmc::DkImage::loadIcon(":/nomacs/img/save.svg");
    icons[cancel_icon] = nmc::DkImage::loadIcon(":/nomacs/img/close.svg");
    icons[pan_icon] = nmc::DkImage::loadIcon(":/nomacs/img/pan.svg");
    icons[pan_icon].addPixmap(nmc::DkImage::loadIcon(":/nomacs/img/pan-checked.svg"), QIcon::Normal, QIcon::On);
    icons[scale_icon] = nmc::DkImage::loadIcon(":/nomacsPluginImgTrans/img/scale.svg");
    icons[rotate_icon] = nmc::DkImage::loadIcon(":/nomacsPluginImgTrans/img/rotate-plugin.svg");
    icons[shear_icon] = nmc::DkImage::loadIcon(":/nomacsPluginImgTrans/img/shear.svg");
}

void DkImgTransformationsToolBar::createLayout(int defaultMode)
{
    QList<QKeySequence> enterSc;
    enterSc.append(QKeySequence(Qt::Key_Enter));
    enterSc.append(QKeySequence(Qt::Key_Return));

    QAction *applyAction = new QAction(icons[apply_icon], tr("Apply (ENTER)"), this);
    applyAction->setShortcuts(enterSc);
    applyAction->setObjectName("applyAction");

    QAction *cancelAction = new QAction(icons[cancel_icon], tr("Cancel (ESC)"), this);
    cancelAction->setShortcut(QKeySequence(Qt::Key_Escape));
    cancelAction->setObjectName("cancelAction");

    panAction = new QAction(icons[pan_icon], tr("Pan"), this);
    panAction->setShortcut(QKeySequence(Qt::Key_P));
    panAction->setObjectName("panAction");
    panAction->setCheckable(true);
    panAction->setChecked(false);

    scaleAction = new QAction(icons[scale_icon], tr("Scale"), this);
    scaleAction->setShortcut(QKeySequence(Qt::Key_S));
    scaleAction->setObjectName("scaleAction");
    scaleAction->setCheckable(true);

    rotateAction = new QAction(icons[rotate_icon], tr("Rotate"), this);
    rotateAction->setShortcut(QKeySequence(Qt::Key_R));
    rotateAction->setObjectName("rotateAction");
    rotateAction->setCheckable(true);

    shearAction = new QAction(icons[shear_icon], tr("Shear"), this);
    shearAction->setShortcut(QKeySequence(Qt::Key_H));
    shearAction->setObjectName("shearAction");
    shearAction->setCheckable(true);

    // scale X value
    scaleXBox = new QDoubleSpinBox(this);
    scaleXBox->setObjectName("scaleXBox");
    scaleXBox->setMinimum(0.1);
    scaleXBox->setMaximum(2.5);
    scaleXBox->setSingleStep(0.01);
    scaleXBox->setDecimals(2);
    scaleXBox->setToolTip(tr("Scale in x direction"));
    scaleXBox->setStatusTip(scaleXBox->toolTip());

    // scale Y value
    scaleYBox = new QDoubleSpinBox(this);
    scaleYBox->setObjectName("scaleYBox");
    scaleYBox->setMinimum(0.1);
    scaleYBox->setMaximum(2.5);
    scaleYBox->setSingleStep(0.01);
    scaleYBox->setDecimals(2);
    scaleYBox->setToolTip(tr("Scale in y direction"));
    scaleYBox->setStatusTip(scaleYBox->toolTip());

    // rotation value
    rotationBox = new QDoubleSpinBox(this);
    rotationBox->setObjectName("rotationBox");
    rotationBox->setMinimum(-180);
    rotationBox->setMaximum(180);
    rotationBox->setSingleStep(0.1);
    rotationBox->setDecimals(1);
    rotationBox->setWrapping(true);
    rotationBox->setSuffix(dk_degree_str);
    QString tip = tr("Rotation angle [-180%1,180%2]").arg(dk_degree_str).arg(dk_degree_str);
    rotationBox->setToolTip(tip);
    rotationBox->setStatusTip(tip);

    // auto rotation selection
    autoRotateButton = new QPushButton(tr("Auto &Rotate"), this);
    autoRotateButton->setObjectName("autoRotateButton");
    autoRotateButton->setToolTip(tr("Automatically rotate image"));
    autoRotateButton->setStatusTip(autoRotateButton->toolTip());

    // show lines for automatic angle detection
    showLinesBox = new QCheckBox(tr("Show Angle Lines"), this);
    showLinesBox->setObjectName("showLinesBox");
    showLinesBox->setCheckState(Qt::Checked);
    showLinesBox->setToolTip(tr("Show lines for angle detection."));
    showLinesBox->setStatusTip(tr("Show lines (red) for angle detection. Green lines correspond to the selected angle."));

    // crop rotated image
    cropEnabledBox = new QCheckBox(tr("Crop Image"), this);
    cropEnabledBox->setObjectName("cropEnabledBox");
    cropEnabledBox->setCheckState(Qt::Unchecked);
    cropEnabledBox->setToolTip(tr("Crop rotated image if possible"));
    cropEnabledBox->setStatusTip(cropEnabledBox->toolTip());

    // shear X value
    shearXBox = new QDoubleSpinBox(this);
    shearXBox->setObjectName("shearXBox");
    shearXBox->setMinimum(-2);
    shearXBox->setMaximum(2);
    shearXBox->setSingleStep(0.01);
    shearXBox->setDecimals(2);
    shearXBox->setToolTip(tr("Shear in x direction"));
    shearXBox->setStatusTip(shearXBox->toolTip());

    // shear Y value
    shearYBox = new QDoubleSpinBox(this);
    shearYBox->setObjectName("shearYBox");
    shearYBox->setMinimum(-2);
    shearYBox->setMaximum(2);
    shearYBox->setSingleStep(0.01);
    shearYBox->setDecimals(2);
    shearYBox->setToolTip(tr("Shear in y direction"));
    shearYBox->setStatusTip(shearYBox->toolTip());

    // crop customization
    QStringList guides;
    guides << QT_TRANSLATE_NOOP("nmc::DkImgTransformationsToolBar", "Guides") << QT_TRANSLATE_NOOP("nmc::DkImgTransformationsToolBar", "Rule of Thirds")
           << QT_TRANSLATE_NOOP("nmc::DkImgTransformationsToolBar", "Grid");
    guideBox = new QComboBox(this);
    guideBox->addItems(guides);
    guideBox->setObjectName("guideBox");
    guideBox->setToolTip(tr("Show Guides in the Preview"));
    guideBox->setStatusTip(guideBox->toolTip());

    QActionGroup *modesGroup = new QActionGroup(this);
    modesGroup->addAction(scaleAction);
    modesGroup->addAction(rotateAction);
    modesGroup->addAction(shearAction);

    switch (defaultMode) {
    case mode_scale:
        scaleAction->setChecked(true);
        break;
    case mode_rotate:
        rotateAction->setChecked(true);
        break;
    case mode_shear:
        shearAction->setChecked(true);
        break;
    default:
        scaleAction->setChecked(true);
        break;
    }

    toolbarWidgetList = QMap<QString, QAction *>();

    addAction(applyAction);
    addAction(cancelAction);
    addAction(panAction);
    addSeparator();
    addAction(scaleAction);
    addAction(rotateAction);
    addAction(shearAction);
    addSeparator();
    toolbarWidgetList.insert(scaleXBox->objectName(), this->addWidget(scaleXBox));
    toolbarWidgetList.insert(scaleYBox->objectName(), this->addWidget(scaleYBox));
    toolbarWidgetList.insert(rotationBox->objectName(), this->addWidget(rotationBox));
#ifdef WITH_OPENCV
    toolbarWidgetList.insert(autoRotateButton->objectName(), this->addWidget(autoRotateButton));
    toolbarWidgetList.insert(showLinesBox->objectName(), this->addWidget(showLinesBox));
#endif
    toolbarWidgetList.insert(cropEnabledBox->objectName(), this->addWidget(cropEnabledBox));
    toolbarWidgetList.insert(shearXBox->objectName(), this->addWidget(shearXBox));
    toolbarWidgetList.insert(shearYBox->objectName(), this->addWidget(shearYBox));
    addSeparator();
    addWidget(guideBox);

    modifyLayout(defaultMode);
}

void DkImgTransformationsToolBar::modifyLayout(int mode)
{
    switch (mode) {
    case mode_scale:
        toolbarWidgetList.value(rotationBox->objectName())->setVisible(false);
#ifdef WITH_OPENCV
        toolbarWidgetList.value(autoRotateButton->objectName())->setVisible(false);
        toolbarWidgetList.value(showLinesBox->objectName())->setVisible(false);
#endif
        toolbarWidgetList.value(cropEnabledBox->objectName())->setVisible(false);
        toolbarWidgetList.value(scaleXBox->objectName())->setVisible(true);
        toolbarWidgetList.value(scaleYBox->objectName())->setVisible(true);
        toolbarWidgetList.value(shearXBox->objectName())->setVisible(false);
        toolbarWidgetList.value(shearYBox->objectName())->setVisible(false);
        scaleXBox->setValue(1);
        scaleYBox->setValue(1);
        break;
    case mode_rotate:
        toolbarWidgetList.value(scaleXBox->objectName())->setVisible(false);
        toolbarWidgetList.value(scaleYBox->objectName())->setVisible(false);
        toolbarWidgetList.value(rotationBox->objectName())->setVisible(true);
#ifdef WITH_OPENCV
        toolbarWidgetList.value(autoRotateButton->objectName())->setVisible(true);
        toolbarWidgetList.value(showLinesBox->objectName())->setVisible(true);
#endif
        toolbarWidgetList.value(cropEnabledBox->objectName())->setVisible(true);
        toolbarWidgetList.value(shearXBox->objectName())->setVisible(false);
        toolbarWidgetList.value(shearYBox->objectName())->setVisible(false);
        rotationBox->setValue(0);
        break;
    case mode_shear:
        toolbarWidgetList.value(scaleXBox->objectName())->setVisible(false);
        toolbarWidgetList.value(scaleYBox->objectName())->setVisible(false);
        toolbarWidgetList.value(rotationBox->objectName())->setVisible(false);
#ifdef WITH_OPENCV
        toolbarWidgetList.value(autoRotateButton->objectName())->setVisible(false);
        toolbarWidgetList.value(showLinesBox->objectName())->setVisible(false);
#endif
        toolbarWidgetList.value(cropEnabledBox->objectName())->setVisible(false);
        toolbarWidgetList.value(shearXBox->objectName())->setVisible(true);
        toolbarWidgetList.value(shearYBox->objectName())->setVisible(true);
        shearXBox->setValue(0);
        shearYBox->setValue(0);
        break;
    }
}

void DkImgTransformationsToolBar::setVisible(bool visible)
{
    if (visible) {
        rotationBox->setValue(0);
        scaleXBox->setValue(1);
        scaleYBox->setValue(1);
        shearXBox->setValue(0);
        shearYBox->setValue(0);
        panAction->setChecked(false);
    }

    QToolBar::setVisible(visible);
}

void DkImgTransformationsToolBar::on_applyAction_triggered()
{
    emit applySignal();
}

void DkImgTransformationsToolBar::on_cancelAction_triggered()
{
    emit cancelSignal();
}

void DkImgTransformationsToolBar::on_panAction_toggled(bool checked)
{
    emit panSignal(checked);
}

void DkImgTransformationsToolBar::on_scaleAction_toggled(bool checked)
{
    if (checked) {
        updateAffineTransformPluginSettings(mode_scale, settings_mode);
        modifyLayout(mode_scale);
        emit modeChangedSignal(mode_scale);
    }
}

void DkImgTransformationsToolBar::on_rotateAction_toggled(bool checked)
{
    if (checked) {
        updateAffineTransformPluginSettings(mode_rotate, settings_mode);
        modifyLayout(mode_rotate);
        emit modeChangedSignal(mode_rotate);
    }
}

void DkImgTransformationsToolBar::on_shearAction_toggled(bool checked)
{
    if (checked) {
        updateAffineTransformPluginSettings(mode_shear, settings_mode);
        modifyLayout(mode_shear);
        emit modeChangedSignal(mode_shear);
    }
}

void DkImgTransformationsToolBar::on_scaleXBox_valueChanged(double val)
{
    emit scaleXValSignal(val);
}

void DkImgTransformationsToolBar::on_scaleYBox_valueChanged(double val)
{
    emit scaleYValSignal(val);
}

void DkImgTransformationsToolBar::on_shearXBox_valueChanged(double val)
{
    emit shearXValSignal(val);
}

void DkImgTransformationsToolBar::on_shearYBox_valueChanged(double val)
{
    emit shearYValSignal(val);
}

void DkImgTransformationsToolBar::on_rotationBox_valueChanged(double val)
{
    rotationBox->setValue(val);
    emit rotationValSignal(val);
}

void DkImgTransformationsToolBar::on_autoRotateButton_clicked()
{
    emit calculateAutoRotationSignal();
}

void DkImgTransformationsToolBar::on_showLinesBox_stateChanged(int val)
{
    updateAffineTransformPluginSettings(val, settings_lines);
    emit showLinesSignal((val == Qt::Checked));
}

void DkImgTransformationsToolBar::on_cropEnabledBox_stateChanged(int val)
{
    updateAffineTransformPluginSettings(val, settings_crop);
    emit cropEnabledSignal((val == Qt::Checked));
}

void DkImgTransformationsToolBar::on_guideBox_currentIndexChanged(int val)
{
    updateAffineTransformPluginSettings(val, settings_guide);
    emit guideStyleSignal(val);
}

void DkImgTransformationsToolBar::setRotationValue(double val)
{
    if (val > 180)
        val -= 360;
    rotationBox->setValue(val);
}

void DkImgTransformationsToolBar::setScaleValue(QPointF val)
{
    scaleXBox->setValue(val.x());
    scaleYBox->setValue(val.y());
}

void DkImgTransformationsToolBar::setShearValue(QPointF val)
{
    shearXBox->setValue(val.x());
    shearYBox->setValue(val.y());
}

void DkImgTransformationsToolBar::setCropState(int val)
{
    cropEnabledBox->setChecked(val);
}

void DkImgTransformationsToolBar::setGuideLineState(int val)
{
    guideBox->setCurrentIndex(val);
}

void DkImgTransformationsToolBar::setAngleLineState(int val)
{
    showLinesBox->setChecked(val);
}

void DkImgTransformationsToolBar::updateAffineTransformPluginSettings(int val, int type)
{
    QSettings settings;

    switch (type) {
    case settings_mode:
        settings.setValue("affineTransformPlugin/mode", val);
        break;
    case settings_guide:
        settings.setValue("affineTransformPlugin/guideMode", val);
        break;
    case settings_crop:
        settings.setValue("affineTransformPlugin/cropEnabled", val);
        break;
    case settings_lines:
        settings.setValue("affineTransformPlugin/angleLines", val);
        break;
    }
}

/*-----------------------------------DkInteractionRects ---------------------------------------------*/
DkInteractionRects::DkInteractionRects(QRect imgRect, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    init();
    updateRects(imgRect);
}

DkInteractionRects::DkInteractionRects(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    init();
}

void DkInteractionRects::init()
{
    size = QSize(40, 40);
    intrRect = QVector<QRect>();
    intrCursors = QVector<QCursor>();
    intrCursors.push_back(Qt::SizeFDiagCursor);
    intrCursors.push_back(Qt::SizeBDiagCursor);
    intrCursors.push_back(Qt::SizeFDiagCursor);
    intrCursors.push_back(Qt::SizeBDiagCursor);
    intrCursors.push_back(Qt::SizeVerCursor);
    intrCursors.push_back(Qt::SizeVerCursor);
    intrCursors.push_back(Qt::SizeHorCursor);
    intrCursors.push_back(Qt::SizeHorCursor);
}

void DkInteractionRects::updateRects(QRect imgRect)
{
    intrRect.clear();

    QRect rect = QRect(imgRect.topLeft(), size);
    rect.moveCenter(imgRect.topLeft());
    intrRect.push_back(rect);
    rect.moveCenter(imgRect.topRight());
    intrRect.push_back(rect);
    rect.moveCenter(imgRect.bottomRight());
    intrRect.push_back(rect);
    rect.moveCenter(imgRect.bottomLeft());
    intrRect.push_back(rect);
    rect.moveCenter(imgRect.topLeft() + QPoint(imgRect.width() / 2, 0));
    intrRect.push_back(rect);
    rect.moveCenter(imgRect.bottomLeft() + QPoint(imgRect.width() / 2, 0));
    intrRect.push_back(rect);
    rect.moveCenter(imgRect.topLeft() + QPoint(0, imgRect.height() / 2));
    intrRect.push_back(rect);
    rect.moveCenter(imgRect.topRight() + QPoint(0, imgRect.height() / 2));
    intrRect.push_back(rect);
}

DkInteractionRects::~DkInteractionRects()
{
}

void DkInteractionRects::draw(QPainter *painter)
{
    QRectF visibleRect(QPointF(), QSizeF(5, 5));
    QRectF whiteRect(QPointF(), QSizeF(9, 9));

    QSizeF sizeVR = painter->worldTransform().inverted().mapRect(visibleRect).size();
    QSizeF sizeWR = painter->worldTransform().inverted().mapRect(whiteRect).size();

    visibleRect = QRectF(QPointF(), sizeVR);
    whiteRect = QRectF(QPointF(), sizeWR);

    for (int i = 0; i < intrRect.size(); i++) {
        visibleRect.moveCenter(intrRect.at(i).center());
        whiteRect.moveCenter(intrRect.at(i).center());

        painter->setBrush(QColor(255, 255, 255, 100));
        painter->drawRect(whiteRect);
        painter->setBrush(QColor(0, 0, 0));
        painter->drawRect(visibleRect);
    }
}

QPoint DkInteractionRects::getCenter()
{
    return QPoint(size.width() / 2, size.height() / 2);
}

QCursor DkInteractionRects::getCursorShape(int idx)
{
    return intrCursors.at(idx);
}

QVector<QRect> DkInteractionRects::getInteractionRects()
{
    return intrRect;
}

void DkInteractionRects::setInitialValues(QRect rect)
{
    initialPoints = QVector<QPointF>();
    initialPoints.push_back(rect.topLeft());
    initialPoints.push_back(rect.topRight());
    initialPoints.push_back(rect.bottomRight());
    initialPoints.push_back(rect.bottomLeft());
    initialPoints.push_back(rect.topLeft() + QPoint(rect.width() / 2, 0));
    initialPoints.push_back(rect.bottomLeft() + QPoint(rect.width() / 2, 0));
    initialPoints.push_back(rect.topLeft() + QPoint(0, rect.height() / 2));
    initialPoints.push_back(rect.topRight() + QPoint(0, rect.height() / 2));

    initialSize = rect.size();
}

QSize DkInteractionRects::getInitialSize()
{
    return initialSize;
}

QPointF DkInteractionRects::getInitialPoint(int idx)
{
    return initialPoints.at(idx);
}

};
