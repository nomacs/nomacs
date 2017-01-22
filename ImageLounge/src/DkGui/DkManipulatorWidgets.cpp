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
 [1] http://www.nomacs.org/
 [2] https://github.com/nomacs/
 [3] http://download.nomacs.org
 *******************************************************************************************************/

#include "DkManipulatorWidgets.h"
#include "DkActionManager.h"
#include "DkWidgets.h"
#include "DkTimer.h"
#include "DkUtils.h"
#include "DkSettings.h"
#include "DkImageStorage.h"
#include "DkBasicWidgets.h"
#include "DkManipulatorsIpl.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QVBoxLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QCheckBox>
#pragma warning(pop)

namespace nmc {

// DkManipulatorWidget --------------------------------------------------------------------
DkManipulatorWidget::DkManipulatorWidget(QWidget* parent) : DkWidget(parent) {
	
	// create widgets
	// TODO: do not create them here...?!
	DkActionManager& am = DkActionManager::instance();
	mWidgets << new DkTinyPlanetWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_tiny_planet), this);
	mWidgets << new DkUnsharpMaskWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_unsharp_mask), this);
	mWidgets << new DkRotateWidget(am.manipulatorManager().manipulatorExt(DkManipulatorManager::m_rotate), this);

	setObjectName("DkPreferenceTabs");
	createLayout();

	for (QAction* a : am.manipulatorActions())
		connect(a, SIGNAL(triggered()), this, SLOT(selectManipulator()), Qt::UniqueConnection);
}

void DkManipulatorWidget::createLayout() {

	// actions
	QWidget* actionWidget = new QWidget(this);
	QVBoxLayout* aLayout = new QVBoxLayout(actionWidget);
	aLayout->setAlignment(Qt::AlignTop);
	aLayout->setContentsMargins(0, 0, 0, 0);
	aLayout->setSpacing(0);

	QButtonGroup* group = new QButtonGroup(this);

	DkActionManager& am = DkActionManager::instance();
	for (QAction* a : am.manipulatorActions()) {

		DkTabEntryWidget* mpl = new DkTabEntryWidget(a->icon(), a->text(), this);
		connect(mpl, SIGNAL(clicked()), a, SIGNAL(triggered()), Qt::UniqueConnection);	// TODO: different connection if ManipulatorExt?
		
		aLayout->addWidget(mpl);
		group->addButton(mpl);
	}

	QString scrollbarStyle = 
		QString("QScrollBar:vertical {border: 1px solid " + DkUtils::colorToString(DkSettingsManager::param().display().hudFgdColor) + "; background: rgba(0,0,0,0); width: 7px; margin: 0 0 0 0;}")
		+ QString("QScrollBar::handle:vertical {background: " + DkUtils::colorToString(DkSettingsManager::param().display().hudFgdColor) + "; min-height: 0px;}")
		+ QString("QScrollBar::add-line:vertical {height: 0px;}")
		+ QString("QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: rgba(0,0,0,0); width: 1px;}")
		+ QString("QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {height: 0;}");

	QScrollArea* actionScroller = new QScrollArea(this);
	actionScroller->setStyleSheet(scrollbarStyle + actionScroller->styleSheet());
	actionScroller->setWidgetResizable(true);
	actionScroller->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	actionScroller->setWidget(actionWidget);
	actionScroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	// preview
	mPreview = new QLabel(this);

	// undo
	QPixmap pm = DkImage::colorizePixmap(QIcon(":/nomacs/img/rotate-cc.svg").pixmap(QSize(32, 32)), QColor(255, 255, 255));
	QPushButton* undoButton = new QPushButton(pm, "", this);
	undoButton->setFlat(true);
	undoButton->setIconSize(QSize(32, 32));
	undoButton->setObjectName("DkRestartButton");
	undoButton->setStatusTip(tr("Undo"));
	connect(undoButton, SIGNAL(clicked()), am.action(DkActionManager::menu_edit_undo), SIGNAL(triggered()));

	pm = DkImage::colorizePixmap(QIcon(":/nomacs/img/rotate-cw.svg").pixmap(QSize(32, 32)), QColor(255, 255, 255));
	QPushButton* redoButton = new QPushButton(pm, "", this);
	redoButton->setFlat(true);
	redoButton->setIconSize(QSize(32, 32));
	redoButton->setObjectName("DkRestartButton");
	redoButton->setStatusTip(tr("Undo"));
	connect(undoButton, SIGNAL(clicked()), am.action(DkActionManager::menu_edit_redo), SIGNAL(triggered()));

	QWidget* buttonWidget = new QWidget(this);
	QHBoxLayout* buttonLayout = new QHBoxLayout(buttonWidget);
	buttonLayout->setContentsMargins(0, 0, 0, 0);
	buttonLayout->addWidget(undoButton);
	buttonLayout->addWidget(redoButton);

	QWidget* mplWidget = new QWidget(this);
	QVBoxLayout* mplLayout = new QVBoxLayout(mplWidget);
	mplLayout->setAlignment(Qt::AlignTop);
	
	for (QWidget* w : mWidgets) 
		mplLayout->addWidget(w);
	mplLayout->addWidget(mPreview);
	mplLayout->addWidget(buttonWidget);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(actionScroller);
	layout->addWidget(mplWidget);
}

QImage DkManipulatorWidget::scaledPreview(const QImage& img) const {

	QImage imgR;

	if (img.height() > img.width())
		imgR = img.scaledToHeight(qMin(img.height(), mMaxPreview));
	else
		imgR = img.scaledToWidth(qMin(img.width(), mMaxPreview));

	return imgR;
}

void DkManipulatorWidget::setImage(QSharedPointer<DkImageContainerT> imgC) {
	mImgC = imgC;

	if (mImgC) {

		QImage img = mImgC->imageScaledToWidth(qMin(mPreview->width(), 300));
		img = scaledPreview(img);

		mPreview->setPixmap(QPixmap::fromImage(img));
		mPreview->show();
	}
	else
		mPreview->hide();
}

void DkManipulatorWidget::selectManipulator() {

	QAction* action = dynamic_cast<QAction*>(QObject::sender());

	DkActionManager& am = DkActionManager::instance();
	QSharedPointer<DkBaseManipulator> mpl = am.manipulatorManager().manipulator(action);
	QSharedPointer<DkBaseManipulatorExt> mplExt = qSharedPointerDynamicCast<DkBaseManipulatorExt>(mpl);

	// compute preview
	if (mpl && mImgC) {
		DkTimer dt;
		QImage img = mpl->apply(mImgC->imageScaledToWidth(qMin(mPreview->width(), 300)));
		img = scaledPreview(img);

		if (!img.isNull())
			mPreview->setPixmap(QPixmap::fromImage(img));
		qDebug() << "preview computed in " << dt;
	}

	for (QWidget* w : mWidgets)
		w->hide();

	if (!mplExt)
		return;

	mplExt->widget()->show();
}

// DkMainpulatorDoc --------------------------------------------------------------------
DkEditDock::DkEditDock(const QString& title, QWidget* parent, Qt::WindowFlags flags) : DkDockWidget(title, parent, flags) {

	createLayout();
}

void DkEditDock::createLayout() {

	mMplWidget = new DkManipulatorWidget(this);
	setWidget(mMplWidget);
}

void DkEditDock::setImage(QSharedPointer<DkImageContainerT> imgC) {
	mMplWidget->setImage(imgC);
}

// DkManipulatorWidget --------------------------------------------------------------------
DkBaseManipulatorWidget::DkBaseManipulatorWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget* parent) : DkWidget(parent) {
	mBaseManipulator = manipulator;
}

QSharedPointer<DkBaseManipulatorExt> DkBaseManipulatorWidget::baseManipulator() const {
	return mBaseManipulator;
}

// DkTinyPlanetWidget --------------------------------------------------------------------
DkTinyPlanetWidget::DkTinyPlanetWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget* parent) : DkBaseManipulatorWidget(manipulator, parent) {
	createLayout();
	QMetaObject::connectSlotsByName(this);

	manipulator->setWidget(this);
}

void DkTinyPlanetWidget::createLayout() {

	// post processing sliders
	DkSlider* scaleSlider = new DkSlider(tr("Planet Size"), this);
	scaleSlider->setObjectName("scaleSlider");
	scaleSlider->setMinimum(1);
	scaleSlider->setMaximum(1000);
	scaleSlider->setValue(manipulator()->size());

	DkSlider* angleSlider = new DkSlider(tr("Angle"), this);
	angleSlider->setObjectName("angleSlider");
	angleSlider->setValue(manipulator()->angle());
	angleSlider->setMinimum(-180);
	angleSlider->setMaximum(179);

	QCheckBox* invertBox = new QCheckBox(tr("Invert Planet"), this);
	invertBox->setObjectName("invertBox");
	invertBox->setChecked(manipulator()->inverted());

	QVBoxLayout* sliderLayout = new QVBoxLayout(this);
	sliderLayout->addWidget(scaleSlider);
	sliderLayout->addWidget(angleSlider);
	sliderLayout->addWidget(invertBox);
}

void DkTinyPlanetWidget::on_scaleSlider_valueChanged(int val) {
	manipulator()->setSize(val);
}

void DkTinyPlanetWidget::on_angleSlider_valueChanged(int val) {
	manipulator()->setAngle(val);
}

void DkTinyPlanetWidget::on_invertBox_toggled(bool val) {
	manipulator()->setInverted(val);
}

QSharedPointer<DkTinyPlanetManipulator> DkTinyPlanetWidget::manipulator() const {
	return qSharedPointerDynamicCast<DkTinyPlanetManipulator>(baseManipulator());
}

// DkUnsharlpMaskWidget --------------------------------------------------------------------
DkUnsharpMaskWidget::DkUnsharpMaskWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget* parent) : DkBaseManipulatorWidget(manipulator, parent) {
	createLayout();
	QMetaObject::connectSlotsByName(this);

	manipulator->setWidget(this);
}

void DkUnsharpMaskWidget::createLayout() {

	// post processing sliders
	DkSlider* sigmaSlider = new DkSlider(tr("Sigma"), this);
	sigmaSlider->setObjectName("angleSlider");
	sigmaSlider->setValue(manipulator()->sigma());
	//darkenSlider->hide();

	DkSlider* amountSlider = new DkSlider(tr("Amount"), this);
	amountSlider->setObjectName("amountSlider");
	amountSlider->setValue(manipulator()->amount());

	QVBoxLayout* sliderLayout = new QVBoxLayout(this);
	sliderLayout->addWidget(sigmaSlider);
	sliderLayout->addWidget(amountSlider);
}

void DkUnsharpMaskWidget::on_sigmaSlider_valueChanged(int val) {
	manipulator()->setSigma(val);
}

void DkUnsharpMaskWidget::on_amountSlider_valueChanged(int val) {
	manipulator()->setAmount(val);
}

QSharedPointer<DkUnsharpMaskManipulator> DkUnsharpMaskWidget::manipulator() const {
	return qSharedPointerDynamicCast<DkUnsharpMaskManipulator>(baseManipulator());
}

// DkRotateWidget --------------------------------------------------------------------
DkRotateWidget::DkRotateWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget* parent) : DkBaseManipulatorWidget(manipulator, parent) {
	createLayout();
	QMetaObject::connectSlotsByName(this);

	manipulator->setWidget(this);
}


QSharedPointer<DkRotateManipulator> DkRotateWidget::manipulator() const {
	return qSharedPointerDynamicCast<DkRotateManipulator>(baseManipulator());
}

void DkRotateWidget::createLayout() {

	DkSlider* angleSlider = new DkSlider(tr("Angle"), this);
	angleSlider->setObjectName("angleSlider");
	angleSlider->setValue(manipulator()->angle());
	angleSlider->setMinimum(-180);
	angleSlider->setMaximum(180);

	QVBoxLayout* sliderLayout = new QVBoxLayout(this);
	sliderLayout->addWidget(angleSlider);
}
void DkRotateWidget::on_angleSlider_valueChanged(int val) {
	manipulator()->setAngle(val);
}



}