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

#include "DkBasicWidgets.h"
#include "DkImageStorage.h"
#include "DkSettings.h"
#include "DkUtils.h"

#include <cassert>

#pragma warning(push, 0) // no warnings from includes
#include <QColorDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidgetAction>
#pragma warning(pop)

namespace nmc
{

// DkSlider --------------------------------------------------------------------
DkSlider::DkSlider(QString title, QWidget *parent)
    : DkWidget(parent)
{
    createLayout();

    // init
    titleLabel->setText(title);

    // defaults
    setMinimum(0);
    setMaximum(100);
    setTickInterval(1);
    setValue(50);
}

void DkSlider::createLayout()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    QWidget *dummy = new QWidget(this);
    QHBoxLayout *titleLayout = new QHBoxLayout(dummy);
    titleLayout->setContentsMargins(0, 0, 0, 5);

    QWidget *dummyBounds = new QWidget(this);
    QHBoxLayout *boundsLayout = new QHBoxLayout(dummyBounds);
    boundsLayout->setContentsMargins(0, 0, 0, 0);

    titleLabel = new QLabel(this);

    sliderBox = new QSpinBox(this);

    slider = new QSlider(this);
    slider->setOrientation(Qt::Horizontal);

    minValLabel = new QLabel(this);
    maxValLabel = new QLabel(this);

    // hide min-max for now - delete in future releases...
    minValLabel->hide();
    maxValLabel->hide();

    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(sliderBox);

    boundsLayout->addWidget(minValLabel);
    boundsLayout->addStretch();
    boundsLayout->addWidget(maxValLabel);

    layout->addWidget(dummy);
    layout->addWidget(slider);
    layout->addWidget(dummyBounds);

    // connect
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
    connect(sliderBox, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
}

QSlider *DkSlider::getSlider() const
{
    return slider;
}

void DkSlider::setMinimum(int minValue)
{
    slider->setMinimum(minValue);
    sliderBox->setMinimum(minValue);
    minValLabel->setText(QString::number(minValue));
}

void DkSlider::setMaximum(int maxValue)
{
    slider->setMaximum(maxValue);
    sliderBox->setMaximum(maxValue);
    maxValLabel->setText(QString::number(maxValue));
}

void DkSlider::setTickInterval(int ticValue)
{
    slider->setTickInterval(ticValue);
}

int DkSlider::value() const
{
    return slider->value();
}

void DkSlider::setFocus(Qt::FocusReason reason)
{
    sliderBox->setFocus(reason);
}

void DkSlider::setValue(int value)
{
    slider->blockSignals(true);
    slider->setValue(value);
    slider->blockSignals(false);

    sliderBox->blockSignals(true);
    sliderBox->setValue(value);
    sliderBox->blockSignals(false);

    emit valueChanged(value);
}

// DkDoubleSlider --------------------------------------------------------------------
DkDoubleSlider::DkDoubleSlider(const QString &title, QWidget *parent)
    : DkWidget(parent)
{
    createLayout();

    // init
    mTitleLabel->setText(title);

    // defaults
    setMinimum(0.0);
    setMaximum(1.0);
    setTickInterval(0.01);
    setValue(0.5);
}

void DkDoubleSlider::createLayout()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    QWidget *dummy = new QWidget(this);
    QHBoxLayout *titleLayout = new QHBoxLayout(dummy);
    titleLayout->setContentsMargins(0, 0, 0, 5);

    mTitleLabel = new QLabel(this);

    mSliderBox = new QDoubleSpinBox(this);

    mSlider = new QSlider(this);
    mSlider->setOrientation(Qt::Horizontal);

    titleLayout->addWidget(mTitleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(mSliderBox);

    layout->addWidget(dummy);
    layout->addWidget(mSlider);

    // connect
    connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(setIntValue(int)));
    connect(mSliderBox, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
}

int DkDoubleSlider::map(double val) const
{
    double minv = 0.0, maxv = 0.0;

    if (mCenter != 0.0) {
        if (val > mCenter) {
            minv = mCenter;
            maxv = mSliderBox->maximum();
        } else {
            minv = mCenter;
            maxv = mSliderBox->minimum();
        }
    } else {
        minv = mSliderBox->minimum();
        maxv = mSliderBox->maximum();
    }

    // normalize
    double nVal = (val - minv) / (maxv - minv);

    if (mSliderInverted)
        nVal = 1.0 - nVal;

    double v = nVal * mSlider->maximum();

    if (mCenter != 0.0 && mSliderInverted)
        v -= qRound(mSlider->maximum() / 2.0);
    else if (mCenter != 0.0)
        v += qRound(mSlider->maximum() / 2.0);

    return qRound(v);
}

double DkDoubleSlider::mapInv(int val) const
{
    if (mCenter == 0.0) {
        double minv = mSliderBox->minimum();
        double maxv = mSliderBox->maximum();

        double nVal = val / (double)mSlider->maximum();

        if (mSliderInverted)
            nVal = 1.0 - nVal;

        return nVal * (maxv - minv) + minv;
    } else {
        double mc = mSlider->maximum() / 2.0;
        bool left = val < mc;

        if (mSliderInverted)
            left = !left;

        double minv, maxv;
        if (left) {
            minv = mSliderBox->minimum();
            maxv = mCenter;
        } else {
            maxv = mSliderBox->maximum();
            minv = mCenter;
        }

        double nVal = val / (double)mc;

        if (mSliderInverted)
            nVal = 1.0 - nVal;
        else
            nVal -= 1.0;

        return nVal * (maxv - minv) + mCenter;
    }
}

QSlider *DkDoubleSlider::getSlider() const
{
    return mSlider;
}

void DkDoubleSlider::setMinimum(double minValue)
{
    mSliderBox->setMinimum(minValue);
}

void DkDoubleSlider::setMaximum(double maxValue)
{
    mSliderBox->setMaximum(maxValue);
}

void DkDoubleSlider::setCenterValue(double center)
{
    mCenter = center;
}

void DkDoubleSlider::setTickInterval(double ticValue)
{
    mSlider->setMaximum(qRound(1.0 / ticValue));
}

double DkDoubleSlider::value() const
{
    return mSliderBox->value();
}

void DkDoubleSlider::setFocus(Qt::FocusReason reason)
{
    mSliderBox->setFocus(reason);
}

void DkDoubleSlider::setSliderInverted(bool inverted)
{
    mSliderInverted = inverted;
}

void DkDoubleSlider::setValue(double value)
{
    mSlider->blockSignals(true);
    mSlider->setValue(map(value));
    mSlider->blockSignals(false);

    mSliderBox->blockSignals(true);
    mSliderBox->setValue(value);
    mSliderBox->blockSignals(false);

    emit valueChanged(value);
}

void DkDoubleSlider::setIntValue(int value)
{
    double sVal = mapInv(value);

    mSlider->blockSignals(true);
    mSlider->setValue(value);
    mSlider->blockSignals(false);

    mSliderBox->blockSignals(true);
    mSliderBox->setValue(sVal);
    mSliderBox->blockSignals(false);

    emit valueChanged(sVal);
}

// DkColorChooser ------------------------------------
DkColorChooser::DkColorChooser(QColor defaultColor, QString text, QWidget *parent, Qt::WindowFlags flags)
    : DkWidget(parent, flags)
{
    this->defaultColor = defaultColor;
    this->mText = text;

    init();
}

void DkColorChooser::init()
{
    mAccepted = false;

    colorDialog = new QColorDialog(this);
    colorDialog->setObjectName("colorDialog");
    colorDialog->setOption(QColorDialog::ShowAlphaChannel, true);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(11, 0, 11, 0);

    QLabel *colorLabel = new QLabel(mText, this);
    colorButton = new QPushButton("", this);
    colorButton->setFlat(true);
    colorButton->setObjectName("colorButton");
    colorButton->setAutoDefault(false);

    QPushButton *resetButton = new QPushButton(tr("Reset"), this);
    resetButton->setObjectName("resetButton");
    resetButton->setAutoDefault(false);

    QWidget *colWidget = new QWidget(this);
    QHBoxLayout *hLayout = new QHBoxLayout(colWidget);
    hLayout->setContentsMargins(11, 0, 11, 0);
    hLayout->setAlignment(Qt::AlignLeft);

    hLayout->addWidget(colorButton);
    hLayout->addWidget(resetButton);

    vLayout->addWidget(colorLabel);
    vLayout->addWidget(colWidget);

    setColor(defaultColor);
    QMetaObject::connectSlotsByName(this);
}

bool DkColorChooser::isAccept() const
{
    return mAccepted;
}

void DkColorChooser::enableAlpha(bool enable)
{
    colorDialog->setOption(QColorDialog::ShowAlphaChannel, enable);
}

void DkColorChooser::setColor(const QColor &color)
{
    colorDialog->setCurrentColor(color);
    colorButton->setStyleSheet("QPushButton {background-color: " + DkUtils::colorToString(color) + "; border: 1px solid #888; min-height: 24px}");
    if (mSettingColor)
        *mSettingColor = color;
}

void DkColorChooser::setColor(QColor *color)
{
    if (color) {
        mSettingColor = color;
        setColor(*color);
    }
}

QColor DkColorChooser::getColor()
{
    return colorDialog->currentColor();
}

void DkColorChooser::on_resetButton_clicked()
{
    setColor(defaultColor);
    emit resetClicked();
}

void DkColorChooser::on_colorButton_clicked()
{
    // incorrect color? - see: https://bugreports.qt.io/browse/QTBUG-42988
    colorDialog->show();
}

void DkColorChooser::on_colorDialog_accepted()
{
    setColor(colorDialog->currentColor());
    mAccepted = true;
    emit accepted();
}

// -------------------------------------------------------------------- DkColorEdit
DkColorEdit::DkColorEdit(const QColor &col, QWidget *parent)
    : DkWidget(parent)
{
    createLayout();
    setColor(col);
}

void DkColorEdit::createLayout()
{
    mColBoxes.resize(c_end);

    for (int idx = 0; idx < mColBoxes.size(); idx++) {
        mColBoxes[idx] = new QSpinBox(this);
        mColBoxes[idx]->setMinimum(0);
        mColBoxes[idx]->setMaximum(255);
        connect(mColBoxes[idx], SIGNAL(valueChanged(int)), this, SLOT(colorChanged()));
    }

    mColBoxes[r]->setPrefix("R: ");
    mColBoxes[g]->setPrefix("G: ");
    mColBoxes[b]->setPrefix("B: ");

    mColHash = new QLineEdit(this);
    connect(mColHash, SIGNAL(textEdited(const QString &)), this, SLOT(hashChanged(const QString &)));
    connect(mColHash, SIGNAL(editingFinished()), this, SLOT(hashEditFinished()));

    QGridLayout *gl = new QGridLayout(this);
    gl->addWidget(mColBoxes[r], 1, 1);
    gl->addWidget(mColBoxes[g], 2, 1);
    gl->addWidget(mColBoxes[b], 3, 1);
    gl->addWidget(mColHash, 4, 1);
}

void DkColorEdit::setColor(const QColor &col)
{
    mColor = col;

    mColBoxes[r]->setValue(col.red());
    mColBoxes[g]->setValue(col.green());
    mColBoxes[b]->setValue(col.blue());

    mColHash->setText(mColor.name());
}

QColor DkColorEdit::color() const
{
    return mColor;
}

void DkColorEdit::colorChanged()
{
    mColor = QColor(mColBoxes[r]->value(), mColBoxes[g]->value(), mColBoxes[b]->value());
    mColHash->setText(mColor.name());

    emit newColor(mColor);
}

void DkColorEdit::hashChanged(const QString &name)
{
    if (!name.startsWith("#"))
        mColHash->setText("#" + mColHash->text());
}

void DkColorEdit::hashEditFinished()
{
    QColor nc;
    nc.setNamedColor(mColHash->text());

    if (nc.isValid()) {
        setColor(nc);
        emit newColor(nc);
    } else
        mColHash->setText(mColor.name());
}

// -------------------------------------------------------------------- DkColorPane
DkColorPane::DkColorPane(QWidget *parent)
    : DkWidget(parent)
{
    mColor = QColor(255, 0, 0);
}

QColor DkColorPane::color() const
{
    return pos2Color(mPos);
}

void DkColorPane::setHue(int hue)
{
    mColor.setHsvF(hue / 360.0, mColor.saturationF(), mColor.valueF());
    update();
    emit colorSelected(color());
}

double DkColorPane::hue() const
{
    return mColor.hueF();
}

void DkColorPane::setColor(const QColor &col)
{
    setHue(col.hue());
    setPos(color2Pos(col));
}

void DkColorPane::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);

    // setup corners (white, pure color, black, black)
    QColor c00, c01, c11, c10;
    c00.setHsvF(mColor.hueF(), 0, 1);
    c01.setHsvF(mColor.hueF(), 1, 1);
    c10.setHsvF(mColor.hueF(), 0, 0);
    c11.setHsvF(mColor.hueF(), 1, 0);

    int w = width();
    int h = height();

    // draw
    for (int idx = 0; idx < h; idx++) {
        QColor ccs = ipl(c00, c10, (double)idx / h);
        QColor cce = ipl(c01, c11, (double)idx / h);

        QLinearGradient g(QPoint(0, 0), QPoint(w, 0));
        g.setColorAt(0, ccs);
        g.setColorAt(1, cce);

        QRect r(0, idx, w, 1);
        p.fillRect(r, g);
    }

    QColor c = color();

    // draw current selection
    QPen pen;
    pen.setColor(brightness(c) < 0.5 ? Qt::white : Qt::black);
    p.setPen(pen);
    QRectF cPick(0, 0, 11, 11);
    cPick.moveCenter(mPos);
    p.drawEllipse(cPick);

    QWidget::paintEvent(ev);
}

void DkColorPane::mouseMoveEvent(QMouseEvent *me)
{
    if (me->buttons() == Qt::LeftButton)
        setPos(me->pos());
    QWidget::mouseMoveEvent(me);
}

void DkColorPane::mousePressEvent(QMouseEvent *me)
{
    if (me->button() == Qt::LeftButton)
        setPos(me->pos());
    QWidget::mousePressEvent(me);
}

void DkColorPane::mouseReleaseEvent(QMouseEvent *me)
{
    if (me->button() == Qt::LeftButton)
        setPos(me->pos());
    QWidget::mouseReleaseEvent(me);
}

void DkColorPane::resizeEvent(QResizeEvent *re)
{
    setPos(mPos);
    QWidget::resizeEvent(re);
}

QPoint DkColorPane::color2Pos(const QColor &col) const
{
    return QPoint(qRound(col.saturationF() * width()), qRound((1.0 - col.valueF()) * height()));
}

QColor DkColorPane::pos2Color(const QPoint &pos) const
{
    // setup corners (white, pure color, black, black)
    QColor c00, c01, c11, c10;
    c00.setHsvF(mColor.hueF(), 0, 1);
    c01.setHsvF(mColor.hueF(), 1, 1);
    c10.setHsvF(mColor.hueF(), 0, 0);
    c11.setHsvF(mColor.hueF(), 1, 0);

    QColor ccs = ipl(c00, c10, (double)pos.y() / height());
    QColor cce = ipl(c01, c11, (double)pos.y() / height());

    return ipl(ccs, cce, (double)pos.x() / width());
}

QColor DkColorPane::ipl(const QColor &c0, const QColor &c1, double alpha) const
{
    assert(alpha >= 0 && alpha <= 1.0);

    double r = c0.redF() * (1.0 - alpha) + c1.redF() * alpha;
    double g = c0.greenF() * (1.0 - alpha) + c1.greenF() * alpha;
    double b = c0.blueF() * (1.0 - alpha) + c1.blueF() * alpha;

    return QColor::fromRgbF(r, g, b);
}

void DkColorPane::setPos(const QPoint &pos)
{
    mPos.setX(qMin(qMax(pos.x(), 0), width()));
    mPos.setY(qMin(qMax(pos.y(), 0), height()));

    update();
    emit colorSelected(color());
}

double DkColorPane::brightness(const QColor &col) const
{
    return std::sqrt(col.redF() * col.redF() * 0.241 + col.greenF() * col.greenF() * 0.691 + col.blueF() * col.blueF() * 0.068);
}

// -------------------------------------------------------------------- DkColorPicker
DkColorPicker::DkColorPicker(QWidget *parent)
    : DkWidget(parent)
{
    createLayout();
    QMetaObject::connectSlotsByName(this);
}

void DkColorPicker::createLayout()
{
    int bs = qRound(20 * DkSettingsManager::param().dpiScaleFactor());

    // color pane
    mColorPane = new DkColorPane(this);
    mColorPane->setObjectName("mColorPane");
    mColorPane->setBaseSize(100, 100);
    mColorPane->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    // hue slider
    QSlider *hueSlider = new QSlider(this);
    hueSlider->setObjectName("cpHueSlider");
    hueSlider->setMaximum(360);
    hueSlider->setValue(360);
    hueSlider->setFixedWidth(bs);

    // current color
    mColorPreview = new QLabel("", this);
    mColorPreview->setFixedHeight(bs);

    QPushButton *mMenu = new QPushButton(DkImage::loadIcon(":/nomacs/img/bars.svg", QSize(bs, bs), Qt::white), "", this);
    mMenu->setObjectName("flatWhite");
    mMenu->setFlat(true);
    mMenu->setFixedSize(bs, bs);

    QGridLayout *hb = new QGridLayout(this);
    hb->setContentsMargins(0, 0, 0, 0);
    hb->addWidget(mColorPane, 0, 0);
    hb->addWidget(hueSlider, 0, 1);
    hb->addWidget(mColorPreview, 1, 0);
    hb->addWidget(mMenu, 1, 1);

    connect(hueSlider, SIGNAL(valueChanged(int)), mColorPane, SLOT(setHue(int)));
    connect(mColorPane, SIGNAL(colorSelected(const QColor &)), this, SIGNAL(colorSelected(const QColor &)));
    connect(mColorPane, SIGNAL(colorSelected(const QColor &)), this, SLOT(setColor(const QColor &)));
    connect(mMenu, SIGNAL(clicked()), this, SLOT(showMenu()));

    setColor(mColorPane->color());
}

void DkColorPicker::showMenu(const QPoint &pos)
{
    if (!mContextMenu) {
        mContextMenu = new QMenu(this);
        mColorEdit = new DkColorEdit(color(), this);
        connect(mColorEdit, SIGNAL(newColor(const QColor &)), this, SLOT(setColor(const QColor &)));
        connect(mColorEdit, SIGNAL(newColor(const QColor &)), mColorPane, SLOT(setColor(const QColor &)));

        QWidgetAction *a = new QWidgetAction(this);
        a->setDefaultWidget(mColorEdit);
        mContextMenu->addAction(a);
    }

    mColorEdit->setColor(color());
    mContextMenu->exec(!pos.isNull() ? pos : mapToGlobal(geometry().bottomRight()));
}

void DkColorPicker::setColor(const QColor &col)
{
    mColorPreview->setStyleSheet("background-color: " + DkUtils::colorToString(col));
}

QColor DkColorPicker::color() const
{
    return mColorPane->color();
}

void DkColorPicker::contextMenuEvent(QContextMenuEvent *cme)
{
    showMenu(cme->globalPos());

    // do not propagate
    // QWidget::contextMenuEvent(cme);
}

// -------------------------------------------------------------------- DkRectWidget
DkRectWidget::DkRectWidget(const QRect &r, QWidget *parent)
    : DkWidget(parent)
{
    createLayout();
    setRect(r);
}

void DkRectWidget::setRect(const QRect &r)
{
    blockSignals(true);
    mSpCropRect[crop_x]->setValue(r.x());
    mSpCropRect[crop_y]->setValue(r.y());
    mSpCropRect[crop_width]->setValue(r.width());
    mSpCropRect[crop_height]->setValue(r.height());
    blockSignals(false);
}

void DkRectWidget::setSizeOnly(bool sizeOnly)
{
    mCropXLabel->setEnabled(!sizeOnly);
    mSpCropRect[crop_x]->setEnabled(!sizeOnly);
    mCropYLabel->setEnabled(!sizeOnly);
    mSpCropRect[crop_y]->setEnabled(!sizeOnly);

    if (sizeOnly)
    {
        blockSignals(true);
        mSpCropRect[crop_x]->setSpecialValueText(" ");
        mSpCropRect[crop_y]->setSpecialValueText(" ");
        mSpCropRect[crop_x]->setValue(0);
        mSpCropRect[crop_y]->setValue(0);
        blockSignals(false);
    }
    else
    {
        mSpCropRect[crop_x]->setSpecialValueText("");
        mSpCropRect[crop_y]->setSpecialValueText("");
    }
}

QRect DkRectWidget::rect() const
{
    return QRect(mSpCropRect[crop_x]->value(), mSpCropRect[crop_y]->value(), mSpCropRect[crop_width]->value(), mSpCropRect[crop_height]->value());
}

void DkRectWidget::updateRect()
{
    emit updateRectSignal(rect());
}

void DkRectWidget::createLayout()
{
    mSpCropRect.resize(crop_end);

    mCropXLabel = new QLabel(tr("x:"));
    mSpCropRect[crop_x] = new QSpinBox(this);
    mCropXLabel->setBuddy(mSpCropRect[crop_x]);

    mCropYLabel = new QLabel(tr("y:"));
    mSpCropRect[crop_y] = new QSpinBox(this);
    mCropYLabel->setBuddy(mSpCropRect[crop_y]);

    mCropWLabel = new QLabel(tr("width:"));
    mSpCropRect[crop_width] = new QSpinBox(this);
    mCropWLabel->setBuddy(mSpCropRect[crop_width]);

    mCropHLabel = new QLabel(tr("height:"));
    mSpCropRect[crop_height] = new QSpinBox(this);
    mCropHLabel->setBuddy(mSpCropRect[crop_height]);

    for (QSpinBox *sp : mSpCropRect) {
        sp->setSuffix(tr(" px"));
        sp->setMinimum(0);
        sp->setMaximum(100000);
        connect(sp, SIGNAL(valueChanged(int)), this, SLOT(updateRect()));
    }

    QHBoxLayout *cropLayout = new QHBoxLayout(this);
    cropLayout->setContentsMargins(0, 0, 0, 0);
    cropLayout->addWidget(mCropXLabel);
    cropLayout->addWidget(mSpCropRect[crop_x]);
    cropLayout->addWidget(mCropYLabel);
    cropLayout->addWidget(mSpCropRect[crop_y]);
    cropLayout->addWidget(mCropWLabel);
    cropLayout->addWidget(mSpCropRect[crop_width]);
    cropLayout->addWidget(mCropHLabel);
    cropLayout->addWidget(mSpCropRect[crop_height]);
}

}
