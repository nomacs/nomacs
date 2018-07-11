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
#include "DkUtils.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QLabel>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QColorDialog>
#pragma warning(pop)

namespace nmc {

// DkSlider --------------------------------------------------------------------
DkSlider::DkSlider(QString title, QWidget* parent) : QWidget(parent) {

	createLayout();

	// init
	titleLabel->setText(title);

	// defaults
	setMinimum(0);	
	setMaximum(100);
	setTickInterval(1);
	setValue(50);
}

void DkSlider::createLayout() {

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(0,0,0,0);

	QWidget* dummy = new QWidget(this);
	QHBoxLayout* titleLayout = new QHBoxLayout(dummy);
	titleLayout->setContentsMargins(0,0,0,5);

	QWidget* dummyBounds = new QWidget(this);
	QHBoxLayout* boundsLayout = new QHBoxLayout(dummyBounds);
	boundsLayout->setContentsMargins(0,0,0,0);

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

QSlider* DkSlider::getSlider() const {
	return slider;
}

void DkSlider::setMinimum(int minValue) {
	slider->setMinimum(minValue);
	sliderBox->setMinimum(minValue);
	minValLabel->setText(QString::number(minValue));
}

void DkSlider::setMaximum(int maxValue) {
	slider->setMaximum(maxValue);
	sliderBox->setMaximum(maxValue);
	maxValLabel->setText(QString::number(maxValue));
}

void DkSlider::setTickInterval(int ticValue) {
	slider->setTickInterval(ticValue);
}

int DkSlider::value() const {
	return slider->value();
}

void DkSlider::setFocus(Qt::FocusReason reason) {
	sliderBox->setFocus(reason);
}

void DkSlider::setValue(int value) {

	slider->blockSignals(true);
	slider->setValue(value);
	slider->blockSignals(false);

	sliderBox->blockSignals(true);
	sliderBox->setValue(value);
	sliderBox->blockSignals(false);

	emit valueChanged(value);
}

// DkDoubleSlider --------------------------------------------------------------------
DkDoubleSlider::DkDoubleSlider(const QString& title, QWidget* parent) : QWidget(parent) {

	createLayout();

	// init
	mTitleLabel->setText(title);

	// defaults
	setMinimum(0.0);	
	setMaximum(1.0);
	setTickInterval(0.01);
	setValue(0.5);
}

void DkDoubleSlider::createLayout() {

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(0,0,0,0);

	QWidget* dummy = new QWidget(this);
	QHBoxLayout* titleLayout = new QHBoxLayout(dummy);
	titleLayout->setContentsMargins(0,0,0,5);

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

QSlider* DkDoubleSlider::getSlider() const {
	return mSlider;
}

void DkDoubleSlider::setMinimum(double minValue) {
	mSliderBox->setMinimum(minValue);
}

void DkDoubleSlider::setMaximum(double maxValue) {
	mSliderBox->setMaximum(maxValue);
}

void DkDoubleSlider::setTickInterval(double ticValue) {
	mSlider->setMaximum(qRound(1.0 / ticValue));
}

double DkDoubleSlider::value() const {
	return mSliderBox->value();
}

void DkDoubleSlider::setFocus(Qt::FocusReason reason) {
	mSliderBox->setFocus(reason);
}

void DkDoubleSlider::setSliderInverted(bool inverted) {
	mSliderInverted = inverted;
}

void DkDoubleSlider::setValue(double value) {

	double dr = mSliderBox->maximum() - mSliderBox->minimum();
	double nVal = (value - mSliderBox->minimum()) / dr;

	if (mSliderInverted)
		nVal = 1.0 - nVal;
	
	int sVal = qRound(nVal * mSlider->maximum());

	mSlider->blockSignals(true);
	mSlider->setValue(sVal);
	mSlider->blockSignals(false);

	mSliderBox->blockSignals(true);
	mSliderBox->setValue(value);
	mSliderBox->blockSignals(false);

	emit valueChanged(value);
}

void DkDoubleSlider::setIntValue(int value) {

	double dr = mSliderBox->maximum() - mSliderBox->minimum();
	double nVal = value / (double)mSlider->maximum();
	if (mSliderInverted)
		nVal = 1.0 - nVal;
	double sVal = nVal * dr + mSliderBox->minimum();
	
	mSlider->blockSignals(true);
	mSlider->setValue(value);
	mSlider->blockSignals(false);

	mSliderBox->blockSignals(true);
	mSliderBox->setValue(sVal);
	mSliderBox->blockSignals(false);

	emit valueChanged(sVal);
}

// DkColorChooser ------------------------------------
DkColorChooser::DkColorChooser(QColor defaultColor, QString text, QWidget* parent, Qt::WindowFlags flags) : QWidget(parent, flags) {

	this->defaultColor = defaultColor;
	this->mText = text;

	init();

}

void DkColorChooser::init() {

	mAccepted = false;

	colorDialog = new QColorDialog(this);
	colorDialog->setObjectName("colorDialog");
	colorDialog->setOption(QColorDialog::ShowAlphaChannel, true);

	QVBoxLayout* vLayout = new QVBoxLayout(this);
	vLayout->setContentsMargins(11,0,11,0);

	QLabel* colorLabel = new QLabel(mText, this);
	colorButton = new QPushButton("", this);
	colorButton->setFlat(true);
	colorButton->setObjectName("colorButton");
	colorButton->setAutoDefault(false);

	QPushButton* resetButton = new QPushButton(tr("Reset"), this);
	resetButton->setObjectName("resetButton");
	resetButton->setAutoDefault(false);

	QWidget* colWidget = new QWidget(this);
	QHBoxLayout* hLayout = new QHBoxLayout(colWidget);
	hLayout->setContentsMargins(11,0,11,0);
	hLayout->setAlignment(Qt::AlignLeft);

	hLayout->addWidget(colorButton);
	hLayout->addWidget(resetButton);

	vLayout->addWidget(colorLabel);
	vLayout->addWidget(colWidget);

	setColor(defaultColor);
	QMetaObject::connectSlotsByName(this);
}

bool DkColorChooser::isAccept() const {
	return mAccepted;
}

void DkColorChooser::enableAlpha(bool enable) {
	colorDialog->setOption(QColorDialog::ShowAlphaChannel, enable);
}

void DkColorChooser::setColor(const QColor& color) {

	colorDialog->setCurrentColor(color);
	colorButton->setStyleSheet("QPushButton {background-color: " + DkUtils::colorToString(color) + "; border: 1px solid #888; min-height: 24px}");
	if (mSettingColor)
		*mSettingColor = color;

}

void DkColorChooser::setColor(QColor* color) {

	if (color) {
		mSettingColor = color;
		setColor(*color);
	}
}


QColor DkColorChooser::getColor() {
	return colorDialog->currentColor();
}

void DkColorChooser::on_resetButton_clicked() {
	setColor(defaultColor);
	emit resetClicked();
}

void DkColorChooser::on_colorButton_clicked() {

	// incorrect color? - see: https://bugreports.qt.io/browse/QTBUG-42988
	colorDialog->show();
}

void DkColorChooser::on_colorDialog_accepted() {

	setColor(colorDialog->currentColor());
	mAccepted = true;
	emit accepted();
}

DkRectWidget::DkRectWidget(const QRect& r, QWidget* parent) : QWidget(parent) {

	createLayout();
	setRect(r);
}

void DkRectWidget::setRect(const QRect & r) {

	mSpCropRect[crop_x]->setValue(r.x());
	mSpCropRect[crop_y]->setValue(r.y());
	mSpCropRect[crop_width]->setValue(r.width());
	mSpCropRect[crop_height]->setValue(r.height());
}

QRect DkRectWidget::rect() const {
	return QRect(mSpCropRect[crop_x]->value(),
		mSpCropRect[crop_y]->value(),
		mSpCropRect[crop_width]->value(),
		mSpCropRect[crop_height]->value());
}

void DkRectWidget::updateRect() {

	emit updateRectSignal(rect());
}

void DkRectWidget::createLayout() {

	mSpCropRect.resize(crop_end);

	QLabel* lbCropX = new QLabel(tr("x:"));
	mSpCropRect[crop_x] = new QSpinBox(this);
	lbCropX->setBuddy(mSpCropRect[crop_x]);

	QLabel* lbCropY = new QLabel(tr("y:"));
	mSpCropRect[crop_y] = new QSpinBox(this);
	lbCropY->setBuddy(mSpCropRect[crop_y]);

	QLabel* lbCropWidth = new QLabel(tr("width:"));
	mSpCropRect[crop_width] = new QSpinBox(this);
	lbCropWidth->setBuddy(mSpCropRect[crop_width]);

	QLabel* lbCropHeight = new QLabel(tr("height:"));
	mSpCropRect[crop_height] = new QSpinBox(this);
	lbCropHeight->setBuddy(mSpCropRect[crop_height]);

	for (QSpinBox* sp : mSpCropRect) {
		sp->setSuffix(tr(" px"));
		sp->setMinimum(0);
		sp->setMaximum(100000);
		connect(sp, SIGNAL(valueChanged(int)), this, SLOT(updateRect()));
	}

	QHBoxLayout* cropLayout = new QHBoxLayout(this);
	cropLayout->setMargin(0);
	cropLayout->addWidget(lbCropX);
	cropLayout->addWidget(mSpCropRect[crop_x]);
	cropLayout->addWidget(lbCropY);
	cropLayout->addWidget(mSpCropRect[crop_y]);
	cropLayout->addWidget(lbCropWidth);
	cropLayout->addWidget(mSpCropRect[crop_width]);
	cropLayout->addWidget(lbCropHeight);
	cropLayout->addWidget(mSpCropRect[crop_height]);
}

}