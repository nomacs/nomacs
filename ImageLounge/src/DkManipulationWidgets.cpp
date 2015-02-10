/*******************************************************************************************************
 DkManipulationWidgets.cpp
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

#include "DkManipulationWidgets.h"

namespace nmc {

// static variables
int DkImageManipulationWidget::brightness;
int DkImageManipulationWidget::contrast;
int DkImageManipulationWidget::saturation;
int DkImageManipulationWidget::hue;
float DkImageManipulationWidget::gamma;
float DkImageManipulationWidget::exposure;
char DkImageManipulationWidget::manipulationType;
bool DkImageManipulationWidget::slidersReset;
std::vector<historyData> DkImageManipulationWidget::historyDataVec;
std::vector<DkImageManipulationWidget*> DkImageManipulationWidget::historyToolsVec;
bool DkImageManipulationWidget::prepareUndo;
QPushButton* DkUndoRedo::buttonUndo;
QPushButton* DkUndoRedo::buttonRedo;
bool DkImageManipulationWidget::doARedraw;
#ifdef WITH_OPENCV
		Mat DkImageManipulationWidget::imgMat;
		Mat DkImageManipulationWidget::origMat;
		Mat DkImageManipulationWidget::tempLUT;
#endif

/**
* Image manipulation dialog with image manipulation tools and preview
 **/
DkImageManipulationDialog::DkImageManipulationDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	init();
}

DkImageManipulationDialog::~DkImageManipulationDialog() {

}

/**
* initialize image manipulation dialog - set sizes
 **/
void DkImageManipulationDialog::init() {

	dialogWidth = 700;
	dialogHeight = 600;
	toolsWidth = 200;
	previewMargin = 20;
	previewWidth = dialogWidth - toolsWidth - 2 * previewMargin;
	previewHeight = dialogHeight - previewMargin - 70;

	setWindowTitle(tr("Image Manipulation Tools"));
	setFixedSize(dialogWidth, dialogHeight);
	createLayout();

	DkImageManipulationWidget::clearHistoryVectors();
	DkImageManipulationWidget::setEmptyManipulationType();
#ifdef WITH_OPENCV
	DkImageManipulationWidget::createMatLut();
#endif
}
/**
* reset dialog slider values
 **/
void DkImageManipulationDialog::resetValues() {
	
	brightnessWidget->setToolsValue((float)brightnessWidget->getDefaultValue());
	contrastWidget->setToolsValue((float)contrastWidget->getDefaultValue());
	saturationWidget->setToolsValue((float)saturationWidget->getDefaultValue());
	hueWidget->setToolsValue((float)hueWidget->getDefaultValue());
	gammaWidget->setToolsValue((float)gammaWidget->getDefaultValueF());
	exposureWidget->setToolsValue((float)exposureWidget->convertSliderValToExposure(exposureWidget->getDefaultValueF()));

	DkImageManipulationWidget::clearHistoryVectors();
	DkImageManipulationWidget::setEmptyManipulationType();
#ifdef WITH_OPENCV
	DkImageManipulationWidget::createMatLut();
#endif
	DkImageManipulationWidget::setPrepareUndo(true);
	DkUndoRedo::enableUndoButton(false);
	DkUndoRedo::enableRedoButton(false);

	emit isNotGrayscaleImg(true);
}

/**
* create image manipulation dialog layout
 **/
void DkImageManipulationDialog::createLayout() {

	// central widget - preview image
	QWidget* centralWidget = new QWidget(this);
	previewLabel = new QLabel(centralWidget);
	previewLabel->setGeometry(QRect(QPoint(previewMargin, previewMargin), QSize(previewWidth, previewHeight)));

	// east widget - sliders
	QWidget* eastWidget = new QWidget(this);
	eastWidget->setMinimumWidth(toolsWidth);
	eastWidget->setMaximumWidth(toolsWidth);
	eastWidget->setContentsMargins(0,10,10,0);
	QVBoxLayout* toolsLayout = new QVBoxLayout(eastWidget);
	toolsLayout->setContentsMargins(0,0,0,0);

	brightnessWidget = new DkBrightness(eastWidget, this);
	contrastWidget = new DkContrast(eastWidget, this);
	saturationWidget = new DkSaturation(eastWidget, this);
	hueWidget = new DkHue(eastWidget, this);
	gammaWidget = new DkGamma(eastWidget, this);
	exposureWidget = new DkExposure(eastWidget, this);
	undoredoWidget = new DkUndoRedo(eastWidget, this);

	connect(this, SIGNAL(isNotGrayscaleImg(bool)), saturationWidget, SLOT(setEnabled(bool)));
	connect(this, SIGNAL(isNotGrayscaleImg(bool)), hueWidget, SLOT(setEnabled(bool)));
	connect(hueWidget, SIGNAL(setSaturationSliderColor(QString)), saturationWidget, SLOT(setSliderStyle(QString)));

	toolsLayout->addWidget(brightnessWidget);
	toolsLayout->addWidget(contrastWidget);
	toolsLayout->addWidget(saturationWidget);
	toolsLayout->addWidget(hueWidget);
	toolsLayout->addWidget(gammaWidget);
	toolsLayout->addWidget(exposureWidget);
	toolsLayout->addWidget(undoredoWidget);

	eastWidget->setLayout(toolsLayout);
	
	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	buttons->setContentsMargins(10,10,10,10);
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	BorderLayout* borderLayout = new BorderLayout;
	borderLayout->addWidget(buttons, BorderLayout::South);
	borderLayout->addWidget(centralWidget, BorderLayout::Center);
	borderLayout->addWidget(eastWidget, BorderLayout::East);
	this->setSizeGripEnabled(false);

	this->setLayout(borderLayout);
}

/**
* rescale viewport image - it is used as a preview for the manipulation tools changes
 **/
void DkImageManipulationDialog::createImgPreview() {

	if (!img || img->isNull())
		return;
	
	QPoint lt;
	float rW = previewWidth / (float) img->width();
	float rH = previewHeight / (float) img->height();
	float rMin = (rW < rH) ? rW : rH;

	if(rMin < 1) {
		if(rW < rH) lt = QPoint(0, qRound(img->height() * (rH - rMin) / 2.0f));
		else {
			 lt = QPoint(qRound(img->width() * (rW - rMin) / 2.0f), 0);
		}
	}
	else lt = QPoint(qRound((previewWidth - img->width()) / 2.0f), qRound((previewHeight - img->height()) / 2.0f));

	QSize imgSizeScaled = QSize(img->size());
	if(rMin < 1) imgSizeScaled *= rMin;

	previewImgRect = QRect(lt, imgSizeScaled);

	previewImgRect.setTop(previewImgRect.top()+1);
	previewImgRect.setLeft(previewImgRect.left()+1);
	previewImgRect.setWidth(previewImgRect.width()-1);			// we have a border... correct that...
	previewImgRect.setHeight(previewImgRect.height()-1);

	if(rMin < 1) imgPreview = img->scaled(imgSizeScaled, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	else imgPreview = *img;
	
	if (imgPreview.format() == QImage::Format_Mono || imgPreview.format() == QImage::Format_MonoLSB || 
		imgPreview.format() == QImage::Format_Indexed8 || imgPreview.isGrayscale()) emit isNotGrayscaleImg(false);

#ifdef WITH_OPENCV
	
	Mat imgMat = DkImage::qImage2Mat(imgPreview);
	// imgMat.convertTo(imgMat, CV_32FC1, 1.0f/255.0f);  // for testing purposes
	
	DkImageManipulationWidget::setMatImg(imgMat);
	DkImageManipulationWidget::setOrigMatImg(imgMat);
#endif

}

/**
* update current preview image
* @param updated image
**/
void DkImageManipulationDialog::updateImg(QImage updatedImg) {

	imgPreview = updatedImg;
	drawImgPreview();
}

/**
* draw preview image
 **/
void DkImageManipulationDialog::drawImgPreview() {

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
 * constructor for the abstract class DkImageManipulationWidget - all image manipulation widgets are created from it
 * @param parent widget
 * @param viewport - needed for getting current images
 **/
DkImageManipulationWidget::DkImageManipulationWidget(QWidget *parent, DkImageManipulationDialog *parentDialog)
	: QWidget(parent) {

	this->leftSpacing = 10;
	this->topSpacing = 10;
	this->margin = 10;
	this->sliderLength = parent->minimumWidth() - 2 * leftSpacing;
	this->valueUpdated = false;
	this->manipDialog = parentDialog;
	slidersReset = false;
	this->doARedraw = true;

	connect(this, SIGNAL(updateDialogImgSignal(QImage)), parent->parentWidget(), SLOT(updateImg(QImage)));

	// create gradient for changing saturation slider background
	hueGradientImg = QImage(181, 10, QImage::Format_ARGB32);
	QLinearGradient hueGradient = QLinearGradient(hueGradientImg.rect().topLeft(), hueGradientImg.rect().topRight());
	hueGradient.setColorAt(0,  QColor("#ff0000"));
	hueGradient.setColorAt(0.167,  QColor("#ffff00"));
	hueGradient.setColorAt(0.333,  QColor("#00ff00"));
	hueGradient.setColorAt(0.5,  QColor("#00ffff"));
	hueGradient.setColorAt(0.666,  QColor("#0000ff"));
	hueGradient.setColorAt(0.833,  QColor("#ff00ff"));
	hueGradient.setColorAt(1,  QColor("#ff0000"));

	QPainter painter(&hueGradientImg);
	painter.fillRect(hueGradientImg.rect(), hueGradient);
	painter.end();

};

DkImageManipulationWidget::~DkImageManipulationWidget() {


};

/**
 * slider spin box slot: update value and redraw image
 * @param changed value
 **/
void DkImageManipulationWidget::updateSliderSpinBox(int val) {

	if(!valueUpdated) {
		valueUpdated = true;
		this->sliderSpinBox->setValue(val);
		if (!slidersReset && doARedraw) redrawImage();
	}
	else valueUpdated = false;

};

/**
 * slider double spin box slot: update value and redraw image
 * @param changed value
 **/
void DkImageManipulationWidget::updateDoubleSliderSpinBox(int val) {

	if(!valueUpdated) {
		valueUpdated = true;
		if (this->name.compare("DkGamma") != 0) this->sliderSpinBoxDouble->setValue(val/100.0);
		else this->sliderSpinBoxDouble->setValue(this->gammaSliderValues[val]);
		if (!slidersReset && doARedraw) redrawImage();
	}
	else valueUpdated = false;

};

/**
 * slider slot: update value and redraw image
 * @param changed value
 **/
void DkImageManipulationWidget::updateSliderVal(int val) {

	if(!valueUpdated) {
		valueUpdated = true;
		this->slider->setValue(val);
		if (!slidersReset && doARedraw) redrawImage();
	}
	else valueUpdated = false;

};

/**
 * slider slot: update value from double spinbox and redraw image
 * @param changed value
 **/
void DkImageManipulationWidget::updateDoubleSliderVal(double val) {
	
	if(!valueUpdated) {
		valueUpdated = true;		
		if (this->name.compare("DkGamma") != 0) this->slider->setValue(qRound(val * 100));
		else this->slider->setValue(findClosestValue(this->gammaSliderValues, val, 0, 199));		
		if (!slidersReset && doARedraw) redrawImage();
	}
	else valueUpdated = false;

};

/**
* reset all image manipulation tools slider except the one in the input parameter
* needed when changing from one slider to another
**/
void DkImageManipulationWidget::resetSliderValues(char exceptionSlider) {
	
	slidersReset = true;
	if (exceptionSlider != manipulationBrightness) manipDialog->getBrightnessWidget()->setToolsValue((float)manipDialog->getBrightnessWidget()->getDefaultValue());
	if (exceptionSlider != manipulationContrast) manipDialog->getContrastWidget()->setToolsValue((float)manipDialog->getContrastWidget()->getDefaultValue());
	if (exceptionSlider != manipulationSaturation) manipDialog->getSaturationWidget()->setToolsValue((float)manipDialog->getSaturationWidget()->getDefaultValue());
	if (exceptionSlider != manipulationHue) manipDialog->getHueWidget()->setToolsValue((float)manipDialog->getHueWidget()->getDefaultValue());
	if (exceptionSlider != manipulationGamma) manipDialog->getGammaWidget()->setToolsValue((float)manipDialog->getGammaWidget()->getDefaultValueF());
	if (exceptionSlider != manipulationExposure) manipDialog->getExposureWidget()->setToolsValue((float)
		manipDialog->getExposureWidget()->convertSliderValToExposure(manipDialog->getExposureWidget()->getDefaultValueF()));
	slidersReset = false;
}

/**
* set a new tool value
* @param new value
**/
void DkImageManipulationWidget::setToolsValue(float val) {

	if (this->name.compare("DkBrightness") == 0) { brightness = (int)val; slider->setValue((int)val);}
	else if (this->name.compare("DkContrast") == 0) { contrast = (int)val; slider->setValue((int)val);}
	else if (this->name.compare("DkSaturation") == 0) { saturation = (int)val; slider->setValue((int)val);}
	else if (this->name.compare("DkHue") == 0) { hue = (int)val; slider->setValue((int)val);}
	else if (this->name.compare("DkGamma") == 0) { gamma = val; sliderSpinBoxDouble->setValue(val);}
	else if (this->name.compare("DkExposure") == 0) { exposure = val; sliderSpinBoxDouble->setValue(manipDialog->getExposureWidget()->convertExposureToSliderVal(val));}

};

/**
* overloaded method for setting a new tool value for a pairs of sliders
* @param new value 1
* @param new value 2
**/
void DkImageManipulationWidget::setToolsValue(float val1, float val2) {

	if (this->name.compare("DkBrightness") == 0 || this->name.compare("DkContrast") == 0) { 
		brightness = (int)val1; 
		manipDialog->getBrightnessWidget()->slider->setValue((int)val1);
		contrast = (int)val2; 
		manipDialog->getContrastWidget()->slider->setValue((int)val2);
	}
	if (this->name.compare("DkSaturation") == 0 || this->name.compare("DkHue") == 0) { 
		saturation = (int)val1; 
		manipDialog->getSaturationWidget()->slider->setValue((int)val1);
		hue = (int)val2; 
		manipDialog->getHueWidget()->slider->setValue((int)val2);
	}
	else if (this->name.compare("DkGamma") == 0) { 
		gamma = val1; 
		sliderSpinBoxDouble->setValue(val1);
	}
	else if (this->name.compare("DkExposure") == 0) { 
		exposure = val1; 
		sliderSpinBoxDouble->setValue(manipDialog->getExposureWidget()->convertExposureToSliderVal(val1));
	}

};

/**
* return tool value
* @return tool value
**/
float DkImageManipulationWidget::getToolsValue() {

	if (this->name.compare("DkBrightness") == 0) return (float)slider->value();
	else if (this->name.compare("DkContrast") == 0) return (float)slider->value();
	else if (this->name.compare("DkSaturation") == 0) return (float)slider->value();
	else if (this->name.compare("DkHue") == 0) return (float)slider->value();
	else if (this->name.compare("DkGamma") == 0) return (float)sliderSpinBoxDouble->value();
	else if (this->name.compare("DkExposure") == 0) return (float)sliderSpinBoxDouble->value();
	else return 0;
};

/**
* bisection for finding the number in input array that is closest to the selected number
* @param input array
* @param number to search for
* @param bisection lower interval
* @param bisection upper interval
* @return the closest number found
**/
int DkImageManipulationWidget::findClosestValue(double *values, double closestVal, int i1, int i2) {

	if(i1==i2) return i1;
	else if((i2 - i1) == 1) {

		double x1 = closestVal - values[i1];
		double x2 = values[i2] - closestVal;
		if (x1 == x2) return i2;
		else if (x1 > x2) return i2;
		else return i1;
	}

	int iM = (i1 + i2)/2;
	if (values[iM] == closestVal) return iM;

	if(closestVal > values[iM]) return findClosestValue(values, closestVal, iM, i2);
	else return findClosestValue(values, closestVal, i1, iM);
}

// clear manipulation tools history vectors
void DkImageManipulationWidget::clearHistoryVectors() {
	
	historyDataVec = std::vector<historyData>();
	historyToolsVec= std::vector<DkImageManipulationWidget*>();
};

// enable undo/redo functions
void DkImageManipulationWidget::prepareUndoRedoButtons() {

	prepareUndo = true;
	DkUndoRedo::enableUndoButton(true);
	DkUndoRedo::enableRedoButton(false);
};

#ifdef WITH_OPENCV

// when the dialog is created create the lookuptable
void DkImageManipulationWidget::createMatLut() {

	tempLUT = createMatLut16();
}

/**
 * create initial 3 channels 16 bit lookup table with numbers from 0 .. 65535
 * @return 16 bit lut
 **/
Mat DkImageManipulationWidget::createMatLut16() {

	Mat lut = Mat(3, 65536, CV_16UC1);
	unsigned short *ptrU;
	for(int i = 0; i < 3; i++) {
		ptrU = lut.ptr<unsigned short>(i);
		for (int j = 0; j < 65536; j++) ptrU[j] = (unsigned short)j;
	}

	return lut;
}

/**
 * apply a calculated CV_16U lookup table to an Mat image
 * @param input image
 * @param input LUT
 * @param is there a need to change to the HSV space (for changes of saturation and hue)
 * @return modified image
 **/
Mat DkImageManipulationWidget::applyLutToImage(Mat inImg, Mat inLUT, bool isMatHsv) {	

	Mat tempImg;

	if(isMatHsv) {
		cvtColor(inImg, tempImg, CV_RGB2HSV);
	}
	else tempImg = inImg.clone();

	std::vector<Mat> imgCh;
	split(tempImg, imgCh);

	unsigned short *ptrLutR = inLUT.ptr<unsigned short>(0);
	unsigned short *ptrLutG = inLUT.ptr<unsigned short>(1);
	unsigned short *ptrLutB = inLUT.ptr<unsigned short>(2);

	if (tempImg.depth() == CV_32F) {

		if (tempImg.channels() < 3) {

			for (int row = 0; row < tempImg.rows; row++)
			{
				float *ptrR = imgCh[0].ptr<float>(row);

				for (int col = 0; col < tempImg.cols; col++) ptrR[col] = (float)ptrLutR[cvRound(ptrR[col] * (inLUT.cols-1))] / 65535.0f;

			}
		}
		else {

			for (int row = 0; row < tempImg.rows; row++)
			{
				float *ptrR = imgCh[0].ptr<float>(row);
				float *ptrG = imgCh[1].ptr<float>(row);
				float *ptrB = imgCh[2].ptr<float>(row);

				for (int col = 0; col < tempImg.cols; col++)
				{
					if(isMatHsv) {

						ptrR[col] = (float)ptrLutR[cvRound(ptrR[col] / 360.0f * (inLUT.cols-1))] / 65535.0f * 360.0f;
						ptrG[col] = (float)ptrLutG[cvRound(ptrG[col] * (inLUT.cols-1))] / 65535.0f;
						ptrB[col] = (float)ptrLutB[cvRound(ptrB[col] * (inLUT.cols-1))] / 65535.0f;
					}
					else {

						ptrR[col] = (float)ptrLutR[cvRound(ptrR[col] * (inLUT.cols-1))] / 65535.0f;
						ptrG[col] = (float)ptrLutG[cvRound(ptrG[col] * (inLUT.cols-1))] / 65535.0f;
						ptrB[col] = (float)ptrLutB[cvRound(ptrB[col] * (inLUT.cols-1))] / 65535.0f;
					}
				}
			}
			merge(imgCh, tempImg);
		}
	}
	else if (tempImg.depth() == CV_8U) {

		if (tempImg.channels() < 3) {

			for (int row = 0; row < tempImg.rows; row++)
			{
				unsigned char *ptrR = imgCh[0].ptr<unsigned char>(row);

				for (int col = 0; col < tempImg.cols; col++) ptrR[col] = (unsigned char) cvRound(ptrLutR[cvRound((ptrR[col] / 255.0f) * (inLUT.cols-1))] / 257.0f);

			}
		}
		else {

			for (int row = 0; row < tempImg.rows; row++)
			{
				unsigned char *ptrR = imgCh[0].ptr<unsigned char>(row);
				unsigned char *ptrG = imgCh[1].ptr<unsigned char>(row);
				unsigned char *ptrB = imgCh[2].ptr<unsigned char>(row);

				for (int col = 0; col < tempImg.cols; col++)
				{
					if(isMatHsv) {

						ptrR[col] = (unsigned char) cvRound(ptrLutR[cvRound((ptrR[col] / 180.0f) * (inLUT.cols-1))] / 65535.0f * 180.0f);
						ptrG[col] = (unsigned char) cvRound(ptrLutG[cvRound((ptrG[col] / 255.0f) * (inLUT.cols-1))] / 257.0f);
						ptrB[col] = (unsigned char) cvRound(ptrLutB[cvRound((ptrB[col] / 255.0f) * (inLUT.cols-1))] / 257.0f);
					}
					else {

						ptrR[col] = (unsigned char) cvRound(ptrLutR[cvRound((ptrR[col] / 255.0f) * (inLUT.cols-1))] / 257.0f);
						ptrG[col] = (unsigned char) cvRound(ptrLutG[cvRound((ptrG[col] / 255.0f) * (inLUT.cols-1))] / 257.0f);
						ptrB[col] = (unsigned char) cvRound(ptrLutB[cvRound((ptrB[col] / 255.0f) * (inLUT.cols-1))] / 257.0f);
					}
				}
			}
			merge(imgCh, tempImg);
		}
	}
	
	if(isMatHsv) {
		Mat retImg;
		cvtColor(tempImg, retImg, CV_HSV2RGB);

		if(inImg.type() == CV_8UC4) {	// the retImg is always CV_8UC3, so for pics in CV_8UC4 we need to add one channel
			std::vector<Mat> inImgCh;
			split(inImg, inImgCh);
			std::vector<Mat> retImgCh;
			split(retImg, retImgCh);
			retImgCh.push_back(inImgCh[3]);
			merge(retImgCh, retImg);
		}
		return retImg;

	}
	else return tempImg;
}

/**
 * called from DkNoMacs.cpp: applies manipulation history to the viewport image
 * @param input image
 * @return modified image
 **/
Mat DkImageManipulationWidget::manipulateImage(Mat inImg){
	
	Mat outImg;
	Mat nullImg;

	if (historyToolsVec.size() > 0) {

		QProgressDialog* progress = new QProgressDialog("Applying changes to image...", "Cancel", 0, 100);
		int step = (int) (100 / historyToolsVec.size());
		progress->setWindowModality(Qt::WindowModal);
		progress->setValue(1);	// a strange behavior of the progress dialog: first setValue shows an empty dialog (setting to zero won't work)
		progress->setValue(2);	// second setValue shows the progress bar with 2% (setting to zero won't work)
		progress->setValue(0);	// finally set the progress to zero

		outImg = inImg.clone();
		Mat lut16 = createMatLut16();
		unsigned int i;
		for (i = 0; i < historyToolsVec.size(); i++) {

			Mat lut = lut16.clone();
			lut = historyToolsVec[i]->compute(lut, historyDataVec[i].arg1, historyDataVec[i].arg2);
			progress->setValue((int)(step*(i+0.5f)));
			if (progress->wasCanceled()) break;
			outImg = applyLutToImage(outImg, lut, historyDataVec[i].isHsv);
			progress->setValue(step*(i+1)-1);
			if (progress->wasCanceled()) break;
		}

		progress->close(); 

		if (historyToolsVec.size() != i) return nullImg;
	}

	return outImg;

}

/**
 * change brightness or contrast of an image
 * @param input LUT
 * @param brightness value
 * @param contrast value
 * @return changed LUT
 **/
Mat DkImageManipulationWidget::changeBrightnessAndContrast(Mat inLUT, float brightnessVal, float contrastVal) {

	Mat outLUT = inLUT.clone();

	unsigned short* ptrU;
	double delta;
	double a, b;
	
	// the code for changing brightness and contrast is based on the code from: http://mehrez.kristou.org/opencv-change-contrast-and-brightness-of-an-image/
	if( contrastVal > 0 ) {

		delta = 127.0f * contrastVal / 100.0f;
		a = 255.0f / (255.0f - delta * 2.0f);
		b = a * (brightnessVal - delta);
	}
	else {

		delta = -128.0f * contrastVal / 100.0f;
		a = (256.0f - delta * 2.0f) / 255.0f;
		b = a * brightnessVal + delta;
	}

	for(int i = 0; i < 3; i++) {

		ptrU = outLUT.ptr<unsigned short>(i);

		for (int col = 0; col < outLUT.cols; col++) {

			int v = cvRound((a * ptrU[col] / 257.0f + b) * 257);
			if (v < 0) v = 0;
			else if (v > 65535) v = 65535;

			ptrU[col] = (unsigned char)v;
		}
	}

	return outLUT;
};

/**
 * change saturation or hue of an image
 * @param input LUT
 * @param saturation value
 * @param hue value
 * @return changed LUT
 **/
Mat DkImageManipulationWidget::changeSaturationAndHue(Mat inLUT, float saturationVal, float hueVal) {

	Mat outLUT = inLUT.clone();

	hueVal *= -1.0f;	// fix the difference between label and real hue

	// make a gaussian kernel for positive saturation -> this weights bright and dark colors less -> results in uniform saturation change
	int ks = 65536;
	float sigma = ks/6.0f;
	Mat gKernel = Mat(1, ks, CV_32FC1);
	float* kernelPtr = gKernel.ptr<float>();

	for (int idx = 0, x = -cvFloor(ks/2); idx < ks; idx++,x++) 
		kernelPtr[idx] = (float)(exp(-(x*x)/(2*sigma*sigma)));	// 1/(sqrt(2pi)*sigma) -> discrete normalization
	normalize(gKernel, gKernel, 1.0f, 0.0f, NORM_MINMAX);

	//for (int idx = 0, x = -cvFloor(ks/2); idx < ks; idx++,x++) 
	//	kernelPtr[idx] = ((idx < ks*0.5f) ? idx/(ks*0.5f) : 2.0f - (float)idx/(ks*0.5f));	// 1/(sqrt(2pi)*sigma) -> discrete normalization
	////normalize(gKernel, gKernel, 1.0f, 0.0f, NORM_MINMAX);

	float sat = saturationVal * 257;  //convert to 16 bit

	const unsigned short* vPtrU;
	unsigned short* sPtrU;
	unsigned short* hPtrU;
	
	hPtrU = outLUT.ptr<unsigned short>(0);  // first channel (hue)
	sPtrU = outLUT.ptr<unsigned short>(1);	// second channel (saturation)
	vPtrU = outLUT.ptr<unsigned short>(2);	// third channel (value)
		
	for (int col = 0; col < outLUT.cols; col++) {

		if (sat > 0) {
			sPtrU[col] += (unsigned short)cvRound(sat * kernelPtr[cvRound(vPtrU[col]/65535.0f*(ks-1))]* (1.0f-sPtrU[col]/65535.0f));
		}
		else {
			// linear for negative saturation values
			if (sPtrU[col] > -sat)
				sPtrU[col] += (unsigned short)sat;
			else
				sPtrU[col] = 0;
		}

		int newHue = hPtrU[col] + cvRound(hueVal * 65535.0f / 360.0f);
		if(newHue >= 65535) newHue -= 65535;
		else if(newHue < 0) newHue += 65535;

		hPtrU[col] = (unsigned short)newHue;
	}

	return outLUT;
}

/**
 * change gamma of an image
 * @param input LUT
 * @param gamma value
 * @return changed LUT
 **/
Mat DkImageManipulationWidget::changeGamma(Mat inLUT, float g) {

	Mat outLUT = inLUT.clone();

	unsigned short* ptrU;
	for(int i = 0; i < 3; i++) {

		ptrU = outLUT.ptr<unsigned short>(i);

		for (int col = 0; col < outLUT.cols; col++) {

			ptrU[col] = (unsigned short)cvRound(pow(ptrU[col] / (float)(outLUT.cols - 1), 1.0f/g) * (outLUT.cols - 1));
		}
	}

	return outLUT;
}

/**
 * change exposure of an image
 * @param input LUT
 * @param exposure value
 * @return changed LUT
 **/
Mat DkImageManipulationWidget::changeExposure(Mat inLUT, float exposure) {

/*
The code for changing the exposire is based on LibRaw implementation

LibRaw is free software; you can redistribute it and/or modify
it under the terms of the one of three licenses as you choose:

1. GNU LESSER GENERAL PUBLIC LICENSE version 2.1
   (See file LICENSE.LGPL provided in LibRaw distribution archive for details).

2. COMMON DEVELOPMENT AND DISTRIBUTION LICENSE (CDDL) Version 1.0
   (See file LICENSE.CDDL provided in LibRaw distribution archive for details).

3. LibRaw Software License 27032010
   (See file LICENSE.LibRaw.pdf provided in LibRaw distribution archive for details).
*/

	Mat outLUT = inLUT.clone();

	unsigned short* ptrU;
		
	if(exposure <= 1) 
		for(int i = 0; i < 3; i++) {

			ptrU = outLUT.ptr<unsigned short>(i);
			for (int col = 0; col < outLUT.cols; col++) {
				ptrU[col] = (unsigned short)cvRound(ptrU[col] * exposure);
			}
		}
	else {

		float x1,x2,y1,y2;
		float smooth = 0.5f;

		float cstops = log(exposure)/log(2.0f);
		float room = cstops*2;
		float roomlin = powf(2.0f,room);
		x2 = 65535.0;
		x1 = (x2+1)/roomlin-1;
		y1 = x1*exposure;
		y2 = x2*(1+(1-smooth)*(exposure-1));
		float sq3x=powf(x1*x1*x2,1.0f/3.0f);
		float B = (y2-y1+exposure*(3*x1-3.0f*sq3x)) / (x2+2.0f*x1-3.0f*sq3x);
		float A = (exposure - B)*3.0f*powf(x1*x1,1.0f/3.0f);
		float CC = y2 - A*powf(x2,1.0f/3.0f)-B*x2;
		
		for(int i = 0; i < 3; i++) {

			ptrU = outLUT.ptr<unsigned short>(i);
			for (int col = 0; col < outLUT.cols; col++) {

				float X = (float) ptrU[col];
				float Y = A*powf(X, 1.0f/3.0f) + B*X + CC;
				if (ptrU[col] < x1) ptrU[col] = (unsigned short)cvRound((float) ptrU[col] * exposure);
				else ptrU[col] = (Y < 0) ? 0 : ((Y > 65535) ? 65535 : (unsigned short)cvRound(Y));
			}
		}
	}

	return outLUT;
}
#endif

// Brightness widget
DkBrightness::DkBrightness(QWidget *parent, DkImageManipulationDialog *parentDialog) 
	: DkImageManipulationWidget(parent, parentDialog){

	name = QString("DkBrightness");
	defaultValue = 0;

	minVal = -100;
	middleVal = defaultValue;
	maxVal = 100;

	sliderTitle = new QLabel(tr("Brightness"), this);
	sliderTitle->move(leftSpacing, topSpacing);

	slider = new QSlider(this);
	slider->setMinimum(minVal);
	slider->setMaximum(maxVal);
	slider->setValue(middleVal);
	slider->setTickInterval(50);
	slider->setOrientation(Qt::Horizontal);
	slider->setTickPosition(QSlider::TicksBelow);
	slider->setGeometry(QRect(leftSpacing, sliderTitle->geometry().bottom() - 5, sliderLength, 20));
	slider->setObjectName("DkBrightnessSlider");

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
	middleValLabel->move(leftSpacing + sliderLength / 2 - 2, slider->geometry().bottom());

	maxValLabel = new QLabel(QString::number(maxVal), this);
	maxValLabel->move(slider->geometry().right() - 20, slider->geometry().bottom());
	
	brightness = slider->value();

};

DkBrightness::~DkBrightness() {


};

#ifdef WITH_OPENCV
// compute a LUT for a brightness change
Mat DkBrightness::compute(Mat inLut, float val1, float val2) {

	return changeBrightnessAndContrast(inLut, val1, val2);
};
#endif

// change brightness value and redraw image
void DkBrightness::redrawImage() {

	brightness = slider->value();
	historyData currData;
	currData.isHsv = false;
	currData.arg1 = (float)brightness;
	currData.arg2 = (float)contrast;

	if (manipulationType != manipulationBrightness && manipulationType != manipulationContrast) {

		resetSliderValues(manipulationBrightness);
#ifdef WITH_OPENCV
		imgMat = DkImage::qImage2Mat(manipDialog->getImgPreview());
#endif

		historyDataVec.push_back(currData);
		historyToolsVec.push_back(this);
		prepareUndoRedoButtons();

		manipulationType = manipulationBrightness;
	}
	else {

		historyDataVec.back() = currData;
		historyToolsVec.back() = this;
	}

#ifdef WITH_OPENCV
	Mat manipulationLUT = compute(tempLUT, currData.arg1, currData.arg2);
	emit updateDialogImgSignal(DkImage::mat2QImage(applyLutToImage(imgMat, manipulationLUT, currData.isHsv)));
#endif

};

// contrast widget
DkContrast::DkContrast(QWidget *parent, DkImageManipulationDialog *parentDialog) 
	: DkImageManipulationWidget(parent, parentDialog){

	name = QString("DkContrast");
	defaultValue = 0;

	minVal = -100;
	middleVal = defaultValue;
	maxVal = 100;

	sliderTitle = new QLabel(tr("Contrast"), this);
	sliderTitle->move(leftSpacing, topSpacing);

	slider = new QSlider(this);
	slider->setMinimum(minVal);
	slider->setMaximum(maxVal);
	slider->setValue(middleVal);
	slider->setTickInterval(50);
	slider->setOrientation(Qt::Horizontal);
	slider->setTickPosition(QSlider::TicksBelow);
	slider->setGeometry(QRect(leftSpacing, sliderTitle->geometry().bottom() - 5, sliderLength, 20));
	slider->setObjectName("DkBrightnessSlider");

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
	middleValLabel->move(leftSpacing + sliderLength / 2 - 2, slider->geometry().bottom());

	maxValLabel = new QLabel(QString::number(maxVal), this);
	maxValLabel->move(slider->geometry().right() - 20, slider->geometry().bottom());

	contrast = slider->value();

};

DkContrast::~DkContrast() {


};

#ifdef WITH_OPENCV
// compute a LUT for a contrast change
Mat DkContrast::compute(Mat inLut, float val1, float val2) {

	return changeBrightnessAndContrast(inLut, val1, val2);
};
#endif

// change contrast value and redraw image
void DkContrast::redrawImage() {

	contrast = slider->value();
	historyData currData;
	currData.isHsv = false;
	currData.arg1 = (float)brightness;
	currData.arg2 = (float)contrast;

	if (manipulationType != manipulationBrightness && manipulationType != manipulationContrast) {

		resetSliderValues(manipulationContrast);
#ifdef WITH_OPENCV
		imgMat = DkImage::qImage2Mat(manipDialog->getImgPreview());
#endif

		historyDataVec.push_back(currData);
		historyToolsVec.push_back(this);
		prepareUndoRedoButtons();

		manipulationType = manipulationContrast;
	}
	else {

		historyDataVec.back() = currData;
		historyToolsVec.back() = this;
	}

#ifdef WITH_OPENCV
	Mat manipulationLUT = compute(tempLUT, currData.arg1, currData.arg2);
	emit updateDialogImgSignal(DkImage::mat2QImage(applyLutToImage(imgMat, manipulationLUT, currData.isHsv)));
#endif

};

// saturation widget
DkSaturation::DkSaturation(QWidget *parent, DkImageManipulationDialog *parentDialog) 
	: DkImageManipulationWidget(parent, parentDialog){

	name = QString("DkSaturation");
	defaultValue = 0;

	minVal = -255;
	middleVal = defaultValue;
	maxVal = 255;

	sliderTitle = new QLabel(tr("Saturation"), this);
	sliderTitle->move(leftSpacing, topSpacing);

	slider = new QSlider(this);
	slider->setMinimum(minVal);
	slider->setMaximum(maxVal);
	slider->setValue(middleVal);
	slider->setTickInterval(255/5);
	slider->setOrientation(Qt::Horizontal);
	slider->setTickPosition(QSlider::TicksBelow);
	slider->setGeometry(QRect(leftSpacing, sliderTitle->geometry().bottom() - 5, sliderLength, 20));
	slider->setObjectName("DkSaturationSlider");

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
	middleValLabel->move(leftSpacing + sliderLength / 2 - 2, slider->geometry().bottom());

	maxValLabel = new QLabel(QString::number(maxVal), this);
	maxValLabel->move(slider->geometry().right() - 20, slider->geometry().bottom());

	saturation = slider->value();

};

DkSaturation::~DkSaturation() {


};

#ifdef WITH_OPENCV
// compute a LUT for a saturation change
Mat DkSaturation::compute(Mat inLut, float val1, float val2) {

	return changeSaturationAndHue(inLut, val1, val2);
};
#endif

// change saturation value and redraw image
void DkSaturation::redrawImage() {

	saturation = slider->value();
	historyData currData;
	currData.isHsv = true;
	currData.arg1 = (float)saturation;
	currData.arg2 = (float)hue;

	if (manipulationType != manipulationSaturation && manipulationType != manipulationHue) {

		resetSliderValues(manipulationSaturation);
#ifdef WITH_OPENCV
		imgMat = DkImage::qImage2Mat(manipDialog->getImgPreview());
#endif

		historyDataVec.push_back(currData);
		historyToolsVec.push_back(this);
		prepareUndoRedoButtons();

		manipulationType = manipulationSaturation;
	}
	else {

		historyDataVec.back() = currData;
		historyToolsVec.back() = this;
	}

#ifdef WITH_OPENCV
	Mat manipulationLUT = compute(tempLUT, currData.arg1, currData.arg2);
	emit updateDialogImgSignal(DkImage::mat2QImage(applyLutToImage(imgMat, manipulationLUT, currData.isHsv)));
#endif

};

// change saturation slider style
void DkSaturation::setSliderStyle(QString sColor) {
	
	slider->setStyleSheet(
		QString("QSlider::groove:horizontal {border: 1px solid #999999; height: 4px; margin: 2px 0;")
		+ QString("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ffffff, stop:1 ")+ sColor + QString(");} ")
		+ QString("QSlider::handle:horizontal {background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #d2d2d2, stop:1 #e6e6e6); border: 1px solid #5c5c5c; width: 6px; margin:-4px 0px -6px 0px ;border-radius: 3px;}"));
}

// hue widget
DkHue::DkHue(QWidget *parent, DkImageManipulationDialog *parentDialog) 
	: DkImageManipulationWidget(parent, parentDialog){

	name = QString("DkHue");
	defaultValue = 0;

	minVal = -180;
	middleVal = defaultValue;
	maxVal = 180;

	sliderTitle = new QLabel(tr("Hue"), this);
	sliderTitle->move(leftSpacing, topSpacing);

	slider = new QSlider(this);
	slider->setMinimum(minVal);
	slider->setMaximum(maxVal);
	slider->setValue(middleVal);
	slider->setTickInterval(90);
	slider->setOrientation(Qt::Horizontal);
	slider->setTickPosition(QSlider::TicksBelow);
	slider->setGeometry(QRect(leftSpacing, sliderTitle->geometry().bottom() - 5, sliderLength, 20));
	slider->setObjectName("DkHueSlider");

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
	middleValLabel->move(leftSpacing + sliderLength / 2 - 2, slider->geometry().bottom());

	maxValLabel = new QLabel(QString::number(maxVal), this);
	maxValLabel->move(slider->geometry().right() - 10, slider->geometry().bottom());

	hue = slider->value(); // / 2;

};

DkHue::~DkHue() {

	
};

#ifdef WITH_OPENCV
// compute a LUT for a hue change
Mat DkHue::compute(Mat inLut, float val1, float val2) {

	return changeSaturationAndHue(inLut, val1, val2);
};
#endif

// change hue value and redraw image
void DkHue::redrawImage() {

	hue = slider->value();
	historyData currData;
	currData.isHsv = true;
	currData.arg1 = (float)saturation;
	currData.arg2 = (float)hue;

	if (manipulationType != manipulationSaturation && manipulationType != manipulationHue) {

		resetSliderValues(manipulationHue);
#ifdef WITH_OPENCV
		imgMat = DkImage::qImage2Mat(manipDialog->getImgPreview());
#endif

		historyDataVec.push_back(currData);
		historyToolsVec.push_back(this);
		prepareUndoRedoButtons();

		manipulationType = manipulationHue;
	}
	else {

		historyDataVec.back() = currData;
		historyToolsVec.back() = this;
	}

	setSaturationSliderColor(QColor(hueGradientImg.pixel(hue/2 + 90, 0)).name());
#ifdef WITH_OPENCV
	Mat manipulationLUT = compute(tempLUT, currData.arg1, currData.arg2);
	emit updateDialogImgSignal(DkImage::mat2QImage(applyLutToImage(imgMat, manipulationLUT, currData.isHsv)));
#endif

};

// gamma widget
DkGamma::DkGamma(QWidget *parent, DkImageManipulationDialog *parentDialog) 
	: DkImageManipulationWidget(parent, parentDialog){

	name = QString("DkGamma");
	defaultValueF = 1.0f;

	minValD = 0.01f;
	middleValD = defaultValueF;
	maxValD = 9.99f;
	minVal = 0;
	middleVal = 99;
	maxVal = 199;

	for(int i = 0; i < 100; i++) gammaSliderValues[i] = (i+1) / 100.0;
	for(int i = 0; i < 99; i++) gammaSliderValues[100 + i] = (int)(100 * pow(10, (i+1) / 100.0)) / 100.0;
	gammaSliderValues[199] = 9.99;

	sliderTitle = new QLabel(tr("Gamma"), this);
	sliderTitle->move(leftSpacing, topSpacing);

	slider = new QSlider(this);
	slider->setMinimum(minVal);
	slider->setMaximum(maxVal);
	slider->setValue(middleVal);
	slider->setTickInterval(100);
	slider->setOrientation(Qt::Horizontal);
	slider->setTickPosition(QSlider::TicksBelow);
	slider->setGeometry(QRect(leftSpacing, sliderTitle->geometry().bottom() - 5, sliderLength, 20));
	slider->setObjectName("DkBrightnessSlider");

	sliderSpinBoxDouble = new QDoubleSpinBox(this);
	sliderSpinBoxDouble->setGeometry(slider->geometry().right() - 45, sliderTitle->geometry().top(), 45, 20);
	sliderSpinBoxDouble->setMinimum(minValD);
	sliderSpinBoxDouble->setMaximum(maxValD);
	sliderSpinBoxDouble->setValue(middleValD);
	sliderSpinBoxDouble->setSingleStep(0.01);

	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateDoubleSliderSpinBox(int)));
	connect(sliderSpinBoxDouble, SIGNAL(valueChanged(double)), this, SLOT(updateDoubleSliderVal(double)));

	minValLabel = new QLabel(QString::number(minValD), this);
	minValLabel->move(leftSpacing, slider->geometry().bottom());

	middleValLabel = new QLabel(QString::number(middleValD), this);
	middleValLabel->move(leftSpacing + sliderLength / 2 - 2, slider->geometry().bottom());

	maxValLabel = new QLabel(QString::number(maxValD), this);
	maxValLabel->move(slider->geometry().right() - 21, slider->geometry().bottom());

	gamma = (float)sliderSpinBoxDouble->value();

};

DkGamma::~DkGamma() {


};

#ifdef WITH_OPENCV
// compute a LUT for a gamma change
Mat DkGamma::compute(Mat inLut, float val1, float) {

	return changeGamma(inLut, val1);
};
#endif

// change gamma value and redraw image
void DkGamma::redrawImage() {

	gamma = (float)sliderSpinBoxDouble->value();
	historyData currData;
	currData.isHsv = false;
	currData.arg1 = gamma;
	currData.arg2 = 0;

	if (manipulationType != manipulationGamma) {

		resetSliderValues(manipulationGamma);
#ifdef WITH_OPENCV
		imgMat = DkImage::qImage2Mat(manipDialog->getImgPreview());
#endif

		historyDataVec.push_back(currData);
		historyToolsVec.push_back(this);
		prepareUndoRedoButtons();

		manipulationType = manipulationGamma;
	}
	else {

		historyDataVec.back() = currData;
		historyToolsVec.back() = this;
	}

#ifdef WITH_OPENCV
	Mat manipulationLUT = compute(tempLUT, currData.arg1, currData.arg2);
	emit updateDialogImgSignal(DkImage::mat2QImage(applyLutToImage(imgMat, manipulationLUT, currData.isHsv)));
#endif

};

// exposure widget
DkExposure::DkExposure(QWidget *parent, DkImageManipulationDialog *parentDialog) 
	: DkImageManipulationWidget(parent, parentDialog){

	name = QString("DkExposure");
	defaultValueF = 0.0f;

	minValD = -6.0f;
	middleValD = defaultValueF;
	maxValD = 6.0f;
	minVal = -600;
	middleVal = 0;
	maxVal = 600;

	sliderTitle = new QLabel(tr("Exposure"), this);
	sliderTitle->move(leftSpacing, topSpacing);

	slider = new QSlider(this);
	slider->setMinimum(minVal);
	slider->setMaximum(maxVal);
	slider->setValue(middleVal);
	slider->setTickInterval(100);
	slider->setOrientation(Qt::Horizontal);
	slider->setTickPosition(QSlider::TicksBelow);
	slider->setGeometry(QRect(leftSpacing, sliderTitle->geometry().bottom() - 5, sliderLength, 20));
	slider->setObjectName("DkBrightnessSlider");

	sliderSpinBoxDouble = new QDoubleSpinBox(this);
	sliderSpinBoxDouble->setGeometry(slider->geometry().right() - 45, sliderTitle->geometry().top(), 45, 20);
	sliderSpinBoxDouble->setMinimum(minValD);
	sliderSpinBoxDouble->setMaximum(maxValD);
	sliderSpinBoxDouble->setValue(middleValD);
	sliderSpinBoxDouble->setSingleStep(0.01);

	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateDoubleSliderSpinBox(int)));
	connect(sliderSpinBoxDouble, SIGNAL(valueChanged(double)), this, SLOT(updateDoubleSliderVal(double)));

	minValLabel = new QLabel(QString::number((int)minValD), this);
	minValLabel->move(leftSpacing, slider->geometry().bottom());

	middleValLabel = new QLabel(QString::number((int)middleValD), this);
	middleValLabel->move(leftSpacing + sliderLength / 2 - 2, slider->geometry().bottom());

	maxValLabel = new QLabel(QString::number((int)maxValD), this);
	maxValLabel->move(slider->geometry().right() - 6, slider->geometry().bottom());

	exposure = (float)sliderSpinBoxDouble->value();

};

DkExposure::~DkExposure() {


};

#ifdef WITH_OPENCV
// compute a LUT for an exposure change
Mat DkExposure::compute(Mat inLut, float val1, float) {

	return changeExposure(inLut, val1);
};
#endif

// change scale of slider 
float DkExposure::convertSliderValToExposure(float val) {

	if (val >= 0) return 1 + val * 7.0f / 6.0f;
	else return 1 + 0.75f * val / 6.0f;
}

float DkExposure::convertExposureToSliderVal(float val) {

	if (val >= 1) return (val - 1) * 6.0f / 7.0f;
	else return 6 * (val - 1) / 0.75f;
}

// change exposure value and redraw image
void DkExposure::redrawImage() {

	exposure = (float)sliderSpinBoxDouble->value();
	exposure = convertSliderValToExposure(exposure);

	historyData currData;
	currData.isHsv = false;
	currData.arg1 = exposure;
	currData.arg2 = 0;

	if (manipulationType != manipulationExposure) {

		resetSliderValues(manipulationExposure);
#ifdef WITH_OPENCV
		imgMat = DkImage::qImage2Mat(manipDialog->getImgPreview());
#endif

		historyDataVec.push_back(currData);
		historyToolsVec.push_back(this);
		prepareUndoRedoButtons();

		manipulationType = manipulationExposure;
	}
	else {

		historyDataVec.back() = currData;
		historyToolsVec.back() = this;
	}

#ifdef WITH_OPENCV
	Mat manipulationLUT = compute(tempLUT, currData.arg1, currData.arg2);
	emit updateDialogImgSignal(DkImage::mat2QImage(applyLutToImage(imgMat, manipulationLUT, currData.isHsv)));
#endif

};

// undo/redo widget
DkUndoRedo::DkUndoRedo(QWidget *parent, DkImageManipulationDialog *parentDialog) 
	: DkImageManipulationWidget(parent, parentDialog){

	buttonUndo = new QPushButton(tr("&Undo"));
	connect(buttonUndo, SIGNAL(clicked()), this, SLOT(undoPressed()));
	buttonRedo = new QPushButton(tr("&Redo"));
	connect(buttonRedo, SIGNAL(clicked()), this, SLOT(redoPressed()));

	QHBoxLayout* hBoxLayout = new QHBoxLayout(this);
	hBoxLayout->addWidget(buttonUndo);
	hBoxLayout->addWidget(buttonRedo);

	buttonUndo->setDisabled(true);
	buttonRedo->setDisabled(true);

	prepareUndo = false;
};

DkUndoRedo::~DkUndoRedo() {


};

#ifdef WITH_OPENCV
// just a dummy function - needed because it is an virtual function
Mat DkUndoRedo::compute(Mat inLut, float, float) {
	return inLut;
};
#endif

// just a dummy function - needed because it is an virtual function
void DkUndoRedo::redrawImage() {

};

// go backward in manipulation history
void DkUndoRedo::undoPressed() {

	if (prepareUndo) {
		
		historyDataVecCopy = std::vector<historyData>(historyDataVec);
		historyToolsVecCopy= std::vector<DkImageManipulationWidget*>(historyToolsVec);
		//lastSliderValue = historyToolsVec.back()->getToolsValue();
		manipulationTypeHist = manipulationType;

		prepareUndo = false;
	}
	
	buttonRedo->setEnabled(true);

	historyDataVec.pop_back();
	historyToolsVec.pop_back();

#ifdef WITH_OPENCV
	if (historyToolsVec.size() > 0) {

		Mat imgToDisplay = origMat.clone();
		for (unsigned int i = 0; i < historyToolsVec.size(); i++) {

			Mat lut = historyToolsVec[i]->compute(tempLUT, historyDataVec[i].arg1, historyDataVec[i].arg2);
			imgToDisplay = applyLutToImage(imgToDisplay, lut, historyDataVec[i].isHsv);
		}

		imgMat = imgToDisplay.clone();
		emit updateDialogImgSignal(DkImage::mat2QImage(imgToDisplay));
	}
	else {
		buttonUndo->setEnabled(false);
		imgMat = origMat.clone();
		emit updateDialogImgSignal(DkImage::mat2QImage(imgMat));
	}
	
#endif

	resetSliderValues(manipulationEmpty);
	manipulationType = manipulationEmpty;
};

// go forward in manipulation history
void DkUndoRedo::redoPressed() {

	historyDataVec.push_back(historyDataVecCopy[historyDataVec.size()]);
	historyToolsVec.push_back(historyToolsVecCopy[historyToolsVec.size()]);

	if(historyDataVec.size() == historyDataVecCopy.size()) {

		buttonRedo->setEnabled(false);
		doARedraw = false;
		historyToolsVec.back()->setToolsValue(historyDataVec.back().arg1, historyDataVec.back().arg2);
		doARedraw = true;
		manipulationType = (char)manipulationTypeHist;
	}

	buttonUndo->setEnabled(true);

#ifdef WITH_OPENCV
	Mat imgToDisplay = origMat.clone();
	for (unsigned int i = 0; i < historyToolsVec.size(); i++) {

		Mat lut = historyToolsVec[i]->compute(tempLUT, historyDataVec[i].arg1, historyDataVec[i].arg2);
		imgToDisplay = applyLutToImage(imgToDisplay, lut, historyDataVec[i].isHsv);
	}

	if(historyDataVec.size() != historyDataVecCopy.size()) imgMat = imgToDisplay.clone();
	else prepareUndo = true;

	emit updateDialogImgSignal(DkImage::mat2QImage(imgToDisplay));
#endif

};




};
