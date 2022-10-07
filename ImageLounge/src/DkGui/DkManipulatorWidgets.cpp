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
#include "DkManipulatorsIpl.h"
#include "DkSettings.h"
#include "DkTimer.h"
#include "DkUtils.h"
#include "DkWidgets.h"

#pragma warning(push, 0) // no warnings from includes
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#pragma warning(pop)

namespace nmc
{

// DkManipulatorWidget --------------------------------------------------------------------
DkManipulatorWidget::DkManipulatorWidget(QWidget *parent)
    : DkFadeWidget(parent)
{
    // create widgets
    DkActionManager &am = DkActionManager::instance();
    mWidgets << new DkTinyPlanetWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_tiny_planet), this);
    mWidgets << new DkBlurWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_blur), this);
    mWidgets << new DkUnsharpMaskWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_unsharp_mask), this);
    mWidgets << new DkRotateWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_rotate), this);
    mWidgets << new DkResizeWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_resize), this);
    mWidgets << new DkThresholdWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_threshold), this);
    mWidgets << new DkHueWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_hue), this);
    mWidgets << new DkColorWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_color), this);
    mWidgets << new DkExposureWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_exposure), this);

    setObjectName("DkPreferenceTabs");
    createLayout();

    for (QWidget *w : mWidgets)
        w->setObjectName("darkManipulator");

    for (QAction *a : am.manipulatorActions())
        connect(a, SIGNAL(triggered()), this, SLOT(selectManipulator()), Qt::UniqueConnection);
}

void DkManipulatorWidget::createLayout()
{
    // actions
    QWidget *actionWidget = new QWidget(this);
    QVBoxLayout *aLayout = new QVBoxLayout(actionWidget);
    aLayout->setAlignment(Qt::AlignTop);
    aLayout->setContentsMargins(0, 0, 0, 0);
    aLayout->setSpacing(0);

    QButtonGroup *group = new QButtonGroup(this);

    DkActionManager &am = DkActionManager::instance();
    // for (QAction* a : am.manipulatorActions()) {	// if you want to get all
    for (int idx = DkManipulatorManager::m_end; idx < DkManipulatorManager::m_ext_end; idx++) {
        auto mpl = am.manipulatorManager().manipulatorExt((DkManipulatorManager::ManipulatorExtId)idx);

        DkTabEntryWidget *entry = new DkTabEntryWidget(mpl->action()->icon(), mpl->name(), this);
        connect(entry, SIGNAL(clicked()), mpl->action(), SIGNAL(triggered()), Qt::UniqueConnection);

        aLayout->addWidget(entry);
        group->addButton(entry);
    }

    QString scrollbarStyle = QString("QScrollBar:vertical {border: 1px solid " + DkUtils::colorToString(DkSettingsManager::param().display().hudFgdColor)
                                     + "; background: rgba(0,0,0,0); width: 7px; margin: 0 0 0 0;}")
        + QString("QScrollBar::handle:vertical {background: " + DkUtils::colorToString(DkSettingsManager::param().display().hudFgdColor)
                  + "; min-height: 0px;}")
        + QString("QScrollBar::add-line:vertical {height: 0px;}")
        + QString("QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: rgba(0,0,0,0); width: 1px;}")
        + QString("QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {height: 0;}");

    QScrollArea *actionScroller = new QScrollArea(this);
    actionScroller->setStyleSheet(scrollbarStyle + actionScroller->styleSheet());
    actionScroller->setWidgetResizable(true);
    actionScroller->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    actionScroller->setWidget(actionWidget);
    actionScroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // preview
    mTitleLabel = new QLabel(this);
    mTitleLabel->setObjectName("DkManipulatorSettingsTitle");
    mPreview = new QLabel(this);
    mPreview->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // undo
    QPixmap pm = DkImage::colorizePixmap(QIcon(":/nomacs/img/undo.svg").pixmap(QSize(32, 32)), QColor(255, 255, 255));
    QPushButton *undoButton = new QPushButton(pm, "", this);
    undoButton->setFlat(true);
    undoButton->setIconSize(QSize(32, 32));
    undoButton->setObjectName("dark");
    undoButton->setStatusTip(tr("Undo"));
    connect(undoButton, SIGNAL(clicked()), am.action(DkActionManager::menu_edit_undo), SIGNAL(triggered()));

    pm = DkImage::colorizePixmap(QIcon(":/nomacs/img/redo.svg").pixmap(QSize(32, 32)), QColor(255, 255, 255));
    QPushButton *redoButton = new QPushButton(pm, "", this);
    redoButton->setFlat(true);
    redoButton->setIconSize(QSize(32, 32));
    redoButton->setObjectName("dark");
    redoButton->setStatusTip(tr("Redo"));
    connect(redoButton, SIGNAL(clicked()), am.action(DkActionManager::menu_edit_redo), SIGNAL(triggered()));

    QWidget *buttonWidget = new QWidget(this);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addWidget(undoButton);
    buttonLayout->addWidget(redoButton);

    QWidget *mplWidget = new QWidget(this);
    QVBoxLayout *mplLayout = new QVBoxLayout(mplWidget);
    mplLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    mplLayout->addWidget(mTitleLabel);
    for (QWidget *w : mWidgets)
        mplLayout->addWidget(w);
    mplLayout->addWidget(mPreview);
    mplLayout->addWidget(buttonWidget);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(actionScroller);
    layout->addWidget(mplWidget);
}

QImage DkManipulatorWidget::scaledPreview(const QImage &img) const
{
    QImage imgR;

    if (img.height() > img.width())
        imgR = img.scaledToHeight(qMin(img.height(), mMaxPreview));
    else
        imgR = img.scaledToWidth(qMin(img.width(), mMaxPreview));

    return imgR;
}

void DkManipulatorWidget::setImage(QSharedPointer<DkImageContainerT> imgC)
{
    mImgC = imgC;

    if (mImgC) {
        QImage img = mImgC->imageScaledToWidth(qMin(mPreview->width(), mMaxPreview));
        img = scaledPreview(img);

        mPreview->setPixmap(QPixmap::fromImage(img));
        mPreview->show();
    } else
        mPreview->hide();
}

void DkManipulatorWidget::selectManipulator()
{
    QAction *action = dynamic_cast<QAction *>(QObject::sender());

    DkActionManager &am = DkActionManager::instance();
    QSharedPointer<DkBaseManipulator> mpl = am.manipulatorManager().manipulator(action);
    QSharedPointer<DkBaseManipulatorExt> mplExt = qSharedPointerDynamicCast<DkBaseManipulatorExt>(mpl);

    // compute preview
    if (mpl && mImgC) {
        DkTimer dt;
        QImage img = mpl->apply(mImgC->imageScaledToWidth(qMin(mPreview->width(), mMaxPreview)));
        img = scaledPreview(img);

        if (!img.isNull())
            mPreview->setPixmap(QPixmap::fromImage(img));
        qDebug() << "preview computed in " << dt;
    }

    for (QWidget *w : mWidgets)
        w->hide();

    if (!mplExt) {
        mTitleLabel->hide();
        return;
    }

    if (!mplExt->widget()) {
        qCritical() << action->text() << "does not have a corresponding UI";
        return;
    }

    mplExt->widget()->show();
    mTitleLabel->setText(mpl->name());
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
    : DkFadeWidget(parent)
{
    mBaseManipulator = manipulator;
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
    QMetaObject::connectSlotsByName(this);

    manipulator->setWidget(this);
}

void DkTinyPlanetWidget::createLayout()
{
    // post processing sliders
    DkSlider *scaleSlider = new DkSlider(tr("Planet Size"), this);
    scaleSlider->setObjectName("scaleSlider");
    scaleSlider->setMinimum(1);
    scaleSlider->setMaximum(1000);
    scaleSlider->setValue(manipulator()->size());

    DkSlider *angleSlider = new DkSlider(tr("Angle"), this);
    angleSlider->setObjectName("angleSlider");
    angleSlider->setValue(manipulator()->angle());
    angleSlider->setMinimum(-180);
    angleSlider->setMaximum(179);

    QCheckBox *invertBox = new QCheckBox(tr("Invert Planet"), this);
    invertBox->setObjectName("invertBox");
    invertBox->setChecked(manipulator()->inverted());

    QVBoxLayout *sliderLayout = new QVBoxLayout(this);
    sliderLayout->addWidget(scaleSlider);
    sliderLayout->addWidget(angleSlider);
    sliderLayout->addWidget(invertBox);
}

void DkTinyPlanetWidget::on_scaleSlider_valueChanged(int val)
{
    manipulator()->setSize(val);
}

void DkTinyPlanetWidget::on_angleSlider_valueChanged(int val)
{
    manipulator()->setAngle(val);
}

void DkTinyPlanetWidget::on_invertBox_toggled(bool val)
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
    QMetaObject::connectSlotsByName(this);

    manipulator->setWidget(this);
}

void DkBlurWidget::createLayout()
{
    // post processing sliders
    DkSlider *sigmaSlider = new DkSlider(tr("Sigma"), this);
    sigmaSlider->setObjectName("sigmaSlider");
    sigmaSlider->setValue(manipulator()->sigma());
    sigmaSlider->setMaximum(50);

    QVBoxLayout *sliderLayout = new QVBoxLayout(this);
    sliderLayout->addWidget(sigmaSlider);
}

void DkBlurWidget::on_sigmaSlider_valueChanged(int val)
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
    QMetaObject::connectSlotsByName(this);

    manipulator->setWidget(this);
}

void DkUnsharpMaskWidget::createLayout()
{
    // post processing sliders
    DkSlider *sigmaSlider = new DkSlider(tr("Sigma"), this);
    sigmaSlider->setObjectName("sigmaSlider");
    sigmaSlider->setValue(manipulator()->sigma());
    // darkenSlider->hide();

    DkSlider *amountSlider = new DkSlider(tr("Amount"), this);
    amountSlider->setObjectName("amountSlider");
    amountSlider->setValue(manipulator()->amount());

    QVBoxLayout *sliderLayout = new QVBoxLayout(this);
    sliderLayout->addWidget(sigmaSlider);
    sliderLayout->addWidget(amountSlider);
}

void DkUnsharpMaskWidget::on_sigmaSlider_valueChanged(int val)
{
    manipulator()->setSigma(val);
}

void DkUnsharpMaskWidget::on_amountSlider_valueChanged(int val)
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
    QMetaObject::connectSlotsByName(this);

    manipulator->setWidget(this);
}

QSharedPointer<DkRotateManipulator> DkRotateWidget::manipulator() const
{
    return qSharedPointerDynamicCast<DkRotateManipulator>(baseManipulator());
}

void DkRotateWidget::createLayout()
{
    DkSlider *angleSlider = new DkSlider(tr("Angle"), this);
    angleSlider->setObjectName("angleSlider");
    angleSlider->setValue(manipulator()->angle());
    angleSlider->setMinimum(-180);
    angleSlider->setMaximum(180);

    QVBoxLayout *sliderLayout = new QVBoxLayout(this);
    sliderLayout->addWidget(angleSlider);
}

void DkRotateWidget::on_angleSlider_valueChanged(int val)
{
    manipulator()->setAngle(val);
}

// DkRotateWidget --------------------------------------------------------------------
DkResizeWidget::DkResizeWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent)
    : DkBaseManipulatorWidget(manipulator, parent)
{
    createLayout();
    QMetaObject::connectSlotsByName(this);

    manipulator->setWidget(this);

    // I would have loved setObjectName to be virtual : )
    connect(this, SIGNAL(objectNameChanged(const QString &)), this, SLOT(onObjectNameChanged(const QString &)));
}

QSharedPointer<DkResizeManipulator> DkResizeWidget::manipulator() const
{
    return qSharedPointerDynamicCast<DkResizeManipulator>(baseManipulator());
}

void DkResizeWidget::onObjectNameChanged(const QString &name)
{
    if (name == "darkManipulator") {
        // this is a hack: if we don't do this, nmc--DkBaseManipulatorWidget#darkManipulator QComboBox QAbstractItemView get's applied
        // I have the feeling, that this is a Qt issue
        // without this line, all styles are applied to the QComboBox but not to its drop down list (QAbstractItemView)
        mIplBox->setStyleSheet(mIplBox->styleSheet() + " ");
    }
}

void DkResizeWidget::createLayout()
{
    DkDoubleSlider *scaleSlider = new DkDoubleSlider(tr("Scale"), this);
    scaleSlider->setObjectName("scaleFactorSlider");
    scaleSlider->setMinimum(0.1);
    scaleSlider->setCenterValue(1.0);
    scaleSlider->setMaximum(10);
    scaleSlider->setValue(manipulator()->scaleFactor());

    mIplBox = new QComboBox(this);
    mIplBox->setObjectName("iplBox");
    mIplBox->setView(new QListView()); // needed for style
    mIplBox->addItem(tr("Nearest Neighbor"), DkImage::ipl_nearest);
    mIplBox->addItem(tr("Area (best for downscaling)"), DkImage::ipl_area);
    mIplBox->addItem(tr("Linear"), DkImage::ipl_linear);
    mIplBox->addItem(tr("Bicubic (4x4 interpolatia)"), DkImage::ipl_cubic);
    mIplBox->addItem(tr("Lanczos (8x8 interpolation)"), DkImage::ipl_lanczos);
    mIplBox->setCurrentIndex(1);

    QCheckBox *cbGamma = new QCheckBox(tr("Gamma Correction"), this);
    cbGamma->setObjectName("gammaCorrection");

    QVBoxLayout *sliderLayout = new QVBoxLayout(this);
    sliderLayout->setSpacing(10);
    sliderLayout->addWidget(scaleSlider);
    sliderLayout->addWidget(mIplBox);
    sliderLayout->addWidget(cbGamma);
}

void DkResizeWidget::on_scaleFactorSlider_valueChanged(double val)
{
    manipulator()->setScaleFactor(val);
}

void DkResizeWidget::on_iplBox_currentIndexChanged(int idx)
{
    manipulator()->setInterpolation(mIplBox->itemData(idx).toInt());
}

void DkResizeWidget::on_gammaCorrection_toggled(bool checked)
{
    manipulator()->setCorrectGamma(checked);
}

// DkThresholdWidget --------------------------------------------------------------------
DkThresholdWidget::DkThresholdWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent)
    : DkBaseManipulatorWidget(manipulator, parent)
{
    createLayout();
    QMetaObject::connectSlotsByName(this);

    manipulator->setWidget(this);
}

QSharedPointer<DkThresholdManipulator> DkThresholdWidget::manipulator() const
{
    return qSharedPointerDynamicCast<DkThresholdManipulator>(baseManipulator());
}

void DkThresholdWidget::on_colBox_toggled(bool checked)
{
    manipulator()->setColor(checked);
}

void DkThresholdWidget::createLayout()
{
    DkSlider *thrSlider = new DkSlider(tr("Threshold"), this);
    thrSlider->setObjectName("thrSlider");
    thrSlider->setValue(manipulator()->threshold());
    thrSlider->setMinimum(0);
    thrSlider->setMaximum(255);
    thrSlider->setValue(manipulator()->threshold());

    QCheckBox *colBox = new QCheckBox(tr("Color"), this);
    colBox->setObjectName("colBox");
    colBox->setChecked(manipulator()->color());

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(thrSlider);
    layout->addWidget(colBox);
}
void DkThresholdWidget::on_thrSlider_valueChanged(int val)
{
    manipulator()->setThreshold(val);
}

// -------------------------------------------------------------------- DkColorWidget
DkColorWidget::DkColorWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent)
    : DkBaseManipulatorWidget(manipulator, parent)
{
    createLayout();

    QMetaObject::connectSlotsByName(this);

    manipulator->setWidget(this);
    setMinimumHeight(150);
}

QSharedPointer<DkColorManipulator> DkColorWidget::manipulator() const
{
    return qSharedPointerDynamicCast<DkColorManipulator>(baseManipulator());
}

void DkColorWidget::createLayout()
{
    DkColorPicker *cp = new DkColorPicker(this);
    cp->setObjectName("colPicker");

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(cp);
}

void DkColorWidget::on_colPicker_colorSelected(const QColor &col)
{
    manipulator()->setColor(col);
}

// DkHueWidget --------------------------------------------------------------------
DkHueWidget::DkHueWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent)
    : DkBaseManipulatorWidget(manipulator, parent)
{
    createLayout();
    QMetaObject::connectSlotsByName(this);

    manipulator->setWidget(this);
}

QSharedPointer<DkHueManipulator> DkHueWidget::manipulator() const
{
    return qSharedPointerDynamicCast<DkHueManipulator>(baseManipulator());
}

void DkHueWidget::createLayout()
{
    DkSlider *hueSlider = new DkSlider(tr("Hue"), this);
    hueSlider->setObjectName("hueSlider");
    hueSlider->getSlider()->setObjectName("DkHueSlider");
    hueSlider->setValue(manipulator()->hue());
    hueSlider->setMinimum(-180);
    hueSlider->setMaximum(180);

    DkSlider *satSlider = new DkSlider(tr("Saturation"), this);
    satSlider->setObjectName("satSlider");
    satSlider->getSlider()->setObjectName("DkSaturationSlider");
    satSlider->setValue(manipulator()->saturation());
    satSlider->setMinimum(-100);
    satSlider->setMaximum(100);

    DkSlider *brightnessSlider = new DkSlider(tr("Brightness"), this);
    brightnessSlider->setObjectName("brightnessSlider");
    brightnessSlider->getSlider()->setObjectName("DkBrightnessSlider");
    brightnessSlider->setValue(manipulator()->hue());
    brightnessSlider->setMinimum(-100);
    brightnessSlider->setMaximum(100);

    QVBoxLayout *sliderLayout = new QVBoxLayout(this);
    sliderLayout->addWidget(hueSlider);
    sliderLayout->addWidget(satSlider);
    sliderLayout->addWidget(brightnessSlider);
}

void DkHueWidget::on_hueSlider_valueChanged(int val)
{
    manipulator()->setHue(val);
}

void DkHueWidget::on_satSlider_valueChanged(int val)
{
    manipulator()->setSaturation(val);
}

void DkHueWidget::on_brightnessSlider_valueChanged(int val)
{
    manipulator()->setValue(val);
}

// DkExposureWidget --------------------------------------------------------------------
DkExposureWidget::DkExposureWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget *parent)
    : DkBaseManipulatorWidget(manipulator, parent)
{
    createLayout();
    QMetaObject::connectSlotsByName(this);

    manipulator->setWidget(this);
}

QSharedPointer<DkExposureManipulator> DkExposureWidget::manipulator() const
{
    return qSharedPointerDynamicCast<DkExposureManipulator>(baseManipulator());
}

void DkExposureWidget::createLayout()
{
    DkDoubleSlider *exposureSlider = new DkDoubleSlider(tr("Exposure"), this);
    exposureSlider->setObjectName("exposureSlider");
    exposureSlider->setMinimum(-3);
    exposureSlider->setMaximum(3);
    exposureSlider->setTickInterval(0.0005);
    exposureSlider->setValue(manipulator()->exposure());

    DkDoubleSlider *offsetSlider = new DkDoubleSlider(tr("Offset"), this);
    offsetSlider->setObjectName("offsetSlider");
    offsetSlider->setMinimum(-0.5);
    offsetSlider->setMaximum(0.5);
    offsetSlider->setTickInterval(0.001);
    offsetSlider->setValue(manipulator()->offset());

    DkDoubleSlider *gammaSlider = new DkDoubleSlider(tr("Gamma"), this);
    gammaSlider->setObjectName("gammaSlider");
    gammaSlider->setMinimum(0);
    gammaSlider->setCenterValue(1);
    gammaSlider->setMaximum(10);
    gammaSlider->setTickInterval(0.001);
    gammaSlider->setSliderInverted(true);
    gammaSlider->setValue(manipulator()->gamma());

    QVBoxLayout *sliderLayout = new QVBoxLayout(this);
    sliderLayout->addWidget(exposureSlider);
    sliderLayout->addWidget(offsetSlider);
    sliderLayout->addWidget(gammaSlider);
}

void DkExposureWidget::on_exposureSlider_valueChanged(double val)
{
    double tv = val * val;
    if (val < 0)
        tv *= -1.0;
    manipulator()->setExposure(tv);
}

void DkExposureWidget::on_offsetSlider_valueChanged(double val)
{
    manipulator()->setOffset(val);
}

void DkExposureWidget::on_gammaSlider_valueChanged(double val)
{
    manipulator()->setGamma(val);
}

}