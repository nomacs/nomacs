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
#include "DkManipulatorsIpl.h"

#include "DkImageContainer.h"
#include "DkImageStorage.h"
#include "DkSettings.h"

#pragma warning(push, 0) // no warnings from includes
#include <QSharedPointer>
#include <QWidget>
#pragma warning(pop)

namespace nmc
{

// DkBaseManipulator --------------------------------------------------------------------
DkBaseManipulator::DkBaseManipulator(QAction *action)
{
    mAction = action;

    // add default icon
    if (mAction->icon().isNull()) {
        QSize size(22, 22);
        mAction->setIcon(DkImage::loadIcon(":/nomacs/img/sliders.svg", size));
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
DkManipulatorManager::DkManipulatorManager()
{
}

void DkManipulatorManager::createManipulators(QWidget *parent)
{
    if (!mManipulators.empty())
        return;

    QVector<QSharedPointer<DkBaseManipulator>> mpls;
    mpls.resize(m_ext_end);

    QSize size(22, 22);

    // grayscale
    QAction *action;
    action = new QAction(DkImage::loadIcon(":/nomacs/img/grayscale.svg", size), QObject::tr("&Grayscale"), parent);
    action->setStatusTip(QObject::tr("Convert to Grayscale"));
    mpls[m_grayscale] = QSharedPointer<DkGrayScaleManipulator>::create(action);

    // auto adjust
    action = new QAction(DkImage::loadIcon(":/nomacs/img/auto-adjust.svg", size), QObject::tr("&Auto Adjust"), parent);
    action->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_L);
    action->setStatusTip(QObject::tr("Auto Adjust Image Contrast and Color Balance"));
    mpls[m_auto_adjust] = QSharedPointer<DkAutoAdjustManipulator>::create(action);

    // normalize
    action = new QAction(DkImage::loadIcon(":/nomacs/img/normalize.svg", size), QObject::tr("Nor&malize Image"), parent);
    action->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_N);
    action->setStatusTip(QObject::tr("Normalize the Image"));
    mpls[m_normalize] = QSharedPointer<DkNormalizeManipulator>::create(action);

    // flip horizontal
    action = new QAction(DkImage::loadIcon(":/nomacs/img/flip-horizontal.svg", size), QObject::tr("Flip &Horizontal"), parent);
    action->setStatusTip(QObject::tr("Flip Image Horizontally"));
    mpls[m_flip_h] = QSharedPointer<DkFlipHManipulator>::create(action);

    // flip vertical
    action = new QAction(DkImage::loadIcon(":/nomacs/img/flip-vertical.svg", size), QObject::tr("Flip &Vertical"), parent);
    action->setStatusTip(QObject::tr("Flip Image Vertically"));
    mpls[m_flip_v] = QSharedPointer<DkFlipVManipulator>::create(action);

    // invert image
    action = new QAction(DkImage::loadIcon(":/nomacs/img/invert.svg", size), QObject::tr("&Invert Image"), parent);
    action->setStatusTip(QObject::tr("Invert the Image"));
    mpls[m_invert] = QSharedPointer<DkInvertManipulator>::create(action);

    // extended --------------------------------------------------------------------
    // tiny planet
    action = new QAction(DkImage::loadIcon(":/nomacs/img/tiny-planet.svg", size), QObject::tr("&Tiny Planet..."), parent);
    action->setStatusTip(QObject::tr("Create a Tiny Planet"));
    mpls[m_tiny_planet] = QSharedPointer<DkTinyPlanetManipulator>::create(action);

    // tiny planet
    action = new QAction(DkImage::loadIcon(":/nomacs/img/bucket.svg", size), QObject::tr("&Background Color..."), parent);
    action->setStatusTip(QObject::tr("Add a background color"));
    mpls[m_color] = QSharedPointer<DkColorManipulator>::create(action);

    // blur
    action = new QAction(DkImage::loadIcon(":/nomacs/img/blur.svg", size), QObject::tr("&Blur..."), parent);
    action->setStatusTip(QObject::tr("Blur the image"));
    mpls[m_blur] = QSharedPointer<DkBlurManipulator>::create(action);

    // unsharp mask
    action = new QAction(DkImage::loadIcon(":/nomacs/img/sharpen.svg", size), QObject::tr("&Sharpen..."), parent);
    action->setStatusTip(QObject::tr("Sharpens the image by applying an unsharp mask"));
    mpls[m_unsharp_mask] = QSharedPointer<DkUnsharpMaskManipulator>::create(action);

    // rotate
    action = new QAction(DkImage::loadIcon(":/nomacs/img/rotate-cc.svg", size), QObject::tr("&Rotate..."), parent);
    action->setStatusTip(QObject::tr("Rotate the image"));
    mpls[m_rotate] = QSharedPointer<DkRotateManipulator>::create(action);

    // resize
    action = new QAction(DkImage::loadIcon(":/nomacs/img/resize.svg", size), QObject::tr("&Resize..."), parent);
    action->setStatusTip(QObject::tr("Resize the image"));
    mpls[m_resize] = QSharedPointer<DkResizeManipulator>::create(action);

    // threshold
    action = new QAction(DkImage::loadIcon(":/nomacs/img/threshold.svg", size), QObject::tr("&Threshold..."), parent);
    action->setStatusTip(QObject::tr("Threshold the image"));
    mpls[m_threshold] = QSharedPointer<DkThresholdManipulator>::create(action);

    // hue/saturation
    action = new QAction(DkImage::loadIcon(":/nomacs/img/sliders.svg", size), QObject::tr("&Hue/Saturation..."), parent);
    action->setStatusTip(QObject::tr("Change Hue and Saturation"));
    mpls[m_hue] = QSharedPointer<DkHueManipulator>::create(action);

    // exposure
    action = new QAction(DkImage::loadIcon(":/nomacs/img/exposure.svg", size), QObject::tr("&Exposure..."), parent);
    action->setStatusTip(QObject::tr("Change the Exposure and Gamma"));
    mpls[m_exposure] = QSharedPointer<DkExposureManipulator>::create(action);

    mManipulators = mpls;
}

QVector<QAction *> DkManipulatorManager::actions() const
{
    QVector<QAction *> aVec;

    for (auto m : mManipulators)
        aVec << m->action();

    return aVec;
}

QSharedPointer<DkBaseManipulatorExt> DkManipulatorManager::manipulatorExt(const ManipulatorExtId &mId) const
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

    DkManipulatorManager::createManipulators(0);

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
