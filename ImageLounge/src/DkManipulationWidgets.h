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

#include <QWidget>
#include <QDockWidget>
#include <QSlider>
#include <QSpinBox>
#include <QLabel>
#include <QBoxLayout>
#include <QProgressDialog>
#include <QDialogButtonBox>

#include "DkViewPort.h"

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

		int getDefaultValue() { return defaultValue; };
		float getDefaultValueF() { return defaultValueF; };

		static void clearHistoryVectors();

#ifdef WITH_OPENCV
		static void setMatImg(Mat newImg) {
			imgMat = newImg;
		}
		static void setOrigMatImg(Mat newImg) {
			origMat = newImg;
		}
		static void createMatLut();
		static Mat manipulateImage(Mat inImg);

		Mat changeBrightnessAndContrast(Mat inImgMat, float brightnessVal, float contrastVal);
		Mat changeSaturationAndHue(Mat inImgMat, float saturationVal, float hueVal);
		Mat changeGamma(Mat inImgMat, float gamma);
		Mat changeExposure(Mat inImgMat, float exposure);
		
		virtual Mat compute(Mat inLut, float val1, float val2) = 0;

#endif

	protected:
		QSpinBox* sliderSpinBox;
		QDoubleSpinBox* sliderSpinBoxDouble;
		QSlider* slider;
		QLabel* sliderTitle;
		QLabel* minValLabel;
		QLabel* maxValLabel;
		QLabel* middleValLabel;
		int leftSpacing;
		int topSpacing;
		int sliderLength;
		int margin;
		int minVal;
		int maxVal;
		int middleVal;
		double minValD;
		double maxValD;
		double middleValD;
		bool valueUpdated;
		QImage hueGradientImg;
		QString name;
		double gammaSliderValues[200];
		DkImageManipulationDialog *manipDialog;
		int defaultValue;
		float defaultValueF;
		
		static bool doARedraw;

		static int brightness;
		static int contrast;
		static int saturation;
		static int hue;
		static float gamma;
		static float exposure;
		static bool prepareUndo;

#ifdef WITH_OPENCV
		static Mat imgMat;
		static Mat tempLUT;
		static Mat origMat;

		static Mat applyLutToImage(Mat inImg, Mat tempLUT, bool isMatHsv);
		static Mat createMatLut16();
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
		virtual Mat compute(Mat inLut, float val1, float val2);
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
		virtual Mat compute(Mat inLut, float val1, float val2);
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
		virtual Mat compute(Mat inLut, float val1, float val2);
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
		virtual Mat compute(Mat inLut, float val1, float val2);
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
		virtual Mat compute(Mat inLut, float val1, float val2);
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
	virtual Mat compute(Mat inLut, float val1, float val2);
#endif

};


class DkUndoRedo : public DkImageManipulationWidget {
	Q_OBJECT
	
public:
	DkUndoRedo(QWidget *parent, DkImageManipulationDialog *parentDialog);
	~DkUndoRedo();

	static void enableUndoButton(bool val) { buttonUndo->setEnabled(val); };
	static void enableRedoButton(bool val) { buttonRedo->setEnabled(val); };
	
protected:
	virtual void redrawImage();
#ifdef WITH_OPENCV
	virtual Mat compute(Mat inLut, float val1, float val2);
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
		this->img = img;
		createImgPreview();
		drawImgPreview();
	};

	void resetValues();
	QImage getImgPreview() {return imgPreview;};

	DkBrightness *getBrightnessWidget() { return brightnessWidget;};
	DkContrast *getContrastWidget() { return contrastWidget;};
	DkSaturation *getSaturationWidget() { return saturationWidget;};
	DkHue *getHueWidget() { return hueWidget;};
	DkGamma *getGammaWidget() { return gammaWidget;};
	DkExposure *getExposureWidget() { return exposureWidget;};

protected slots:
	void updateImg(QImage updatedImg);

protected:
	QImage *img;
	QImage imgPreview;
	int dialogWidth;
	int dialogHeight;
	QRect previewImgRect;
	QLabel* previewLabel;
	int previewWidth;
	int previewHeight;
	int toolsWidth;
	int previewMargin;

	DkContrast* contrastWidget;
	DkBrightness* brightnessWidget;
	DkSaturation* saturationWidget;
	DkHue* hueWidget;
	DkGamma* gammaWidget;
	DkExposure* exposureWidget;
	DkUndoRedo* undoredoWidget;

	void init();
	void createLayout();
	void createImgPreview();
	void drawImgPreview();

signals:
	void isNotGrayscaleImg(bool isGrayscale);
};

};
