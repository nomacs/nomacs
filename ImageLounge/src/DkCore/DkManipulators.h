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

#include <QAction>

#include "nmc_config.h"

class QSettings;

namespace nmc
{

// nomacs defines

/// <summary>
/// Base class of simple image manipulators.
/// Manipulators are functions that map
/// an image to an image (e.g. grayscale).
/// If this class is extended, batch processing
/// can make use of it too.
/// </summary>
class DllCoreExport DkBaseManipulator
{
public:
    explicit DkBaseManipulator(QAction *action = nullptr);
    virtual ~DkBaseManipulator() = default;

    QString name() const;
    QAction *action() const;

    void setSelected(bool select);
    bool isSelected() const;

    virtual QString errorMessage() const = 0;
    virtual QImage apply(const QImage &img) const = 0;

    virtual void saveSettings(QSettings &settings);
    virtual void loadSettings(QSettings &settings);

private:
    QAction *mAction = nullptr;
    bool mIsSelected = false;
};

/// <summary>
/// Extended image manipulators.
/// These manipulators have settings
/// and a UI that allows users to manipulate
/// the settings.
/// </summary>
/// <seealso cref="DkBaseManipulator" />
class DllCoreExport DkBaseManipulatorExt : public DkBaseManipulator
{
public:
    explicit DkBaseManipulatorExt(QAction *action);

    void setWidget(QWidget *widget);
    QWidget *widget() const;

    void setDirty(bool dirty);
    bool isDirty() const;

private:
    bool mDirty = false;
    QWidget *mWidget = nullptr;
};

class DllCoreExport DkManipulatorManager
{
public:
    DkManipulatorManager();

    // simple manipulators
    enum ManipulatorId {
        m_grayscale = 0,
        m_auto_adjust,
        m_normalize,
        m_invert,
        m_flip_h,
        m_flip_v,

        m_end
    };

    // extended manipulators
    enum ManipulatorExtId {
        m_rotate = m_end,
        m_resize,
        m_threshold,
        m_color,
        m_hue,
        m_exposure,
        m_tiny_planet,
        m_blur,
        m_unsharp_mask,

        m_ext_end
    };

    void createManipulators(QWidget *parent);

    QVector<QAction *> actions() const;

    QSharedPointer<DkBaseManipulatorExt> manipulatorExt(const ManipulatorExtId &mId) const;
    QSharedPointer<DkBaseManipulator> manipulator(const QAction *action) const;
    QSharedPointer<DkBaseManipulator> manipulator(const QString &name) const;
    QVector<QSharedPointer<DkBaseManipulator>> manipulators() const;

    int numSelected() const;

    void loadSettings(QSettings &settings);
    void saveSettings(QSettings &settings) const;

private:
    QVector<QSharedPointer<DkBaseManipulator>> mManipulators;
};
}
