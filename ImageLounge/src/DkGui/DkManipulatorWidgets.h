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

#include "DkBaseWidgets.h"
#include "DkImageContainer.h"
#include "DkManipulatorsIpl.h"

#pragma warning(push, 0) // no warnings from includes

#pragma warning(pop)

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport Q_DECL_IMPORT
#endif
#endif

// Qt defines

namespace nmc
{

// nomacs defines

class DkBaseManipulatorWidget : public DkFadeWidget
{
    Q_OBJECT

public:
    DkBaseManipulatorWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent = 0);

    QSharedPointer<DkBaseManipulatorExt> baseManipulator() const;

private:
    QSharedPointer<DkBaseManipulatorExt> mBaseManipulator;
};

class DkTinyPlanetWidget : public DkBaseManipulatorWidget
{
    Q_OBJECT

public:
    DkTinyPlanetWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent = 0);

    QSharedPointer<DkTinyPlanetManipulator> manipulator() const;

public slots:
    void onScaleSliderValueChanged(int val);
    void onAngleSliderValueChanged(int val);
    void onInvertBoxToggled(bool val);

private:
    void createLayout();
};

class DkBlurWidget : public DkBaseManipulatorWidget
{
    Q_OBJECT

public:
    DkBlurWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent = 0);

    QSharedPointer<DkBlurManipulator> manipulator() const;

public slots:
    void onSigmaSliderValueChanged(int val);

private:
    void createLayout();
};

class DkUnsharpMaskWidget : public DkBaseManipulatorWidget
{
    Q_OBJECT

public:
    DkUnsharpMaskWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent = 0);

    QSharedPointer<DkUnsharpMaskManipulator> manipulator() const;

public slots:
    void onSigmaSliderValueChanged(int val);
    void onAmountSliderValueChanged(int val);

private:
    void createLayout();
};

class DkRotateWidget : public DkBaseManipulatorWidget
{
    Q_OBJECT

public:
    DkRotateWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent = 0);

    QSharedPointer<DkRotateManipulator> manipulator() const;

public slots:
    void onAngleSliderValueChanged(int val);

private:
    void createLayout();
};

class DkResizeWidget : public DkBaseManipulatorWidget
{
    Q_OBJECT

public:
    DkResizeWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent = 0);

    QSharedPointer<DkResizeManipulator> manipulator() const;

public slots:
    void onScaleFactorSliderValueChanged(double val);
    void onIplBoxCurrentIndexChanged(int idx);
    void onGammaCorrectionToggled(bool checked);
    void onObjectNameChanged(const QString &name);

private:
    void createLayout();

    QComboBox *mIplBox;
};

class DkThresholdWidget : public DkBaseManipulatorWidget
{
    Q_OBJECT

public:
    DkThresholdWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent = 0);

    QSharedPointer<DkThresholdManipulator> manipulator() const;

public slots:
    void onThrSliderValueChanged(int val);
    void onColBoxToggled(bool checked);

private:
    void createLayout();
};

class DkHueWidget : public DkBaseManipulatorWidget
{
    Q_OBJECT

public:
    DkHueWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent = 0);

    QSharedPointer<DkHueManipulator> manipulator() const;

public slots:
    void onHueSliderValueChanged(int val);
    void onSatSliderValueChanged(int val);
    void onBrightnessSliderValueChanged(int val);

private:
    void createLayout();
};

class DkColorWidget : public DkBaseManipulatorWidget
{
    Q_OBJECT

public:
    DkColorWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent = 0);

    QSharedPointer<DkColorManipulator> manipulator() const;

public slots:
    void onColPickerColorSelected(const QColor &col);

private:
    void createLayout();
};

class DkExposureWidget : public DkBaseManipulatorWidget
{
    Q_OBJECT

public:
    DkExposureWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent = 0);

    QSharedPointer<DkExposureManipulator> manipulator() const;

public slots:
    void onExposureSliderValueChanged(double val);
    void onOffsetSliderValueChanged(double val);
    void onGammaSliderValueChanged(double val);

private:
    void createLayout();
};

// dock --------------------------------------------------------------------
class DkManipulatorWidget : public DkFadeWidget
{
    Q_OBJECT

public:
    DkManipulatorWidget(QWidget *parent = nullptr);

    void setImage(QSharedPointer<DkImageContainerT> imgC);

public slots:
    void selectManipulator();

private:
    void createLayout();
    QImage scaledPreview(const QImage &img) const;
    void selectManipulatorInner(QSharedPointer<DkBaseManipulatorExt> mpl);

    QVector<DkBaseManipulatorWidget *> mWidgets;

    QLabel *mPreview = nullptr;
    QLabel *mTitleLabel = nullptr;
    const int MaxPreview = 150;
};

class DkEditDock : public DkDockWidget
{
    Q_OBJECT

public:
    DkEditDock(const QString &title, QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

public slots:
    void setImage(QSharedPointer<DkImageContainerT> imgC);

protected:
    void createLayout();

    DkManipulatorWidget *mMplWidget = 0;
};

}
