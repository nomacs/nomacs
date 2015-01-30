/*******************************************************************************************************
 DkMagicCutWidgets.cpp
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

#include "DkMagicCutWidgets.h"


namespace nmc {


// class: DkMagicCut start
DkMagicCut::DkMagicCut() {

	tolerance = 40; // default tolerance

	contourPen.setStyle(Qt::DashLine);
	contourPen.setWidth(0.8);
	contourPen.setColor(QColor(0, 0, 0, 255));
	contourPen.setDashOffset(0);

	maxSize = 0;
}

DkMagicCut::~DkMagicCut() {

}

/**
* Sets the tolerance for homogeneous regions
* @param tol The absolute tolerance value (0..255)
**/
void DkMagicCut::setTolerance(int tol) {

	// normalize Threshold
	/*double maxVal;
	minMaxIdx(imgUC3, 0, &maxVal);
	threshold = ((double)thresh/100)*maxVal;*/

	tolerance = tol;
}

/**
* Sets the image for magic cut selection
* @param img The image
* @param imgMat Pointer to the corresponding Qt image matrix
**/
void DkMagicCut::setImage(cv::Mat img, QTransform *imgMat) {

	qDebug() << "Image Channels: " << img.channels();
	
	if (img.channels() <= 2) {
		// image does not have 3 channels (RGB) -> convert gray to RGB image
		cv::cvtColor(img, img, CV_GRAY2BGR);
	}
	
	// flood Fill function fails when having a 4 channel matrix - create 3 channel mat
	imgUC3.create(img.rows, img.cols, CV_8UC3);
	int from_to[] = { 0,0 , 1,1 , 2,2 };
	cv::mixChannels(&img, 1, &imgUC3, 1, from_to, 3);

	mask.create(img.rows+2, img.cols+2, CV_8UC1);

	// reset the regions mask
	resetRegionMask();

	imgMatrix = imgMat;

	maxSize = (int)(0.4 * img.rows * img.cols);
}

/**
* Check if an area has been selected
* @returns true, if contours exist, i.e. something is selected
**/
bool DkMagicCut::hasContours() {

	return (!contour_points.empty());
}

/**
* Function that is regularly called to update the contour pen to have an animated
* line effect
**/
void DkMagicCut::updateAnimateContours() {

	int dash = contourPen.dashOffset()+2;
	contourPen.setDashOffset(dash % 6);
}


/**
* Resets the region mask according to the label at point xy
* @param xy image point where to reset the area
**/
void DkMagicCut::resetRegionMask(QPoint xy) {

	uchar *ptr;
	// mask is bigger, therefore x+1 and y+1
	ptr = mask.data + mask.step * (xy.y()+1);
	int region = (int) (ptr[xy.x()+1]);
	if (region != 0)
	resetRegionMask(region);
}

/**
* Resets (removes) the region within the mask with label region
* default: resets whole mask
* @param region the label of the region to reset (0 ... reset whole mask)
* @param recalcContours true if contours need to be recalculated
*/
void DkMagicCut::resetRegionMask(int region, bool recalcContours) {

	if(mask.empty()) return;

	if (region == 0) {
		label_it = 1;
		// mask needs to be 2 pixles wider and 2 pixels taller for flood filling
		mask = Scalar::all(0); //cv::Mat::zeros(img.rows+2, img.cols+2, CV_8UC1);
	} else {
		// reset only regions with value: region
		mask.setTo(0, mask == region);

		/*uchar *ptr;
		for (int i=0; i < mask.rows; i++)
		{
			for (int j=0; j <mask.cols; j++)
			{
				ptr = mask.data + mask.step * i;
				if ( (int)(ptr[j]) == region )
				{
					ptr[j] = 0;
				}
			}
		}*/
	}

	if(recalcContours) {
		// update the contours
		calculateContours();
	}
}


/**
* The actual magic wand function performing an OpenCV flood filling starting from a seed point
* @param xy The seed point within the image
* @returns false, if the selected area is too big - otherwise true
* \sa DkMagicCut::calculateContours()
**/
bool DkMagicCut::magicwand(QPoint xy) {
	
	//Rect bRect;
	int area;
	int connectivity = 8;
	int flags = connectivity + (label_it << 8) + 
		(CV_FLOODFILL_FIXED_RANGE | CV_FLOODFILL_MASK_ONLY);

	// get the color of the point
	//QColor col = imgStorage.getImage().pixel(xy);
	
	// The Flood Fill approach
	if (label_it < 255) label_it++;
	else label_it = 1;

	area = cv::floodFill(imgUC3, mask, cv::Point(xy.x(), xy.y()), label_it, 0 /*&bRect*/,  
						 Scalar(tolerance, tolerance, tolerance), 
						 Scalar(tolerance, tolerance, tolerance), flags);

	if(area >= maxSize) {
		// area is too big - reset the mask and give a message
		label_it--;
		resetRegionMask(label_it, false);
		return false;
	} else {
		calculateContours();
	}

	return true;

	/* The RECURSIVE approach */
	/*label_it++;
	seededRegionGrowing(xy.x(), xy.y(), col.redF()*255, col.greenF()*255, col.blueF()*255);
	// dilate the found blobs to receive better results
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
	cv::dilate(mask, mask, element);
	calculateContours();*/
	// DEBUG
	/*std::cout << "Region mask: " << std::endl;
	uchar *ptr;
	for (int i=0; i < mask.rows; i++)
	{
		for (int j=0; j <mask.cols; j++)
		{
			ptr = mask.data + mask.step * i;
			std::cout << (int)(ptr[j]) << " ; ";
		}
		std::cout << std::endl;
	}*/
}


/**
* Calculates the contours (vector of points for each contour) for the extracted blobs.
* Calculated every time the mask changes.
**/
void DkMagicCut::calculateContours() {

	// region mask has blobs now -> find contours
	contour_points.clear();

	// the mask is larger than the image - copy only needed parts
	// find contours changes the mask, therefore clone it
	// select the ROI
	//cv::Mat roi(mask, Rect(2,2,mask.cols-1,mask.rows-1));
	cv::Mat roi = mask(Range(1,mask.rows-1),Range(1,mask.cols-1));
	cv::Mat roi_clone = roi.clone();
	// dilate the found blobs to receive better results
	cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7, 7));
	cv::dilate(roi_clone, roi_clone, element);
	// + closing of small holes
	cv::dilate(roi_clone, roi_clone, element);
	cv::erode(roi_clone, roi_clone, element);
	// note: types of approximation: 
	// CV_CHAIN_APPROX_NONE, CV_CHAIN_APPROX_SIMPLE, CV_CHAIN_APPROX_TC89_L1, CV_CHAIN_APPROX_TC89_KCOS
	cv::findContours(roi_clone, contour_points, CV_RETR_LIST,CV_CHAIN_APPROX_TC89_L1);

	if(contour_points.empty()) return;

	// save contours into Qt objects
	contours = QPainterPath();
	QVector<QPoint> points;
	std::vector<cv::Point> points_cv, points_cv_all;
	QPainterPath polyPaths;

	points_cv_all.clear();

	for (uint i = 0; i < contour_points.size(); i++) {
		points.clear();
		points_cv = contour_points.at(i);
		
		for (uint j = 0; j < points_cv.size(); j++) {

			// push every contour point into a QVector of QPoints for this polygon
			points.push_back(QPoint(points_cv.at(j).x, points_cv.at(j).y));
			// push every point into a single vector for bounding rect calculation
			points_cv_all.push_back(points_cv.at(j));
		}
		// add starting point again
		points.push_back(QPoint(points_cv.at(0).x, points_cv.at(0).y));

		// map to image and add polygon to painter path
		contours.addPolygon(QPolygon(points)/*imgMatrix->map(QPolygon(points))*/);
	}

	// get minimum bounding rect for selected regions
	bRect = cv::boundingRect(points_cv_all);
	points_cv_all.clear();
	// DEBUG: draw the bounding rect
	/*
	points.clear();
	points.push_back(QPoint(bRect.x, bRect.y));
	points.push_back(QPoint(bRect.x + bRect.width, bRect.y));
	points.push_back(QPoint(bRect.x + bRect.width, bRect.y + bRect.height));
	points.push_back(QPoint(bRect.x, bRect.y + bRect.height));
	points.push_back(QPoint(bRect.x, bRect.y));
	contours.addPolygon(imgMatrix.map(QPolygon(points)));
	*/
}

/*
void DkMagicCut::seededRegionGrowing(int x, int y, int ref_r, int ref_g, int ref_b) {

	// check if point within image
	if (x < 0 || x > imgUC3.cols || y < 0 || y > imgUC3.rows)
		return;

	uchar *mask_ptr;
	// get pointer to mask data
	mask_ptr = mask.data + mask.step * y;
	// check if already labeled
	if ((int)mask_ptr[x] != 0)
		return;

	// retrieve number of channels
	int n = imgUC3.channels();
	// get a pointer to the data
	uchar *ptr;

	// read data from current point
	ptr = imgUC3.data + imgUC3.step * y;
	//std::cout << "Iterated point(" << x << "," << y << "): " << (int)red << " " << (int)blue << " " << (int)green << " ; ";

	double dist = pow(((int)ptr[n*x+2] - ref_r), 2) + 
					pow(((int)ptr[n*x+1] - ref_g), 2) +
					pow(((int)ptr[n*x] - ref_b), 2);

	// check if pixel is within threshold
	if ( dist > pow(threshold, 2) ) {
		//std::cout << "Pixel not within: " << (int)mask_ptr[x] << std::endl;
		return;
	}
	// pixel is within threshold
	mask_ptr[x] = (uchar)label_it;
	//std::cout << "Pixel within: " << (int)mask_ptr[x] << std::endl;

	// start further recursion
	seededRegionGrowing(x, y+1, ref_r, ref_g, ref_b);
	seededRegionGrowing(x, y-1, ref_r, ref_g, ref_b);
	seededRegionGrowing(x+1, y, ref_r, ref_g, ref_b);
	seededRegionGrowing(x+1, y+1, ref_r, ref_g, ref_b);
	seededRegionGrowing(x+1, y-1, ref_r, ref_g, ref_b);
	seededRegionGrowing(x-1, y, ref_r, ref_g, ref_b);
	seededRegionGrowing(x-1, y-1, ref_r, ref_g, ref_b);
	seededRegionGrowing(x-1, y+1, ref_r, ref_g, ref_b);
	
}*/

// class: DkMagicCut end


/**
* Image manipulation dialog with image manipulation tools and preview
* @params magicCut The magic cut to be processed
* @params parent The parent widget
* @params flags The window flags for this dialog
* \sa DkMagicCutDialog::init()
**/
DkMagicCutDialog::DkMagicCutDialog(DkMagicCut *magicCut, QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	this->magicCut = magicCut;
	init();
}

DkMagicCutDialog::~DkMagicCutDialog() {

}


/**
* Initializes the magic cut dialog.
* Sets default sizes.
* \sa DkMagicCutDialog::createLayout()
**/
void DkMagicCutDialog::init() {

	isSaved = false;
	withMask = false;
	dialogWidth = 500; //700;
	dialogHeight = 300; //560;
	toolsWidth = 200;
	previewMargin = 10;
	previewWidth = dialogWidth - toolsWidth - 2 * previewMargin;
	previewHeight = dialogHeight - previewMargin - 70;

	setWindowTitle(tr("Cut Tools"));
	setFixedSize(dialogWidth, dialogHeight);
	createLayout();



}


/**
* Creates the image manipulation dialog layout
* \sa DkBoundingBoxSliderWidget
**/
void DkMagicCutDialog::createLayout() {

	// bottom widget - buttons	
	QWidget* bottomWidget = new QWidget(this);
	QHBoxLayout* bottomWidgetHBoxLayout = new QHBoxLayout(bottomWidget);

	//QWidget* browseWidget = new QWidget(this);
	//browseWidget->setFixedWidth(previewWidth + previewMargin);
	//QHBoxLayout* browseWidgetLayout = new QHBoxLayout(browseWidget);
	//lineEditPath = new QLineEdit();
	//lineEditPath->setFixedWidth(previewWidth);
	//QPushButton* buttonBrowse = new QPushButton(tr("&Browse..."));
	//connect(buttonBrowse, SIGNAL(clicked()), this, SLOT(browsePressed()));

	QPushButton* buttonSave = new QPushButton(tr("&Save"));
	buttonSave->setDefault(true);
	connect(buttonSave, SIGNAL(clicked()), this, SLOT(savePressed()));
	QPushButton* buttonCancel = new QPushButton(tr("&Cancel"));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(cancelPressed()));

	QSpacerItem* spacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Expanding);
	
	//browseWidgetLayout->addWidget(lineEditPath);
	//browseWidgetLayout->addWidget(buttonBrowse);

	
	//bottomWidgetHBoxLayout->addWidget(lineEditPath);
	//bottomWidgetHBoxLayout->addWidget(buttonBrowse);
	//bottomWidgetHBoxLayout->addWidget(browseWidget);
	bottomWidgetHBoxLayout->addItem(spacer);
	bottomWidgetHBoxLayout->addWidget(buttonSave);
	bottomWidgetHBoxLayout->addWidget(buttonCancel);	
	
	// central widget - preview image
	QWidget* centralWidget = new QWidget(this);
	previewLabel = new QLabel(centralWidget);
	previewLabel->setGeometry(QRect(QPoint(previewMargin, previewMargin), QSize(previewWidth, previewHeight)));

	// east widget - sliders
	QWidget* eastWidget = new QWidget(this);
	eastWidget->setMinimumWidth(toolsWidth);
	eastWidget->setMaximumWidth(toolsWidth);
	eastWidget->setContentsMargins(0,5,5,0);

	BBslider = new DkBoundingBoxSliderWidget(eastWidget, this);

	//QVBoxLayout* toolsLayout = new QVBoxLayout(eastWidget);
	//toolsLayout->setContentsMargins(0,0,0,0);
	QGridLayout* toolsLayout = new QGridLayout(eastWidget);

	toolsLayout->addWidget(BBslider, 1, 1, 1, 2);

	toolsLayout->addWidget(new QLabel(tr("Apply Mask: ")), 2, 1);
	QCheckBox* cbMask;
	cbMask = new QCheckBox(this);
	toolsLayout->addWidget(cbMask, 2, 2);

	eastWidget->setLayout(toolsLayout);
	
	BorderLayout* borderLayout = new BorderLayout;
	borderLayout->addWidget(bottomWidget, BorderLayout::South);
	borderLayout->addWidget(centralWidget, BorderLayout::Center);
	borderLayout->addWidget(eastWidget, BorderLayout::East);
	this->setSizeGripEnabled(false);

	this->setLayout(borderLayout);


	connect(cbMask, SIGNAL(stateChanged(int)), this, SLOT(applyMaskChecked(int)));
}



/**
* Creates an image preview for the dialog
* \sa DkMagicCutDialog::drawImgPreview()
**/
void DkMagicCutDialog::createImgPreview() {

	/*if (!img || img->empty())
		return;*/
	if (!magicCut->getImage() || magicCut->getImage()->empty())
		return;
	
	cv::Rect* origBB = magicCut->getBoundingRect();

	int x1 = (origBB->x - BBslider->getValue() < 0 ? 0 : origBB->x - BBslider->getValue());
	int y1 = (origBB->y - BBslider->getValue() < 0 ? 0 : origBB->y - BBslider->getValue());
	int x2 = (origBB->x + origBB->width + BBslider->getValue() > magicCut->getImage()->cols ? magicCut->getImage()->cols : origBB->x + origBB->width + BBslider->getValue());
	int y2 = (origBB->y + origBB->height + BBslider->getValue() > magicCut->getImage()->rows ? magicCut->getImage()->rows : origBB->y + origBB->height + BBslider->getValue());
	
	roiRect = new cv::Rect(x1, y1, (x2-x1 < 1 ? 1 : x2-x1), (y2-y1 < 1 ? 1 : y2 - y1));


	// get bounding rect ROI from complete image
	Mat imgRoi = ((*(magicCut->getImage()))(*roiRect)).clone();
	// switch channels for correct displaying
	cv::cvtColor(imgRoi, imgRoi, CV_BGR2RGB);

	
	
	if(withMask) {
		std::vector<Mat> ImgChannels;
		Mat imgUC4;
		// For some unknown reason we have to switch channels again
		cv::cvtColor(imgRoi, imgRoi, CV_BGR2RGB);
		cv::split(imgRoi, ImgChannels);
		ImgChannels.push_back(((*(magicCut->getMask()))(*roiRect)).clone()*255);
		cv::merge(ImgChannels, imgUC4);
		imgQt = DkImage::mat2QImage(imgUC4);

	} else {
		
		imgQt = DkImage::mat2QImage(imgRoi);
	}

	
	QPoint lt;
	float rW = previewWidth / (float) imgQt.width();
	float rH = previewHeight / (float) imgQt.height();
	float rMin = (rW < rH) ? rW : rH;

	if(rMin < 1) {
		if(rW < rH) lt = QPoint(0,(float) imgQt.height() * (rH - rMin) / 2.0f);
		else {
			 lt = QPoint((float) imgQt.width() * (rW - rMin) / 2.0f, 0);
		}
	}
	else lt = QPoint((previewWidth - imgQt.width()) / 2.0f, (previewHeight - imgQt.height()) / 2.0f);

	QSize imgSizeScaled = QSize(imgQt.size());
	if(rMin < 1) imgSizeScaled *= rMin;

	previewImgRect = QRect(lt, imgSizeScaled);

	previewImgRect.setTop(previewImgRect.top()+1);
	previewImgRect.setLeft(previewImgRect.left()+1);
	previewImgRect.setWidth(previewImgRect.width()-1);			// we have a border... correct that...
	previewImgRect.setHeight(previewImgRect.height()-1);

	if(rMin < 1) imgPreview = imgQt.scaled(imgSizeScaled, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	else imgPreview = imgQt;
	
	if (imgPreview.format() == QImage::Format_Mono || imgPreview.format() == QImage::Format_MonoLSB || 
		imgPreview.format() == QImage::Format_Indexed8 || imgPreview.isGrayscale()) emit isNotGrayscaleImg(false);

	/*
#ifdef WITH_OPENCV
	
	Mat imgMat = DkImage::qImage2Mat(imgPreview);
	// imgMat.convertTo(imgMat, CV_32FC1, 1.0f/255.0f);  // for testing purposes
	
	DkMagicCutWidget::setMatImg(imgMat);
	DkMagicCutWidget::setOrigMatImg(imgMat);
#endif*/
	drawImgPreview();
}

/**
* Update current preview image
* @param updated The new image
* \sa DkMagiCutDialog::drawImgPreview()
**/
void DkMagicCutDialog::updateImg(QImage updatedImg) {

	imgPreview = updatedImg;
	drawImgPreview();
}

/**
* Draw the currently set preview image in the dialog
**/
void DkMagicCutDialog::drawImgPreview() {

	QImage preview = QImage(previewWidth,previewHeight, QImage::Format_ARGB32);
	preview.fill(Qt::transparent);
	QPainter painter(&preview);
	painter.setPen(QColor(0,0,0));
	painter.drawRect(0, 0, previewWidth - 1, previewHeight - 1);
	painter.setBackgroundMode(Qt::TransparentMode);
	painter.drawImage(previewImgRect, imgPreview);

	previewLabel->setPixmap(QPixmap::fromImage(preview));
}

/**
* Called when saved is pressed and emits a savedPressed signal for the viewport
* \sa DkDocAnalysisViewPort::saveMagicCutPressed
**/
void DkMagicCutDialog::savePressed() {

	/*QString defaultName = QString("_%1").arg((int)(magicCut->getBoundingRect()->x+magicCut->getBoundingRect()->width/2));
	defaultName.append(QString("_%1").arg((int)(magicCut->getBoundingRect()->y+magicCut->getBoundingRect()->height/2)));
	*/
	//int xCoord = (int)(magicCut->getBoundingRect()->x+magicCut->getBoundingRect()->width/2);
	//int yCoord = (int)(magicCut->getBoundingRect()->y+magicCut->getBoundingRect()->height/2);

	//int xCoord = (int)(magicCut->getBoundingRect()->x);
	//int yCoord = (int)(magicCut->getBoundingRect()->y);
	//int width = (int)(magicCut->getBoundingRect()->width);
	//int height = (int)(magicCut->getBoundingRect()->height);
	int xCoord = roiRect->x;
	int yCoord = roiRect->y;
	int width = roiRect->width;
	int height = roiRect->height;

	emit savePressed(imgQt, xCoord, yCoord, height, width);

	isSaved = true;
	this->close();
}

/**
* Called when cancel is pressed - closes the dialog
**/
void DkMagicCutDialog::cancelPressed() {

	this->close();
}

/**
* Called when apply mask is pressed in the dialog.
* Creates a new image preview with or without the alpha channel mask.
* @param checked 0 = without mask, else with mask
* \sa DkMagicCutDialog::createImgPreview
**/
void DkMagicCutDialog::applyMaskChecked(int checked) {

	if(checked == 0) withMask = false;
	else withMask = true;
	
	createImgPreview();
}

/**
* Event function - called when the dialog is displayed for the first time.
* Creates and draws the image preview
* \sa DkMagicCutDialog::createImgPreview()
**/
void DkMagicCutDialog::showEvent(QShowEvent *event) {
	isSaved = false;

	if(!event->spontaneous()) {

		// create a binary mask
		binaryMask = magicCut->getMask()->clone();
		binaryMask.setTo(1, binaryMask >= 0);
		/*uchar *ptr;
		for (int i=0; i < binaryMask.rows; i++)
		{
			for (int j=0; j <binaryMask.cols; j++)
			{
				ptr = binaryMask.data + binaryMask.step * i;
				if ( (int)(ptr[j]) >= 0 )
				{
					ptr[j] = 1;
				}
			}
		}*/

		createImgPreview();
		drawImgPreview();
	}
}


/**
* Constructor for the slider widget.
* Initializes, generates layout, connects signals.
**/
DkBoundingBoxSliderWidget::DkBoundingBoxSliderWidget(QWidget *parent, DkMagicCutDialog *parentDialog) 
	: QWidget(parent){

	name = QString("DkBoundingBoxSliderWidget");

	this->parentDialog = parentDialog;

	defaultValue = 0;
	leftSpacing = 10;
	topSpacing = 10;
	
	this->sliderLength = parent->minimumWidth() - 2 * leftSpacing - 15;
	
	minVal = -30;
	middleVal = defaultValue;
	maxVal = 30;

	sliderTitle = new QLabel(tr("Bounding Box Offset"), this);
	sliderTitle->move(leftSpacing, topSpacing);

	slider = new QSlider(this);
	slider->setMinimum(minVal);
	slider->setMaximum(maxVal);
	slider->setValue(middleVal);
	slider->setTickInterval(50);
	slider->setOrientation(Qt::Horizontal);
	slider->setTickPosition(QSlider::TicksBelow);
	slider->setGeometry(QRect(leftSpacing, sliderTitle->geometry().bottom() - 5, sliderLength, 20));

	slider->setStyleSheet(
		QString("QSlider::groove:horizontal {border: 1px solid #999999; height: 4px; margin: 2px 0;")
		+ QString("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3c3c3c, stop:1 #c8c8c8) ")
		+ QString(";} ")
		+ QString("QSlider::handle:horizontal {background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #d2d2d2, stop:1 #e6e6e6); border: 1px solid #5c5c5c; width: 6px; margin:-4px 0px -6px 0px ;border-radius: 3px;}"));

	sliderSpinBox = new QSpinBox(this);
	sliderSpinBox->setGeometry(slider->geometry().right() - 45, sliderTitle->geometry().top(), 45, 20);
	sliderSpinBox->setMinimum(minVal);
	sliderSpinBox->setMaximum(maxVal);
	sliderSpinBox->setValue(slider->value());

	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateSliderSpinBox(int)));
	connect(sliderSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSliderVal(int)));

	minValLabel = new QLabel(QString::number(minVal), this);
	minValLabel->move(leftSpacing, slider->geometry().bottom());

	middleValLabel = new QLabel(QString::number(middleVal), this);
	middleValLabel->setMinimumWidth(15);
	middleValLabel->move(leftSpacing + sliderLength / 2 - 2, slider->geometry().bottom());

	maxValLabel = new QLabel(QString::number(maxVal), this);
	maxValLabel->move(slider->geometry().right() - 20, slider->geometry().bottom());
	
	BBoffset = slider->value();

};

DkBoundingBoxSliderWidget::~DkBoundingBoxSliderWidget() {


};

/**
 * Slider spin box slot: update value and redraw image.
 * Creates and updates the image preview of the parent dialog
 * @param val the new offset value
 * \sa DkMagicCutDialog::createImgPreview()
 **/
void DkBoundingBoxSliderWidget::updateSliderSpinBox(int val) {

	BBoffset = val;
	this->sliderSpinBox->setValue(val);
	this->middleValLabel->setText(QString::number(val));
	parentDialog->createImgPreview();
};

/**
 * Slider slot: update value and redraw image
 * @param changed value
 * \sa DkMagicCutDialog::createImgPreview()
 **/
void DkBoundingBoxSliderWidget::updateSliderVal(int val) {

	BBoffset = val;
	this->slider->setValue(val);
	this->middleValLabel->setText(QString::number(val));
	parentDialog->createImgPreview();
};

};