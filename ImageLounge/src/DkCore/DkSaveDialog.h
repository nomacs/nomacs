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

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

#pragma warning(disable: 4251)	// TODO: remove

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

class DllCoreExport DkTifDialog : public QDialog {
	Q_OBJECT

public:
	DkTifDialog(QWidget* parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

	int getCompression() const;

protected:
	void init();
	QRadioButton* noCompressionButton;
	QRadioButton* compressionButton;
	bool isOk;

};

class DllCoreExport DkCompressDialog : public QDialog {
	Q_OBJECT

public:

	enum {
		jpg_dialog,
		j2k_dialog,
		webp_dialog,
		web_dialog,
		avif_dialog,

		dialog_end
	};

	DkCompressDialog(QWidget* parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
	virtual ~DkCompressDialog();

	void imageHasAlpha(bool hasAlpha);
	QColor getBackgroundColor() const;
	int getCompression();
	float getResizeFactor();
	void setImage(const QImage& img);
	void setDialogMode(int dialogMode);
	virtual void accept() override;

public slots:
	void setVisible(bool visible) override;

protected slots:
	void newBgCol();
	void losslessCompression(bool lossless);
	void changeSizeWeb(int);
	void drawPreview();
	void updateFileSizeLabel(float bufferSize = -1, QSize bufferImgSize = QSize(), float factor = -1);
	
protected:
	void init();
	void createLayout();
	void updateSnippets();
	void saveSettings();
	void loadSettings();
	void resizeEvent(QResizeEvent *ev) override;

	enum {
		best_quality = 0,
		high_quality,
		medium_quality,
		low_quality,
		bad_quality,

		end_quality
	};

	QVector<int> mImgQuality;
	QVector<int> mAvifImgQuality;

	int mDialogMode = jpg_dialog;
	bool mHasAlpha = false;
	QColor mBgCol = QColor(255, 255, 255);

	QCheckBox* mCbLossless = 0;
	//DkSlider* mSlider = 0;
	DkColorChooser* mColChooser = 0;
	QLabel* mPreviewLabel = 0;
	QLabel* mPreviewSizeLabel = 0;
	DkBaseViewPort* mOrigView = 0;
	QComboBox* mSizeCombo = 0;
	QComboBox* mCompressionCombo = 0;

	QImage mImg;
	QImage mNewImg;
};

}
