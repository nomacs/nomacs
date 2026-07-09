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

#include "DkManipulators.h"

#include "DkActionManager.h"
#include "DkImageStorage.h"
#include "DkManipulatorsIpl.h"
#include "DkSettings.h"

#include <QSharedPointer>
#include <QWidget>

namespace nmc
{

// DkBaseManipulator --------------------------------------------------------------------
DkBaseManipulator::DkBaseManipulator(QAction *action)
{
    mAction = action;

    // add default icon
    if (mAction->icon().isNull()) {
        mAction->setIcon(DkImage::loadIcon(":/nomacs/img/sliders.svg"));
    }
}

QString DkBaseManipulator::name() const
{
    QString text = mAction->iconText();
    return text.remove("&");
}

QAction *DkBaseManipulator::action() const
{
    return mAction;
}

// DkManipulatorManager --------------------------------------------------------------------
DkManipulatorManager::DkManipulatorManager() = default;

void DkManipulatorManager::createManipulators(QWidget *parent)
{
    Q_ASSERT(mManipulators.empty());

    const QKeySequence no_shortcut{};
    QVector<QAction *> actions;

    DkActionBuilder builder{actions, numManipulators, parent};

    builder.add(ACTION_ID(mpl_grayscale),
                DkImage::loadIcon(":/nomacs/img/grayscale.svg"),
                QObject::tr("&Grayscale"),
                QObject::tr("Convert to Grayscale"),
                no_shortcut);

    builder.add(ACTION_ID(mpl_auto_adjust),
                DkImage::loadIcon(":/nomacs/img/auto-adjust.svg"),
                QObject::tr("&Auto Adjust"),
                QObject::tr("Auto Adjust Image Contrast and Color Balance"),
                Qt::CTRL | Qt::SHIFT | Qt::Key_L);

    builder.add(ACTION_ID(mpl_normalize),
                DkImage::loadIcon(":/nomacs/img/normalize.svg"),
                QObject::tr("Nor&malize Image"),
                QObject::tr("Normalize the Image"),
                Qt::CTRL | Qt::SHIFT | Qt::Key_N);

    builder.add(ACTION_ID(mpl_flip_h),
                DkImage::loadIcon(":/nomacs/img/flip-horizontal.svg"),
                QObject::tr("Flip &Horizontal"),
                QObject::tr("Flip Image Horizontally"),
                no_shortcut);

    builder.add(ACTION_ID(mpl_flip_v),
                DkImage::loadIcon(":/nomacs/img/flip-vertical.svg"),
                QObject::tr("Flip &Vertical"),
                QObject::tr("Flip Image Vertically"),
                no_shortcut);

    builder.add(ACTION_ID(mpl_invert),
                DkImage::loadIcon(":/nomacs/img/invert.svg"),
                QObject::tr("&Invert Image"),
                QObject::tr("Invert the Image"),
                no_shortcut);

    builder.add(ACTION_ID(mpl_tiny_planet),
                DkImage::loadIcon(":/nomacs/img/tiny-planet.svg"),
                QObject::tr("&Tiny Planet..."),
                QObject::tr("Create a Tiny Planet"),
                no_shortcut);

    builder.add(ACTION_ID(mpl_color),
                DkImage::loadIcon(":/nomacs/img/bucket.svg"),
                QObject::tr("&Background Color..."),
                QObject::tr("Add a background color"),
                no_shortcut);

    builder.add(ACTION_ID(mpl_blur),
                DkImage::loadIcon(":/nomacs/img/blur.svg"),
                QObject::tr("&Blur..."),
                QObject::tr("Blur the image"),
                no_shortcut);

    builder.add(ACTION_ID(mpl_unsharp_mask),
                DkImage::loadIcon(":/nomacs/img/sharpen.svg"),
                QObject::tr("&Sharpen..."),
                QObject::tr("Sharpens the image by applying an unsharp mask"),
                no_shortcut);

    builder.add(ACTION_ID(mpl_rotate),
                DkImage::loadIcon(":/nomacs/img/rotate-cc.svg"),
                QObject::tr("&Rotate..."),
                QObject::tr("Rotate the image"),
                no_shortcut);

    builder.add(ACTION_ID(mpl_resize),
                DkImage::loadIcon(":/nomacs/img/resize.svg"),
                QObject::tr("&Resize..."),
                QObject::tr("Resize the image"),
                no_shortcut);

    builder.add(ACTION_ID(mpl_threshold),
                DkImage::loadIcon(":/nomacs/img/threshold.svg"),
                QObject::tr("&Threshold..."),
                QObject::tr("Threshold the image"),
                no_shortcut);

    builder.add(ACTION_ID(mpl_hue),
                DkImage::loadIcon(":/nomacs/img/sliders.svg"),
                QObject::tr("&Hue/Saturation..."),
                QObject::tr("Change Hue and Saturation"),
                no_shortcut);

    builder.add(ACTION_ID(mpl_exposure),
                DkImage::loadIcon(":/nomacs/img/exposure.svg"),
                QObject::tr("&Exposure..."),
                QObject::tr("Change the Exposure and Gamma"),
                no_shortcut);

    mManipulators.resize(numManipulators);

    mManipulators[mpl_grayscale] = QSharedPointer<DkGrayScaleManipulator>::create(actions[mpl_grayscale]);
    mManipulators[mpl_auto_adjust] = QSharedPointer<DkAutoAdjustManipulator>::create(actions[mpl_auto_adjust]);
    mManipulators[mpl_normalize] = QSharedPointer<DkNormalizeManipulator>::create(actions[mpl_normalize]);
    mManipulators[mpl_invert] = QSharedPointer<DkInvertManipulator>::create(actions[mpl_invert]);
    mManipulators[mpl_flip_h] = QSharedPointer<DkFlipHManipulator>::create(actions[mpl_flip_h]);
    mManipulators[mpl_flip_v] = QSharedPointer<DkFlipVManipulator>::create(actions[mpl_flip_v]);

    mManipulators[mpl_rotate] = QSharedPointer<DkRotateManipulator>::create(actions[mpl_rotate]);
    mManipulators[mpl_resize] = QSharedPointer<DkResizeManipulator>::create(actions[mpl_resize]);
    mManipulators[mpl_threshold] = QSharedPointer<DkThresholdManipulator>::create(actions[mpl_threshold]);
    mManipulators[mpl_color] = QSharedPointer<DkColorManipulator>::create(actions[mpl_color]);
    mManipulators[mpl_hue] = QSharedPointer<DkHueManipulator>::create(actions[mpl_hue]);
    mManipulators[mpl_exposure] = QSharedPointer<DkExposureManipulator>::create(actions[mpl_exposure]);
    mManipulators[mpl_tiny_planet] = QSharedPointer<DkTinyPlanetManipulator>::create(actions[mpl_tiny_planet]);
    mManipulators[mpl_blur] = QSharedPointer<DkBlurManipulator>::create(actions[mpl_blur]);
    mManipulators[mpl_unsharp_mask] = QSharedPointer<DkUnsharpMaskManipulator>::create(actions[mpl_unsharp_mask]);
}

QVector<QAction *> DkManipulatorManager::actions() const
{
    QVector<QAction *> aVec;

    for (auto m : mManipulators)
        aVec << m->action();

    return aVec;
}

QSharedPointer<DkBaseManipulatorExt> DkManipulatorManager::manipulatorExt(const ExtendedManipulator &mId) const
{
    return qSharedPointerCast<DkBaseManipulatorExt>(mManipulators[mId]);
}

QSharedPointer<DkBaseManipulator> DkManipulatorManager::manipulator(const QAction *action) const
{
    for (const QSharedPointer<DkBaseManipulator> &m : mManipulators) {
        if (m->action() == action)
            return m;
    }

    if (action)
        qWarning() << "no manipulator matches" << action;

    return QSharedPointer<DkBaseManipulator>();
}

QSharedPointer<DkBaseManipulator> DkManipulatorManager::manipulator(const QString &name) const
{
    for (const QSharedPointer<DkBaseManipulator> &m : mManipulators) {
        if (m->name() == name)
            return m;
    }

    qWarning() << "no manipulator matches" << name;

    return QSharedPointer<DkBaseManipulator>();
}

QVector<QSharedPointer<DkBaseManipulator>> DkManipulatorManager::manipulators() const
{
    return mManipulators;
}

int DkManipulatorManager::numSelected() const
{
    int nSel = 0;
    for (auto mpl : mManipulators) {
        if (mpl->isSelected())
            nSel++;
    }

    return nSel;
}

void DkManipulatorManager::loadSettings(QSettings &settings)
{
    settings.beginGroup("Manipulators");

    DkManipulatorManager::createManipulators(nullptr);

    for (auto mpl : mManipulators)
        mpl->loadSettings(settings);

    settings.endGroup();
}

void DkManipulatorManager::saveSettings(QSettings &settings) const
{
    settings.beginGroup("Manipulators");

    for (auto mpl : mManipulators)
        mpl->saveSettings(settings);

    settings.endGroup();
}

void DkBaseManipulator::setSelected(bool select)
{
    mIsSelected = select;
}

bool DkBaseManipulator::isSelected() const
{
    return mIsSelected;
}

QString DkBaseManipulator::errorMessage() const
{
    return "";
}

void DkBaseManipulator::saveSettings(QSettings &settings)
{
    settings.beginGroup(name());
    settings.setValue("selected", isSelected());
    settings.endGroup();
}

void DkBaseManipulator::loadSettings(QSettings &settings)
{
    settings.beginGroup(name());
    mIsSelected = settings.value("selected", isSelected()).toBool();
    settings.endGroup();
}

// DkBaseMainpulatorExt --------------------------------------------------------------------
DkBaseManipulatorExt::DkBaseManipulatorExt(QAction *action)
    : DkBaseManipulator(action)
{
}

void DkBaseManipulatorExt::setWidget(QWidget *widget)
{
    mWidget = widget;
}

QWidget *DkBaseManipulatorExt::widget() const
{
    return mWidget;
}

void DkBaseManipulatorExt::setDirty(bool dirty)
{
    mDirty = dirty;
}

bool DkBaseManipulatorExt::isDirty() const
{
    return mDirty;
}

}
