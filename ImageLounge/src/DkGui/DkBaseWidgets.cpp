/*******************************************************************************************************
 DkBaseWidgets.cpp
 Created on:	11.02.2015

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2015 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2015 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2015 Florian Kleber <florian@nomacs.org>

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

#include "DkBaseWidgets.h"
#include "DkActionManager.h"
#include "DkSettings.h"
#include "DkUtils.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QAction>
#include <QComboBox>
#include <QDebug>
#include <QEvent>
#include <QGraphicsEffect>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QScrollBar>
#include <QStyleOption>
#include <QTimer>
#pragma warning(pop) // no warnings from includes - end

namespace nmc
{
DkWidget::DkWidget(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
}

DkFadeHelper::DkFadeHelper(QWidget *widget)
    : mWidget(widget)
{
    mOpacityEffect = new QGraphicsOpacityEffect(mWidget);
    mOpacityEffect->setOpacity(0);
    mOpacityEffect->setEnabled(false);

    mWidget->setGraphicsEffect(mOpacityEffect);

    setWidgetVisible(false);
}

bool DkFadeHelper::getCurrentDisplaySetting()
{
    if (!mDisplayBits) {
        qWarning() << "[fade] no display settings available";
        return false;
    }

    if (DkSettingsManager::param().app().currentAppMode < 0 || DkSettingsManager::param().app().currentAppMode >= mDisplayBits->size()) {
        qWarning() << "[fade] illegal app mode: " << DkSettingsManager::param().app().currentAppMode;
        return false;
    }

    return mDisplayBits->testBit(DkSettingsManager::param().app().currentAppMode);
}

void DkFadeHelper::fade(bool show, bool saveSetting)
{
    qDebug().nospace() << "[fade] " << mWidget->metaObject()->className() << " show=" << show << " save=" << saveSetting << " showing=" << mShowing
                       << " hiding=" << mHiding << " visible=" << mWidget->isVisible();

    if (mBlocked) {
        qDebug() << "[fade]" << mWidget->metaObject()->className() << "visibility is blocked";
        return;
    }

    if (mDisplayBits && saveSetting) {
        int bit = DkSettingsManager::param().app().currentAppMode;
        mDisplayBits->setBit(bit, show);
    }

    if (mAction) {
        // assuming the action toggles visibility, it will probably end up back here
        mAction->blockSignals(true);
        mAction->setChecked(show);
        mAction->blockSignals(false);
    }

    bool inProgress = mShowing | mHiding;

    // no-op conditions
    if (!show && mWidget->isHidden())
        return;
    if (show && mWidget->isVisible() && !inProgress)
        return;

    // skip if we are going in the right direction
    if (show && mShowing)
        return;
    if (!show && mHiding)
        return;

    if (show) {
        mShowing = true;
        mHiding = false;
        if (mWidget->isHidden()) {
            mOpacityEffect->setEnabled(true);
            mOpacityEffect->setOpacity(0.0);
            setWidgetVisible(true);
        }

        if (!inProgress)
            animateOpacity();
    } else {
        mShowing = false;
        mHiding = true;

        if (!mOpacityEffect->isEnabled()) {
            mOpacityEffect->setEnabled(true);
            mOpacityEffect->setOpacity(1.0);
        }

        if (!inProgress)
            animateOpacity();
    }
}

void DkFadeHelper::setWidgetVisible(bool visible)
{
    if (mSetWidgetVisible) {
        qCritical() << "[fade] unexpected recursion, probably incorrect usage of FadeMixin";
        return;
    }

    mSetWidgetVisible = true;
    mWidget->setVisible(visible);
    mSetWidgetVisible = false;
}

void DkFadeHelper::animateOpacity()
{
    qreal step = mShowing ? 0.05 : -0.05;
    qreal opacity = mOpacityEffect->opacity() + step;
    opacity = qBound(0.0, opacity, 1.0);

    mOpacityEffect->setOpacity(opacity);

    if (opacity == 0.0)
        setWidgetVisible(false);

    if (opacity == 1.0 || opacity == 0.0) {
        mOpacityEffect->setEnabled(false);
        mHiding = false;
        mShowing = false;
        return;
    }

    // timer will not fire if mWidget is deleted
    QTimer::singleShot(20, mWidget, [this] {
        animateOpacity();
    });
}

// -------------------------------------------------------------------- DkFadeWidget
DkFadeWidget::DkFadeWidget(QWidget *parent, Qt::WindowFlags flags)
    : DkFadeMixin<DkWidget>(parent, flags)
{
    init();
}

void DkFadeWidget::init()
{
    setMouseTracking(true);
}

void DkFadeWidget::paintEvent(QPaintEvent *event)
{
    // fixes stylesheets which are not applied to custom widgets
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}

// DkNamedWidget --------------------------------------------------------------------
DkNamedWidget::DkNamedWidget(const QString &name, QWidget *parent)
    : DkFadeWidget(parent)
{
    mName = name;
}

QString DkNamedWidget::name() const
{
    return mName;
}

// DkLabel --------------------------------------------------------------------
DkLabel::DkLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
{
    setMouseTracking(true);
    mParent = parent;
    mText = text;
    init();
    hide();
}

DkLabel::~DkLabel()
{
}

void DkLabel::init()
{
    mTime = -1;
    mFixedWidth = -1;
    mFontSize = 13; // two sizes larger than default font see:stylesheet.css
    mTextCol = DkSettingsManager::param().display().hudFgdColor;
    mBlocked = false;

    mTimer.setSingleShot(true);
    connect(&mTimer, &QTimer::timeout, this, &DkLabel::hide);

    // default look and feel
    QFont font;
    font.setPointSize(mFontSize);
    QLabel::setFont(font);
    QLabel::setTextInteractionFlags(Qt::TextSelectableByMouse);

    QLabel::setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    updateStyleSheet();
}

void DkLabel::setText(const QString &msg, int time)
{
    mText = msg;
    mTime = time;

    if (!mTime || msg.isEmpty()) {
        hide();
        return;
    }

    setTextToLabel();
    show();

    if (time != -1)
        mTimer.start(time);
}

void DkLabel::showTimed(int time)
{
    mTime = time;

    if (!time) {
        hide();
        return;
    }

    show();

    if (time != -1)
        mTimer.start(time);
}

void DkLabel::setVisible(bool visible)
{
    if (!visible)
        mTimer.stop();
    QLabel::setVisible(visible);
}

QString DkLabel::getText()
{
    return mText;
}
/* dead code
void DkLabel::setFontSize(int fontSize)
{
    mFontSize = fontSize;

    QFont font;
    font.setPointSize(fontSize);
    QLabel::setFont(font);
    QLabel::adjustSize();
}
*/
void DkLabel::stop()
{
    hide();
}

void DkLabel::updateStyleSheet()
{
    QLabel::setStyleSheet("QLabel{color: " + mTextCol.name() + "; margin: " + QString::number(mMargin.y()) + "px " + QString::number(mMargin.x()) + "px "
                          + QString::number(mMargin.y()) + "px " + QString::number(mMargin.x()) + "px;}");
}

void DkLabel::paintEvent(QPaintEvent *event)
{
    if (mBlocked || !mTime) // guarantee that the label is hidden after the time is up
        return;

    QPainter painter(this);
    draw(&painter);
    painter.end();

    QLabel::paintEvent(event);
}

void DkLabel::draw(QPainter *painter)
{
    drawBackground(painter);
}

void DkLabel::setFixedWidth(int fixedWidth)
{
    mFixedWidth = fixedWidth;
    setTextToLabel();
}

void DkLabel::setTextToLabel()
{
    if (mFixedWidth == -1) {
        QLabel::setText(mText);
        QLabel::adjustSize();
    } else {
        setToolTip(mText);
        QLabel::setText(fontMetrics().elidedText(mText, Qt::ElideRight, mFixedWidth - 2 * mMargin.x()));
        QLabel::resize(mFixedWidth, height());
    }
}

DkLabelBg::DkLabelBg(const QString &text, QWidget *parent)
    : DkLabel(text, parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents); // labels should forward mouse events
    setObjectName("DkLabelBg");
}

// DkElidedLabel --------------------------------------------------------------------
DkElidedLabel::DkElidedLabel(QWidget *parent, const QString &text)
    : QLabel("", parent)
{
    setText(text);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
}

void DkElidedLabel::setText(QString text)
{
    content = text;
    updateElision();
}

void DkElidedLabel::resizeEvent(QResizeEvent *event)
{
    updateElision();
    QLabel::resizeEvent(event);
}

void DkElidedLabel::updateElision()
{
    QFontMetrics metrix(font());
    QString clippedText = metrix.elidedText(content, Qt::ElideRight, width());
    QLabel::setText(clippedText);
}

QSize DkElidedLabel::minimumSizeHint()
{
    return QSize(0, QLabel::minimumSizeHint().height());
}

QSize DkElidedLabel::minimumSize()
{
    return QSize(0, QLabel::minimumSize().height());
}

int DkElidedLabel::minimumWidth()
{
    return 0;
}

// DkFadeLabel --------------------------------------------------------------------
DkFadeLabel::DkFadeLabel(const QString &text, QWidget *parent)
    : DkFadeMixin(text, parent)
{
    init();
}

// DkDockWidget --------------------------------------------------------------------
DkDockWidget::DkDockWidget(const QString &title, QWidget *parent /* = 0 */, Qt::WindowFlags flags /* = 0 */)
    : QDockWidget(title, parent, flags)
{
    displaySettingsBits = 0;
    setObjectName("DkDockWidget");
}

DkDockWidget::~DkDockWidget()
{
}

void DkDockWidget::registerAction(QAction *action)
{
    mAction = action;
}

void DkDockWidget::setDisplaySettings(QBitArray *displayBits)
{
    displaySettingsBits = displayBits;
}

bool DkDockWidget::getCurrentDisplaySetting() const
{
    if (!displaySettingsBits)
        return false;

    return testDisplaySettings(*displaySettingsBits);
}

bool DkDockWidget::testDisplaySettings(const QBitArray &displaySettingsBits)
{
    if (DkSettingsManager::param().app().currentAppMode < 0 || DkSettingsManager::param().app().currentAppMode >= displaySettingsBits.size()) {
        qDebug() << "[WARNING] illegal app mode: " << DkSettingsManager::param().app().currentAppMode;
        return false;
    }

    return displaySettingsBits.testBit(DkSettingsManager::param().app().currentAppMode);
}

void DkDockWidget::setVisible(bool visible, bool saveSetting)
{
    QDockWidget::setVisible(visible);

    if (mAction) {
        mAction->blockSignals(true);
        mAction->setChecked(visible);
        mAction->blockSignals(false);
    }

    if (saveSetting && displaySettingsBits && displaySettingsBits->size() > DkSettingsManager::param().app().currentAppMode) {
        displaySettingsBits->setBit(DkSettingsManager::param().app().currentAppMode, visible);
    }
}

void DkDockWidget::closeEvent(QCloseEvent *event)
{
    setVisible(false);

    QDockWidget::closeEvent(event);
}

Qt::DockWidgetArea DkDockWidget::getDockLocationSettings(const Qt::DockWidgetArea &defaultArea) const
{
    DefaultSettings settings;
    Qt::DockWidgetArea location = (Qt::DockWidgetArea)settings.value(objectName(), defaultArea).toInt();

    return location;
}

// DkResizableScrollArea --------------------------------------------------------------------
DkResizableScrollArea::DkResizableScrollArea(QWidget *parent /* = 0 */)
    : QScrollArea(parent)
{
}

bool DkResizableScrollArea::eventFilter(QObject *o, QEvent *e)
{
    if (widget() && o == widget() && e->type() == QEvent::Resize) {
        updateSize();
    }

    return false;
}

void DkResizableScrollArea::updateSize()
{
    if (!widget())
        return;

    updateGeometry();

    if (verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) {
        int height = widget()->minimumSizeHint().height();

        if (horizontalScrollBar()->isVisible())
            height += horizontalScrollBar()->height();
        setMinimumHeight(height);
    }
    if (horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) {
        int width = widget()->minimumSizeHint().width();

        if (verticalScrollBar()->isVisible())
            width += verticalScrollBar()->height();
        setMinimumWidth(width);
    }
}

QSize DkResizableScrollArea::sizeHint() const
{
    if (!widget())
        return QScrollArea::sizeHint();

    widget()->updateGeometry();

    QSize s = QScrollArea::sizeHint();
    QSize ws = widget()->sizeHint();

    if (verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff)
        s.setHeight(ws.height());
    if (horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff)
        s.setWidth(ws.width());

    return s;
}

QSize DkResizableScrollArea::minimumSizeHint() const
{
    if (!widget())
        return QScrollArea::minimumSizeHint();

    QSize s = QScrollArea::minimumSizeHint();
    QSize ws = widget()->minimumSizeHint();

    if (verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff)
        s.setHeight(ws.height());
    if (horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff)
        s.setWidth(ws.width());

    return s;
}

}
