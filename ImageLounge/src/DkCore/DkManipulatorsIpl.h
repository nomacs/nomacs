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

#pragma once

#include "DkManipulators.h"

#include <QAction>

#pragma warning(disable : 4251) // TODO: remove

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

namespace nmc
{

// nomacs defines

class DkGrayScaleManipulator : public DkBaseManipulator
{
public:
    explicit DkGrayScaleManipulator(QAction *action = nullptr);

    QImage apply(const QImage &img) const override;
    QString errorMessage() const override;
};

class DkAutoAdjustManipulator : public DkBaseManipulator
{
public:
    explicit DkAutoAdjustManipulator(QAction *action = nullptr);

    QImage apply(const QImage &img) const override;
    QString errorMessage() const override;
};

class DkNormalizeManipulator : public DkBaseManipulator
{
public:
    explicit DkNormalizeManipulator(QAction *action = nullptr);

    QImage apply(const QImage &img) const override;
    QString errorMessage() const override;
};

class DkInvertManipulator : public DkBaseManipulator
{
public:
    explicit DkInvertManipulator(QAction *action = nullptr);

    QImage apply(const QImage &img) const override;
    QString errorMessage() const override;
};

class DkFlipHManipulator : public DkBaseManipulator
{
public:
    explicit DkFlipHManipulator(QAction *action = nullptr);

    QImage apply(const QImage &img) const override;
    QString errorMessage() const override;
};

class DkFlipVManipulator : public DkBaseManipulator
{
public:
    explicit DkFlipVManipulator(QAction *action = nullptr);

    QImage apply(const QImage &img) const override;
    QString errorMessage() const override;
};

// Extended --------------------------------------------------------------------
class DllCoreExport DkTinyPlanetManipulator : public DkBaseManipulatorExt
{
public:
    explicit DkTinyPlanetManipulator(QAction *action);

    QImage apply(const QImage &img) const override;
    QString errorMessage() const override;

    void setSize(int size);
    int size() const;

    void setAngle(int angle);
    int angle() const;

    void setInverted(bool invert);
    bool inverted() const;

private:
    int mSize = 30;
    int mAngle = 0;
    bool mInverted = false;
};

class DllCoreExport DkColorManipulator : public DkBaseManipulatorExt
{
public:
    explicit DkColorManipulator(QAction *action);

    QImage apply(const QImage &img) const override;
    QString errorMessage() const override;

    void setColor(const QColor &col);
    QColor color() const;

private:
    QColor mColor = Qt::white;
};

class DllCoreExport DkBlurManipulator : public DkBaseManipulatorExt
{
public:
    explicit DkBlurManipulator(QAction *action);

    QImage apply(const QImage &img) const override;
    QString errorMessage() const override;

    void setSigma(int sigma);
    int sigma() const;

private:
    int mSigma = 5;
};

class DllCoreExport DkUnsharpMaskManipulator : public DkBaseManipulatorExt
{
public:
    explicit DkUnsharpMaskManipulator(QAction *action);

    QImage apply(const QImage &img) const override;
    QString errorMessage() const override;

    void setSigma(int sigma);
    int sigma() const;

    void setAmount(int amount);
    int amount() const;

private:
    int mSigma = 30;
    int mAmount = 15;
};

class DllCoreExport DkRotateManipulator : public DkBaseManipulatorExt
{
public:
    explicit DkRotateManipulator(QAction *action);

    QImage apply(const QImage &img) const override;
    QString errorMessage() const override;

    void setAngle(int angle);
    int angle() const;

private:
    int mAngle = 0;
};

class DllCoreExport DkResizeManipulator : public DkBaseManipulatorExt
{
public:
    explicit DkResizeManipulator(QAction *action);

    QImage apply(const QImage &img) const override;
    QString errorMessage() const override;

    void setScaleFactor(double sf);
    double scaleFactor() const;

    void setInterpolation(int ipl);
    int interpolation() const;

    void setCorrectGamma(bool ug);
    bool correctGamma() const;

private:
    double mScaleFactor = 1.0;
    int mInterpolation = 1;
    bool mCorrectGamma = false;
};

class DllCoreExport DkThresholdManipulator : public DkBaseManipulatorExt
{
public:
    explicit DkThresholdManipulator(QAction *action);

    QImage apply(const QImage &img) const override;
    QString errorMessage() const override;

    void setThreshold(int thr);
    int threshold() const;

    void setColor(bool col);
    bool color() const;

private:
    int mThreshold = 128;
    bool mColor = false;
};

class DllCoreExport DkHueManipulator : public DkBaseManipulatorExt
{
public:
    explicit DkHueManipulator(QAction *action);

    QImage apply(const QImage &img) const override;
    QString errorMessage() const override;

    void setHue(int hue);
    int hue() const;

    void setSaturation(int sat);
    int saturation() const;

    void setValue(int val);
    int value() const;

private:
    int mHue = 0;
    int mSat = 0;
    int mValue = 0;
};

class DllCoreExport DkExposureManipulator : public DkBaseManipulatorExt
{
public:
    explicit DkExposureManipulator(QAction *action);

    QImage apply(const QImage &img) const override;
    QString errorMessage() const override;

    void setExposure(double exposure);
    double exposure() const;

    void setOffset(double offset);
    double offset() const;

    void setGamma(double gamma);
    double gamma() const;

private:
    double mExposure = 0.0;
    double mOffset = 0.0;
    double mGamma = 1.0;
};
}