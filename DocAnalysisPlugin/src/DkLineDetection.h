/*******************************************************************************************************
 DkLineDetection.h
 Created on:	20.10.2014
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2012 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2012 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2012 Florian Kleber <florian@nomacs.org>

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
#include <QDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QLayout>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <numeric>      // std::accumulate
#include <windows.h> 
#include <iostream>
#include "BorderLayout.h"


//#include "DkImage.h"
#include "DkImageStorage.h"

namespace nmc {

class DkLineDetectionDialog;

/**
* Main class implementing line detection.
**/
class DkLineDetection {
	
	private:
		cv::Mat image; /**< The original image **/
		cv::Mat lpp_image; /**< Local projection profile of the image **/
		cv::Mat lowerTextLines; /**< The optimized lower text lines image mask **/
		cv::Mat upperTextLines; /**< The optimized upper text lines image mask **/
		cv::Mat basicLowerTextLines; /**< The basic calculated lower text lines (basis for optimization) **/
		cv::Mat basicUpperTextLines; /**< The basic calculated upper text lines (basis for optimization) **/
		QImage bottomLines; /**< Lower (bottom) text lines converted to an Qt image with alpha channel **/
		QImage topLines; /**< Upper (top) text lines converted to an Qt image with alpha channel **/
		bool hasLines; /**< True, if lines are available **/
		bool recalc; /**< True, if recalculation neccessary **/

		bool debug;

		/**
		* Parameters that are used for calculating and optimizing the text lines
		**/
		struct parameters {
			int stripeLength; /**< The width of the vertical stripes to calculate the local projection profile **/
			int halfStripeLength; /**< Half the stripeLength **/
			float sigma; /**< Sigma for the 1D gaussian derivative kernel used to find local minima and maxima **/
			int nonExtremaKernelSize; /**< The kernel size for non-extrema suppression of the local minima and maxima of the LLP **/
			float maxThresh; /**< Threshold for a local maximum to be a real maximum **/
			float alpha; /**< Alpha value of the text line images **/
			bool optimizeImage; /**< Flag to declare if the optimization algorithm shall be run **/
			int sobelFilterX; /**< 1 means to enable Sobel-filtering in x-direction for edge detection during optimization **/
			int sobelFilterY; /**< 1 means to enable Sobel-filtering in y-direction for edge detection during optimization **/
			int sobelFilterSize; /** The size of the Sobel filter **/
			int boxFilterSizeX; /** Width of the box filter to blur the edge images **/
			int boxFilterSizeY; /** Height of the box filter to blur the edge images **/
			int removeShort; /**< 1 means to remove short text lines within the line image **/
			float rescale;
		};
		parameters params; /**< Current parameters for calculation and optimization **/

		// define
		enum morph_border{DK_BORDER_ZERO = 0, DK_BORDER_FLIP};

		void calcLocalProjectionProfile();
		void findLocalMinima();
		void nonExtremaSuppression(cv::Mat *histogram, cv::Mat *maxima, cv::Mat *minima);
		void nonExtremaSuppression2D(cv::Mat *histogram, cv::Mat *maxima, cv::Mat *minima);
		void createTextLineImages();
		void optimizeLineImg1(cv::Mat segLineImg, cv::Mat *lowertextLineImg, cv::Mat *uppertextLineImg);
		void optimizeLineImg(cv::Mat *segLineImg, cv::Mat *lowertextLineImg, cv::Mat *uppertextLineImg);
		static cv::Mat convolveIntegralImage(const cv::Mat src, const int kernelSizeX, const int kernelSizeY, const int norm);
		cv::Mat removeShortLines(cv::Mat img, int minLength);

		std::string getImageType(int number);
		void debugOutputMat(cv::Mat *mat, std::string message);
		bool setWindow(int Width, int Height);
		bool compareMat(cv::Mat mat1, cv::Mat mat2, std::string text = "");

	public:
		DkLineDetection();
		~DkLineDetection();

		void setImage(cv::Mat img);
		cv::Mat getImage();
		void startLineDetection();
		void setParameters(int stripeWidth, int nonExtrKernelSize, bool optimize, 
			bool sobelX, bool sobelY, int sobelKernelSize, int boxFilterSizeX, 
			int boxFilterSizeY, int removeShort/*, float rescale*/);
		bool hasTextLines() { return hasLines; }
		QImage getBottomLines() { return bottomLines; }
		QImage getTopLines() { return topLines; }
		

};


/**
* The dialog class which allows configuration of the parameters for the line
* detection algorithm.
**/
class DkLineDetectionDialog : public QDialog {
	Q_OBJECT

	public:
		DkLineDetectionDialog(DkLineDetection *lineDetector, QWidget* parent = 0, Qt::WindowFlags flags = 0);
		~DkLineDetectionDialog();

		void setDefaultConfiguration();

	protected:
		int dialogWidth;
		int dialogHeight;

		void init();
		void createLayout();

		void showEvent(QShowEvent *event);

	protected slots:
		void detectLinesPressed();
		void cancelPressed();
		void enableOptimizationSettings(int);

	private:
		DkLineDetection *lineDetector; /**< The corresponding line detector tool **/

		int margin;
		// UI elements
		QSpinBox *spinnerStripeLength;
		QSpinBox *spinnerNonExtKernelSize;
		QCheckBox *checkOptimize;
		QCheckBox *checkSobelX;
		QCheckBox *checkSobelY;
		QComboBox *comboBoxSobelSize;
		QSpinBox *spinnerFilterSizeX;
		QSpinBox *spinnerFilterSizeY;
		QCheckBox *checkRemoveShort;
		//QDoubleSpinBox *spinnerRescale;

		// old values for resetting on cancel
		int oldStripeLength;
		int oldNonExtremaKernelSize; /**< The kernel size for non-extrema suppression of the local minima and maxima of the LLP **/
		bool oldOptimizeImage; /**< Flag to declare if the optimization algorithm shall be run **/
		int oldSobelFilterX; /**< 1 means to enable Sobel-filtering in x-direction for edge detection during optimization **/
		int oldSobelFilterY; /**< 1 means to enable Sobel-filtering in y-direction for edge detection during optimization **/
		int oldSobelFilterSize; /** The size of the Sobel filter **/
		int oldBoxFilterSizeX; /**< Width of the box filter to blur the edge images **/
		int oldBoxFilterSizeY; /**< Height of the box filter to blur the edge images **/
		int oldRemoveShort; /**< 1 means to remove short lines during text line detection **/

};

};
