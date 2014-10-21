/*******************************************************************************************************
 DkMagicCutWidgets.h
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
#include <QDockWidget>
#include <QSlider>
#include <QSpinBox>
#include <QLabel>
#include <QBoxLayout>
#include <QProgressDialog>
#include "BorderLayout.h"

#include "DkImageStorage.h"
#include "DkWidgets.h"


namespace nmc {

class DkMagicCutDialog;


/**
* Main class for performing a magic wand cut after clicking in the image.
* The tool performs a flood fill to include homogeneous pixels into the cut
* and provides a mask with the labeled regions and it's contours
*/ 
class DkMagicCut {

public:
	DkMagicCut();
	~DkMagicCut();

	void setTolerance(int tol);
	void setImage(cv::Mat img, QTransform *imgMat);

	// magic wand selection functions
	bool magicwand(QPoint xy);
	void resetRegionMask(int region = 0, bool recalcContours = true);
	void resetRegionMask(QPoint xy);
	bool hasContours();
	void updateAnimateContours();
	
	// get functions
	QPen getContourPen() { return contourPen; };
	QPainterPath getContourPath() { return contours; };
	cv::Mat *getMask() { return &mask; };
	cv::Mat *getImage() { return &imgUC3; };
	cv::Rect *getBoundingRect() { return &bRect; };

	

private:
	cv::Mat imgUC3; /**< Input image */
	cv::Mat mask; /**< Blob masks with region */
	QTransform *imgMatrix; /**< Mapping of contour points to image */
	cv::Rect bRect; /**< Bounding rect for selection */
	int tolerance; /**< The tolerance depicting homogeneous regions */
	int label_it; /**< Iterator for labeling the regions */
	int maxSize; /**< The maximum size of a homogeneous regions */

	// contour: drawing and animation
	QPen contourPen; /**< Style of the regions contour lines */
	QPainterPath contours; /**< Qt polygons for drawing contour lines */
	std::vector<std::vector<cv::Point> > contour_points; /**< Vector containing vector of points for each region contour */
	void calculateContours();
};

/**
* Widget consisting of a slider for setting the offset (padding) of the bounding box of a magic wand cut
*/
class DkBoundingBoxSliderWidget : public QWidget {

	Q_OBJECT
	
	public:
		DkBoundingBoxSliderWidget(QWidget *parent, DkMagicCutDialog *parentDialog);
		~DkBoundingBoxSliderWidget();

		int getValue() { return BBoffset; };

	private:
		int BBoffset; /**< Offset on the original region bounding box */

		int minVal; /**< The minimum offset value */
		int middleVal;
		int maxVal; /**< The maximum offset value */
		int defaultValue; /**< Default offset value */
		int leftSpacing;
		int topSpacing;
		int sliderLength;
		QString name;
	
		QSpinBox* sliderSpinBox;
		QSlider* slider;
		QLabel* sliderTitle;
		QLabel* minValLabel;
		QLabel* maxValLabel;
		QLabel* middleValLabel;

		DkMagicCutDialog *parentDialog; /**< The owner of this widget */

	protected slots:
		void updateSliderSpinBox(int val);
		void updateSliderVal(int val);

};

/**
* The dialog for configuring and saving a magic wand cut
*/
class DkMagicCutDialog : public QDialog {
	Q_OBJECT

	private:
		bool withMask; /**< True if cut saved with alpha channel (mask) */
		DkMagicCut *magicCut; /**< The magic cut to save */
		cv::Mat binaryMask; /**< The binary mask of the whole image depicting the magic cut area */

	public:
		DkMagicCutDialog(DkMagicCut *magicCut, QWidget* parent = 0, Qt::WindowFlags flags = 0);
		~DkMagicCutDialog();
		void createImgPreview();
		bool wasSavePressed() {
			return isSaved;
		};

		QImage getImgPreview() {return imgPreview;};

	protected slots:
		void savePressed();
		void cancelPressed();
		void applyMaskChecked(int);
		void updateImg(QImage updatedImg);

	protected:
		bool isSaved; /**< True after signal of saving has been emitted */
		QImage imgQt; /**< The ROI as Qt image */
		cv::Rect* roiRect; /**< The bounding box rectangle */
		QImage imgPreview; /**< Scaled preview image (if scaling necessary) */
		int dialogWidth;
		int dialogHeight;
		QRect previewImgRect; /**< Rectangular area where preview image is to be drawn */
		QLabel* previewLabel;
		int previewWidth;
		int previewHeight;
		int toolsWidth;
		int previewMargin;
		DkBoundingBoxSliderWidget *BBslider; /**< Slider for setting bounding box offset */

		void init();
		void createLayout();
		void showEvent(QShowEvent *event);
		void drawImgPreview();

	signals:
		void isNotGrayscaleImg(bool isGrayscale); /**< Signal to declare if or if not image is not in a grayscale image format */
		/**
		* Signal to save the specified magic cut (as image with metadata about position of the cut within the original image)
		* @param saveImg The image to be saved
		* @param xCoord The x-coordinate of the upper left corner of the bounding box of the cut
		* @param yCoord The y-coordinate of the upper left corner of the bounding box of the but
		* @param height The height of the bounding box (y-extent)
		* @param width The width of the bounding box (x-extent)
		**/
		void savePressed(QImage saveImg, int xCoord, int yCoord, int height, int width);
};

};
