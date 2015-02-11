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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QDialog>
#pragma warning(pop)		// no warnings from includes - end

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport
#endif
#endif

// Qt defines
class QRadioButton;
class QCheckBox;
class QLabel;
class QComboBox;

namespace nmc {

// nomacs defines
class DkSlider;
class DkColorChooser;
class DkBaseViewPort;

class DllExport DkTifDialog : public QDialog {
	Q_OBJECT

public:
	DkTifDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	int getCompression() const;

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

	void imageHasAlpha(bool hasAlpha);
	QColor getBackgroundColor() const;
	int getCompression();
	float getResizeFactor();
	void setImage(QImage* img);
	void setDialogMode(int dialogMode);
	virtual void accept();

public slots:
	void setVisible(bool visible);

protected slots:
	void newBgCol();
	void losslessCompression(bool lossless);
	void changeSizeWeb(int);
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
	DkBaseViewPort* origView;
	QComboBox* sizeCombo;

	void init();
	void createLayout();
	void updateSnippets();
	void saveSettings();
	void loadSettings();
};

};
