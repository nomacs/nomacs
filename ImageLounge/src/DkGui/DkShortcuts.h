/*******************************************************************************************************
 DkShortcuts.h
 Created on:	03.28.2025

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2025 Scrubs <scrubbbbs@gmail.com>

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

#pragma warning(push, 0) // no warnings from includes - begin
#include <QHash>
#include <QKeySequence>
#include <QObject>
#include <QVector>
#pragma warning(pop) // no warnings from includes - end

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

class QAction;

namespace nmc
{

/**
 * Allows using global shortcut context for most nomacs shortcuts, but without
 * breaking widgets that have default keys, or use keyPressEvent() etc which
 * are not customizable by the user.
 */
class DllCoreExport DkShortcutEventFilter : public QObject
{
    Q_DISABLE_COPY_MOVE(DkShortcutEventFilter)

public:
    static DkShortcutEventFilter *instance();

    /**
     * Reserve keys to prevent shortcuts from taking them, or release keys back to shortcuts
     *
     * @param target event receiving key press events
     * @param keys   list of shortcuts to reserve, replacing any default/existing reservation
     *
     * @note target is identified by QObject::objectName()
     *
     * @details When a widget uses key events (not recommended, use actions!),
     * it should call this function to reserve that key. Which will prevent
     * any shortcuts from stealing it.
     *
     * By default, keys are reserved based on the widget type,
     * e.g. QTreeView reserves up/down arrows as that is hard-coded into Qt
     **/
    static void reserveKeys(const QWidget *target, const QVector<QKeySequence> &keys = {});

protected:
    DkShortcutEventFilter()
        : QObject()
    {
    }

    bool eventFilter(QObject *target, QEvent *e);

    QHash<QString, QVector<QKeySequence>> mReservedKeys;
};

/**
 * Allows a widget to bind actions that supercede global actions,
 * without the dreaded "ambiguous shortcut" warning from Qt.
 *
 * This would be used for widgets that cannot use DkActionManager
 * for some reason but still would like to use actions.
 *
 * Alternatively, widgets can use keyPressEvent() and reserveKeys()
 */
class DllCoreExport DkActionEventFilter : public QObject
{
public:
    DkActionEventFilter(QObject *parent)
        : QObject(parent)
    {
    }

    void addAction(QAction *action)
    {
        mActions.append(action);
    }

    void clearActions()
    {
        mActions.clear();
    }

protected:
    bool eventFilter(QObject *target, QEvent *e) override;

    QList<QAction *> mActions;
};

}
