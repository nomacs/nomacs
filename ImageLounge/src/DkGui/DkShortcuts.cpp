/*******************************************************************************************************
 DkShortcuts.cpp
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

#include "DkShortcuts.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QAction>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QMainWindow>
#include <QTreeView>
#pragma warning(pop) // no warnings from includes - end

namespace nmc
{

DkShortcutEventFilter *DkShortcutEventFilter::instance()
{
    static auto *ptr = new DkShortcutEventFilter; // we don't care about destruction
    return ptr;
}

void DkShortcutEventFilter::reserveKeys(const QWidget *target, const QVector<QKeySequence> &keys)
{
    QString name = target->objectName();
    if (name.isEmpty()) {
        qCritical() << "[shortcuts] reserveKeys requires object name";
        return;
    }

    qDebug() << "[shortcuts]" << name << "reserving" << keys;

    instance()->mReservedKeys.insert(name, keys);
}

bool DkShortcutEventFilter::eventFilter(QObject *target, QEvent *e)
{
    // reject shortcuts that would interfere with widgets, this allows
    // shortcuts to bind to more keys than would normally be possible,
    // e.g. arrow keys, space bar, home/end, pgup/pgdown, letters w/o modifiers

    // to test this, bind one of the keys below to a shortcut; you should
    // not be able to activate the shortcut when the associated widget is focused

    // note: be performance-minded (try not to allocate memory, etc),
    // since all application events and keypress events need to be handled,
    // not just ones the ones bound to shortcuts

    if (Q_LIKELY(e->type() != QEvent::ShortcutOverride))
        return false;

    const QWidget *widget = dynamic_cast<QWidget *>(target);
    if (Q_UNLIKELY(!widget))
        return false;

    // only shortcuts of concern are bound in the main window
    if (Q_UNLIKELY(!widget->window()->metaObject()->inherits(&QMainWindow::staticMetaObject)))
        return false;

    const QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
    const QKeySequence keySeq(keyEvent->modifiers() | keyEvent->key());

    // widget needs to to allow its own shortcuts, for example, on the "Enter" key of DkExplorer
    for (QAction *a : widget->actions()) {
        if (QKeySequence::ExactMatch == a->shortcut().matches(keySeq)) {
            qDebug() << "[shortcuts] " << target->metaObject()->className() << "allowing own action:" << a->text() << keySeq;
            return false;
        }
    }

    // Override the keys/widgets that conflict
    // - To verify the keys, look up the widget source code in Qt (https://codebrowser.dev)
    // - Do not reserve left/right generally; for viewport shortcuts and we do not scroll horizontally, usually
    // - Widgets with text input already override keys
    // - Some widget keys use QKeySequence::StandardKey instead of Qt::Key,
    //   which can be different across platforms
    bool reserved = false;
    const QMetaObject *reservedType = nullptr; // object type that defined the reservation
    QString reservedName; // QObject::objectName()

    // if the widget inherits one of these classes, look for conflicts
    static const struct {
        const QMetaObject *type;
        const QVector<QKeySequence> keys;
    } reservedKeys[] = {
        // this comes before QAbstractItemView or else - as a subclass - it is never checked
        {&QTreeView::staticMetaObject,
         {
             // omit left/right
             Qt::Key_Up,
             Qt::Key_Down,
             Qt::SHIFT | Qt::Key_Right, // expand
             Qt::SHIFT | Qt::Key_Left, // collapse
             Qt::Key_PageUp,
             Qt::Key_PageDown,
             Qt::Key_Home,
             Qt::Key_End,
             Qt::Key_Asterisk, // expand recursive
             Qt::Key_Plus, // expand (alt)
             Qt::Key_Minus, // collapse (alt)
         }},
        {&QAbstractItemView::staticMetaObject,
         {
             // omit left/right
             Qt::Key_Up,
             Qt::Key_Down,
             Qt::Key_PageUp,
             Qt::Key_PageDown,
             Qt::Key_Home,
             Qt::Key_End,
         }},
        {&QAbstractScrollArea::staticMetaObject,
         {
             // omit left/right
             Qt::Key_Up,
             Qt::Key_Down,
             QKeySequence::MoveToNextPage,
             QKeySequence::MoveToPreviousPage,
         }},
        {&QAbstractSpinBox::staticMetaObject,
         {
             Qt::Key_Up,
             Qt::Key_Down,
             Qt::Key_PageUp,
             Qt::Key_PageDown,
         }},
        {&QAbstractSlider::staticMetaObject,
         {
             Qt::Key_Left,
             Qt::Key_Right,
             Qt::Key_Up,
             Qt::Key_Down,
             Qt::Key_PageUp,
             Qt::Key_PageDown,
             Qt::Key_Home,
             Qt::Key_End,
         }},
    };

    {
        // keys we reserved with reserveKeys() first take priority
        auto &map = qAsConst(mReservedKeys); // TODO: c++17 std::as_const
        auto it = map.find(widget->objectName());
        if (it != map.end()) {
            if (it.value().contains(keySeq)) {
                reservedType = widget->metaObject();
                reservedName = widget->objectName();
                reserved = true;
            } else {
                qDebug() << "[shortcuts]" << target->metaObject()->className() << target->objectName() << "allowing shortcut" << keySeq;
                return false;
            }
        }
    }

    if (!reserved) {
        for (auto &r : reservedKeys)
            if (widget->metaObject()->inherits(r.type)) {
                reservedType = r.type;
                if (r.keys.contains(keySeq))
                    reserved = true;
                break;
            }
    }

    // filename characters can be used to jump around in item views
    if (!reserved && (reservedType == &QTreeView::staticMetaObject || reservedType == &QAbstractItemView::staticMetaObject)) {
        const QString text = keyEvent->text();
        if (!text.isEmpty() && keyEvent->modifiers() == Qt::NoModifier) {
            if (text[0].isLower() || text[0].isNumber())
                reserved = true;
        }
    }

    // we have a conflict with global shortcut, do not allow it
    if (reserved) {
        qDebug() << "[shortcuts] " << target->metaObject()->className() << target->objectName() << "blocking" << keySeq << "reserved by"
                 << reservedType->className() << reservedName;
        e->accept();
        return true;
    }

    return false;
}

bool DkActionEventFilter::eventFilter(QObject *target, QEvent *event)
{
    (void)target;

    if (Q_LIKELY(event->type() != QEvent::ShortcutOverride && event->type() != QEvent::KeyPress))
        return false;

    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

    int key = keyEvent->modifiers() | keyEvent->key();
    for (auto *a : qAsConst(mActions)) // TODO: c++17 std::as_const()
        if (a->isEnabled() && QKeySequence::ExactMatch == a->shortcut().matches(key)) {
            if (event->type() == QEvent::ShortcutOverride)
                event->accept();
            else if (event->type() == QEvent::KeyPress)
                a->trigger();
            return true;
        }

    return false;
}

}
