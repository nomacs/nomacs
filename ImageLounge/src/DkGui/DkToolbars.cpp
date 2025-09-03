/*******************************************************************************************************
 DkTransferToolBar.cpp
 Created on:	13.02.2012

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2013 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2013 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2013 Florian Kleber <florian@nomacs.org>

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

#include "DkToolbars.h"

#include "DkActionManager.h"
#include "DkBasicWidgets.h"
#include "DkImageStorage.h"
#include "DkMath.h"
#include "DkQuickAccess.h"
#include "DkSettings.h"
#include "DkUtils.h"

#include <QAbstractItemView>
#include <QAction>
#include <QCheckBox>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGradientStops>
#include <QGraphicsOpacityEffect>
#include <QIcon>
#include <QLabel>
#include <QLinearGradient>
#include <QMainWindow>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QToolBar>

namespace nmc
{

// DkMainToolBar --------------------------------------------------------------------
DkMainToolBar::DkMainToolBar(const QString &title, QWidget *parent /* = 0 */)
    : QToolBar(title, parent)
{
    createLayout();
}

void DkMainToolBar::createLayout()
{
    mQuickAccessEdit = new DkQuickAccessEdit(this);
}

void DkMainToolBar::setQuickAccessModel(QStandardItemModel *model)
{
    mQuickAccessEdit->setModel(model);
    addWidget(mQuickAccessEdit);
    mQuickAccessEdit->setFocus(Qt::MouseFocusReason);
}

void DkMainToolBar::closeQuickAccess()
{
    mQuickAccessEdit->clearAccess();
}

void DkMainToolBar::allActionsAdded()
{
    // right align search filters
    auto *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    addWidget(spacer);
    // addWidget(quickFilterEdit);
}

DkQuickAccessEdit *DkMainToolBar::getQuickAccess() const
{
    return mQuickAccessEdit;
}

// DkColorSlider:
DkColorSlider::DkColorSlider(QWidget *parent, qreal normedPos, QColor color, int sliderWidth)
    : DkWidget(parent)
{
    this->setStatusTip(tr("Drag the slider downwards for elimination"));
    this->mNormedPos = normedPos;
    this->mColor = color;
    this->mSliderWidth = sliderWidth;
    mIsActive = false;

    mSliderHalfWidth = qCeil((float)sliderWidth / 2);
    // return (qreal)(pos) / (qreal)(width() - sliderWidth);

    int pos = qRound(normedPos * (parent->width() - sliderWidth - 1));

    setGeometry(pos, 23, sliderWidth + 1, sliderWidth + mSliderHalfWidth + 1);

    show();
}

void DkColorSlider::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.setPen(Qt::black);

    // Draw the filled triangle at the top of the slider:
    if (mIsActive) {
        QPainterPath path;
        path.moveTo(0, mSliderHalfWidth);
        path.lineTo(mSliderHalfWidth, 0);
        path.lineTo(mSliderHalfWidth, 0);
        path.lineTo(mSliderWidth, mSliderHalfWidth);

        painter.fillPath(path, Qt::black);
        painter.drawPath(path);

    }
    // Draw the empty triangle at the top of the slider:
    else {
        painter.drawLine(0, mSliderHalfWidth, mSliderHalfWidth, 0);
        painter.drawLine(mSliderHalfWidth, 0, mSliderWidth, mSliderHalfWidth);
    }

    painter.drawRect(0, mSliderHalfWidth, mSliderWidth, mSliderWidth);
    painter.fillRect(2, mSliderHalfWidth + 2, mSliderWidth - 3, mSliderWidth - 3, mColor);
}

void DkColorSlider::updatePos(int parentWidth)
{
    int pos = qRound(mNormedPos * (parentWidth - mSliderWidth - 1));
    setGeometry(pos, 23, mSliderWidth + 1, mSliderWidth + mSliderHalfWidth + 1);
}

void DkColorSlider::setActive(bool isActive)
{
    mIsActive = isActive;
}

DkColorSlider::~DkColorSlider() = default;

QColor DkColorSlider::getColor()
{
    return mColor;
}

qreal DkColorSlider::getNormedPos()
{
    return mNormedPos;
}

void DkColorSlider::setNormedPos(qreal pos)
{
    mNormedPos = pos;
}

void DkColorSlider::mousePressEvent(QMouseEvent *event)
{
    mIsActive = true;
    mDragStartX = event->pos().x();
    emit sliderActivated(this);
}

void DkColorSlider::mouseMoveEvent(QMouseEvent *event)
{
    // Pass the actual position to the Gradient:
    emit sliderMoved(this, event->pos().x() - mDragStartX, event->pos().y());
}

void DkColorSlider::mouseDoubleClickEvent(QMouseEvent *)
{
    QColor color = QColorDialog::getColor(this->mColor, this);
    if (color.isValid())
        this->mColor = color;

    emit colorChanged(this);
}

DkGradient::DkGradient(QWidget *parent)
    : DkWidget(parent)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

    this->setMinimumWidth(100);
    this->setMaximumWidth(600);

    this->setFixedHeight(40);

    mIsSliderDragged = false;
    mClickAreaHeight = 20;
    mDeleteSliderDist = 50;

    // Note that sliderWidth should be odd, in order to get a pretty rendered slider.
    mSliderWidth = 10;
    mHalfSliderWidth = mSliderWidth / 2;
    mGradient = QLinearGradient(0, 0, width(), height() - mClickAreaHeight);

    mSliders = QVector<DkColorSlider *>();
    init();
}

DkGradient::~DkGradient() = default;

void DkGradient::init()
{
    clearAllSliders();

    addSlider(0, Qt::black);
    addSlider(1, Qt::white);

    updateGradient();
}

void DkGradient::clearAllSliders()
{
    for (int i = 0; i < mSliders.size(); i++) {
        DkColorSlider *slider = mSliders.at(i);
        delete slider;
    }

    mSliders.clear();
}

void DkGradient::setGradient(const QLinearGradient &gradient)
{
    reset();
    clearAllSliders(); // reset adds a slider at the start and end

    this->mGradient.setStops(gradient.stops());

    QVector<QGradientStop> stops = gradient.stops();

    for (int idx = 0; idx < stops.size(); idx++) {
        addSlider(stops.at(idx).first, stops.at(idx).second);
    }

    updateGradient();
    update();
    emit gradientChanged();
}

QLinearGradient DkGradient::getGradient()
{
    return mGradient;
}

void DkGradient::reset()
{
    init();
    update();
}

void DkGradient::resizeEvent(QResizeEvent *event)
{
    if (event->size() == event->oldSize())
        return;

    DkColorSlider *slider;

    for (int i = 0; i < mSliders.size(); i++) {
        slider = mSliders.at(i);
        slider->updatePos(this->width());
    }

    // qDebug() << "resize gradient: " << event->size();

    updateGradient();

    QWidget::resizeEvent(event);
}

void DkGradient::addSlider(qreal pos, QColor color)
{
    auto *actSlider = new DkColorSlider(this, pos, color, mSliderWidth);
    mSliders.append(actSlider);
    connect(actSlider, &DkColorSlider::sliderMoved, this, &DkGradient::moveSlider);
    connect(actSlider, &DkColorSlider::colorChanged, this, &DkGradient::changeColor);
    connect(actSlider, &DkColorSlider::sliderActivated, this, &DkGradient::activateSlider);
}

void DkGradient::insertSlider(qreal pos, QColor col)
{
    // Inserts a new slider at position pos and calculates the color, interpolated from the closest neighbors.

    // Find the neighbors of the new slider, since we need it for the color interpolation:
    QColor actColor;
    qreal dist;
    qreal initValue = DBL_MAX; // std::numeric_limits<qreal>::max();	// >DIR: fix for linux [9.2.2012 markus]
    qreal leftDist = initValue;
    qreal rightDist = initValue;

    int leftIdx = 0, rightIdx = 0;

    for (int i = 0; i < mSliders.size(); i++) {
        dist = mSliders.at(i)->getNormedPos() - pos;
        if (dist < 0) {
            if (std::abs(dist) < leftDist) {
                leftDist = (std::abs(dist));
                leftIdx = i;
            }
        } else if (dist > 0) {
            if (std::abs(dist) < rightDist) {
                rightDist = (std::abs(dist));
                rightIdx = i;
            }
        } else {
            actColor = mSliders.at(i)->getColor();
            break;
        }
    }

    if ((leftDist == initValue) && (rightDist == initValue))
        actColor = Qt::black;
    // The slider is most left:
    else if (leftDist == initValue)
        actColor = mSliders.at(rightIdx)->getColor();
    // The slider is most right:
    else if (rightDist == initValue)
        actColor = mSliders.at(leftIdx)->getColor();
    // The slider has a neighbor to the left and to the right:
    else {
        int rLeft, rRight, rNew, gLeft, gRight, gNew, bLeft, bRight, bNew;

        mSliders.at(leftIdx)->getColor().getRgb(&rLeft, &gLeft, &bLeft);
        mSliders.at(rightIdx)->getColor().getRgb(&rRight, &gRight, &bRight);

        qreal fac = leftDist / (leftDist + rightDist);
        rNew = qRound(rLeft * (1 - fac) + rRight * fac);
        gNew = qRound(gLeft * (1 - fac) + gRight * fac);
        bNew = qRound(bLeft * (1 - fac) + bRight * fac);

        actColor = QColor(rNew, gNew, bNew);
    }

    addSlider(pos, col.isValid() ? col : actColor);
    // The last slider in the list is the last one added, now make this one active:
    activateSlider(mSliders.last());

    updateGradient();
    update();
}

void DkGradient::mousePressEvent(QMouseEvent *event)
{
    QPointF enterPos = event->pos();
    qreal pos = (qreal)(enterPos.x() - mHalfSliderWidth) / (qreal)(width() - mSliderWidth);

    insertSlider(pos);
}

void DkGradient::updateGradient()
{
    mGradient = QLinearGradient(0, 0, width(), height() - mClickAreaHeight);

    for (int i = 0; i < mSliders.size(); i++)
        mGradient.setColorAt(mSliders.at(i)->getNormedPos(), mSliders.at(i)->getColor());
}

QGradientStops DkGradient::getGradientStops()
{
    return mGradient.stops();
}

void DkGradient::moveSlider(DkColorSlider *sender, int dragDistX, int yPos)
{
    // Delete the actual slider:
    if (yPos > mDeleteSliderDist) {
        int idx = mSliders.lastIndexOf(sender);
        if (idx != -1) {
            mSliders.remove(idx);
            delete sender;
            mIsActiveSliderExisting = false;
        }
    }

    // Insert a new slider:
    else {
        int newPos = sender->pos().x() + dragDistX;

        if (newPos < 0)
            newPos = 0;
        else if (newPos > width() - mSliderWidth - 1)
            newPos = width() - mSliderWidth - 1;

        qreal normedSliderPos = getNormedPos(newPos);

        if (normedSliderPos > 1)
            normedSliderPos = 1;
        if (normedSliderPos < 0)
            normedSliderPos = 0;

        DkColorSlider *slider;
        // Check if the position is already assigned to another slider:
        for (int i = 0; i < mSliders.size(); i++) {
            slider = mSliders.at(i);
            if (slider != sender) {
                if (slider->getNormedPos() == normedSliderPos)
                    return;
            }
        }

        sender->setNormedPos(normedSliderPos);
        sender->move(newPos, sender->pos().y());
    }

    updateGradient();
    update();

    emit gradientChanged();
}

qreal DkGradient::getNormedPos(int pos)
{
    return (qreal)(pos) / (qreal)(width() - mSliderWidth);
}

int DkGradient::getAbsolutePos(qreal pos)
{
    return (int)pos * width();
}

void DkGradient::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::gray);

    painter.fillRect(mHalfSliderWidth, 2, width() - mSliderWidth, height() - mClickAreaHeight, mGradient);
    painter.drawRect(mHalfSliderWidth, 2, width() - mSliderWidth, height() - mClickAreaHeight);
}

void DkGradient::mouseReleaseEvent(QMouseEvent *)
{
    // unused
}

void DkGradient::changeColor(DkColorSlider *)
{
    updateGradient();
    update();

    emit gradientChanged();
}

void DkGradient::activateSlider(DkColorSlider *sender)
{
    if (mIsActiveSliderExisting)
        mActiveSlider->setActive(false);
    else
        mIsActiveSliderExisting = true;

    mActiveSlider = sender;
    mActiveSlider->setActive(true);

    update();
}

//
DkTransferToolBar::DkTransferToolBar(QWidget *parent)
    : QToolBar(tr("Pseudo Color Toolbar"), parent)
{
    loadSettings();

    mEnableTFCheckBox = new QCheckBox(tr("Enable"));
    mEnableTFCheckBox->setStatusTip(tr("Enables the Pseudo Color function"));

    this->addWidget(mEnableTFCheckBox);

    // >DIR: more compact gui [2.3.2012 markus]
    this->addSeparator();
    // this->addWidget(new QLabel(tr("Active channel:")));

    mChannelComboBox = new QComboBox(this);
    mChannelComboBox->setStatusTip(tr("Changes the displayed color channel"));
    this->addWidget(mChannelComboBox);

    mHistoryCombo = new QComboBox(this);

    auto *delGradientAction = new QAction(tr("Delete"), mHistoryCombo);
    connect(delGradientAction, &QAction::triggered, this, &DkTransferToolBar::deleteGradient);

    mHistoryCombo->addAction(delGradientAction);
    mHistoryCombo->setContextMenuPolicy(Qt::ActionsContextMenu);

    updateGradientHistory();
    connect(mHistoryCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DkTransferToolBar::switchGradient);
    connect(mHistoryCombo, &QComboBox::customContextMenuRequested, this, &DkTransferToolBar::deleteGradientMenu);

    this->addWidget(mHistoryCombo);

    createIcons();

    mGradient = new DkGradient(this);
    mGradient->setStatusTip(tr("Click into the field for a new slider"));
    this->addWidget(mGradient);

    mEffect = new QGraphicsOpacityEffect(mGradient);
    mEffect->setOpacity(1);
    mGradient->setGraphicsEffect(mEffect);

    // Disable the entire transfer toolbar:
    // enableTF(Qt::Unchecked);

    // Initialize the combo box for color images:
    mImageMode = mode_uninitialized;
    applyImageMode(mode_rgb);

    enableToolBar(false);
    mEnableTFCheckBox->setEnabled(true);

#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    connect(mEnableTFCheckBox, &QCheckBox::checkStateChanged, this, &DkTransferToolBar::enableTFCheckBoxClicked);
#else
    connect(mEnableTFCheckBox, &QCheckBox::stateChanged, this, &DkTransferToolBar::enableTFCheckBoxClicked);
#endif

    connect(mGradient, &DkGradient::gradientChanged, this, &DkTransferToolBar::applyTF);

    // needed for initialization
    connect(this, &DkTransferToolBar::gradientChanged, mGradient, &DkGradient::gradientChanged);

    if (!mOldGradients.empty())
        mGradient->setGradient(mOldGradients.first());
}

DkTransferToolBar::~DkTransferToolBar() = default;

void DkTransferToolBar::createIcons()
{
    // user needs to decide...
    // this->setIconSize(QSize(DkSettingsManager::param().effectiveIconSize(),
    // DkSettingsManager::param().effectiveIconSize()));

    mToolBarIcons.resize(icon_toolbar_end);

    mToolBarIcons[icon_toolbar_reset] = DkImage::loadIcon(":/nomacs/img/gradient-reset.svg");
    mToolBarIcons[icon_toolbar_pipette] = DkImage::loadIcon(":/nomacs/img/pipette.svg");
    mToolBarIcons[icon_toolbar_save] = DkImage::loadIcon(":/nomacs/img/save.svg");

    mToolBarActions.resize(toolbar_end);
    mToolBarActions[toolbar_reset] = new QAction(mToolBarIcons[icon_toolbar_reset], tr("Reset"), this);
    mToolBarActions[toolbar_reset]->setStatusTip(tr("Resets the Pseudo Color function"));
    connect(mToolBarActions[toolbar_reset], &QAction::triggered, this, &DkTransferToolBar::resetGradient);

    // toolBarActions[toolbar_reset]->setToolTip("was geht?");

    mToolBarActions[toolbar_pipette] = new QAction(mToolBarIcons[icon_toolbar_pipette], tr("Select Color"), this);
    mToolBarActions[toolbar_pipette]->setStatusTip(tr("Adds a slider at the selected color value"));
    mToolBarActions[toolbar_pipette]->setCheckable(true);
    mToolBarActions[toolbar_pipette]->setChecked(false);
    connect(mToolBarActions[toolbar_pipette], &QAction::triggered, this, &DkTransferToolBar::pickColor);

    mToolBarActions[toolbar_save] = new QAction(mToolBarIcons[icon_toolbar_save], tr("Save Gradient"), this);
    mToolBarActions[toolbar_save]->setStatusTip(tr("Saves the current Gradient"));
    connect(mToolBarActions[toolbar_save], &QAction::triggered, this, &DkTransferToolBar::saveGradient);

    addActions(mToolBarActions.toList());
}

void DkTransferToolBar::saveSettings()
{
    DefaultSettings settings;
    settings.beginGroup("Pseudo Color");

    settings.beginWriteArray("oldGradients", mOldGradients.size());

    for (int idx = 0; idx < mOldGradients.size(); idx++) {
        settings.setArrayIndex(idx);

        QVector<QGradientStop> stops = mOldGradients.at(idx).stops();
        settings.beginWriteArray("gradient", stops.size());

        for (int sIdx = 0; sIdx < stops.size(); sIdx++) {
            settings.setArrayIndex(sIdx);
            settings.setValue("posRGBA", (float)stops.at(sIdx).first);
            settings.setValue("colorRGBA", stops.at(sIdx).second.rgba());
        }
        settings.endArray();
    }

    settings.endArray();
    settings.endGroup();
}

void DkTransferToolBar::loadSettings()
{
    DefaultSettings settings;
    settings.beginGroup("Pseudo Color");

    int gSize = settings.beginReadArray("oldGradients");

    for (int idx = 0; idx < gSize; idx++) {
        settings.setArrayIndex(idx);

        QVector<QGradientStop> stops;
        int sSize = settings.beginReadArray("gradient");

        for (int sIdx = 0; sIdx < sSize; sIdx++) {
            settings.setArrayIndex(sIdx);

            QGradientStop s;
            s.first = settings.value("posRGBA", 0).toFloat();
            s.second = QColor::fromRgba(settings.value("colorRGBA", QColor().rgba()).toInt());
            qDebug() << "pos: " << s.first << " col: " << s.second;
            stops.append(s);
        }
        settings.endArray();

        QLinearGradient g;
        g.setStops(stops);
        mOldGradients.append(g);
    }

    settings.endArray();
    settings.endGroup();
}

void DkTransferToolBar::deleteGradientMenu(QPoint pos)
{
    auto *cm = new QMenu(this);
    auto *delAction = new QAction("Delete", this);
    connect(delAction, &QAction::triggered, this, &DkTransferToolBar::deleteGradient);
    cm->popup(mHistoryCombo->mapToGlobal(pos));
    cm->exec();
}

void DkTransferToolBar::deleteGradient()
{
    int idx = mHistoryCombo->currentIndex();

    if (idx >= 0 && idx < mOldGradients.size()) {
        mOldGradients.remove(idx);
        mHistoryCombo->removeItem(idx);
    }
}

void DkTransferToolBar::resizeEvent(QResizeEvent *event)
{
    mGradient->resize(event->size().width() - mGradient->x(), 40);
}

void DkTransferToolBar::insertSlider(qreal pos)
{
    mGradient->insertSlider(pos);
}

void DkTransferToolBar::setImageMode(int mode)
{
    qDebug() << "and I received...";
    applyImageMode(mode);
}

void DkTransferToolBar::applyImageMode(int mode)
{
    if (mode == mImageMode)
        return;

    mImageMode = mode;

    bool enabled = mImageMode != mode_invalid_format;
    enableToolBar(enabled);

    if (!enabled)
        return;

    disconnect(mChannelComboBox,
               QOverload<int>::of(&QComboBox::currentIndexChanged),
               this,
               &DkTransferToolBar::changeChannel);
    mChannelComboBox->clear();

    if (mode == mode_gray) {
        mChannelComboBox->addItem(tr("Gray"));
    } else if (mode == mode_rgb) {
        mChannelComboBox->addItem(tr("RGB"));
        mChannelComboBox->addItem(tr("Red"));
        mChannelComboBox->addItem(tr("Green"));
        mChannelComboBox->addItem(tr("Blue"));
    }

    mChannelComboBox->setCurrentIndex(0);

    connect(mChannelComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DkTransferToolBar::changeChannel);
}

void DkTransferToolBar::pickColor(bool enabled)
{
    emit pickColorRequest(enabled);
}

void DkTransferToolBar::enableTFCheckBoxClicked(int state)
{
    bool enabled;
    if (state == Qt::Checked)
        enabled = true;
    else
        enabled = false;

    enableToolBar(enabled);

    // At this point the checkbox is disabled, hence enable it...
    mEnableTFCheckBox->setEnabled(true);

    if (enabled)
        mEnableTFCheckBox->setStatusTip(tr("Disables the Pseudo Color function"));
    else
        mEnableTFCheckBox->setStatusTip(tr("Enables the Pseudo Color function"));

    emit tFEnabled(enabled);
    emit gradientChanged();
}

void DkTransferToolBar::enableToolBar(bool enable)
{
    mEnableTFCheckBox->setEnabled(enable);

    QObjectList list = this->children();

    for (int i = 0; i < list.count(); i++) {
        if (QWidget *action = qobject_cast<QWidget *>(list.at(i)))
            action->setEnabled(enable);
    }

    if (enable)
        mEffect->setOpacity(1);
    else
        mEffect->setOpacity(.5);
}

void DkTransferToolBar::applyTF()
{
    QGradientStops stops = mGradient->getGradientStops();

    emit colorTableChanged(stops);
}

void DkTransferToolBar::changeChannel(int index)
{
    emit channelChanged(index);
}

void DkTransferToolBar::resetGradient()
{
    mGradient->reset();

    QGradientStops stops = mGradient->getGradientStops();

    emit colorTableChanged(stops);
}

void DkTransferToolBar::paintEvent(QPaintEvent *event)
{
    QToolBar::paintEvent(event);
}

void DkTransferToolBar::pickColorCancelled()
{
    QAction *action = mToolBarActions[toolbar_pipette];
    action->blockSignals(true);
    action->setChecked(false);
    action->blockSignals(false);
}

void DkTransferToolBar::updateGradientHistory()
{
    mHistoryCombo->clear();
    mHistoryCombo->setIconSize(QSize(50, 10));

    for (int idx = 0; idx < mOldGradients.size(); idx++) {
        QPixmap cg(50, 10);
        QLinearGradient g(QPoint(0, 0), QPoint(50, 0));
        g.setStops(mOldGradients[idx].stops());
        QPainter p(&cg);
        p.fillRect(cg.rect(), g);
        mHistoryCombo->addItem(cg, tr(""));
    }
}

void DkTransferToolBar::switchGradient(int idx)
{
    if (idx >= 0 && idx < mOldGradients.size()) {
        mGradient->setGradient(mOldGradients[idx]);
    }
}

void DkTransferToolBar::saveGradient()
{
    mOldGradients.prepend(mGradient->getGradient());
    updateGradientHistory();
    saveSettings();
}

// DkCropToolbar --------------------------------------------------------------------
DkCropToolBar::DkCropToolBar(const QString &title, QWidget *parent /* = 0 */)
    : QToolBar(title, parent)
{
    createIcons();
    createLayout();

    setIconSize(
        QSize(DkSettingsManager::param().effectiveIconSize(this), DkSettingsManager::param().effectiveIconSize(this)));
    setObjectName("cropToolBar");
}

DkCropToolBar::~DkCropToolBar()
{
    // save settings
    saveSettings();
}

void DkCropToolBar::loadSettings()
{
    DefaultSettings settings;
    settings.beginGroup("Crop");

    mHorValBox->setValue(settings.value("AspectRatioHorizontal", 0).toInt());
    mVerValBox->setValue(settings.value("AspectRatioVertical", 0).toInt());
    mGuideBox->setCurrentIndex(settings.value("guides", 1).toInt());
    mInvertAction->setChecked(settings.value("inverted", false).toBool());
    mInfoAction->setChecked(settings.value("info", true).toBool());
    mCbMeta->setChecked(settings.value("cropToMetadata", false).toBool());
    settings.endGroup();
}

void DkCropToolBar::saveSettings()
{
    DefaultSettings settings;
    settings.beginGroup("Crop");

    settings.setValue("AspectRatioHorizontal", mHorValBox->value());
    settings.setValue("AspectRatioVertical", mVerValBox->value());
    settings.setValue("guides", mGuideBox->currentIndex());
    settings.setValue("inverted", mInvertAction->isChecked());
    settings.setValue("info", mInfoAction->isChecked());
    settings.setValue("cropToMetadata", mCbMeta->isChecked());
    settings.endGroup();
}

void DkCropToolBar::createIcons()
{
    // create icons
    mIcons.resize(icons_end);

    mIcons[crop_icon] = DkImage::loadIcon(":/nomacs/img/crop.svg");
    mIcons[cancel_icon] = DkImage::loadIcon(":/nomacs/img/close.svg");
    mIcons[pan_icon] = DkImage::loadIcon(":/nomacs/img/pan.svg");
    mIcons[pan_icon].addFile(":/nomacs/img/pan-checked.svg", QSize(), QIcon::Normal, QIcon::On);
    mIcons[invert_icon] = DkImage::loadIcon(":/nomacs/img/crop-invert.svg");
    mIcons[invert_icon].addFile(":/nomacs/img/crop-invert-checked.svg", QSize(), QIcon::Normal, QIcon::On);
    mIcons[info_icon] = DkImage::loadIcon(":/nomacs/img/info.svg");
}

void DkCropToolBar::createLayout()
{
    QList<QKeySequence> enterSc;
    enterSc.append(QKeySequence(Qt::Key_Enter));
    enterSc.append(QKeySequence(Qt::Key_Return));

    auto *cropAction = new QAction(mIcons[crop_icon], tr("Crop (ENTER)"), this);
    cropAction->setShortcuts(enterSc);
    connect(cropAction, &QAction::triggered, this, &DkCropToolBar::onCropActionTriggered);

    auto *cancelAction = new QAction(mIcons[cancel_icon], tr("Cancel (ESC)"), this);
    cancelAction->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(cancelAction, &QAction::triggered, this, &DkCropToolBar::onCancelActionTriggered);

    mPanAction = new QAction(mIcons[pan_icon], tr("Pan"), this);
    mPanAction->setShortcut(QKeySequence(Qt::Key_P));
    mPanAction->setCheckable(true);
    mPanAction->setChecked(false);
    connect(mPanAction, &QAction::toggled, this, &DkCropToolBar::onPanActionToggled);

    QStringList ratios;
    ratios << "1:1" << "4:3" << "5:4" << "14:10" << "14:11" << "16:9" << "16:10";
    ratios.prepend(tr("User Defined"));
    ratios.prepend(tr("No Aspect Ratio"));
    mRatioBox = new QComboBox(this);
    mRatioBox->addItems(ratios);
    connect(mRatioBox, &QComboBox::currentTextChanged, this, &DkCropToolBar::onRatioBoxCurrentIndexChanged);

    mHorValBox = new QDoubleSpinBox(this);
    mHorValBox->setSpecialValueText("  ");
    mHorValBox->setToolTip(tr("Horizontal Constraint"));
    mHorValBox->setStatusTip(mHorValBox->toolTip());
    connect(mHorValBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DkCropToolBar::onHorValBoxValueChanged);

    auto *swapAction = new QAction(DkImage::loadIcon(":/nomacs/img/swap.svg"), tr("Swap"), this);
    swapAction->setToolTip(tr("Swap Dimensions"));
    swapAction->setStatusTip(swapAction->toolTip());
    connect(swapAction, &QAction::triggered, this, &DkCropToolBar::onSwapActionTriggered);

    mVerValBox = new QDoubleSpinBox(this);
    mVerValBox->setSpecialValueText("  ");
    mVerValBox->setToolTip(tr("Vertical Constraint"));
    mVerValBox->setStatusTip(mVerValBox->toolTip());
    connect(mVerValBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DkCropToolBar::onVerValBoxValueChanged);

    mAngleBox = new QDoubleSpinBox(this);
    mAngleBox->setSuffix(dk_degree_str);
    mAngleBox->setMinimum(-180);
    mAngleBox->setMaximum(180);
    connect(mAngleBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DkCropToolBar::onAngleBoxValueChanged);

    // background color
    mBgCol = QColor(0, 0, 0, 0);
    mBgColButton = new QPushButton(this);
    mBgColButton->setStyleSheet("QPushButton {background-color: " + DkUtils::colorToString(mBgCol)
                                + "; border: 1px solid #888;}");
    mBgColButton->setToolTip(tr("Background Color"));
    mBgColButton->setStatusTip(mBgColButton->toolTip());
    connect(mBgColButton, &QPushButton::clicked, this, &DkCropToolBar::onBgColButtonClicked);

    mColorDialog = new QColorDialog(this);
    mColorDialog->setObjectName("colorDialog");
    mColorDialog->setOption(QColorDialog::ShowAlphaChannel, true);

    // crop customization
    QStringList guides;
    guides << tr("Guides") << tr("Rule of Thirds") << tr("Grid");
    mGuideBox = new QComboBox(this);
    mGuideBox->addItems(guides);
    mGuideBox->setToolTip(tr("Show Guides in the Preview"));
    mGuideBox->setStatusTip(mGuideBox->toolTip());
    connect(mGuideBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DkCropToolBar::onGuideBoxCurrentIndexChanged);

    mInvertAction = new QAction(mIcons[invert_icon], tr("Invert Crop Tool Color"), this);
    mInvertAction->setCheckable(true);
    mInvertAction->setChecked(false);
    connect(mInvertAction, &QAction::toggled, this, &DkCropToolBar::onInvertActionToggled);

    mInfoAction = new QAction(mIcons[info_icon], tr("Show Info"), this);
    mInfoAction->setCheckable(true);
    mInfoAction->setChecked(false);
    connect(mInfoAction, &QAction::toggled, this, &DkCropToolBar::onInfoActionToggled);

    mCbMeta = new QCheckBox(tr("Crop to Metadata"), this);
    mCbMeta->setChecked(false);
    mCbMeta->hide(); // "disabled" for now (keep it simple)

    mCropRect = new DkRectWidget(QRect(), this);
    mCropRect->setObjectName("cropRect");

    addAction(cropAction);
    addAction(mPanAction);
    addAction(cancelAction);
    addSeparator();
    addWidget(mRatioBox);
    addWidget(mHorValBox);
    addAction(swapAction);
    addWidget(mVerValBox);
    addWidget(mAngleBox);
    addSeparator();
    addWidget(mBgColButton);
    addSeparator();
    addWidget(mGuideBox);
    addAction(mInvertAction);
    addAction(mInfoAction);
    addWidget(mCbMeta);
    addSeparator();
    addWidget(mCropRect);

    connect(mCropRect, &DkRectWidget::updateRectSignal, this, &DkCropToolBar::updateRectSignal);
}

void DkCropToolBar::setVisible(bool visible)
{
    if (!visible)
        emit colorSignal(Qt::NoBrush);
    else
        emit colorSignal(mBgCol);

    if (visible) {
        mPanAction->setChecked(false);
        mAngleBox->setValue(0);
    }

    QToolBar::setVisible(visible);
}

void DkCropToolBar::setAspectRatio(const QPointF &aRatio)
{
    mHorValBox->setValue(aRatio.x());
    mVerValBox->setValue(aRatio.y());
}

void DkCropToolBar::setRect(const QRect &r)
{
    mCropRect->setRect(r);
}

void DkCropToolBar::onCropActionTriggered()
{
    emit cropSignal(mCbMeta->isChecked());
}

void DkCropToolBar::onCancelActionTriggered()
{
    emit cancelSignal();
}

void DkCropToolBar::onInvertActionToggled(bool checked)
{
    emit shadingHint(checked);
}

void DkCropToolBar::onInfoActionToggled(bool checked)
{
    emit showInfo(checked);
}

void DkCropToolBar::onSwapActionTriggered()
{
    int tmpV = qRound(mHorValBox->value());
    mHorValBox->setValue(mVerValBox->value());
    mVerValBox->setValue(tmpV);
}

void DkCropToolBar::onAngleBoxValueChanged(double val)
{
    emit angleSignal(DK_DEG2RAD * val);
}

void DkCropToolBar::angleChanged(double val)
{
    double angle = val * DK_RAD2DEG;
    while (angle > 90)
        angle -= 180;
    while (angle <= -90)
        angle += 180;

    mAngleBox->blockSignals(true);
    mAngleBox->setValue(angle);
    mAngleBox->blockSignals(false);
}

void DkCropToolBar::onBgColButtonClicked()
{
    QColor tmpCol = mBgCol;
    if (!tmpCol.alpha())
        tmpCol.setAlpha(255); // avoid frustrated users

    mColorDialog->setCurrentColor(tmpCol);
    int ok = mColorDialog->exec();

    if (ok == QDialog::Accepted) {
        mBgCol = mColorDialog->currentColor();
        mBgColButton->setStyleSheet("QPushButton {background-color: " + DkUtils::colorToString(mBgCol)
                                    + "; border: 1px solid #888;}");
        emit colorSignal(mBgCol);
    }
}

void DkCropToolBar::onRatioBoxCurrentIndexChanged(const QString &text)
{
    // user defined -> do nothing
    if (mRatioBox->currentIndex() == 1)
        return;

    // no aspect ratio -> clear boxes
    if (mRatioBox->currentIndex() == 0) {
        mHorValBox->setValue(0);
        mVerValBox->setValue(0);
        return;
    }

    QStringList vals = text.split(":");

    qDebug() << vals;

    if (vals.size() == 2) {
        mHorValBox->setValue(vals[0].toDouble());
        mVerValBox->setValue(vals[1].toDouble());
    }
}

void DkCropToolBar::onGuideBoxCurrentIndexChanged(int idx)
{
    emit paintHint(idx);
}

void DkCropToolBar::onVerValBoxValueChanged(double val)
{
    // just pass it on
    onHorValBoxValueChanged(val);
}

void DkCropToolBar::onHorValBoxValueChanged(double)
{
    DkVector diag = DkVector((float)mHorValBox->value(), (float)mVerValBox->value());
    emit aspectRatio(diag);

    QString rs = QString::number(mHorValBox->value()) + ":" + QString::number(mVerValBox->value());

    int idx = mRatioBox->findText(rs);

    if (idx != -1)
        mRatioBox->setCurrentIndex(idx);
    else if (mHorValBox->value() == 0 && mVerValBox->value() == 0)
        mRatioBox->setCurrentIndex(0);
    else
        mRatioBox->setCurrentIndex(1);
}

void DkCropToolBar::onPanActionToggled(bool checked)
{
    emit panSignal(checked);
}

// -------------------------------------------------------------------- DkToolBarManager
DkToolBarManager::DkToolBarManager() = default;

DkToolBarManager &DkToolBarManager::inst()
{
    static DkToolBarManager inst;
    return inst;
}

void DkToolBarManager::createDefaultToolBar()
{
    if (mToolBar)
        return;

    auto nomacs = dynamic_cast<QMainWindow *>(DkUtils::getMainWindow());
    Q_ASSERT(nomacs);

    mToolBar = new DkMainToolBar(QObject::tr("Toolbar"), nomacs);
    mToolBar->setObjectName("EditToolBar");

    int is = DkSettingsManager::param().effectiveIconSize(nomacs);
    mToolBar->setIconSize(QSize(is, is));

    // add actions
    DkActionManager &am = DkActionManager::instance();
    mToolBar->addAction(am.action(DkActionManager::menu_file_prev));
    mToolBar->addAction(am.action(DkActionManager::menu_file_next));
    mToolBar->addSeparator();

    mToolBar->addAction(am.action(DkActionManager::menu_file_open));
    mToolBar->addAction(am.action(DkActionManager::menu_file_open_dir));
    mToolBar->addAction(am.action(DkActionManager::menu_file_save));
    mToolBar->addAction(am.action(DkActionManager::menu_edit_delete));
    mToolBar->addAction(am.action(DkActionManager::menu_tools_filter));
    mToolBar->addSeparator();

    // view
    mToolBar->addAction(am.action(DkActionManager::menu_view_zoom_in));
    mToolBar->addAction(am.action(DkActionManager::menu_view_zoom_out));
    mToolBar->addSeparator();

    // edit
    mToolBar->addAction(am.action(DkActionManager::menu_edit_copy));
    mToolBar->addAction(am.action(DkActionManager::menu_edit_paste));
    mToolBar->addSeparator();
    mToolBar->addAction(am.action(DkActionManager::menu_edit_rotate_ccw));
    mToolBar->addAction(am.action(DkActionManager::menu_edit_rotate_cw));
    mToolBar->addSeparator();
    mToolBar->addAction(am.action(DkActionManager::menu_edit_crop));
    mToolBar->addAction(am.action(DkActionManager::menu_edit_transform));
    mToolBar->addSeparator();
    mToolBar->addAction(am.action(DkActionManager::menu_edit_undo));
    mToolBar->addAction(am.action(DkActionManager::menu_edit_redo));
    mToolBar->addSeparator();

    // view
    mToolBar->addAction(am.action(DkActionManager::menu_view_fullscreen));
    mToolBar->addAction(am.action(DkActionManager::menu_view_reset));
    mToolBar->addAction(am.action(DkActionManager::menu_view_100));
    mToolBar->addSeparator();

    mToolBar->addAction(am.action(DkActionManager::menu_view_gps_map));
    mToolBar->allActionsAdded();

    mMovieToolBar = nomacs->addToolBar(QObject::tr("Movie ToolBar"));
    mMovieToolBar->setObjectName("movieToolbar");
    mMovieToolBar->setIconSize(QSize(is, is));
    mMovieToolBar->addAction(am.action(DkActionManager::menu_view_movie_prev));
    mMovieToolBar->addAction(am.action(DkActionManager::menu_view_movie_pause));
    mMovieToolBar->addAction(am.action(DkActionManager::menu_view_movie_next));

    nomacs->addToolBar(mToolBar);
}

void DkToolBarManager::show(bool show, bool permanent)
{
    showDefaultToolBar(show, permanent);
    showMovieToolBar(show);
    showToolBarsTemporarily(show);
}

void DkToolBarManager::restore()
{
    if (mToolBar)
        mToolBar->setVisible(DkSettingsManager::param().app().showToolBar);
    if (mMovieToolBar)
        mMovieToolBar->setVisible(DkSettingsManager::param().app().showMovieToolBar);
    if (mTransferToolBar)
        mTransferToolBar->setVisible(true); // always visible when pseudocolor is enabled
}

void DkToolBarManager::showToolBar(QToolBar *toolbar, bool show)
{
    if (!toolbar)
        return;

    showToolBarsTemporarily(!show);
    auto *nomacs = dynamic_cast<QMainWindow *>(DkUtils::getMainWindow());
    Q_ASSERT(nomacs);

    if (show) {
        if (!mToolBar)
            createDefaultToolBar();

        nomacs->addToolBar(nomacs->toolBarArea(mToolBar), toolbar);
    } else
        nomacs->removeToolBar(toolbar);

    toolbar->setVisible(show);
}

void DkToolBarManager::showToolBarsTemporarily(bool show)
{
    if (show) {
        for (QToolBar *t : mHiddenToolBars)
            t->show();
    } else {
        auto *nomacs = dynamic_cast<QMainWindow *>(DkUtils::getMainWindow());
        Q_ASSERT(nomacs);

        mHiddenToolBars.clear();
        QList<QToolBar *> tbs = nomacs->findChildren<QToolBar *>();

        for (QToolBar *t : tbs) {
            if (t->isVisible()) {
                t->hide();
                mHiddenToolBars.append(t);
            }
        }
    }
}

void DkToolBarManager::showDefaultToolBar(bool show, bool permanent)
{
    if (!show && !mToolBar)
        return;

    if (!mToolBar)
        createDefaultToolBar();

    if (mToolBar->isVisible() == show)
        return;

    if (permanent)
        DkSettingsManager::param().app().showToolBar = show;
    DkActionManager::instance()
        .action(DkActionManager::menu_panel_toolbar)
        ->setChecked(DkSettingsManager::param().app().showToolBar);

    mToolBar->setVisible(show);
}

void DkToolBarManager::showMovieToolBar(bool show)
{
    auto *nomacs = dynamic_cast<QMainWindow *>(DkUtils::getMainWindow());
    Q_ASSERT(nomacs);

    // set movie toolbar into current toolbar
    if (mMovieToolbarArea == Qt::NoToolBarArea && show)
        mMovieToolbarArea = nomacs->toolBarArea(mToolBar);

    if (show)
        nomacs->addToolBar(mMovieToolbarArea, mMovieToolBar);
    else {
        // remember if the user changed it
        Qt::ToolBarArea nta = nomacs->toolBarArea(mMovieToolBar);

        if (nta != Qt::NoToolBarArea && mMovieToolBar->isVisible())
            mMovieToolbarArea = nomacs->toolBarArea(mMovieToolBar);
        nomacs->removeToolBar(mMovieToolBar);
    }

    if (mToolBar && mToolBar->isVisible())
        mMovieToolBar->setVisible(show);
}

void DkToolBarManager::createTransferToolBar()
{
    auto *nomacs = dynamic_cast<QMainWindow *>(DkUtils::getMainWindow());
    Q_ASSERT(nomacs);

    mTransferToolBar = new DkTransferToolBar(nomacs);

    // add this toolbar below all previous toolbars
    nomacs->addToolBarBreak();
    nomacs->addToolBar(mTransferToolBar);
    mTransferToolBar->setObjectName("TransferToolBar");

    int is = DkSettingsManager::param().effectiveIconSize(nomacs);
    mTransferToolBar->setIconSize(QSize(is, is));
}

DkMainToolBar *DkToolBarManager::defaultToolBar() const
{
    return mToolBar;
}

DkTransferToolBar *DkToolBarManager::transferToolBar() const
{
    return mTransferToolBar;
}

}
