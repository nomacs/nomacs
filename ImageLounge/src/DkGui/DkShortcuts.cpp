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
#include "DkTimer.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QDockWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QMainWindow>
#include <QSet>
#include <QTreeView>
#pragma warning(pop) // no warnings from includes - end
#include <utility>

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

static bool resolveShortcut(const QWidget *target,
                            const QSet<const QWidget *> &skip,
                            const QKeySequence &keySeq,
                            int depth = 0)
{
    if (!target->isVisible())
        return false;
    if (skip.contains(target))
        return false;

    // breadth-first resolution descending into children
    // qDebug().noquote() << "[shortcut] resolving:" << keySeq << QString(' ').repeated(depth * 2) <<
    // target->metaObject()->className() << target->objectName();
    for (QAction *a : target->actions()) {
        if (!a->isEnabled())
            continue;

        for (QKeySequence &s : a->shortcuts())
            if (QKeySequence::ExactMatch == s.matches(keySeq)) {
                qDebug() << "[shortcuts] resolved ambiguous" << keySeq << "=>" << target->metaObject()->className()
                         << target->objectName() << a->text();
                a->trigger();
                return true;
            }
    }

    for (QObject *o : target->children()) {
        if (!o->metaObject()->inherits(&QWidget::staticMetaObject))
            continue;
        bool found = resolveShortcut(static_cast<QWidget *>(o), skip, keySeq, depth++);
        if (found)
            return found;
    }

    return false;
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

    if (Q_UNLIKELY(e->type() == QEvent::Shortcut)) {
        auto *evt = static_cast<QShortcutEvent *>(e);
        if (!evt->isAmbiguous())
            return false;

        // stop normal shortcut propagation
        evt->accept();

        // we have to start looking somewhere, this seems reasonable
        const QWidget *w = qApp->focusWidget();
        // const QWidget *w = QApplication::widgetAt(QCursor::pos());

        // resolve shortcuts by breadth-first to the children,
        // then up through parent widgets
        DkTimer dt;
        QSet<const QWidget *> skip;
        while (w) {
            bool found = resolveShortcut(w, skip, evt->key());
            if (found) {
                return true;
            }

            // we're trying the parent next, avoid checking this child
            skip.insert(w);

            auto *o = w->parent();
            if (!o)
                break;

            if (!o->metaObject()->inherits(&QWidget::staticMetaObject))
                break;
            w = static_cast<QWidget *>(o);
        }

        qWarning() << "[shortcuts] " << target->metaObject()->className() << target->objectName()
                   << "unable to resolve ambiguous shortcut" << evt->shortcutId() << evt->key();

        return false;
    }

    if (Q_LIKELY(e->type() != QEvent::ShortcutOverride))
        return false;

    // QEvent::ShortcutOverride sends QKeyEvent, we accept it to and return true
    // to block the shortcut, or return false to allow it
    const QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);

    const QWidget *widget = dynamic_cast<QWidget *>(target);
    if (Q_UNLIKELY(!widget))
        return false;

    // we are only concerned with shortcuts parented to the main window and floating docks,
    // other windows/dialogs are not a problem, currently
    if (Q_UNLIKELY(!widget->window()->metaObject()->inherits(&QMainWindow::staticMetaObject)
                   && !widget->window()->metaObject()->inherits(&QDockWidget::staticMetaObject)))
        return false;

    const QKeySequence keySeq(keyEvent->modifiers() | keyEvent->key());

    // widget needs to to allow its own shortcuts, for example, on the "Enter" key of DkExplorer
    for (const QAction *a : widget->actions()) {
        if (!a->isEnabled())
            continue;

        for (QKeySequence &s : a->shortcuts())
            if (QKeySequence::ExactMatch == s.matches(keySeq)) {
                qDebug() << "[shortcuts] " << target->metaObject()->className() << "allowing own action:" << a->text()
                         << keySeq;
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
        auto &map = std::as_const(mReservedKeys);
        auto it = map.find(widget->objectName());
        if (it != map.end()) {
            if (it.value().contains(keySeq)) {
                reservedType = widget->metaObject();
                reservedName = widget->objectName();
                reserved = true;
            } else {
                qDebug() << "[shortcuts]" << target->metaObject()->className() << target->objectName()
                         << "allowing shortcut" << keySeq;
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
    if (!reserved
        && (reservedType == &QTreeView::staticMetaObject || reservedType == &QAbstractItemView::staticMetaObject)) {
        const QString text = keyEvent->text();
        if (!text.isEmpty() && keyEvent->modifiers() == Qt::NoModifier) {
            if (text[0].isLower() || text[0].isNumber())
                reserved = true;
        }
    }

    // we have a conflict with global shortcut, do not allow it
    if (reserved) {
        qDebug() << "[shortcuts] " << target->metaObject()->className() << target->objectName() << "blocking" << keySeq
                 << "reserved by" << reservedType->className() << reservedName;
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
    for (QAction *a : std::as_const(mActions)) {
        if (!a->isEnabled())
            continue;

        for (QKeySequence &s : a->shortcuts())
            if (QKeySequence::ExactMatch == s.matches(key)) {
                if (event->type() == QEvent::ShortcutOverride)
                    event->accept();
                else // KeyPress
                    a->trigger();
                return true;
            }
    }

    return false;
}

}
