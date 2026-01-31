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

#include "DkManipulatorWidgets.h"

#include "DkActionManager.h"
#include "DkBasicWidgets.h"
#include "DkImageStorage.h"
#include "DkSettings.h"
#include "DkUtils.h"
#include "DkWidgets.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QColorSpace>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>

namespace nmc
{
// DkManipulatorWidget --------------------------------------------------------------------
DkManipulatorWidget::DkManipulatorWidget(QWidget *parent)
    : DkWidget(parent)
{
    // create widgets
    DkActionManager &am = DkActionManager::instance();
    mWidgets << new DkTinyPlanetWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_tiny_planet),
                                       this);
    mWidgets << new DkBlurWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_blur), this);
    mWidgets << new DkUnsharpMaskWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_unsharp_mask),
                                        this);
    mWidgets << new DkRotateWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_rotate), this);
    mWidgets << new DkResizeWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_resize), this);
    mWidgets << new DkThresholdWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_threshold), this);
    mWidgets << new DkHueWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_hue), this);
    mWidgets << new DkColorWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_color), this);
    mWidgets << new DkExposureWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_exposure), this);

    setObjectName("DkPreferenceTabs");
    createLayout();

    for (auto w : mWidgets)
        w->setObjectName("darkManipulator");

    for (QAction *a : am.manipulatorActions())
        connect(a, &QAction::triggered, this, &DkManipulatorWidget::selectManipulator, Qt::UniqueConnection);
}

void DkManipulatorWidget::createLayout()
{
    // actions
    auto *actionWidget = new QWidget(this);
    auto *aLayout = new QVBoxLayout(actionWidget);
    aLayout->setAlignment(Qt::AlignTop);
    aLayout->setContentsMargins(0, 0, 0, 0);
    aLayout->setSpacing(0);

    auto *group = new QButtonGroup(this);

    DkActionManager &am = DkActionManager::instance();
    for (int idx = DkManipulatorManager::m_end; idx < DkManipulatorManager::m_ext_end; idx++) {
        const auto extIdx = static_cast<DkManipulatorManager::ManipulatorExtId>(idx);
        auto mpl = am.manipulatorManager().manipulatorExt(extIdx);

        auto *entry = new DkTabEntryWidget(mpl->action()->icon(), mpl->name(), this);
        connect(entry, &DkTabEntryWidget::clicked, this, [this, mpl]() {
            selectManipulatorInner(mpl);
        });

        aLayout->addWidget(entry);
        group->addButton(entry);
    }

    QString scrollbarStyle = QString("QScrollBar:vertical {border: 1px solid "
                                     + DkUtils::colorToString(DkSettingsManager::param().display().hudFgdColor)
                                     + "; background: rgba(0,0,0,0); width: 7px; margin: 0 0 0 0;}")
        + QString("QScrollBar::handle:vertical {background: "
                  + DkUtils::colorToString(DkSettingsManager::param().display().hudFgdColor) + "; min-height: 0px;}")
        + QString("QScrollBar::add-line:vertical {height: 0px;}")
        + QString("QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: rgba(0,0,0,0); width: "
                  "1px;}")
        + QString("QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {height: 0;}");

    auto *actionScroller = new QScrollArea(this);
    actionScroller->setStyleSheet(scrollbarStyle + actionScroller->styleSheet());
    actionScroller->setWidgetResizable(true);
    actionScroller->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    actionScroller->setMinimumWidth(300);
    actionScroller->setWidget(actionWidget);
    actionScroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // preview
    mTitleLabel = new QLabel(this);
    mTitleLabel->setObjectName("DkManipulatorSettingsTitle");
    mPreview = new QLabel(this);
    mPreview->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    mPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mPreview->setContentsMargins(8, 8, 8, 8);

    auto *mplWidget = new QWidget(this);
    auto *mplLayout = new QVBoxLayout(mplWidget);
    mplLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    mplLayout->addWidget(mTitleLabel);
    for (auto w : mWidgets)
        mplLayout->addWidget(w);
    mplLayout->addWidget(mPreview);
    mplWidget->setMinimumHeight(350);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(actionScroller);
    layout->addWidget(mplWidget);

    // I couldn't find the focus widget so just get them all
    for (QWidget *w : this->findChildren<QWidget *>())
        w->setFocusPolicy(Qt::ClickFocus);
}

void DkManipulatorWidget::setImage(QSharedPointer<DkImageContainerT> imgC)
{
    if (imgC) {
        QImage img = imgC->image();
        QSize newSize = img.size().scaled(mPreview->contentsRect().size(), Qt::KeepAspectRatio);
        img = img.scaledToWidth(newSize.width(), Qt::SmoothTransformation);
        img = DkImage::convertToColorSpaceInPlace(this, img);
        mPreview->setPixmap(QPixmap::fromImage(img));
        mPreview->show();
    } else {
        mPreview->hide();
    }
}

void DkManipulatorWidget::selectManipulator()
{
    const auto *action = dynamic_cast<QAction *>(QObject::sender());

    DkActionManager &am = DkActionManager::instance();
    QSharedPointer<DkBaseManipulator> mpl = am.manipulatorManager().manipulator(action);
    QSharedPointer<DkBaseManipulatorExt> mplExt = qSharedPointerDynamicCast<DkBaseManipulatorExt>(mpl);
    selectManipulatorInner(mplExt);
}

void DkManipulatorWidget::selectManipulatorInner(QSharedPointer<DkBaseManipulatorExt> mplExt)
{
    if (!mplExt) {
        mTitleLabel->hide();
        for (auto w : mWidgets) {
            w->hide();
        }
        return;
    }

    if (!mplExt->widget()) {
        qCritical() << mplExt->name() << "does not have a corresponding UI";
        return;
    }

    for (auto w : mWidgets) {
        // Do not hide the target widget, otherwise we will lose focus
        // if we are typing on the current widget.
        if (w == mplExt->widget()) {
            continue;
        }
        w->hide();
    }

    mplExt->widget()->show();
    mTitleLabel->setText(mplExt->name());
}

// DkMainpulatorDoc --------------------------------------------------------------------
DkEditDock::DkEditDock(const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : DkDockWidget(title, parent, flags)
{
    setObjectName("DkEditDock");
    createLayout();
}

void DkEditDock::createLayout()
{
    mMplWidget = new DkManipulatorWidget(this);
    setWidget(mMplWidget);
}

void DkEditDock::setImage(QSharedPointer<DkImageContainerT> imgC)
{
    mMplWidget->setImage(imgC);
}

// DkManipulatorWidget --------------------------------------------------------------------
DkBaseManipulatorWidget::DkBaseManipulatorWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent)
    : DkWidget(parent)
{
    mBaseManipulator = manipulator;
    setVisible(false);
}

QSharedPointer<DkBaseManipulatorExt> DkBaseManipulatorWidget::baseManipulator() const
{
    return mBaseManipulator;
}

// DkTinyPlanetWidget --------------------------------------------------------------------
DkTinyPlanetWidget::DkTinyPlanetWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent)
    : DkBaseManipulatorWidget(manipulator, parent)
{
    createLayout();

    manipulator->setWidget(this);
}

void DkTinyPlanetWidget::createLayout()
{
    // post processing sliders
    auto *scaleSlider = new DkSlider(tr("Planet Size"), this);
    scaleSlider->setMinimum(1);
    scaleSlider->setMaximum(1000);
    scaleSlider->setValue(manipulator()->size());
    connect(scaleSlider, &DkSlider::valueChanged, this, &DkTinyPlanetWidget::onScaleSliderValueChanged);

    auto *angleSlider = new DkSlider(tr("Angle"), this);
    angleSlider->setValue(manipulator()->angle());
    angleSlider->setMinimum(-180);
    angleSlider->setMaximum(179);
    connect(angleSlider, &DkSlider::valueChanged, this, &DkTinyPlanetWidget::onAngleSliderValueChanged);

    auto *invertBox = new QCheckBox(tr("Invert Planet"), this);
    invertBox->setChecked(manipulator()->inverted());
    connect(invertBox, &QCheckBox::toggled, this, &DkTinyPlanetWidget::onInvertBoxToggled);

    auto *sliderLayout = new QVBoxLayout(this);
    sliderLayout->addWidget(scaleSlider);
    sliderLayout->addWidget(angleSlider);
    sliderLayout->addWidget(invertBox);
}

void DkTinyPlanetWidget::onScaleSliderValueChanged(int val)
{
    manipulator()->setSize(val);
}

void DkTinyPlanetWidget::onAngleSliderValueChanged(int val)
{
    manipulator()->setAngle(val);
}

void DkTinyPlanetWidget::onInvertBoxToggled(bool val)
{
    manipulator()->setInverted(val);
}

QSharedPointer<DkTinyPlanetManipulator> DkTinyPlanetWidget::manipulator() const
{
    return qSharedPointerDynamicCast<DkTinyPlanetManipulator>(baseManipulator());
}

// DkBlurWidget --------------------------------------------------------------------
DkBlurWidget::DkBlurWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent)
    : DkBaseManipulatorWidget(manipulator, parent)
{
    createLayout();

    manipulator->setWidget(this);
}

void DkBlurWidget::createLayout()
{
    // post processing sliders
    auto *sigmaSlider = new DkSlider(tr("Sigma"), this);
    sigmaSlider->setValue(manipulator()->sigma());
    sigmaSlider->setMinimum(1);
    sigmaSlider->setMaximum(50);
    connect(sigmaSlider, &DkSlider::valueChanged, this, &DkBlurWidget::onSigmaSliderValueChanged);

    auto *sliderLayout = new QVBoxLayout(this);
    sliderLayout->addWidget(sigmaSlider);
}

void DkBlurWidget::onSigmaSliderValueChanged(int val)
{
    manipulator()->setSigma(val);
}

QSharedPointer<DkBlurManipulator> DkBlurWidget::manipulator() const
{
    return qSharedPointerDynamicCast<DkBlurManipulator>(baseManipulator());
}

// DkUnsharpMaskWidget --------------------------------------------------------------------
DkUnsharpMaskWidget::DkUnsharpMaskWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent)
    : DkBaseManipulatorWidget(manipulator, parent)
{
    createLayout();

    manipulator->setWidget(this);
}

void DkUnsharpMaskWidget::createLayout()
{
    // post processing sliders
    auto *sigmaSlider = new DkSlider(tr("Sigma"), this);
    sigmaSlider->setMinimum(1);
    sigmaSlider->setValue(manipulator()->sigma());
    connect(sigmaSlider, &DkSlider::valueChanged, this, &DkUnsharpMaskWidget::onSigmaSliderValueChanged);

    auto *amountSlider = new DkSlider(tr("Amount"), this);
    amountSlider->setValue(manipulator()->amount());
    connect(amountSlider, &DkSlider::valueChanged, this, &DkUnsharpMaskWidget::onAmountSliderValueChanged);

    auto *sliderLayout = new QVBoxLayout(this);
    sliderLayout->addWidget(sigmaSlider);
    sliderLayout->addWidget(amountSlider);
}

void DkUnsharpMaskWidget::onSigmaSliderValueChanged(int val)
{
    manipulator()->setSigma(val);
}

void DkUnsharpMaskWidget::onAmountSliderValueChanged(int val)
{
    manipulator()->setAmount(val);
}

QSharedPointer<DkUnsharpMaskManipulator> DkUnsharpMaskWidget::manipulator() const
{
    return qSharedPointerDynamicCast<DkUnsharpMaskManipulator>(baseManipulator());
}

// DkRotateWidget --------------------------------------------------------------------
DkRotateWidget::DkRotateWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent)
    : DkBaseManipulatorWidget(manipulator, parent)
{
    createLayout();

    manipulator->setWidget(this);
}

QSharedPointer<DkRotateManipulator> DkRotateWidget::manipulator() const
{
    return qSharedPointerDynamicCast<DkRotateManipulator>(baseManipulator());
}

void DkRotateWidget::createLayout()
{
    auto *angleSlider = new DkSlider(tr("Angle"), this);
    angleSlider->setValue(manipulator()->angle());
    angleSlider->setMinimum(-180);
    angleSlider->setMaximum(180);
    connect(angleSlider, &DkSlider::valueChanged, this, &DkRotateWidget::onAngleSliderValueChanged);

    auto *sliderLayout = new QVBoxLayout(this);
    sliderLayout->addWidget(angleSlider);
}

void DkRotateWidget::onAngleSliderValueChanged(int val)
{
    manipulator()->setAngle(val);
}

// DkRotateWidget --------------------------------------------------------------------
DkResizeWidget::DkResizeWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent)
    : DkBaseManipulatorWidget(manipulator, parent)
{
    createLayout();

    manipulator->setWidget(this);

    // I would have loved setObjectName to be virtual : )
    connect(this, &DkResizeWidget::objectNameChanged, this, &DkResizeWidget::onObjectNameChanged);
}

QSharedPointer<DkResizeManipulator> DkResizeWidget::manipulator() const
{
    return qSharedPointerDynamicCast<DkResizeManipulator>(baseManipulator());
}

void DkResizeWidget::onObjectNameChanged(const QString &name)
{
    if (name == "darkManipulator") {
        // this is a hack: if we don't do this, nmc--DkBaseManipulatorWidget#darkManipulator QComboBox QAbstractItemView
        // get's applied I have the feeling, that this is a Qt issue without this line, all styles are applied to the
        // QComboBox but not to its drop down list (QAbstractItemView)
        mIplBox->setStyleSheet(mIplBox->styleSheet() + " ");
    }
}

void DkResizeWidget::createLayout()
{
    auto *scaleSlider = new DkDoubleSlider(tr("Scale"), this);
    scaleSlider->setMinimum(0.1);
    scaleSlider->setCenterValue(1.0);
    scaleSlider->setMaximum(10);
    scaleSlider->setValue(manipulator()->scaleFactor());
    connect(scaleSlider, &DkDoubleSlider::valueChanged, this, &DkResizeWidget::onScaleFactorSliderValueChanged);

    mIplBox = new QComboBox(this);
    mIplBox->setView(new QListView()); // needed for style
    mIplBox->addItem(tr("Nearest Neighbor"), DkImage::ipl_nearest);
    mIplBox->addItem(tr("Area (best for downscaling)"), DkImage::ipl_area);
    mIplBox->addItem(tr("Linear"), DkImage::ipl_linear);
    mIplBox->addItem(tr("Bicubic (4x4 interpolatia)"), DkImage::ipl_cubic);
    mIplBox->addItem(tr("Lanczos (8x8 interpolation)"), DkImage::ipl_lanczos);
    mIplBox->setCurrentIndex(1);
    connect(mIplBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DkResizeWidget::onIplBoxCurrentIndexChanged);

    auto *cbGamma = new QCheckBox(tr("Gamma Correction"), this);
    connect(cbGamma, &QCheckBox::toggled, this, &DkResizeWidget::onGammaCorrectionToggled);

    auto *sliderLayout = new QVBoxLayout(this);
    sliderLayout->setSpacing(10);
    sliderLayout->addWidget(scaleSlider);
    sliderLayout->addWidget(mIplBox);
    sliderLayout->addWidget(cbGamma);
}

void DkResizeWidget::onScaleFactorSliderValueChanged(double val)
{
    manipulator()->setScaleFactor(val);
}

void DkResizeWidget::onIplBoxCurrentIndexChanged(int idx)
{
    manipulator()->setInterpolation(mIplBox->itemData(idx).toInt());
}

void DkResizeWidget::onGammaCorrectionToggled(bool checked)
{
    manipulator()->setCorrectGamma(checked);
}

// DkThresholdWidget --------------------------------------------------------------------
DkThresholdWidget::DkThresholdWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent)
    : DkBaseManipulatorWidget(manipulator, parent)
{
    createLayout();

    manipulator->setWidget(this);
}

QSharedPointer<DkThresholdManipulator> DkThresholdWidget::manipulator() const
{
    return qSharedPointerDynamicCast<DkThresholdManipulator>(baseManipulator());
}

void DkThresholdWidget::onColBoxToggled(bool checked)
{
    manipulator()->setColor(checked);
}

void DkThresholdWidget::createLayout()
{
    auto *thrSlider = new DkSlider(tr("Threshold"), this);
    thrSlider->setValue(manipulator()->threshold());
    thrSlider->setMinimum(0);
    thrSlider->setMaximum(255);
    thrSlider->setValue(manipulator()->threshold());
    connect(thrSlider, &DkSlider::valueChanged, this, &DkThresholdWidget::onThrSliderValueChanged);

    auto *colBox = new QCheckBox(tr("Color"), this);
    colBox->setChecked(manipulator()->color());
    connect(colBox, &QCheckBox::toggled, this, &DkThresholdWidget::onColBoxToggled);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(thrSlider);
    layout->addWidget(colBox);
}
void DkThresholdWidget::onThrSliderValueChanged(int val)
{
    manipulator()->setThreshold(val);
}

// -------------------------------------------------------------------- DkColorWidget
DkColorWidget::DkColorWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent)
    : DkBaseManipulatorWidget(manipulator, parent)
{
    createLayout();

    manipulator->setWidget(this);
    setMinimumHeight(150);
}

QSharedPointer<DkColorManipulator> DkColorWidget::manipulator() const
{
    return qSharedPointerDynamicCast<DkColorManipulator>(baseManipulator());
}

void DkColorWidget::createLayout()
{
    auto *cp = new DkColorPicker(this);
    connect(cp, &DkColorPicker::colorSelected, this, &DkColorWidget::onColPickerColorSelected);

    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(cp);
}

void DkColorWidget::onColPickerColorSelected(const QColor &col)
{
    manipulator()->setColor(col);
}

// DkHueWidget --------------------------------------------------------------------
DkHueWidget::DkHueWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent)
    : DkBaseManipulatorWidget(manipulator, parent)
{
    createLayout();

    manipulator->setWidget(this);
}

QSharedPointer<DkHueManipulator> DkHueWidget::manipulator() const
{
    return qSharedPointerDynamicCast<DkHueManipulator>(baseManipulator());
}

void DkHueWidget::createLayout()
{
    auto *hueSlider = new DkSlider(tr("Hue"), this);
    hueSlider->getSlider()->setObjectName("DkHueSlider");
    hueSlider->setValue(manipulator()->hue());
    hueSlider->setMinimum(-180);
    hueSlider->setMaximum(180);
    connect(hueSlider, &DkSlider::valueChanged, this, &DkHueWidget::onHueSliderValueChanged);

    auto *satSlider = new DkSlider(tr("Saturation"), this);
    satSlider->getSlider()->setObjectName("DkSaturationSlider");
    satSlider->setValue(manipulator()->saturation());
    satSlider->setMinimum(-100);
    satSlider->setMaximum(100);
    connect(satSlider, &DkSlider::valueChanged, this, &DkHueWidget::onSatSliderValueChanged);

    auto *brightnessSlider = new DkSlider(tr("Brightness"), this);
    brightnessSlider->getSlider()->setObjectName("DkBrightnessSlider");
    brightnessSlider->setValue(manipulator()->hue());
    brightnessSlider->setMinimum(-100);
    brightnessSlider->setMaximum(100);
    connect(brightnessSlider, &DkSlider::valueChanged, this, &DkHueWidget::onBrightnessSliderValueChanged);

    auto *sliderLayout = new QVBoxLayout(this);
    sliderLayout->addWidget(hueSlider);
    sliderLayout->addWidget(satSlider);
    sliderLayout->addWidget(brightnessSlider);
}

void DkHueWidget::onHueSliderValueChanged(int val)
{
    manipulator()->setHue(val);
}

void DkHueWidget::onSatSliderValueChanged(int val)
{
    manipulator()->setSaturation(val);
}

void DkHueWidget::onBrightnessSliderValueChanged(int val)
{
    manipulator()->setValue(val);
}

// DkExposureWidget --------------------------------------------------------------------
DkExposureWidget::DkExposureWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent)
    : DkBaseManipulatorWidget(manipulator, parent)
{
    createLayout();

    manipulator->setWidget(this);
}

QSharedPointer<DkExposureManipulator> DkExposureWidget::manipulator() const
{
    return qSharedPointerDynamicCast<DkExposureManipulator>(baseManipulator());
}

void DkExposureWidget::createLayout()
{
    auto *exposureSlider = new DkDoubleSlider(tr("Exposure"), this);
    exposureSlider->setMinimum(-3);
    exposureSlider->setMaximum(3);
    exposureSlider->setTickInterval(0.0005);
    exposureSlider->setValue(manipulator()->exposure());
    connect(exposureSlider, &DkDoubleSlider::valueChanged, this, &DkExposureWidget::onExposureSliderValueChanged);

    auto *offsetSlider = new DkDoubleSlider(tr("Offset"), this);
    offsetSlider->setMinimum(-0.5);
    offsetSlider->setMaximum(0.5);
    offsetSlider->setTickInterval(0.001);
    offsetSlider->setValue(manipulator()->offset());
    connect(offsetSlider, &DkDoubleSlider::valueChanged, this, &DkExposureWidget::onOffsetSliderValueChanged);

    auto *gammaSlider = new DkDoubleSlider(tr("Gamma"), this);
    gammaSlider->setMinimum(0);
    gammaSlider->setCenterValue(1);
    gammaSlider->setMaximum(10);
    gammaSlider->setTickInterval(0.001);
    gammaSlider->setSliderInverted(true);
    gammaSlider->setValue(manipulator()->gamma());
    connect(gammaSlider, &DkDoubleSlider::valueChanged, this, &DkExposureWidget::onGammaSliderValueChanged);

    auto *sliderLayout = new QVBoxLayout(this);
    sliderLayout->addWidget(exposureSlider);
    sliderLayout->addWidget(offsetSlider);
    sliderLayout->addWidget(gammaSlider);
}

void DkExposureWidget::onExposureSliderValueChanged(double val)
{
    double tv = val * val;
    if (val < 0)
        tv *= -1.0;
    manipulator()->setExposure(tv);
}

void DkExposureWidget::onOffsetSliderValueChanged(double val)
{
    manipulator()->setOffset(val);
}

void DkExposureWidget::onGammaSliderValueChanged(double val)
{
    manipulator()->setGamma(val);
}

}
