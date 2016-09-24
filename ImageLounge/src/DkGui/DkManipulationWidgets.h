/*******************************************************************************************************
 DkManipulationWidgets.h
 Created on:	04.06.2012
 
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
#include <QWidget>
#include <QDialog>
#pragma warning(pop)		// no warnings from includes - end

#ifdef WITH_OPENCV

#ifdef Q_OS_WIN
#pragma warning(disable: 4996)
#endif

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#endif

// Qt defines
class QSpinBox;
class QDoubleSpinBox;
class QSlider;
class QLabel;
class QPushButton;

namespace nmc {

class DkImageManipulationDialog;

struct historyData {
	float arg1, arg2;
	bool isHsv;
};

class DkImageManipulationWidget : public QWidget {

	Q_OBJECT
	
	public:
		DkImageManipulationWidget(QWidget *parent, DkImageManipulationDialog *parentDialog);
		~DkImageManipulationWidget();

		static void setEmptyManipulationType() { manipulationType = manipulationEmpty; };
		static void setPrepareUndo(bool val) { prepareUndo = val;};

		void setToolsValue(float val);
		void setToolsValue(float val1, float val2);
		float getToolsValue();

		int getDefaultValue() { return mDefaultValue; };
		float getDefaultValueF() { return mDefaultValueF; };

		static void clearHistoryVectors();

#ifdef WITH_OPENCV
		static void setMatImg(cv::Mat newImg) {
			imgMat = newImg;
		}
		static void setOrigMatImg(cv::Mat newImg) {
			origMat = newImg;
		}
		static void createMatLut();
		static cv::Mat manipulateImage(cv::Mat inImg);

		cv::Mat changeBrightnessAndContrast(cv::Mat inImgMat, float brightnessVal, float contrastVal);
		cv::Mat changeSaturationAndHue(cv::Mat inImgMat, float saturationVal, float hueVal);
		cv::Mat changeGamma(cv::Mat inImgMat, float gamma);
		cv::Mat changeExposure(cv::Mat inImgMat, float exposure);
		
		virtual cv::Mat compute(cv::Mat inLut, float val1, float val2) = 0;

#endif

	protected:
		QSpinBox* mSliderSpinBox;
		QDoubleSpinBox* mSliderSpinBoxDouble;
		QSlider* mSlider;
		QLabel* mSliderTitle;
		QLabel* mMinValLabel;
		QLabel* mMaxValLabel;
		QLabel* mMiddleValLabel;
		int mLeftSpacing		= 10;
		int mTopSpacing			= 10;
		int mSliderLength		= 0;
		int mMinVal				= -100;	
		int mMaxVal				= 100;
		int mMiddleVal			= 0;
		double mMinValD			= -100.0;
		double mMaxValD			= 100.0;
		double mMiddleValD		= 0.0;
		bool mValueUpdated		= false;
		QImage mHueGradientImg;
		QString mName;
		double mGammaSliderValues[200];
		DkImageManipulationDialog* mManipDialog = 0;
		int mDefaultValue		= 0;
		float mDefaultValueF	= 0.0f;
		
		static bool doARedraw;

		static int brightness;
		static int contrast;
		static int saturation;
		static int hue;
		static float gamma;
		static float sExposure;
		static bool prepareUndo;

#ifdef WITH_OPENCV
		static cv::Mat imgMat;
		static cv::Mat tempLUT;
		static cv::Mat origMat;

		static cv::Mat applyLutToImage(cv::Mat inImg, cv::Mat tempLUT, bool isMatHsv);
		static cv::Mat createMatLut16();
#endif

		int findClosestValue(double *values, double closestVal, int i1, int i2);
		void resetSliderValues(char exceptionSlider);
		virtual void redrawImage() = 0;
		void prepareUndoRedoButtons();

		enum {
			manipulationEmpty,
			manipulationBrightness,
			manipulationContrast,
			manipulationSaturation,
			manipulationHue,
			manipulationGamma,
			manipulationExposure
		};

		static std::vector<historyData> historyDataVec;
		static std::vector<DkImageManipulationWidget*> historyToolsVec;

		static char manipulationType;
		static bool slidersReset;

	protected slots:

		void updateSliderSpinBox(int val);
		void updateDoubleSliderSpinBox(int val);
		void updateSliderVal(int val);
		void updateDoubleSliderVal(double val);

	signals:
		void updateDialogImgSignal(QImage updatedImg);

};

class DkBrightness : public DkImageManipulationWidget {

	Q_OBJECT
	
	public:
		DkBrightness(QWidget *parent, DkImageManipulationDialog *parentDialog);
		~DkBrightness();
	
	protected:
		virtual void redrawImage();
#ifdef WITH_OPENCV
		virtual cv::Mat compute(cv::Mat inLut, float val1, float val2);
#endif

};

class DkContrast : public DkImageManipulationWidget {

	Q_OBJECT
	
	public:
		DkContrast(QWidget *parent, DkImageManipulationDialog *parentDialog);
		~DkContrast();
	
	protected:
		virtual void redrawImage();
#ifdef WITH_OPENCV
		virtual cv::Mat compute(cv::Mat inLut, float val1, float val2);
#endif

};

class DkSaturation : public DkImageManipulationWidget {

	Q_OBJECT
	
	public:
		DkSaturation(QWidget *parent, DkImageManipulationDialog *parentDialog);
		~DkSaturation();
	
	protected:
		virtual void redrawImage();
#ifdef WITH_OPENCV
		virtual cv::Mat compute(cv::Mat inLut, float val1, float val2);
#endif

	protected slots:
		void setSliderStyle(QString sColor);

};

class DkHue : public DkImageManipulationWidget {

	Q_OBJECT
	
	public:
		DkHue(QWidget *parent, DkImageManipulationDialog *parentDialog);
		~DkHue();
	
	protected:
		virtual void redrawImage();
#ifdef WITH_OPENCV
		virtual cv::Mat compute(cv::Mat inLut, float val1, float val2);
#endif

	signals:
		void setSaturationSliderColor(QString sColor);
};

class DkGamma : public DkImageManipulationWidget {

	Q_OBJECT
	
	public:
		DkGamma(QWidget *parent, DkImageManipulationDialog *parentDialog);
		~DkGamma();
	
	protected:
		virtual void redrawImage();
#ifdef WITH_OPENCV
		virtual cv::Mat compute(cv::Mat inLut, float val1, float val2);
#endif

};

class DkExposure : public DkImageManipulationWidget {

	Q_OBJECT
	
public:
	DkExposure(QWidget *parent, DkImageManipulationDialog *parentDialog);
	~DkExposure();
	float convertExposureToSliderVal(float val);
	float convertSliderValToExposure(float val);

protected:
	virtual void redrawImage();
#ifdef WITH_OPENCV
	virtual cv::Mat compute(cv::Mat inLut, float val1, float val2);
#endif

};


class DkUndoRedo : public DkImageManipulationWidget {
	Q_OBJECT
	
public:
	DkUndoRedo(QWidget *parent, DkImageManipulationDialog *parentDialog);
	~DkUndoRedo();

	static void enableUndoButton(bool val);
	static void enableRedoButton(bool val);
	
protected:
	virtual void redrawImage();
#ifdef WITH_OPENCV
	virtual cv::Mat compute(cv::Mat inLut, float val1, float val2);
#endif
	static QPushButton* buttonUndo;
	static QPushButton* buttonRedo;

	std::vector<historyData> historyDataVecCopy;
	std::vector<DkImageManipulationWidget*> historyToolsVecCopy;
	int manipulationTypeHist;

protected slots:
	void undoPressed();
	void redoPressed();

//friend void DkImageManipulationWidget::prepareUndoRedoButtons();
};

// Image manipulation dialog with image manipulation tools
class DkImageManipulationDialog : public QDialog {
	Q_OBJECT

public:
	DkImageManipulationDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~DkImageManipulationDialog();
	
	void setImage(QImage *img) {
		mImg = img;
		createImgPreview();
		drawImgPreview();
	};

	void resetValues();
	QImage getImgPreview() {return mImgPreview;};

	DkBrightness *getBrightnessWidget() { return mBrightnessWidget;};
	DkContrast *getContrastWidget() { return mContrastWidget;};
	DkSaturation *getSaturationWidget() { return mSaturationWidget;};
	DkHue *getHueWidget() { return mHueWidget;};
	DkGamma *getGammaWidget() { return mGammaWidget;};
	DkExposure *getExposureWidget() { return mExposureWidget;};

protected slots:
	void updateImg(QImage updatedImg);

protected:
	QImage *mImg;
	QImage mImgPreview;
	QRect mPreviewImgRect;
	QLabel* mPreviewLabel;
	int mPreviewWidth;
	int mPreviewHeight;
	int mToolsWidth;
	int mPreviewMargin;

	DkContrast* mContrastWidget;
	DkBrightness* mBrightnessWidget;
	DkSaturation* mSaturationWidget;
	DkHue* mHueWidget;
	DkGamma* mGammaWidget;
	DkExposure* mExposureWidget;
	DkUndoRedo* mUndoRedoWidget;

	void init();
	void createLayout();
	void createImgPreview();
	void drawImgPreview();

signals:
	void isNotGrayscaleImg(bool isGrayscale);
};

};
