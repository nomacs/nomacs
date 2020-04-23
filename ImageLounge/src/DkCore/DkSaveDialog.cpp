/*******************************************************************************************************
 DkSaveDialog.cpp
 Created on:	03.07.2013
 
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

#include "DkSaveDialog.h"
#include "DkUtils.h"
#include "DkBasicWidgets.h"
#include "DkBasicLoader.h"
#include "DkBaseViewPort.h"
#include "DkSettings.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QBuffer>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QCheckBox>
#include <QSettings>
#include <QComboBox>
#include <QDebug>
#include <QLabel>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

// tiff dialog --------------------------------------------------------------------
DkTifDialog::DkTifDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {
	init();
}

void DkTifDialog::init() {

	isOk = false;
	setWindowTitle("TIF compression");
	//setFixedSize(270, 146);
	setLayout(new QVBoxLayout(this));

	//QWidget* buttonWidget = new QWidget(this);
	QGroupBox* buttonWidget = new QGroupBox(tr("TIF compression"), this);
	QVBoxLayout* vBox = new QVBoxLayout(buttonWidget);
	QButtonGroup* bGroup = new QButtonGroup(buttonWidget);
	noCompressionButton = new QRadioButton( tr("&no compression"), this);
	compressionButton = new QRadioButton(tr("&LZW compression (lossless)"), this);
	compressionButton->setChecked(true);
	bGroup->addButton(noCompressionButton);
	bGroup->addButton(compressionButton);

	vBox->addWidget(noCompressionButton);
	vBox->addWidget(compressionButton);

	// mButtons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	layout()->addWidget(buttonWidget);
	layout()->addWidget(buttons);
}

int DkTifDialog::getCompression() const {

	return (noCompressionButton->isChecked()) ? 0 : 1;
}

// DkCompressionDialog --------------------------------------------------------------------
DkCompressDialog::DkCompressDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	setObjectName("DkCompressionDialog");
	createLayout();
	init();

	resize(DkUtils::getInitialDialogSize());
}

DkCompressDialog::~DkCompressDialog() {
	
	// save settings
	saveSettings();
}

void DkCompressDialog::saveSettings() {

	DefaultSettings settings;
	settings.beginGroup(objectName());
	settings.setValue("CompressionCombo" + QString::number(mDialogMode), mCompressionCombo->currentIndex());
	
	if (mDialogMode != webp_dialog)
		settings.setValue("bgCompressionColor" + QString::number(mDialogMode), getBackgroundColor().rgba());
	settings.endGroup();
}


void DkCompressDialog::loadSettings() {

	DefaultSettings settings;
	settings.beginGroup(objectName());

	mBgCol = settings.value("bgCompressionColor" + QString::number(mDialogMode), QColor(255,255,255).rgba()).toInt();
	int cIdx = settings.value("CompressionCombo" + QString::number(mDialogMode), 0).toInt();

	if (cIdx >= 0 && cIdx < mCompressionCombo->count())
		mCompressionCombo->setCurrentIndex(cIdx);
	mColChooser->setColor(mBgCol);
	newBgCol();
	settings.endGroup();
}

void DkCompressDialog::resizeEvent(QResizeEvent * ev) {

	drawPreview();
	QDialog::resizeEvent(ev);
}

void DkCompressDialog::init() {

	mHasAlpha = false;

	if (mDialogMode == jpg_dialog || mDialogMode == j2k_dialog) {

		if (mDialogMode == jpg_dialog)
			setWindowTitle(tr("JPG Settings"));
		else
			setWindowTitle(tr("J2K Settings"));

		mCompressionCombo->show();
		mColChooser->show();
		mCbLossless->hide();
		mSizeCombo->hide();
		mCompressionCombo->setEnabled(true);
	}
	else if (mDialogMode == webp_dialog) {
		setWindowTitle(tr("WebP Settings"));
		mColChooser->setEnabled(false);
		mCompressionCombo->show();
		mColChooser->show();

#if QT_VERSION < 0x050000
		mCbLossless->show();
#endif
		mSizeCombo->hide();
		losslessCompression(mCbLossless->isChecked());
	}
	else if (mDialogMode == web_dialog) {

		setWindowTitle(tr("Save for Web"));

		mSizeCombo->show();
		mCompressionCombo->hide();
		mColChooser->hide();
		mCbLossless->hide();
	}
	loadSettings();

}

void DkCompressDialog::createLayout() {

	QLabel* origLabelText = new QLabel(tr("Original"), this);
	origLabelText->setAlignment(Qt::AlignHCenter);
	QLabel* newLabel = new QLabel(tr("New"), this);
	newLabel->setAlignment(Qt::AlignHCenter);

	// shows the original image
	mOrigView = new DkBaseViewPort(this);
	mOrigView->setForceFastRendering(true);
	mOrigView->setPanControl(QPointF(0.0f, 0.0f));
	connect(mOrigView, SIGNAL(imageUpdated()), this, SLOT(drawPreview()));

	//// maybe we should report this: 
	//// if a stylesheet (with border) is set, the var
	//// cornerPaintingRect in QAbstractScrollArea (which we don't even need : )
	//// is invalid which blocks re-paints unless the widget gets a focus...
	//origView->setStyleSheet("QViewPort{border: 1px solid #888;}");

	// shows the preview
	mPreviewLabel = new QLabel(this);
	mPreviewLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);

	// size combo for web
	mSizeCombo = new QComboBox(this);
	mSizeCombo->addItem(tr("Small  (800 x 600)"), 800);
	mSizeCombo->addItem(tr("Medium (1024 x 786)"), 1024);
	mSizeCombo->addItem(tr("Large  (1920 x 1080)"), 1920);
	mSizeCombo->addItem(tr("Original Size"), -1);
	connect(mSizeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changeSizeWeb(int)));

	mCompressionCombo = new QComboBox(this);
	mCompressionCombo->addItem(tr("Best Quality"), 100);
	mCompressionCombo->addItem(tr("High Quality"), 97);
	mCompressionCombo->addItem(tr("Medium Quality"), 90);
	mCompressionCombo->addItem(tr("Low Quality"), 80);
	mCompressionCombo->addItem(tr("Bad Quality"), 60);
	mCompressionCombo->setCurrentIndex(1);
	connect(mCompressionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(drawPreview()));

	// lossless
	mCbLossless = new QCheckBox(tr("Lossless Compression"), this);
	connect(mCbLossless, SIGNAL(toggled(bool)), this, SLOT(losslessCompression(bool)));

	mPreviewSizeLabel = new QLabel();
	mPreviewSizeLabel->setAlignment(Qt::AlignRight);

	// color chooser
	mColChooser = new DkColorChooser(mBgCol, tr("Background Color"), this);
	mColChooser->setVisible(mHasAlpha);
	mColChooser->enableAlpha(false);
	connect(mColChooser, SIGNAL(accepted()), this, SLOT(newBgCol()));

	QWidget* previewWidget = new QWidget(this);
	QGridLayout* previewLayout = new QGridLayout(previewWidget);
	previewLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
	previewLayout->setColumnStretch(0,1);
	previewLayout->setColumnStretch(1,1);

	previewLayout->addWidget(origLabelText, 0, 0);
	previewLayout->addWidget(newLabel, 0, 1);
	previewLayout->addWidget(mOrigView, 1, 0);
	previewLayout->addWidget(mPreviewLabel, 1, 1);
	previewLayout->addWidget(mCompressionCombo, 2, 0);
	previewLayout->addWidget(mColChooser, 2, 1, 1, 3);
	previewLayout->addWidget(mCbLossless, 3, 0);
	previewLayout->addWidget(mSizeCombo, 4, 0);
	previewLayout->addWidget(mPreviewSizeLabel, 5, 1);

	// mButtons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	buttons->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
	buttons->button(QDialogButtonBox::Ok)->setAutoDefault(true);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(previewWidget);
	layout->addWidget(buttons);

	mCompressionCombo->setFocus();

}

void DkCompressDialog::updateSnippets() {

	if (mImg.isNull() || !isVisible())
		return;

	mOrigView->setImage(mImg);
	mOrigView->fullView();
	mOrigView->zoomConstraints(mOrigView->get100Factor());

}

void DkCompressDialog::drawPreview() {

	if (mImg.isNull() || !isVisible())
		return;

	QImage origImg = mOrigView->getCurrentImageRegion();
	mNewImg = QImage(origImg.size(), QImage::Format_ARGB32);

	if ((mDialogMode == jpg_dialog || mDialogMode == j2k_dialog) && mHasAlpha)
		mNewImg.fill(mBgCol.rgb());
	else if ((mDialogMode == jpg_dialog || mDialogMode == web_dialog) && !mHasAlpha)
		mNewImg.fill(palette().color(QPalette::Background).rgb());
	else
		mNewImg.fill(QColor(0,0,0,0).rgba());
	 
	QPainter bgPainter(&mNewImg);
	bgPainter.drawImage(origImg.rect(), origImg, origImg.rect());
	bgPainter.end();

	if (mDialogMode == jpg_dialog) {
		// pre-compute the jpg compression
		QByteArray ba;
		QBuffer buffer(&ba);
		buffer.open(QIODevice::ReadWrite);
		mNewImg.save(&buffer, "JPG", getCompression());
		mNewImg.loadFromData(ba, "JPG");
		updateFileSizeLabel((float)ba.size(), origImg.size());
	}
	else if (mDialogMode == j2k_dialog) {
		// pre-compute the jpg compression
		QByteArray ba;
		QBuffer buffer(&ba);
		buffer.open(QIODevice::ReadWrite);
		mNewImg.save(&buffer, "J2K", getCompression());
		mNewImg.loadFromData(ba, "J2K");
		updateFileSizeLabel((float)ba.size(), origImg.size());
		qDebug() << "using j2k...";
	}
	else if (mDialogMode == webp_dialog && getCompression() != -1) {
		// pre-compute the jpg compression
		QByteArray ba;
		QBuffer buffer(&ba);
		buffer.open(QIODevice::ReadWrite);
		mNewImg.save(&buffer, "WEBP", getCompression());
		mNewImg.loadFromData(ba, "WEBP");
		updateFileSizeLabel((float)ba.size(), origImg.size());
		qDebug() << "using webp...";
	}
	else if (mDialogMode == web_dialog) {

		float factor = getResizeFactor();
		if (factor != -1)
			mNewImg = DkImage::resizeImage(mNewImg, QSize(), factor, DkImage::ipl_area);

		if (!mHasAlpha) {
			// pre-compute the jpg compression
			QByteArray ba;
			QBuffer buffer(&ba);
			buffer.open(QIODevice::ReadWrite);
			mNewImg.save(&buffer, "JPG", getCompression());
			mNewImg.loadFromData(ba, "JPG");
			updateFileSizeLabel((float)ba.size(), origImg.size(), factor);
		}
		else {
			// pre-compute the png compression
			QByteArray ba;
			QBuffer buffer(&ba);
			buffer.open(QIODevice::ReadWrite);
			mNewImg.save(&buffer, "PNG");
			mNewImg.loadFromData(ba, "PNG");
			updateFileSizeLabel((float)ba.size(), origImg.size(), factor);
		}
	}
	else
		updateFileSizeLabel();

	//previewLabel->setScaledContents(true);
	QImage img = mNewImg.scaled(mPreviewLabel->size(), Qt::KeepAspectRatio, Qt::FastTransformation);
	mPreviewLabel->setPixmap(QPixmap::fromImage(img));
}

void DkCompressDialog::updateFileSizeLabel(float bufferSize, QSize bufferImgSize, float factor) {

	if (bufferImgSize.isEmpty())
		bufferImgSize = mNewImg.size();

	if (mImg.isNull() || bufferSize == -1 || bufferImgSize.isNull()) {
		mPreviewSizeLabel->setText(tr("File Size: --"));
		mPreviewSizeLabel->setEnabled(false);
		return;
	}
	mPreviewSizeLabel->setEnabled(true);

	if (factor == -1.0f)
		factor = 1.0f;

	float depth = (mDialogMode == jpg_dialog || mDialogMode == j2k_dialog || (mDialogMode == web_dialog && mHasAlpha)) ? 24.0f : (float)mImg.depth();	// jpg uses always 24 bit
	
	float rawBufferSize = bufferImgSize.width()*bufferImgSize.height()*depth/8.0f;
	float rawImgSize = factor*(mImg.width()*mImg.height()*depth/8.0f);

	mPreviewSizeLabel->setText(tr("File Size: ~%1").arg(DkUtils::readableByte(rawImgSize*bufferSize/rawBufferSize)));
}

void DkCompressDialog::imageHasAlpha(bool hasAlpha) {
	mHasAlpha = hasAlpha;
	mColChooser->setVisible(hasAlpha);
}

QColor DkCompressDialog::getBackgroundColor() const {
	return mBgCol;
}

int DkCompressDialog::getCompression() {

	int compression = -1;
	if ((mDialogMode == jpg_dialog || !mCbLossless->isChecked()) && mDialogMode != web_dialog)
		compression = mCompressionCombo->itemData(mCompressionCombo->currentIndex()).toInt();
	else if (mDialogMode == web_dialog)
		compression = 80;

	return compression;
}

float DkCompressDialog::getResizeFactor() {

	float factor = -1;
	float finalEdge = (float)mSizeCombo->itemData(mSizeCombo->currentIndex()).toInt();
	float maxEdge = (float)std::max(mImg.width(), mImg.height());

	if (finalEdge != -1 && maxEdge > finalEdge)
		factor = finalEdge/maxEdge;

	return factor;
}

void DkCompressDialog::setImage(const QImage& img) {
	mImg = img;
	updateSnippets();
	drawPreview();
}

void DkCompressDialog::setDialogMode(int dialogMode) {
	mDialogMode = dialogMode;
	init();
}

void DkCompressDialog::accept() {

	saveSettings();

	QDialog::accept();
}

// slots
void DkCompressDialog::setVisible(bool visible) {

	QDialog::setVisible(visible);

	if (visible) {
		updateSnippets();
		drawPreview();
		mOrigView->zoomConstraints(mOrigView->get100Factor());
	}
}

void DkCompressDialog::newBgCol() {
	
	mBgCol = mColChooser->getColor();
	drawPreview();
}

void DkCompressDialog::losslessCompression(bool lossless) {

	mCompressionCombo->setEnabled(!lossless);
	drawPreview();
}

void DkCompressDialog::changeSizeWeb(int) {
	drawPreview();
}

}
