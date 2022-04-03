/*******************************************************************************************************
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2016 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2016 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2016 Florian Kleber <florian@nomacs.org>

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

 related links:
 [1] https://nomacs.org/
 [2] https://github.com/nomacs/
 [3] http://download.nomacs.org
 *******************************************************************************************************/

#include "DkManipulatorsIpl.h"

#include "DkImageStorage.h"
#include "DkMath.h"

#pragma warning(push, 0) // no warnings from includes
#include <QDebug>
#include <QSharedPointer>
#pragma warning(pop)

namespace nmc
{

// DkGrayScaleManipulator --------------------------------------------------------------------
DkGrayScaleManipulator::DkGrayScaleManipulator(QAction *action)
    : DkBaseManipulator(action)
{
}

QImage DkGrayScaleManipulator::apply(const QImage &img) const
{
    if (img.isNull())
        return img;

    return DkImage::grayscaleImage(img);
}

QString DkGrayScaleManipulator::errorMessage() const
{
    return QObject::tr("Could not convert to grayscale");
}

// DkAutoAdjustManipulator --------------------------------------------------------------------
DkAutoAdjustManipulator::DkAutoAdjustManipulator(QAction *action)
    : DkBaseManipulator(action)
{
}

QImage DkAutoAdjustManipulator::apply(const QImage &img) const
{
    QImage imgR = img;
    if (DkImage::autoAdjustImage(imgR))
        return imgR;

    return QImage();
}

QString DkAutoAdjustManipulator::errorMessage() const
{
    return QString(QObject::tr("Cannot auto adjust"));
}

// DkNormalizeManipulator --------------------------------------------------------------------
DkNormalizeManipulator::DkNormalizeManipulator(QAction *action)
    : DkBaseManipulator(action)
{
}

QImage DkNormalizeManipulator::apply(const QImage &img) const
{
    QImage imgR = img;
    if (DkImage::normImage(imgR)) {
        return imgR;
    }

    return QImage();
}

QString DkNormalizeManipulator::errorMessage() const
{
    return QObject::tr("The Image is Already Normalized...");
}

// DkInvertManipulator --------------------------------------------------------------------
DkInvertManipulator::DkInvertManipulator(QAction *action)
    : DkBaseManipulator(action)
{
}

QImage DkInvertManipulator::apply(const QImage &img) const
{
    QImage imgR = img;
    imgR.invertPixels();
    return imgR;
}

QString DkInvertManipulator::errorMessage() const
{
    return QObject::tr("Cannot invert image");
}

// Flip Horizontally --------------------------------------------------------------------
DkFlipHManipulator::DkFlipHManipulator(QAction *action)
    : DkBaseManipulator(action)
{
}

QImage DkFlipHManipulator::apply(const QImage &img) const
{
    return img.mirrored(true, false);
}

QString DkFlipHManipulator::errorMessage() const
{
    return QObject::tr("Cannot flip image");
}

// Flip Vertically --------------------------------------------------------------------
DkFlipVManipulator::DkFlipVManipulator(QAction *action)
    : DkBaseManipulator(action)
{
}

QImage DkFlipVManipulator::apply(const QImage &img) const
{
    return img.mirrored(false, true);
}

QString DkFlipVManipulator::errorMessage() const
{
    return QObject::tr("Cannot flip image");
}

// DkTinyPlanetManipulator --------------------------------------------------------------------
DkTinyPlanetManipulator::DkTinyPlanetManipulator(QAction *action)
    : DkBaseManipulatorExt(action)
{
}

QImage DkTinyPlanetManipulator::apply(const QImage &img) const
{
#ifdef WITH_OPENCV
    int ms = qMax(img.width(), img.height());
    QSize s(ms, ms);

    QImage imgR = img.copy();
    DkImage::tinyPlanet(imgR, size(), angle() * DK_DEG2RAD, s, inverted());
    return imgR;
#else
    Q_UNUSED(img);
    return QImage(); // trigger warning
#endif
}

QString DkTinyPlanetManipulator::errorMessage() const
{
    return QObject::tr("Sorry, I could not create a tiny planet");
}

void DkTinyPlanetManipulator::setAngle(int angle)
{
    if (angle == mAngle)
        return;

    mAngle = angle;
    action()->trigger();
}

int DkTinyPlanetManipulator::angle() const
{
    return mAngle;
}

void DkTinyPlanetManipulator::setSize(int size)
{
    if (mSize == size)
        return;

    mSize = size;
    action()->trigger();
}

int DkTinyPlanetManipulator::size() const
{
    return mSize;
}

void DkTinyPlanetManipulator::setInverted(bool inverted)
{
    if (mInverted == inverted)
        return;

    mInverted = inverted;
    action()->trigger();
}

bool DkTinyPlanetManipulator::inverted() const
{
    return mInverted;
}

// DkUnsharpMaskManipulator --------------------------------------------------------------------
DkBlurManipulator::DkBlurManipulator(QAction *action)
    : DkBaseManipulatorExt(action)
{
}

QImage DkBlurManipulator::apply(const QImage &img) const
{
    QImage imgC = img.copy();
    DkImage::gaussianBlur(imgC, (float)sigma());
    return imgC;
}

QString DkBlurManipulator::errorMessage() const
{
    // so give me coffee & TV
    return QObject::tr("Cannot blur image");
}

void DkBlurManipulator::setSigma(int sigma)
{
    if (mSigma == sigma)
        return;

    mSigma = sigma;
    action()->trigger();
}

int DkBlurManipulator::sigma() const
{
    return mSigma;
}

// DkUnsharpMaskManipulator --------------------------------------------------------------------
DkUnsharpMaskManipulator::DkUnsharpMaskManipulator(QAction *action)
    : DkBaseManipulatorExt(action)
{
}

QImage DkUnsharpMaskManipulator::apply(const QImage &img) const
{
    QImage imgC = img.copy();
    DkImage::unsharpMask(imgC, (float)sigma(), 1.0f + amount() / 100.0f);
    return imgC;
}

QString DkUnsharpMaskManipulator::errorMessage() const
{
    return QObject::tr("Cannot sharpen image");
}

void DkUnsharpMaskManipulator::setSigma(int sigma)
{
    if (mSigma == sigma)
        return;

    mSigma = sigma;
    action()->trigger();
}

int DkUnsharpMaskManipulator::sigma() const
{
    return mSigma;
}

void DkUnsharpMaskManipulator::setAmount(int amount)
{
    if (mAmount == amount)
        return;

    mAmount = amount;
    action()->trigger();
}

int DkUnsharpMaskManipulator::amount() const
{
    return mAmount;
}

// Rotate Manipulator --------------------------------------------------------------------
DkRotateManipulator::DkRotateManipulator(QAction *action)
    : DkBaseManipulatorExt(action)
{
}

QImage DkRotateManipulator::apply(const QImage & img) const {
	return DkImage::rotateImage(img, angle());
}

QString DkRotateManipulator::errorMessage() const
{
    return QObject::tr("Cannot rotate image");
}

void DkRotateManipulator::setAngle(int angle)
{
    if (angle == mAngle)
        return;

    mAngle = angle;
    action()->trigger();
}

int DkRotateManipulator::angle() const
{
    return mAngle;
}

// -------------------------------------------------------------------- DkResizeManipulator
DkResizeManipulator::DkResizeManipulator(QAction *action)
    : DkBaseManipulatorExt(action)
{
}

QImage DkResizeManipulator::apply(const QImage &img) const
{
    if (mScaleFactor == 1.0)
        return img;

    return DkImage::resizeImage(img, QSize(), mScaleFactor, mInterpolation, mCorrectGamma);
}

QString DkResizeManipulator::errorMessage() const
{
    return QObject::tr("Cannot resize image");
}

void DkResizeManipulator::setScaleFactor(double sf)
{
    mScaleFactor = sf;
    action()->trigger();
}

double DkResizeManipulator::scaleFactor() const
{
    return mScaleFactor;
}

void DkResizeManipulator::setInterpolation(int ipl)
{
    mInterpolation = ipl;
    action()->trigger();
}

int DkResizeManipulator::interpolation() const
{
    return mInterpolation;
}

void DkResizeManipulator::setCorrectGamma(bool cg)
{
    mCorrectGamma = cg;
    action()->trigger();
}

bool DkResizeManipulator::correctGamma() const
{
    return mCorrectGamma;
}

// Rotate Manipulator --------------------------------------------------------------------
DkThresholdManipulator::DkThresholdManipulator(QAction *action)
    : DkBaseManipulatorExt(action)
{
}

QImage DkThresholdManipulator::apply(const QImage &img) const
{
    return DkImage::thresholdImage(img, threshold(), color());
}

QString DkThresholdManipulator::errorMessage() const
{
    return QObject::tr("Cannot threshold image");
}

void DkThresholdManipulator::setThreshold(int thr)
{
    if (thr == mThreshold)
        return;

    mThreshold = thr;
    action()->trigger();
}

int DkThresholdManipulator::threshold() const
{
    return mThreshold;
}

void DkThresholdManipulator::setColor(bool col)
{
    if (col == mColor)
        return;

    mColor = col;
    action()->trigger();
}

bool DkThresholdManipulator::color() const
{
    return mColor;
}

// DkHueManipulator --------------------------------------------------------------------
DkHueManipulator::DkHueManipulator(QAction *action)
    : DkBaseManipulatorExt(action)
{
}

QImage DkHueManipulator::apply(const QImage &img) const
{
    return DkImage::hueSaturation(img, hue(), saturation(), value());
}

QString DkHueManipulator::errorMessage() const
{
    return QObject::tr("Cannot change Hue/Saturation");
}

void DkHueManipulator::setHue(int hue)
{
    if (mHue == hue)
        return;

    mHue = hue;
    action()->trigger();
}

int DkHueManipulator::hue() const
{
    return mHue;
}

void DkHueManipulator::setSaturation(int sat)
{
    if (mSat == sat)
        return;

    mSat = sat;
    action()->trigger();
}

int DkHueManipulator::saturation() const
{
    return mSat;
}

void DkHueManipulator::setValue(int val)
{
    if (mValue == val)
        return;

    mValue = val;
    action()->trigger();
}

int DkHueManipulator::value() const
{
    return mValue;
}

DkExposureManipulator::DkExposureManipulator(QAction *action)
    : DkBaseManipulatorExt(action)
{
}

QImage DkExposureManipulator::apply(const QImage &img) const
{
    return DkImage::exposure(img, exposure(), offset(), gamma());
}

QString DkExposureManipulator::errorMessage() const
{
    return QObject::tr("Cannot apply exposure");
}

void DkExposureManipulator::setExposure(double exposure)
{
    if (mExposure == exposure)
        return;

    mExposure = exposure;
    action()->trigger();
}

double DkExposureManipulator::exposure() const
{
    return mExposure;
}

void DkExposureManipulator::setOffset(double offset)
{
    if (mOffset == offset)
        return;

    mOffset = offset;
    action()->trigger();
}

double DkExposureManipulator::offset() const
{
    return mOffset;
}

void DkExposureManipulator::setGamma(double gamma)
{
    if (mGamma == gamma)
        return;

    mGamma = gamma;
    action()->trigger();
}

double DkExposureManipulator::gamma() const
{
    return mGamma;
}

// -------------------------------------------------------------------- DkColorManipulator
DkColorManipulator::DkColorManipulator(QAction *action)
    : DkBaseManipulatorExt(action)
{
}

QImage DkColorManipulator::apply(const QImage &img) const
{
    return DkImage::bgColor(img, color());
}

QString DkColorManipulator::errorMessage() const
{
    return QObject::tr("Cannot draw background color");
}

void DkColorManipulator::setColor(const QColor &col)
{
    if (mColor == col)
        return;

    mColor = col;
    action()->trigger();
}

QColor DkColorManipulator::color() const
{
    return mColor;
}

}