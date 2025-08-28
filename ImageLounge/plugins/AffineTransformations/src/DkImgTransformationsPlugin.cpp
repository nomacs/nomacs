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

#include <QAction>
#include <QActionGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QMouseEvent>
#include <QPushButton>

#define PI 3.14159265

namespace nmp
{

/*-----------------------------------DkImgTransformationsPlugin ---------------------------------------------*/

/**
 *	Constructor
 **/
DkImgTransformationsPlugin::DkImgTransformationsPlugin() = default;

/**
 *	Destructor
 **/
DkImgTransformationsPlugin::~DkImgTransformationsPlugin() = default;

/**
 * Returns descriptive image
 **/
QImage DkImgTransformationsPlugin::image() const
{
    return QImage(":/nomacsPluginImgTrans/img/description.png");
}
bool DkImgTransformationsPlugin::hideHUD() const
{
    return true;
}

/**
 * Main function: runs plugin based on its ID
 * @param run ID
 * @param current image in the Nomacs mViewport
 **/
QSharedPointer<nmc::DkImageContainer> DkImgTransformationsPlugin::runPlugin(
    const QString &runID,
    QSharedPointer<nmc::DkImageContainer> imgC) const
{
    Q_UNUSED(runID);

    // for a mViewport plugin runID and image are null
    if (mViewport && imgC) {
        auto *transformVp = qobject_cast<DkImgTransformationsViewPort *>(mViewport);

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
    if (mImgTransformationsToolbar) {
        delete mImgTransformationsToolbar;
        mImgTransformationsToolbar = nullptr;
    }
}

void DkImgTransformationsViewPort::init()
{
    mDefaultMode = mode_scale;
    QSettings settings;
    settings.beginGroup("affineTransformPlugin");
    mDefaultMode = settings.value("mode", mDefaultMode).toInt();
    mGuideMode = settings.value("guideMode", guide_no_guide).toInt();
    mRotCropEnabled = (settings.value("cropEnabled", Qt::Unchecked).toInt() == Qt::Checked);
    mAngleLinesEnabled = (settings.value("angleLines", Qt::Checked).toInt() == Qt::Checked);
    settings.endGroup();

    mSelectedMode = mDefaultMode;
    mPanning = false;
    mCancelTriggered = false;
    mDefaultCursor = Qt::ArrowCursor;
    mRotatingCursor = QCursor(QPixmap(":/nomacs/img/rotating-cursor.png"));
    setCursor(mDefaultCursor);
    setMouseTracking(true);
    mScaleValues = QPointF(1, 1);
    mShearValues = QPointF(0, 0);
    mRotationValue = 0;
    mInsideIntrRect = false;
    mIntrIdx = 100;
    mRotationCenter = QPoint();

    mIntrRect = new DkInteractionRects(this);
    mSkewEstimator = DkSkewEstimator(this);

    mImgTransformationsToolbar = new DkImgTransformationsToolBar(tr("ImgTransformations Toolbar"), mDefaultMode, this);

    mImgTransformationsToolbar->setCropState((mRotCropEnabled) ? Qt::Checked : Qt::Unchecked);
    mImgTransformationsToolbar->setGuideLineState(mGuideMode);
    mImgTransformationsToolbar->setAngleLineState((mAngleLinesEnabled) ? Qt::Checked : Qt::Unchecked);

    connect(mImgTransformationsToolbar, SIGNAL(scaleXValSignal(double)), this, SLOT(setScaleXValue(double)));
    connect(mImgTransformationsToolbar, SIGNAL(scaleYValSignal(double)), this, SLOT(setScaleYValue(double)));
    connect(mImgTransformationsToolbar, SIGNAL(shearXValSignal(double)), this, SLOT(setShearXValue(double)));
    connect(mImgTransformationsToolbar, SIGNAL(shearYValSignal(double)), this, SLOT(setShearYValue(double)));
    connect(mImgTransformationsToolbar, SIGNAL(rotationValSignal(double)), this, SLOT(setRotationValue(double)));
    connect(mImgTransformationsToolbar, SIGNAL(calculateAutoRotationSignal()), this, SLOT(calculateAutoRotation()));
    connect(mImgTransformationsToolbar, SIGNAL(cropEnabledSignal(bool)), this, SLOT(setCropEnabled(bool)));
    connect(mImgTransformationsToolbar, SIGNAL(showLinesSignal(bool)), this, SLOT(setAngleLinesEnabled(bool)));
    connect(mImgTransformationsToolbar, SIGNAL(modeChangedSignal(int)), this, SLOT(setMode(int)));
    connect(mImgTransformationsToolbar, SIGNAL(guideStyleSignal(int)), this, SLOT(setGuideStyle(int)));
    connect(mImgTransformationsToolbar, SIGNAL(panSignal(bool)), this, SLOT(setPanning(bool)));
    connect(mImgTransformationsToolbar, SIGNAL(cancelSignal()), this, SLOT(discardChangesAndClose()));
    connect(mImgTransformationsToolbar, SIGNAL(applySignal()), this, SLOT(applyChangesAndClose()));
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
    if (event->buttons() == Qt::LeftButton
        && (event->modifiers() == nmc::DkSettingsManager::param().global().altMod || mPanning)) {
        setCursor(Qt::ClosedHandCursor);
        event->setModifiers(Qt::NoModifier); // we want a 'normal' action in the mViewport
        event->ignore();
        return;
    }

    if (mSelectedMode == mode_scale) {
        QVector<QRect> rects = mIntrRect->getInteractionRects();
        int currIdx;
        for (currIdx = 0; currIdx < rects.size(); currIdx++) {
            if (rects.at(currIdx).contains(map(event->pos()))) {
                mIntrIdx = currIdx;
                mInsideIntrRect = true;
                break;
            }
        }

        if (currIdx >= rects.size())
            mIntrIdx = 100;
    } else if (mSelectedMode == mode_rotate) {
        if (event->buttons() == Qt::LeftButton) {
            mReferencePoint = map(event->pos());
            mRotationValueTemp = mRotationValue;
        }
    } else if (mSelectedMode == mode_shear) {
        if (event->buttons() == Qt::LeftButton) {
            mShearValuesDir = QPointF(1, 0);

            nmc::DkVector c(mRotationCenter);
            nmc::DkVector xn(map(event->pos()));
            xn = c - xn;

            if ((xn.angle() > mImgRatioAngle && xn.angle() < PI - mImgRatioAngle)
                || (xn.angle() < -mImgRatioAngle && xn.angle() > -(PI - mImgRatioAngle))) {
                setCursor(Qt::SizeVerCursor);
                mShearValuesDir = QPointF(0, 1);
            } else
                setCursor(Qt::SizeHorCursor);

            mReferencePoint = map(event->pos());
            mShearValuesTemp = mShearValues;
        }
    }
    // no propagation
}

void DkImgTransformationsViewPort::mouseMoveEvent(QMouseEvent *event)
{
    // panning -> redirect to mViewport
    if (event->modifiers() == nmc::DkSettingsManager::param().global().altMod || mPanning) {
        event->setModifiers(Qt::NoModifier);
        event->ignore();
        update();
        return;
    }

    if (mSelectedMode == mode_scale) {
        QVector<QRect> rects = mIntrRect->getInteractionRects();

        if (mInsideIntrRect) {
            if (mIntrIdx < rects.size()) {
                setCursor(mIntrRect->getCursorShape(mIntrIdx));

                QSize initSize = mIntrRect->getInitialSize();
                QPointF initPoint = mIntrRect->getInitialPoint(mIntrIdx);
                int sign = 1;

                if (mIntrIdx < 6) {
                    if (mIntrIdx == 2 || mIntrIdx == 3 || mIntrIdx == 5)
                        sign = -1;
                    mScaleValues.setY(
                        qMin(2.5,
                             qMax(0.1,
                                  (initSize.height() * 0.5 + sign * (initPoint.y() - map(event->pos()).y()))
                                      / (initSize.height() * 0.5))));
                }

                sign = 1;
                if ((mIntrIdx >= 6) || (mIntrIdx < 4)) {
                    if (mIntrIdx == 2 || mIntrIdx == 1 || mIntrIdx == 7)
                        sign = -1;
                    mScaleValues.setX(
                        qMin(2.5,
                             qMax(0.1,
                                  (initSize.width() * 0.5 + sign * (initPoint.x() - map(event->pos()).x()))
                                      / (initSize.width() * 0.5))));
                }

                mImgTransformationsToolbar->setScaleValue(mScaleValues);
                this->repaint();
            }

        } else {
            int currIdx;

            for (currIdx = 0; currIdx < rects.size(); currIdx++) {
                if (rects.at(currIdx).contains(map(event->pos()))) {
                    setCursor(mIntrRect->getCursorShape(currIdx));
                    break;
                }
            }

            if (currIdx >= rects.size())
                setCursor(mDefaultCursor);
        }
    } else if (mSelectedMode == mode_rotate) {
        if (event->buttons() == Qt::LeftButton) {
            nmc::DkVector c(mRotationCenter);
            nmc::DkVector xt(mReferencePoint);
            nmc::DkVector xn(map(event->pos()));

            // compute the direction vector;
            xt = c - xt;
            xn = c - xn;
            double angle = xn.angle() - xt.angle();

            mRotationValue = mRotationValueTemp + angle / PI * 180;
            if (mRotationValue >= 360)
                mRotationValue -= 360;
            if (mRotationValue < 0)
                mRotationValue += 360;

            mImgTransformationsToolbar->setRotationValue(mRotationValue);
            // this->repaint();
        }
    } else if (mSelectedMode == mode_shear) {
        if (event->buttons() != Qt::LeftButton) {
            nmc::DkVector c(mRotationCenter);
            nmc::DkVector xn(map(event->pos()));
            xn = c - xn;

            if ((xn.angle() > mImgRatioAngle && xn.angle() < PI - mImgRatioAngle)
                || (xn.angle() < -mImgRatioAngle && xn.angle() > -(PI - mImgRatioAngle)))
                setCursor(Qt::SizeVerCursor);
            else
                setCursor(Qt::SizeHorCursor);
        } else if (event->buttons() == Qt::LeftButton) {
            QPoint currPoint = map(event->pos());

            mShearValues.setX(mShearValuesTemp.x()
                              + mShearValuesDir.x() * (currPoint.x() - mReferencePoint.x()) * 0.001);
            mShearValues.setY(mShearValuesTemp.y()
                              + mShearValuesDir.y() * (currPoint.y() - mReferencePoint.y()) * 0.001);

            mImgTransformationsToolbar->setShearValue(mShearValues);
        }
    }
}

void DkImgTransformationsViewPort::mouseReleaseEvent(QMouseEvent *event)
{
    mInsideIntrRect = false;
    mIntrIdx = 100;

    // panning -> redirect to mViewport
    if (event->modifiers() == nmc::DkSettingsManager::param().global().altMod || mPanning) {
        setCursor(mDefaultCursor);
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
        auto *mViewport = dynamic_cast<nmc::DkBaseViewPort *>(parent());
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

    if (mSelectedMode == mode_scale) {
        painter.save();

        imgRectT.setSize(QSizeF(imgRectT.width() * mScaleValues.x(), imgRectT.height() * mScaleValues.y()).toSize());
        imgRectT.translate(QPointF(imgRectT.width() * 0.5 * (1 - mScaleValues.x()) * (1 / mScaleValues.x()),
                                   imgRectT.height() * 0.5 * (1 - mScaleValues.y()) * (1 / mScaleValues.y()))
                               .toPoint());

        affineTransform.scale(mScaleValues.x(), mScaleValues.y());
        affineTransform.translate(inImage.width() * 0.5 * (1 - mScaleValues.x()) * (1 / mScaleValues.x()),
                                  inImage.height() * 0.5 * (1 - mScaleValues.y()) * (1 / mScaleValues.y()));
    } else if (mSelectedMode == mode_rotate) {
        painter.save();

        double diag = qSqrt(inImage.height() * inImage.height() + inImage.width() * inImage.width());
        double initAngle = qAcos(inImage.width() / diag) * 180 / PI;
        affineTransform.translate(0.5 * inImage.width() - diag * 0.5 * qCos((initAngle + mRotationValue) * PI / 180.0),
                                  0.5 * inImage.height() - diag * 0.5 * qSin((initAngle + mRotationValue) * PI / 180.0));
        affineTransform.rotate(mRotationValue);

        painter.fillRect(affineTransform.mapRect(inImage.rect()), Qt::white);
        imgRectT = affineTransform.mapRect(inImage.rect());
    } else if (mSelectedMode == mode_shear) {
        affineTransform.shear(mShearValues.x(), mShearValues.y());

        QRect transfRect = affineTransform.mapRect(inImage.rect());
        int signX = (mShearValues.x() < 0) ? -1 : 1;
        int signY = (mShearValues.y() < 0) ? -1 : 1;

        affineTransform.reset();
        affineTransform.translate(signX * (inImage.width() / 2 - transfRect.width() / 2),
                                  signY * (inImage.height() / 2 - transfRect.height() / 2));
        affineTransform.shear(mShearValues.x(), mShearValues.y());

        painter.fillRect(affineTransform.mapRect(inImage.rect()), Qt::white);
    }

    affineTransform *= painter.transform();

    painter.setTransform(affineTransform);

    painter.drawImage(inImage.rect(), inImage);

    drawGuide(&painter, QPolygonF(QRectF(imgRect)), mGuideMode);
    painter.drawRect(imgRect);

    if (mSelectedMode == mode_scale) {
        mIntrRect->updateRects(imgRectT);
        painter.restore();
        mIntrRect->draw(&painter);
    } else if (mSelectedMode == mode_rotate) {
        if (mAngleLinesEnabled) {
            QPen linePen(nmc::DkSettingsManager::param().display().highlightColor,
                         qCeil(2.0 * imgRect.width() / 1000.0),
                         Qt::SolidLine);
            QColor hCAlpha(50, 50, 50);
            hCAlpha.setAlpha(200);

            // QPen linePen(Qt::red, qCeil(3.0 * imgRect.width() / 1000.0), Qt::SolidLine);
            QVector<QVector4D> lines = mSkewEstimator.getLines();
            QVector<int> lineTypes = mSkewEstimator.getLineTypes();
            for (int i = 0; i < lines.size(); i++) {
                (lineTypes.at(i)) ? linePen.setColor(nmc::DkSettingsManager::param().display().highlightColor)
                                  : linePen.setColor(hCAlpha);
                painter.setPen(linePen);
                painter.drawLine(QPointF(lines.at(i).x(), lines.at(i).y()), QPointF(lines.at(i).z(), lines.at(i).w()));
            }
        }

        painter.restore();
        if (mRotCropEnabled) {
            double newHeight = -((double)(inImage.height())
                                 - (double)(inImage.width()) * qAbs(qTan(mRotationValue * PI / 180)))
                / (qAbs(qTan(mRotationValue * PI / 180)) * qAbs(qSin(mRotationValue * PI / 180))
                   - qAbs(qCos(mRotationValue * PI / 180)));
            QSize cropSize = QSize(qRound(((double)(inImage.width()) - newHeight * qAbs(qSin(mRotationValue * PI / 180)))
                                          / qAbs(qCos(mRotationValue * PI / 180))),
                                   qRound(newHeight));
            QRect cropRect = QRect(QPointF(mRotationCenter.x() - 0.5 * cropSize.width(),
                                           mRotationCenter.y() - 0.5 * cropSize.height())
                                       .toPoint(),
                                   cropSize);

            if (cropSize.width() <= qSqrt(inImage.height() * inImage.height() + inImage.width() * inImage.width())
                && cropSize.height()
                    <= qSqrt(inImage.height() * inImage.height() + inImage.width() * inImage.width())) {
                QBrush cropBrush = QBrush(QColor(128, 128, 128, 200));
                painter.fillRect(imgRectT.left(),
                                 imgRectT.top(),
                                 imgRectT.width(),
                                 -imgRectT.top() + cropRect.top(),
                                 cropBrush);
                painter.fillRect(imgRectT.left(),
                                 cropRect.bottom() + 1,
                                 imgRectT.width(),
                                 -cropRect.bottom() + imgRectT.bottom(),
                                 cropBrush);
                painter.fillRect(imgRectT.left(),
                                 cropRect.top(),
                                 cropRect.left() - imgRectT.left(),
                                 cropRect.height(),
                                 cropBrush);
                painter.fillRect(cropRect.right() + 1,
                                 cropRect.top(),
                                 -cropRect.right() + imgRectT.right(),
                                 cropRect.height(),
                                 cropBrush);

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
        auto *mViewport = dynamic_cast<nmc::DkBaseViewPort *>(parent());
        if (mViewport) {
            QImage inImage = mViewport->getImage();
            QTransform affineTransform = QTransform();

            if (mSelectedMode == mode_scale) {
                affineTransform.scale(mScaleValues.x(), mScaleValues.y());

                QImage paintedImage = QImage(affineTransform.mapRect(inImage.rect()).size(), inImage.format());
                QPainter imagePainter(&paintedImage);
                imagePainter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
                imagePainter.setTransform(affineTransform);
                imagePainter.drawImage(QPoint(0, 0), inImage);
                imagePainter.end();

                return paintedImage;
            } else if (mSelectedMode == mode_rotate) {
                double diag = qSqrt(inImage.height() * inImage.height() + inImage.width() * inImage.width());
                double initAngle = qAcos(inImage.width() / diag) * 180 / PI;
                affineTransform.translate(0.5 * inImage.width()
                                              - diag * 0.5 * qCos((initAngle + mRotationValue) * PI / 180.0),
                                          0.5 * inImage.height()
                                              - diag * 0.5 * qSin((initAngle + mRotationValue) * PI / 180.0));
                affineTransform.rotate(mRotationValue);
                affineTransform.translate(-inImage.width() / 2, -inImage.height() / 2);

                QImage paintedImage = QImage(affineTransform.mapRect(inImage.rect()).size(), inImage.format());
                QPainter imagePainter(&paintedImage);
                imagePainter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
                imagePainter.fillRect(paintedImage.rect(), Qt::white);
                affineTransform.reset();
                affineTransform.translate(paintedImage.width() / 2, paintedImage.height() / 2);
                affineTransform.rotate(mRotationValue);
                affineTransform.translate(-inImage.width() / 2, -inImage.height() / 2);
                imagePainter.setTransform(affineTransform);
                imagePainter.drawImage(QPoint(0, 0), inImage);
                imagePainter.end();

                if (mRotCropEnabled) {
                    QRect croppedImageRect = paintedImage.rect();

                    QSize cropSize = QSize();
                    double newHeight = -((double)(inImage.height())
                                         - (double)(inImage.width()) * qAbs(qTan(mRotationValue * PI / 180)))
                        / (qAbs(qTan(mRotationValue * PI / 180)) * qAbs(qSin(mRotationValue * PI / 180))
                           - qAbs(qCos(mRotationValue * PI / 180)));
                    cropSize = QSize(qRound(
                                         ((double)(inImage.width()) - newHeight * qAbs(qSin(mRotationValue * PI / 180)))
                                         / qAbs(qCos(mRotationValue * PI / 180))),
                                     qRound(newHeight));
                    if (cropSize.width()
                            <= qSqrt(inImage.height() * inImage.height() + inImage.width() * inImage.width())
                        && cropSize.height()
                            <= qSqrt(inImage.height() * inImage.height() + inImage.width() * inImage.width()))
                        croppedImageRect = QRect(QPointF(0.5 * paintedImage.width() - 0.5 * cropSize.width(),
                                                         0.5 * paintedImage.height() - 0.5 * cropSize.height())
                                                     .toPoint(),
                                                 cropSize);
                    QImage croppedImage = paintedImage.copy(croppedImageRect);

                    return croppedImage;
                }
                return paintedImage;
            } else if (mSelectedMode == mode_shear) {
                affineTransform.shear(mShearValues.x(), mShearValues.y());
                /*
                QRect transfRect = affineTransform.mapRect(inImage.rect());
                int signX = (shearValues.x() < 0) ? -1 : 1;
                int signY = (shearValues.y() < 0) ? -1 : 1;

                affineTransform.reset();
                affineTransform.translate(signX*(inImage.width()/2-transfRect.width()/2),
                signY*(inImage.height()/2-transfRect.height()/2));
                affineTransform.shear(shearValues.x(),shearValues.y());
                */
                QImage paintedImage = QImage(affineTransform.mapRect(inImage.rect()).size(), inImage.format());
                QPainter imagePainter(&paintedImage);
                imagePainter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
                imagePainter.fillRect(paintedImage.rect(), Qt::white);
                affineTransform.reset();
                affineTransform.translate(paintedImage.width() / 2, paintedImage.height() / 2);
                affineTransform.shear(mShearValues.x(), mShearValues.y());
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
    mSelectedMode = mode;
    setCursor(mDefaultCursor);

    if (mode == mode_rotate)
        setCursor(mRotatingCursor);
    else if (mode == mode_shear)
        setCursor(Qt::SizeVerCursor);

    this->repaint();
}

void DkImgTransformationsViewPort::setScaleXValue(double val)
{
    this->mScaleValues.setX(val);
    this->repaint();
}

void DkImgTransformationsViewPort::setScaleYValue(double val)
{
    this->mScaleValues.setY(val);
    this->repaint();
}

void DkImgTransformationsViewPort::setShearXValue(double val)
{
    this->mShearValues.setX(val);
    this->repaint();
}

void DkImgTransformationsViewPort::setShearYValue(double val)
{
    this->mShearValues.setY(val);
    this->repaint();
}

void DkImgTransformationsViewPort::setRotationValue(double val)
{
    if (val < 0)
        val += 360;
    this->mRotationValue = val;
    this->repaint();
}

void DkImgTransformationsViewPort::setCropEnabled(bool enabled)
{
    this->mRotCropEnabled = enabled;
    this->repaint();
}

void DkImgTransformationsViewPort::setAngleLinesEnabled(bool enabled)
{
    this->mAngleLinesEnabled = enabled;
    this->repaint();
}

void DkImgTransformationsViewPort::calculateAutoRotation()
{
    if (parent()) {
        auto *mViewport = dynamic_cast<nmc::DkBaseViewPort *>(parent());
        if (mViewport) {
            QImage img = mViewport->getImage();

            if (img.width() > 10 && img.height() > 10) {
                mSkewEstimator.setImage(img);
                mRotationValue = mSkewEstimator.getSkewAngle();
                if (mRotationValue < 0)
                    mRotationValue += 360;
                mImgTransformationsToolbar->setRotationValue(mRotationValue);
                this->repaint();
                return;
            }
        }
    }

    mRotationValue = 0;
    mImgTransformationsToolbar->setRotationValue(mRotationValue);
}

void DkImgTransformationsViewPort::setPanning(bool checked)
{
    this->mPanning = checked;
    if (checked)
        mDefaultCursor = Qt::OpenHandCursor;
    else
        mDefaultCursor = Qt::CrossCursor;
    setCursor(mDefaultCursor);
}

void DkImgTransformationsViewPort::applyChangesAndClose()
{
    mCancelTriggered = false;
    emit closePlugin();
}

void DkImgTransformationsViewPort::discardChangesAndClose()
{
    mCancelTriggered = true;
    emit closePlugin();
}

bool DkImgTransformationsViewPort::isCanceled()
{
    return mCancelTriggered;
}

void DkImgTransformationsViewPort::setGuideStyle(int guideMode)
{
    this->mGuideMode = guideMode;
    this->repaint();
}

void DkImgTransformationsViewPort::setVisible(bool visible)
{
    if (parent()) {
        auto *mViewport = dynamic_cast<nmc::DkBaseViewPort *>(parent());
        if (mViewport) {
            mIntrRect->setInitialValues(mViewport->getImage().rect());
            mRotationCenter = QPoint(mViewport->getImage().width() / 2, mViewport->getImage().height() / 2);

            mImgRatioAngle = atan2(mViewport->getImage().height(), mViewport->getImage().width());
        }
    }

    if (mImgTransformationsToolbar)
        nmc::DkToolBarManager::inst().showToolBar(mImgTransformationsToolbar, visible);

    setMode(mDefaultMode);
    DkPluginViewPort::setVisible(visible);
}

/*-----------------------------------DkImgTransformationsToolBar ---------------------------------------------*/
DkImgTransformationsToolBar::DkImgTransformationsToolBar(const QString &title,
                                                         int defaultMode,
                                                         QWidget *parent /* = 0 */)
    : QToolBar(title, parent)
{
    createIcons();
    createLayout(defaultMode);
    QMetaObject::connectSlotsByName(this);
}

DkImgTransformationsToolBar::~DkImgTransformationsToolBar() = default;

void DkImgTransformationsToolBar::createIcons()
{
    // create icons
    mIcons.resize(icons_end);

    mIcons[apply_icon] = nmc::DkImage::loadIcon(":/nomacs/img/save.svg");
    mIcons[cancel_icon] = nmc::DkImage::loadIcon(":/nomacs/img/close.svg");
    mIcons[pan_icon] = nmc::DkImage::loadIcon(":/nomacs/img/pan.svg");
    mIcons[pan_icon].addFile(":/nomacs/img/pan-checked.svg", QSize(), QIcon::Normal, QIcon::On);
    mIcons[scale_icon] = nmc::DkImage::loadIcon(":/nomacsPluginImgTrans/img/scale.svg");
    mIcons[rotate_icon] = nmc::DkImage::loadIcon(":/nomacsPluginImgTrans/img/rotate-plugin.svg");
    mIcons[shear_icon] = nmc::DkImage::loadIcon(":/nomacsPluginImgTrans/img/shear.svg");
}

void DkImgTransformationsToolBar::createLayout(int defaultMode)
{
    QList<QKeySequence> enterSc;
    enterSc.append(QKeySequence(Qt::Key_Enter));
    enterSc.append(QKeySequence(Qt::Key_Return));

    auto *applyAction = new QAction(mIcons[apply_icon], tr("Apply (ENTER)"), this);
    applyAction->setShortcuts(enterSc);
    applyAction->setObjectName("applyAction");

    auto *cancelAction = new QAction(mIcons[cancel_icon], tr("Cancel (ESC)"), this);
    cancelAction->setShortcut(QKeySequence(Qt::Key_Escape));
    cancelAction->setObjectName("cancelAction");

    mPanAction = new QAction(mIcons[pan_icon], tr("Pan"), this);
    mPanAction->setShortcut(QKeySequence(Qt::Key_P));
    mPanAction->setObjectName("panAction");
    mPanAction->setCheckable(true);
    mPanAction->setChecked(false);

    auto *scaleAction = new QAction(mIcons[scale_icon], tr("Scale"), this);
    scaleAction->setShortcut(QKeySequence(Qt::Key_S));
    scaleAction->setObjectName("scaleAction");
    scaleAction->setCheckable(true);

    auto *rotateAction = new QAction(mIcons[rotate_icon], tr("Rotate"), this);
    rotateAction->setShortcut(QKeySequence(Qt::Key_R));
    rotateAction->setObjectName("rotateAction");
    rotateAction->setCheckable(true);

    auto *shearAction = new QAction(mIcons[shear_icon], tr("Shear"), this);
    shearAction->setShortcut(QKeySequence(Qt::Key_H));
    shearAction->setObjectName("shearAction");
    shearAction->setCheckable(true);

    // scale X value
    mScaleXBox = new QDoubleSpinBox(this);
    mScaleXBox->setObjectName("scaleXBox");
    mScaleXBox->setMinimum(0.1);
    mScaleXBox->setMaximum(2.5);
    mScaleXBox->setSingleStep(0.01);
    mScaleXBox->setDecimals(2);
    mScaleXBox->setToolTip(tr("Scale in x direction"));
    mScaleXBox->setStatusTip(mScaleXBox->toolTip());

    // scale Y value
    mScaleYBox = new QDoubleSpinBox(this);
    mScaleYBox->setObjectName("scaleYBox");
    mScaleYBox->setMinimum(0.1);
    mScaleYBox->setMaximum(2.5);
    mScaleYBox->setSingleStep(0.01);
    mScaleYBox->setDecimals(2);
    mScaleYBox->setToolTip(tr("Scale in y direction"));
    mScaleYBox->setStatusTip(mScaleYBox->toolTip());

    // rotation value
    mRotationBox = new QDoubleSpinBox(this);
    mRotationBox->setObjectName("rotationBox");
    mRotationBox->setMinimum(-180);
    mRotationBox->setMaximum(180);
    mRotationBox->setSingleStep(0.1);
    mRotationBox->setDecimals(1);
    mRotationBox->setWrapping(true);
    mRotationBox->setSuffix(dk_degree_str);
    QString tip = tr("Rotation angle [-180%1,180%2]").arg(dk_degree_str).arg(dk_degree_str);
    mRotationBox->setToolTip(tip);
    mRotationBox->setStatusTip(tip);

    // auto rotation selection
    mAutoRotateButton = new QPushButton(tr("Auto &Rotate"), this);
    mAutoRotateButton->setObjectName("autoRotateButton");
    mAutoRotateButton->setToolTip(tr("Automatically rotate image"));
    mAutoRotateButton->setStatusTip(mAutoRotateButton->toolTip());

    // show lines for automatic angle detection
    mShowLinesBox = new QCheckBox(tr("Show Angle Lines"), this);
    mShowLinesBox->setObjectName("showLinesBox");
    mShowLinesBox->setCheckState(Qt::Checked);
    mShowLinesBox->setToolTip(tr("Show lines for angle detection."));
    mShowLinesBox->setStatusTip(
        tr("Show lines (red) for angle detection. Green lines correspond to the selected angle."));

    // crop rotated image
    mCropEnabledBox = new QCheckBox(tr("Crop Image"), this);
    mCropEnabledBox->setObjectName("cropEnabledBox");
    mCropEnabledBox->setCheckState(Qt::Unchecked);
    mCropEnabledBox->setToolTip(tr("Crop rotated image if possible"));
    mCropEnabledBox->setStatusTip(mCropEnabledBox->toolTip());

    // shear X value
    mShearXBox = new QDoubleSpinBox(this);
    mShearXBox->setObjectName("shearXBox");
    mShearXBox->setMinimum(-2);
    mShearXBox->setMaximum(2);
    mShearXBox->setSingleStep(0.01);
    mShearXBox->setDecimals(2);
    mShearXBox->setToolTip(tr("Shear in x direction"));
    mShearXBox->setStatusTip(mShearXBox->toolTip());

    // shear Y value
    mShearYBox = new QDoubleSpinBox(this);
    mShearYBox->setObjectName("shearYBox");
    mShearYBox->setMinimum(-2);
    mShearYBox->setMaximum(2);
    mShearYBox->setSingleStep(0.01);
    mShearYBox->setDecimals(2);
    mShearYBox->setToolTip(tr("Shear in y direction"));
    mShearYBox->setStatusTip(mShearYBox->toolTip());

    // crop customization
    QStringList guides;
    guides << QT_TRANSLATE_NOOP("nmc::DkImgTransformationsToolBar", "Guides")
           << QT_TRANSLATE_NOOP("nmc::DkImgTransformationsToolBar", "Rule of Thirds")
           << QT_TRANSLATE_NOOP("nmc::DkImgTransformationsToolBar", "Grid");
    mGuideBox = new QComboBox(this);
    mGuideBox->addItems(guides);
    mGuideBox->setObjectName("guideBox");
    mGuideBox->setToolTip(tr("Show Guides in the Preview"));
    mGuideBox->setStatusTip(mGuideBox->toolTip());

    auto *modesGroup = new QActionGroup(this);
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

    mToolbarWidgetList = QMap<QString, QAction *>();

    addAction(applyAction);
    addAction(cancelAction);
    addAction(mPanAction);
    addSeparator();
    addAction(scaleAction);
    addAction(rotateAction);
    addAction(shearAction);
    addSeparator();
    mToolbarWidgetList.insert(mScaleXBox->objectName(), this->addWidget(mScaleXBox));
    mToolbarWidgetList.insert(mScaleYBox->objectName(), this->addWidget(mScaleYBox));
    mToolbarWidgetList.insert(mRotationBox->objectName(), this->addWidget(mRotationBox));
#ifdef WITH_OPENCV
    mToolbarWidgetList.insert(mAutoRotateButton->objectName(), this->addWidget(mAutoRotateButton));
    mToolbarWidgetList.insert(mShowLinesBox->objectName(), this->addWidget(mShowLinesBox));
#endif
    mToolbarWidgetList.insert(mCropEnabledBox->objectName(), this->addWidget(mCropEnabledBox));
    mToolbarWidgetList.insert(mShearXBox->objectName(), this->addWidget(mShearXBox));
    mToolbarWidgetList.insert(mShearYBox->objectName(), this->addWidget(mShearYBox));
    addSeparator();
    addWidget(mGuideBox);

    modifyLayout(defaultMode);
}

void DkImgTransformationsToolBar::modifyLayout(int mode)
{
    switch (mode) {
    case mode_scale:
        mToolbarWidgetList.value(mRotationBox->objectName())->setVisible(false);
#ifdef WITH_OPENCV
        mToolbarWidgetList.value(mAutoRotateButton->objectName())->setVisible(false);
        mToolbarWidgetList.value(mShowLinesBox->objectName())->setVisible(false);
#endif
        mToolbarWidgetList.value(mCropEnabledBox->objectName())->setVisible(false);
        mToolbarWidgetList.value(mScaleXBox->objectName())->setVisible(true);
        mToolbarWidgetList.value(mScaleYBox->objectName())->setVisible(true);
        mToolbarWidgetList.value(mShearXBox->objectName())->setVisible(false);
        mToolbarWidgetList.value(mShearYBox->objectName())->setVisible(false);
        mScaleXBox->setValue(1);
        mScaleYBox->setValue(1);
        break;
    case mode_rotate:
        mToolbarWidgetList.value(mScaleXBox->objectName())->setVisible(false);
        mToolbarWidgetList.value(mScaleYBox->objectName())->setVisible(false);
        mToolbarWidgetList.value(mRotationBox->objectName())->setVisible(true);
#ifdef WITH_OPENCV
        mToolbarWidgetList.value(mAutoRotateButton->objectName())->setVisible(true);
        mToolbarWidgetList.value(mShowLinesBox->objectName())->setVisible(true);
#endif
        mToolbarWidgetList.value(mCropEnabledBox->objectName())->setVisible(true);
        mToolbarWidgetList.value(mShearXBox->objectName())->setVisible(false);
        mToolbarWidgetList.value(mShearYBox->objectName())->setVisible(false);
        mRotationBox->setValue(0);
        break;
    case mode_shear:
        mToolbarWidgetList.value(mScaleXBox->objectName())->setVisible(false);
        mToolbarWidgetList.value(mScaleYBox->objectName())->setVisible(false);
        mToolbarWidgetList.value(mRotationBox->objectName())->setVisible(false);
#ifdef WITH_OPENCV
        mToolbarWidgetList.value(mAutoRotateButton->objectName())->setVisible(false);
        mToolbarWidgetList.value(mShowLinesBox->objectName())->setVisible(false);
#endif
        mToolbarWidgetList.value(mCropEnabledBox->objectName())->setVisible(false);
        mToolbarWidgetList.value(mShearXBox->objectName())->setVisible(true);
        mToolbarWidgetList.value(mShearYBox->objectName())->setVisible(true);
        mShearXBox->setValue(0);
        mShearYBox->setValue(0);
        break;
    }
}

void DkImgTransformationsToolBar::setVisible(bool visible)
{
    if (visible) {
        mRotationBox->setValue(0);
        mScaleXBox->setValue(1);
        mScaleYBox->setValue(1);
        mShearXBox->setValue(0);
        mShearYBox->setValue(0);
        mPanAction->setChecked(false);
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
    mRotationBox->setValue(val);
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
    mRotationBox->setValue(val);
}

void DkImgTransformationsToolBar::setScaleValue(QPointF val)
{
    mScaleXBox->setValue(val.x());
    mScaleYBox->setValue(val.y());
}

void DkImgTransformationsToolBar::setShearValue(QPointF val)
{
    mShearXBox->setValue(val.x());
    mShearYBox->setValue(val.y());
}

void DkImgTransformationsToolBar::setCropState(int val)
{
    mCropEnabledBox->setChecked(val);
}

void DkImgTransformationsToolBar::setGuideLineState(int val)
{
    mGuideBox->setCurrentIndex(val);
}

void DkImgTransformationsToolBar::setAngleLineState(int val)
{
    mShowLinesBox->setChecked(val);
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
    mSize = QSize(40, 40);
    mIntrRect = QVector<QRect>();
    mIntrCursors = QVector<QCursor>();
    mIntrCursors.push_back(Qt::SizeFDiagCursor);
    mIntrCursors.push_back(Qt::SizeBDiagCursor);
    mIntrCursors.push_back(Qt::SizeFDiagCursor);
    mIntrCursors.push_back(Qt::SizeBDiagCursor);
    mIntrCursors.push_back(Qt::SizeVerCursor);
    mIntrCursors.push_back(Qt::SizeVerCursor);
    mIntrCursors.push_back(Qt::SizeHorCursor);
    mIntrCursors.push_back(Qt::SizeHorCursor);
}

void DkInteractionRects::updateRects(QRect imgRect)
{
    mIntrRect.clear();

    QRect rect = QRect(imgRect.topLeft(), mSize);
    rect.moveCenter(imgRect.topLeft());
    mIntrRect.push_back(rect);
    rect.moveCenter(imgRect.topRight());
    mIntrRect.push_back(rect);
    rect.moveCenter(imgRect.bottomRight());
    mIntrRect.push_back(rect);
    rect.moveCenter(imgRect.bottomLeft());
    mIntrRect.push_back(rect);
    rect.moveCenter(imgRect.topLeft() + QPoint(imgRect.width() / 2, 0));
    mIntrRect.push_back(rect);
    rect.moveCenter(imgRect.bottomLeft() + QPoint(imgRect.width() / 2, 0));
    mIntrRect.push_back(rect);
    rect.moveCenter(imgRect.topLeft() + QPoint(0, imgRect.height() / 2));
    mIntrRect.push_back(rect);
    rect.moveCenter(imgRect.topRight() + QPoint(0, imgRect.height() / 2));
    mIntrRect.push_back(rect);
}

DkInteractionRects::~DkInteractionRects() = default;

void DkInteractionRects::draw(QPainter *painter)
{
    QRectF visibleRect(QPointF(), QSizeF(5, 5));
    QRectF whiteRect(QPointF(), QSizeF(9, 9));

    QSizeF sizeVR = painter->worldTransform().inverted().mapRect(visibleRect).size();
    QSizeF sizeWR = painter->worldTransform().inverted().mapRect(whiteRect).size();

    visibleRect = QRectF(QPointF(), sizeVR);
    whiteRect = QRectF(QPointF(), sizeWR);

    for (int i = 0; i < mIntrRect.size(); i++) {
        visibleRect.moveCenter(mIntrRect.at(i).center());
        whiteRect.moveCenter(mIntrRect.at(i).center());

        painter->setBrush(QColor(255, 255, 255, 100));
        painter->drawRect(whiteRect);
        painter->setBrush(QColor(0, 0, 0));
        painter->drawRect(visibleRect);
    }
}

QPoint DkInteractionRects::getCenter()
{
    return QPoint(mSize.width() / 2, mSize.height() / 2);
}

QCursor DkInteractionRects::getCursorShape(int idx)
{
    return mIntrCursors.at(idx);
}

QVector<QRect> DkInteractionRects::getInteractionRects()
{
    return mIntrRect;
}

void DkInteractionRects::setInitialValues(QRect rect)
{
    mInitialPoints = QVector<QPointF>();
    mInitialPoints.push_back(rect.topLeft());
    mInitialPoints.push_back(rect.topRight());
    mInitialPoints.push_back(rect.bottomRight());
    mInitialPoints.push_back(rect.bottomLeft());
    mInitialPoints.push_back(rect.topLeft() + QPoint(rect.width() / 2, 0));
    mInitialPoints.push_back(rect.bottomLeft() + QPoint(rect.width() / 2, 0));
    mInitialPoints.push_back(rect.topLeft() + QPoint(0, rect.height() / 2));
    mInitialPoints.push_back(rect.topRight() + QPoint(0, rect.height() / 2));

    mInitialSize = rect.size();
}

QSize DkInteractionRects::getInitialSize()
{
    return mInitialSize;
}

QPointF DkInteractionRects::getInitialPoint(int idx)
{
    return mInitialPoints.at(idx);
}

};
