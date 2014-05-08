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

	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	layout()->addWidget(buttonWidget);
	layout()->addWidget(buttons);
}


// DkCompressionDialog --------------------------------------------------------------------
DkCompressDialog::DkCompressDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	dialogMode = jpg_dialog;	// default
	bgCol = QColor(255,255,255);

	setObjectName("DkCompressionDialog");
	createLayout();
	init();
}

DkCompressDialog::~DkCompressDialog() {
	saveSettings();
}

void DkCompressDialog::saveSettings() {

	QSettings settings;
	settings.beginGroup(objectName());
	settings.setValue("Compression" + QString::number(dialogMode), getCompression());
	
	if (dialogMode != webp_dialog)
		settings.setValue("BackgroundColor" + QString::number(dialogMode), getBackgroundColor());
}


void DkCompressDialog::loadSettings() {

	qDebug() << "loading new settings...";

	QSettings settings;
	settings.beginGroup(objectName());

	bgCol = settings.value("BackgroundColor" + QString::number(dialogMode), QColor(255,255,255)).value<QColor>();
	int compression = settings.value("Compression" + QString::number(dialogMode), 80).toInt();

	slider->setValue(compression);
	colChooser->setColor(bgCol);
	newBgCol();
}

void DkCompressDialog::init() {

	hasAlpha = false;
	img = 0;

	if (dialogMode == jpg_dialog || dialogMode == j2k_dialog) {

		if (dialogMode == jpg_dialog)
			setWindowTitle(tr("JPG Settings"));
		else
			setWindowTitle(tr("J2K Settings"));

		slider->show();
		colChooser->show();
		cbLossless->hide();
		sizeCombo->hide();
		slider->setEnabled(true);
	}
	else if (dialogMode == webp_dialog) {
		setWindowTitle(tr("WebP Settings"));
		colChooser->setEnabled(false);
		slider->show();
		colChooser->show();
		cbLossless->show();
		sizeCombo->hide();
		losslessCompression(cbLossless->isChecked());
	}
	else if (dialogMode == web_dialog) {

		setWindowTitle(tr("Save for Web"));

		sizeCombo->show();
		slider->hide();
		colChooser->hide();
		cbLossless->hide();
	}

	loadSettings();

}

void DkCompressDialog::createLayout() {

	QLabel* origLabelText = new QLabel(tr("Original"), this);
	origLabelText->setAlignment(Qt::AlignHCenter);
	QLabel* newLabel = new QLabel(tr("New"), this);
	newLabel->setAlignment(Qt::AlignHCenter);

	// shows the original image
	origView = new DkBaseViewPort(this);
	//origView->resize(80, 80);
	//origView->setMinimumSize(20,20);
	origView->setForceFastRendering(true);
	origView->setPanControl(QPointF(0.0f, 0.0f));
	connect(origView, SIGNAL(imageUpdated()), this, SLOT(drawPreview()));

	//// maybe we should report this: 
	//// if a stylesheet (with border) is set, the var
	//// cornerPaintingRect in QAbstractScrollArea (which we don't even need : )
	//// is invalid which blocks re-paints unless the widget gets a focus...
	//origView->setStyleSheet("QViewPort{border: 1px solid #888;}");

	// shows the preview
	previewLabel = new QLabel(this);
	//origView->setMinimumSize(20,20);
	previewLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);
	//previewLabel->setStyleSheet("QLabel{border: 1px solid #888;}");

	// size combo for web
	sizeCombo = new QComboBox(this);
	sizeCombo->addItem(tr("Small  (800 x 600)"), 600);
	sizeCombo->addItem(tr("Medium (1024 x 786)"), 786);
	sizeCombo->addItem(tr("Large  (1920 x 1080)"), 1080);
	sizeCombo->addItem(tr("Original Size"), -1);
	connect(sizeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changeSizeWeb(int)));

	// slider
	slider = new DkSlider(tr("Image Quality"), this);
	slider->setValue(80);
	slider->setTickInterval(10);
	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(drawPreview()));

	// lossless
	cbLossless = new QCheckBox(tr("Lossless Compression"), this);
	connect(cbLossless, SIGNAL(toggled(bool)), this, SLOT(losslessCompression(bool)));

	previewSizeLabel = new QLabel();
	previewSizeLabel->setAlignment(Qt::AlignRight);

	// color chooser
	colChooser = new DkColorChooser(bgCol, tr("Background Color"), this);
	colChooser->setEnabled(hasAlpha);
	colChooser->enableAlpha(false);
	connect(colChooser, SIGNAL(accepted()), this, SLOT(newBgCol()));

	//QWidget* dummy = new QWidget();
	//QHBoxLayout* dummyLayout = new QHBoxLayout(dummy);
	//dummyLayout->addWidget(colChooser);
	//dummyLayout->addStretch();
	//dummyLayout->addWidget(previewSizeLabel);

	QWidget* previewWidget = new QWidget(this);
	QGridLayout* previewLayout = new QGridLayout(previewWidget);
	previewLayout->setAlignment(Qt::AlignHCenter);
	previewLayout->setColumnStretch(0,1);
	previewLayout->setColumnStretch(1,1);

	previewLayout->addWidget(origLabelText, 0, 0);
	previewLayout->addWidget(newLabel, 0, 1);
	previewLayout->addWidget(origView, 1, 0);
	previewLayout->addWidget(previewLabel, 1, 1);
	previewLayout->addWidget(slider, 2, 0);
	previewLayout->addWidget(colChooser, 2, 1);
	previewLayout->addWidget(cbLossless, 3, 0);
	previewLayout->addWidget(sizeCombo, 4, 0);
	previewLayout->addWidget(previewSizeLabel, 4, 1);

	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	buttons->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
	buttons->button(QDialogButtonBox::Ok)->setAutoDefault(true);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(previewWidget);
	//layout->addStretch(30);
	layout->addWidget(buttons);

	//slider->setFocus(Qt::ActiveWindowFocusReason);

}

void DkCompressDialog::updateSnippets() {

	if (!img || !isVisible())
		return;

	// TODO: no pointer -> threads
	origView->setImage(*img);
	origView->fullView();
	origView->zoomConstraints(origView->get100Factor());

	//// fix layout issues - sorry
	//origView->setFixedWidth(width()*0.5f-30);
	//previewLabel->setFixedWidth(origView->width());
}

void DkCompressDialog::drawPreview() {

	if (!img || !isVisible())
		return;

	QImage origImg = origView->getCurrentImageRegion();
	qDebug() << "orig img size: " << origImg.size();
	qDebug() << "min size: " << origView->minimumSize();
	newImg = QImage(origImg.size(), QImage::Format_ARGB32);

	if ((dialogMode == jpg_dialog || dialogMode == j2k_dialog) && hasAlpha)
		newImg.fill(bgCol.rgb());
	else if ((dialogMode == jpg_dialog || dialogMode == web_dialog) && !hasAlpha)
		newImg.fill(palette().color(QPalette::Background));
	else
		newImg.fill(QColor(0,0,0,0));
	 
	QPainter bgPainter(&newImg);
	bgPainter.drawImage(origImg.rect(), origImg, origImg.rect());
	bgPainter.end();

	if (dialogMode == jpg_dialog) {
		// pre-compute the jpg compression
		QByteArray ba;
		QBuffer buffer(&ba);
		buffer.open(QIODevice::ReadWrite);
		newImg.save(&buffer, "JPG", slider->value());
		newImg.loadFromData(ba, "JPG");
		updateFileSizeLabel(ba.size(), origImg.size());
	}
	else if (dialogMode == j2k_dialog) {
		// pre-compute the jpg compression
		QByteArray ba;
		QBuffer buffer(&ba);
		buffer.open(QIODevice::ReadWrite);
		newImg.save(&buffer, "J2K", slider->value());
		newImg.loadFromData(ba, "J2K");
		updateFileSizeLabel(ba.size(), origImg.size());
		qDebug() << "using j2k...";
	}
	else if (dialogMode == webp_dialog && getCompression() != -1) {
		// pre-compute the webp compression
		DkBasicLoader loader;
		QSharedPointer<QByteArray> buffer(new QByteArray());
		loader.saveWebPFile(newImg, buffer, getCompression(), 0);
		qDebug() << "webP buffer size: " << buffer->size();
		loader.loadWebPFile(QFileInfo(), buffer);
		newImg = loader.image();
		updateFileSizeLabel(buffer->size(), origImg.size());
	}
	else if (dialogMode == web_dialog) {

		float factor = getResizeFactor();
		if (factor != -1)
			newImg = DkImage::resizeImage(newImg, QSize(), factor, DkImage::ipl_area);

		if (!hasAlpha) {
			// pre-compute the jpg compression
			QByteArray ba;
			QBuffer buffer(&ba);
			buffer.open(QIODevice::ReadWrite);
			newImg.save(&buffer, "JPG", getCompression());
			newImg.loadFromData(ba, "JPG");
			updateFileSizeLabel(ba.size(), origImg.size(), factor);
		}
		else
			updateFileSizeLabel();
	}
	else
		updateFileSizeLabel();

	//previewLabel->setScaledContents(true);
	QImage img = newImg.scaled(previewLabel->size(), Qt::KeepAspectRatio, Qt::FastTransformation);
	previewLabel->setPixmap(QPixmap::fromImage(img));
}

void DkCompressDialog::updateFileSizeLabel(float bufferSize, QSize bufferImgSize, float factor) {

	if (bufferImgSize.isEmpty())
		bufferImgSize = newImg.size();

	if (img == 0 || bufferSize == -1 || bufferImgSize.isNull()) {
		previewSizeLabel->setText(tr("File Size: --"));
		previewSizeLabel->setEnabled(false);
		return;
	}
	previewSizeLabel->setEnabled(true);

	if (factor == -1.0f)
		factor = 1.0f;

	float depth = (dialogMode == jpg_dialog || dialogMode == j2k_dialog || (dialogMode == web_dialog && hasAlpha)) ? 24 : img->depth();	// jpg uses always 24 bit
	
	float rawBufferSize = bufferImgSize.width()*bufferImgSize.height()*depth/8.0f;
	float rawImgSize = factor*(img->width()*img->height()*depth/8.0f);

	//qDebug() << "I need: " << rawImgSize*bufferSize/rawBufferSize << " bytes because buffer size: " << bufferSize;
	//qDebug() << "new image: " << newImg.size() << " full image: " << img->size() << " depth: " << depth;

	previewSizeLabel->setText(tr("File Size: ~%1").arg(DkUtils::readableByte(rawImgSize*bufferSize/rawBufferSize)));
}

void DkCompressDialog::accept() {

	saveSettings();

	QDialog::accept();
}

}
