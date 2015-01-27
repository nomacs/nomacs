/*******************************************************************************************************
 DkSaveDialog.h
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

#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QButtonGroup>

#include "DkBaseViewPort.h"
#include "DkWidgets.h"

namespace nmc {

class DkTifDialog : public QDialog {
	Q_OBJECT

public:
	DkTifDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	int getCompression() {

		return (noCompressionButton->isChecked()) ? 0 : 1;
	};

	// TODO: make it a bit more stylish

protected:
	void init();
	QRadioButton* noCompressionButton;
	QRadioButton* compressionButton;
	bool isOk;

};

class DllExport DkCompressDialog : public QDialog {
	Q_OBJECT

public:

	enum {
		jpg_dialog,
		j2k_dialog,
		webp_dialog,
		web_dialog,

		dialog_end
	};

	DkCompressDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~DkCompressDialog();

	void imageHasAlpha(bool hasAlpha) {
		this->hasAlpha = hasAlpha;
		colChooser->setEnabled(hasAlpha);
	};

	QColor getBackgroundColor() {
		return bgCol;
	};

	int getCompression() {

		int compression = -1;
		if ((dialogMode == jpg_dialog || !cbLossless->isChecked()) && dialogMode != web_dialog)
			compression = slider->value();
		else if (dialogMode == web_dialog)
			compression = 80;

		return compression;
	};

	float getResizeFactor() {

		float factor = -1;
		float finalEdge = sizeCombo->itemData(sizeCombo->currentIndex()).toInt();
		float minEdge = std::min(img->width(), img->height());

		if (finalEdge != -1 && minEdge > finalEdge)
			factor = finalEdge/minEdge;

		qDebug() << "factor: " << factor;

		return factor;
	};


	void setImage(QImage* img) {
		this->img = img;
		updateSnippets();
		drawPreview();
	};

	void setDialogMode(int dialogMode) {
		this->dialogMode = dialogMode;
		init();
	};

	virtual void accept();

public slots:

	void setVisible(bool visible) {

		QDialog::setVisible(visible);

		if (visible) {
			updateSnippets();
			drawPreview();
			origView->zoomConstraints(origView->get100Factor());
		}
	};

protected slots:

	void newBgCol() {
		bgCol = colChooser->getColor();
		qDebug() << "new bg col...";
		drawPreview();
	};

	void losslessCompression(bool lossless) {

		slider->setEnabled(!lossless);
		drawPreview();
	};

	void changeSizeWeb(int) {
		drawPreview();
	};

	void drawPreview();

	void updateFileSizeLabel(float bufferSize = -1, QSize bufferImgSize = QSize(), float factor = -1);
	
protected:
	int dialogMode;
	bool hasAlpha;
	QColor bgCol;

	QCheckBox* cbLossless;
	DkSlider* slider;
	DkColorChooser* colChooser;
	QImage* img;
	QImage origImg;
	QImage newImg;
	QLabel* previewLabel;
	QLabel* previewSizeLabel;
	//QLabel* origLabel;
	DkBaseViewPort* origView;
	QComboBox* sizeCombo;

	void init();
	void createLayout();
	void updateSnippets();
	void saveSettings();
	void loadSettings();
};

};
