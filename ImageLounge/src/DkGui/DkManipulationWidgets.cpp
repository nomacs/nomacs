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
#include "BorderLayout.h"
#include "DkImageStorage.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QWidget>
#include <QDockWidget>
#include <QSlider>
#include <QSpinBox>
#include <QLabel>
#include <QBoxLayout>
#include <QProgressDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QPainter>
#include <QApplication>
#include <qmath.h>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

// static variables
int DkImageManipulationWidget::brightness;
int DkImageManipulationWidget::contrast;
int DkImageManipulationWidget::saturation;
int DkImageManipulationWidget::hue;
float DkImageManipulationWidget::gamma;
float DkImageManipulationWidget::sExposure;
char DkImageManipulationWidget::manipulationType;
bool DkImageManipulationWidget::slidersReset;
std::vector<historyData> DkImageManipulationWidget::historyDataVec;
std::vector<DkImageManipulationWidget*> DkImageManipulationWidget::historyToolsVec;
bool DkImageManipulationWidget::prepareUndo;
QPushButton* DkUndoRedo::buttonUndo;
QPushButton* DkUndoRedo::buttonRedo;
bool DkImageManipulationWidget::doARedraw;
#ifdef WITH_OPENCV
		cv::Mat DkImageManipulationWidget::imgMat;
		cv::Mat DkImageManipulationWidget::origMat;
		cv::Mat DkImageManipulationWidget::tempLUT;
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

	int dialogWidth = 700;
	int dialogHeight = 600;
	mToolsWidth = 200;
	mPreviewMargin = 20;
	mPreviewWidth = dialogWidth - mToolsWidth - 2 * mPreviewMargin;
	mPreviewHeight = dialogHeight - mPreviewMargin - 70;

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
	
	mBrightnessWidget->setToolsValue((float)mBrightnessWidget->getDefaultValue());
	mContrastWidget->setToolsValue((float)mContrastWidget->getDefaultValue());
	mSaturationWidget->setToolsValue((float)mSaturationWidget->getDefaultValue());
	mHueWidget->setToolsValue((float)mHueWidget->getDefaultValue());
	mGammaWidget->setToolsValue((float)mGammaWidget->getDefaultValueF());
	mExposureWidget->setToolsValue((float)mExposureWidget->convertSliderValToExposure(mExposureWidget->getDefaultValueF()));

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
	mPreviewLabel = new QLabel(centralWidget);
	mPreviewLabel->setGeometry(QRect(QPoint(mPreviewMargin, mPreviewMargin), QSize(mPreviewWidth, mPreviewHeight)));

	// east widget - sliders
	QWidget* eastWidget = new QWidget(this);
	eastWidget->setMinimumWidth(mToolsWidth);
	eastWidget->setMaximumWidth(mToolsWidth);
	eastWidget->setContentsMargins(0,10,10,0);
	QVBoxLayout* toolsLayout = new QVBoxLayout(eastWidget);
	toolsLayout->setContentsMargins(0,0,0,0);

	mBrightnessWidget = new DkBrightness(eastWidget, this);
	mContrastWidget = new DkContrast(eastWidget, this);
	mSaturationWidget = new DkSaturation(eastWidget, this);
	mHueWidget = new DkHue(eastWidget, this);
	mGammaWidget = new DkGamma(eastWidget, this);
	mExposureWidget = new DkExposure(eastWidget, this);
	mUndoRedoWidget = new DkUndoRedo(eastWidget, this);

	connect(this, SIGNAL(isNotGrayscaleImg(bool)), mSaturationWidget, SLOT(setEnabled(bool)));
	connect(this, SIGNAL(isNotGrayscaleImg(bool)), mHueWidget, SLOT(setEnabled(bool)));
	connect(mHueWidget, SIGNAL(setSaturationSliderColor(QString)), mSaturationWidget, SLOT(setSliderStyle(QString)));

	toolsLayout->addWidget(mBrightnessWidget);
	toolsLayout->addWidget(mContrastWidget);
	toolsLayout->addWidget(mSaturationWidget);
	toolsLayout->addWidget(mHueWidget);
	toolsLayout->addWidget(mGammaWidget);
	toolsLayout->addWidget(mExposureWidget);
	toolsLayout->addWidget(mUndoRedoWidget);

	eastWidget->setLayout(toolsLayout);
	
	// mButtons
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
* mRescale mViewport image - it is used as a preview for the manipulation tools changes
 **/
void DkImageManipulationDialog::createImgPreview() {

	if (!mImg || mImg->isNull())
		return;
	
	QPoint lt;
	float rW = mPreviewWidth / (float) mImg->width();
	float rH = mPreviewHeight / (float) mImg->height();
	float rMin = (rW < rH) ? rW : rH;

	if(rMin < 1) {
		if(rW < rH) lt = QPoint(0, qRound(mImg->height() * (rH - rMin) / 2.0f));
		else {
			 lt = QPoint(qRound(mImg->width() * (rW - rMin) / 2.0f), 0);
		}
	}
	else lt = QPoint(qRound((mPreviewWidth - mImg->width()) / 2.0f), qRound((mPreviewHeight - mImg->height()) / 2.0f));

	QSize imgSizeScaled = QSize(mImg->size());
	if(rMin < 1) imgSizeScaled *= rMin;

	mPreviewImgRect = QRect(lt, imgSizeScaled);

	mPreviewImgRect.setTop(mPreviewImgRect.top()+1);
	mPreviewImgRect.setLeft(mPreviewImgRect.left()+1);
	mPreviewImgRect.setWidth(mPreviewImgRect.width()-1);			// we have a border... correct that...
	mPreviewImgRect.setHeight(mPreviewImgRect.height()-1);

	if(rMin < 1) mImgPreview = mImg->scaled(imgSizeScaled, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	else mImgPreview = *mImg;
	
	if (mImgPreview.format() == QImage::Format_Mono || mImgPreview.format() == QImage::Format_MonoLSB || 
		mImgPreview.format() == QImage::Format_Indexed8 || mImgPreview.isGrayscale()) emit isNotGrayscaleImg(false);

#ifdef WITH_OPENCV
	
	cv::Mat imgMat = DkImage::qImage2Mat(mImgPreview);
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

	mImgPreview = updatedImg;
	drawImgPreview();
}

/**
* draw preview image
 **/
void DkImageManipulationDialog::drawImgPreview() {

	QImage preview = QImage(mPreviewWidth,mPreviewHeight, QImage::Format_ARGB32);
	preview.fill(Qt::transparent);
	QPainter painter(&preview);
	painter.setPen(QColor(0,0,0));
	painter.drawRect(0, 0, mPreviewWidth - 1, mPreviewHeight - 1);
	painter.setBackgroundMode(Qt::TransparentMode);
	painter.drawImage(mPreviewImgRect, mImgPreview);

	mPreviewLabel->setPixmap(QPixmap::fromImage(preview));
}

/**
 * constructor for the abstract class DkImageManipulationWidget - all image manipulation widgets are created from it
 * @param parent widget
 * @param mViewport - needed for getting current images
 **/
DkImageManipulationWidget::DkImageManipulationWidget(QWidget *parent, DkImageManipulationDialog *parentDialog)
	: QWidget(parent) {

	this->mSliderLength = parent->minimumWidth() - 2 * mLeftSpacing;
	this->mManipDialog = parentDialog;
	slidersReset = false;
	this->doARedraw = true;

	connect(this, SIGNAL(updateDialogImgSignal(QImage)), parent->parentWidget(), SLOT(updateImg(QImage)));

	// create gradient for changing saturation slider background
	mHueGradientImg = QImage(181, 10, QImage::Format_ARGB32);
	QLinearGradient hueGradient = QLinearGradient(mHueGradientImg.rect().topLeft(), mHueGradientImg.rect().topRight());
	hueGradient.setColorAt(0,  QColor("#ff0000"));
	hueGradient.setColorAt(0.167,  QColor("#ffff00"));
	hueGradient.setColorAt(0.333,  QColor("#00ff00"));
	hueGradient.setColorAt(0.5,  QColor("#00ffff"));
	hueGradient.setColorAt(0.666,  QColor("#0000ff"));
	hueGradient.setColorAt(0.833,  QColor("#ff00ff"));
	hueGradient.setColorAt(1,  QColor("#ff0000"));

	QPainter painter(&mHueGradientImg);
	painter.fillRect(mHueGradientImg.rect(), hueGradient);
	painter.end();

};

DkImageManipulationWidget::~DkImageManipulationWidget() {


};

/**
 * slider spin box slot: update value and redraw image
 * @param changed value
 **/
void DkImageManipulationWidget::updateSliderSpinBox(int val) {

	if(!mValueUpdated) {
		mValueUpdated = true;
		this->mSliderSpinBox->setValue(val);
		if (!slidersReset && doARedraw) redrawImage();
	}
	else mValueUpdated = false;

};

/**
 * slider double spin box slot: update value and redraw image
 * @param changed value
 **/
void DkImageManipulationWidget::updateDoubleSliderSpinBox(int val) {

	if(!mValueUpdated) {
		mValueUpdated = true;
		if (this->mName.compare("DkGamma") != 0) this->mSliderSpinBoxDouble->setValue(val/100.0);
		else this->mSliderSpinBoxDouble->setValue(this->mGammaSliderValues[val]);
		if (!slidersReset && doARedraw) redrawImage();
	}
	else mValueUpdated = false;

};

/**
 * slider slot: update value and redraw image
 * @param changed value
 **/
void DkImageManipulationWidget::updateSliderVal(int val) {

	if(!mValueUpdated) {
		mValueUpdated = true;
		this->mSlider->setValue(val);
		if (!slidersReset && doARedraw) redrawImage();
	}
	else mValueUpdated = false;

};

/**
 * slider slot: update value from double spinbox and redraw image
 * @param changed value
 **/
void DkImageManipulationWidget::updateDoubleSliderVal(double val) {
	
	if(!mValueUpdated) {
		mValueUpdated = true;		
		if (this->mName.compare("DkGamma") != 0) this->mSlider->setValue(qRound(val * 100));
		else this->mSlider->setValue(findClosestValue(this->mGammaSliderValues, val, 0, 199));		
		if (!slidersReset && doARedraw) redrawImage();
	}
	else mValueUpdated = false;

};

/**
* reset all image manipulation tools slider except the one in the input parameter
* needed when changing from one slider to another
**/
void DkImageManipulationWidget::resetSliderValues(char exceptionSlider) {
	
	slidersReset = true;
	if (exceptionSlider != manipulationBrightness) mManipDialog->getBrightnessWidget()->setToolsValue((float)mManipDialog->getBrightnessWidget()->getDefaultValue());
	if (exceptionSlider != manipulationContrast) mManipDialog->getContrastWidget()->setToolsValue((float)mManipDialog->getContrastWidget()->getDefaultValue());
	if (exceptionSlider != manipulationSaturation) mManipDialog->getSaturationWidget()->setToolsValue((float)mManipDialog->getSaturationWidget()->getDefaultValue());
	if (exceptionSlider != manipulationHue) mManipDialog->getHueWidget()->setToolsValue((float)mManipDialog->getHueWidget()->getDefaultValue());
	if (exceptionSlider != manipulationGamma) mManipDialog->getGammaWidget()->setToolsValue((float)mManipDialog->getGammaWidget()->getDefaultValueF());
	if (exceptionSlider != manipulationExposure) mManipDialog->getExposureWidget()->setToolsValue((float)
		mManipDialog->getExposureWidget()->convertSliderValToExposure(mManipDialog->getExposureWidget()->getDefaultValueF()));
	slidersReset = false;
}

/**
* set a new tool value
* @param new value
**/
void DkImageManipulationWidget::setToolsValue(float val) {

	if (this->mName.compare("DkBrightness") == 0) { brightness = (int)val; mSlider->setValue((int)val);}
	else if (this->mName.compare("DkContrast") == 0) { contrast = (int)val; mSlider->setValue((int)val);}
	else if (this->mName.compare("DkSaturation") == 0) { saturation = (int)val; mSlider->setValue((int)val);}
	else if (this->mName.compare("DkHue") == 0) { hue = (int)val; mSlider->setValue((int)val);}
	else if (this->mName.compare("DkGamma") == 0) { gamma = val; mSliderSpinBoxDouble->setValue(val);}
	else if (this->mName.compare("DkExposure") == 0) { sExposure = val; mSliderSpinBoxDouble->setValue(mManipDialog->getExposureWidget()->convertExposureToSliderVal(val));}

};

/**
* overloaded method for setting a new tool value for a pairs of sliders
* @param new value 1
* @param new value 2
**/
void DkImageManipulationWidget::setToolsValue(float val1, float val2) {

	if (this->mName.compare("DkBrightness") == 0 || this->mName.compare("DkContrast") == 0) { 
		brightness = (int)val1; 
		mManipDialog->getBrightnessWidget()->mSlider->setValue((int)val1);
		contrast = (int)val2; 
		mManipDialog->getContrastWidget()->mSlider->setValue((int)val2);
	}
	if (this->mName.compare("DkSaturation") == 0 || this->mName.compare("DkHue") == 0) { 
		saturation = (int)val1; 
		mManipDialog->getSaturationWidget()->mSlider->setValue((int)val1);
		hue = (int)val2; 
		mManipDialog->getHueWidget()->mSlider->setValue((int)val2);
	}
	else if (this->mName.compare("DkGamma") == 0) { 
		gamma = val1; 
		mSliderSpinBoxDouble->setValue(val1);
	}
	else if (this->mName.compare("DkExposure") == 0) { 
		sExposure = val1; 
		mSliderSpinBoxDouble->setValue(mManipDialog->getExposureWidget()->convertExposureToSliderVal(val1));
	}

};

/**
* return tool value
* @return tool value
**/
float DkImageManipulationWidget::getToolsValue() {

	if (this->mName.compare("DkBrightness") == 0) return (float)mSlider->value();
	else if (this->mName.compare("DkContrast") == 0) return (float)mSlider->value();
	else if (this->mName.compare("DkSaturation") == 0) return (float)mSlider->value();
	else if (this->mName.compare("DkHue") == 0) return (float)mSlider->value();
	else if (this->mName.compare("DkGamma") == 0) return (float)mSliderSpinBoxDouble->value();
	else if (this->mName.compare("DkExposure") == 0) return (float)mSliderSpinBoxDouble->value();
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
cv::Mat DkImageManipulationWidget::createMatLut16() {

	cv::Mat lut = cv::Mat(3, 65536, CV_16UC1);
	unsigned short *ptrU;
	for(int i = 0; i < 3; i++) {
		ptrU = lut.ptr<unsigned short>(i);
		for (int j = 0; j < 65536; j++) ptrU[j] = (unsigned short)j;
	}

	return lut;
}

/**
 * apply a calculated CV_16U lookup table to an cv::Mat image
 * @param input image
 * @param input LUT
 * @param is there a need to change to the HSV space (for changes of saturation and hue)
 * @return modified image
 **/
cv::Mat DkImageManipulationWidget::applyLutToImage(cv::Mat inImg, cv::Mat inLUT, bool isMatHsv) {	

	cv::Mat tempImg;

	if(isMatHsv) {
		cvtColor(inImg, tempImg, CV_RGB2HSV);
	}
	else tempImg = inImg.clone();

	std::vector<cv::Mat> imgCh;
	split(tempImg, imgCh);

	unsigned short *ptrLutR = inLUT.ptr<unsigned short>(0);
	unsigned short *ptrLutG = inLUT.ptr<unsigned short>(1);
	unsigned short *ptrLutB = inLUT.ptr<unsigned short>(2);

	if (tempImg.depth() == CV_32F) {

		if (tempImg.channels() < 3) {

			for (int row = 0; row < tempImg.rows; row++)
			{
				float *ptrR = imgCh[0].ptr<float>(row);

				for (int col = 0; col < tempImg.cols; col++) 
					ptrR[col] = (float)ptrLutR[qRound(ptrR[col] * (inLUT.cols-1))] / 65535.0f;

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

						ptrR[col] = (float)ptrLutR[qRound(ptrR[col] / 360.0f * (inLUT.cols-1))] / 65535.0f * 360.0f;
						ptrG[col] = (float)ptrLutG[qRound(ptrG[col] * (inLUT.cols-1))] / 65535.0f;
						ptrB[col] = (float)ptrLutB[qRound(ptrB[col] * (inLUT.cols-1))] / 65535.0f;
					}
					else {

						ptrR[col] = (float)ptrLutR[qRound(ptrR[col] * (inLUT.cols-1))] / 65535.0f;
						ptrG[col] = (float)ptrLutG[qRound(ptrG[col] * (inLUT.cols-1))] / 65535.0f;
						ptrB[col] = (float)ptrLutB[qRound(ptrB[col] * (inLUT.cols-1))] / 65535.0f;
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

				for (int col = 0; col < tempImg.cols; col++) 
					ptrR[col] = (unsigned char) qRound(ptrLutR[qRound((ptrR[col] / 255.0f) * (inLUT.cols-1))] / 257.0f);

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

						ptrR[col] = (unsigned char) qRound(ptrLutR[qRound((ptrR[col] / 180.0f) * (inLUT.cols-1))] / 65535.0f * 180.0f);
						ptrG[col] = (unsigned char) qRound(ptrLutG[qRound((ptrG[col] / 255.0f) * (inLUT.cols-1))] / 257.0f);
						ptrB[col] = (unsigned char) qRound(ptrLutB[qRound((ptrB[col] / 255.0f) * (inLUT.cols-1))] / 257.0f);
					}
					else {

						ptrR[col] = (unsigned char) qRound(ptrLutR[qRound((ptrR[col] / 255.0f) * (inLUT.cols-1))] / 257.0f);
						ptrG[col] = (unsigned char) qRound(ptrLutG[qRound((ptrG[col] / 255.0f) * (inLUT.cols-1))] / 257.0f);
						ptrB[col] = (unsigned char) qRound(ptrLutB[qRound((ptrB[col] / 255.0f) * (inLUT.cols-1))] / 257.0f);
					}
				}
			}
			merge(imgCh, tempImg);
		}
	}
	
	if(isMatHsv) {
		cv::Mat retImg;
		cvtColor(tempImg, retImg, CV_HSV2RGB);

		if(inImg.type() == CV_8UC4) {	// the retImg is always CV_8UC3, so for pics in CV_8UC4 we need to add one channel
			std::vector<cv::Mat> inImgCh;
			split(inImg, inImgCh);
			std::vector<cv::Mat> retImgCh;
			split(retImg, retImgCh);
			retImgCh.push_back(inImgCh[3]);
			merge(retImgCh, retImg);
		}
		return retImg;

	}
	else return tempImg;
}

/**
 * called from DkNoMacs.cpp: applies manipulation history to the mViewport image
 * @param input image
 * @return modified image
 **/
cv::Mat DkImageManipulationWidget::manipulateImage(cv::Mat inImg){
	
	cv::Mat outImg;
	cv::Mat nullImg;

	if (historyToolsVec.size() > 0) {

		QProgressDialog* progress = new QProgressDialog("Applying changes to image...", "Cancel", 0, 100, qApp->activeWindow());
		int step = (int) (100 / historyToolsVec.size());
		progress->setWindowModality(Qt::WindowModal);
		progress->setValue(1);	// a strange behavior of the progress dialog: first setValue shows an empty dialog (setting to zero won't work)
		progress->setValue(2);	// second setValue shows the progress bar with 2% (setting to zero won't work)
		progress->setValue(0);	// finally set the progress to zero

		outImg = inImg.clone();
		cv::Mat lut16 = createMatLut16();
		unsigned int i;
		for (i = 0; i < historyToolsVec.size(); i++) {

			cv::Mat lut = lut16.clone();
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
cv::Mat DkImageManipulationWidget::changeBrightnessAndContrast(cv::Mat inLUT, float brightnessVal, float contrastVal) {

	cv::Mat outLUT = inLUT.clone();

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

			int v = qRound((a * ptrU[col] / 257.0f + b) * 257);
			if (v < 0) 
				v = 0;
			else if (v > 65535) 
				v = 65535;

			ptrU[col] = (unsigned short)v;
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
cv::Mat DkImageManipulationWidget::changeSaturationAndHue(cv::Mat inLUT, float saturationVal, float hueVal) {

	cv::Mat outLUT = inLUT.clone();

	hueVal *= -1.0f;	// fix the difference between label and real hue

	// make a gaussian kernel for positive saturation -> this weights bright and dark colors less -> results in uniform saturation change
	int ks = 65536;
	float sigma = ks/6.0f;
	cv::Mat gKernel = cv::Mat(1, ks, CV_32FC1);
	float* kernelPtr = gKernel.ptr<float>();

	for (int idx = 0, x = -qFloor(ks/2); idx < ks; idx++,x++) 
		kernelPtr[idx] = (float)(exp(-(x*x)/(2*sigma*sigma)));	// 1/(sqrt(2pi)*sigma) -> discrete normalization
	normalize(gKernel, gKernel, 1.0f, 0.0f, cv::NORM_MINMAX);

	//for (int idx = 0, x = -qFloor(ks/2); idx < ks; idx++,x++) 
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
			sPtrU[col] += (unsigned short)qRound(sat * kernelPtr[qRound(vPtrU[col]/65535.0f*(ks-1))]* (1.0f-sPtrU[col]/65535.0f));
		}
		else {
			// linear for negative saturation values
			if (sPtrU[col] > -sat)
				sPtrU[col] += (unsigned short)sat;
			else
				sPtrU[col] = 0;
		}

		int newHue = hPtrU[col] + qRound(hueVal * 65535.0f / 360.0f);
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
cv::Mat DkImageManipulationWidget::changeGamma(cv::Mat inLUT, float g) {

	cv::Mat outLUT = inLUT.clone();

	unsigned short* ptrU;
	for(int i = 0; i < 3; i++) {

		ptrU = outLUT.ptr<unsigned short>(i);

		for (int col = 0; col < outLUT.cols; col++) {

			ptrU[col] = (unsigned short)qRound(pow(ptrU[col] / (float)(outLUT.cols - 1), 1.0f/g) * (outLUT.cols - 1));
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
cv::Mat DkImageManipulationWidget::changeExposure(cv::Mat inLUT, float exposure) {

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

	cv::Mat outLUT = inLUT.clone();

	unsigned short* ptrU;
		
	if(exposure <= 1) 
		for(int i = 0; i < 3; i++) {

			ptrU = outLUT.ptr<unsigned short>(i);
			for (int col = 0; col < outLUT.cols; col++) {
				ptrU[col] = (unsigned short)qRound(ptrU[col] * exposure);
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
				if (ptrU[col] < x1) ptrU[col] = (unsigned short)qRound((float) ptrU[col] * exposure);
				else ptrU[col] = (Y < 0) ? 0 : ((Y > 65535) ? 65535 : (unsigned short)qRound(Y));
			}
		}
	}

	return outLUT;
}
#endif

// Brightness widget
DkBrightness::DkBrightness(QWidget *parent, DkImageManipulationDialog *parentDialog) 
	: DkImageManipulationWidget(parent, parentDialog){

	mName = QString("DkBrightness");
	mDefaultValue = 0;
	mMiddleVal = mDefaultValue;

	mSliderTitle = new QLabel(tr("Brightness"), this);
	mSliderTitle->move(mLeftSpacing, mTopSpacing);

	mSlider = new QSlider(this);
	mSlider->setMinimum(mMinVal);
	mSlider->setMaximum(mMaxVal);
	mSlider->setValue(mMiddleVal);
	mSlider->setTickInterval(50);
	mSlider->setOrientation(Qt::Horizontal);
	mSlider->setTickPosition(QSlider::TicksBelow);
	mSlider->setGeometry(QRect(mLeftSpacing, mSliderTitle->geometry().bottom() - 5, mSliderLength, 20));
	mSlider->setObjectName("DkBrightnessSlider");

	mSliderSpinBox = new QSpinBox(this);
	mSliderSpinBox->setGeometry(mSlider->geometry().right() - 45, mSliderTitle->geometry().top(), 45, 20);
	mSliderSpinBox->setMinimum(mMinVal);
	mSliderSpinBox->setMaximum(mMaxVal);
	mSliderSpinBox->setValue(mSlider->value());

	connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(updateSliderSpinBox(int)));
	connect(mSliderSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSliderVal(int)));

	mMinValLabel = new QLabel(QString::number(mMinVal), this);
	mMinValLabel->move(mLeftSpacing, mSlider->geometry().bottom());

	mMiddleValLabel = new QLabel(QString::number(mMiddleVal), this);
	mMiddleValLabel->move(mLeftSpacing + mSliderLength / 2 - 2, mSlider->geometry().bottom());

	mMaxValLabel = new QLabel(QString::number(mMaxVal), this);
	mMaxValLabel->move(mSlider->geometry().right() - 20, mSlider->geometry().bottom());
	
	brightness = mSlider->value();

};

DkBrightness::~DkBrightness() {


};

#ifdef WITH_OPENCV
// compute a LUT for a brightness change
cv::Mat DkBrightness::compute(cv::Mat inLut, float val1, float val2) {

	return changeBrightnessAndContrast(inLut, val1, val2);
};
#endif

// change brightness value and redraw image
void DkBrightness::redrawImage() {

	brightness = mSlider->value();
	historyData currData;
	currData.isHsv = false;
	currData.arg1 = (float)brightness;
	currData.arg2 = (float)contrast;

	if (manipulationType != manipulationBrightness && manipulationType != manipulationContrast) {

		resetSliderValues(manipulationBrightness);
#ifdef WITH_OPENCV
		imgMat = DkImage::qImage2Mat(mManipDialog->getImgPreview());
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
	cv::Mat manipulationLUT = compute(tempLUT, currData.arg1, currData.arg2);
	emit updateDialogImgSignal(DkImage::mat2QImage(applyLutToImage(imgMat, manipulationLUT, currData.isHsv)));
#endif

};

// contrast widget
DkContrast::DkContrast(QWidget *parent, DkImageManipulationDialog *parentDialog) 
	: DkImageManipulationWidget(parent, parentDialog){

	mName = QString("DkContrast");
	mDefaultValue = 0;

	mMinVal = -100;
	mMiddleVal = mDefaultValue;
	mMaxVal = 100;

	mSliderTitle = new QLabel(tr("Contrast"), this);
	mSliderTitle->move(mLeftSpacing, mTopSpacing);

	mSlider = new QSlider(this);
	mSlider->setMinimum(mMinVal);
	mSlider->setMaximum(mMaxVal);
	mSlider->setValue(mMiddleVal);
	mSlider->setTickInterval(50);
	mSlider->setOrientation(Qt::Horizontal);
	mSlider->setTickPosition(QSlider::TicksBelow);
	mSlider->setGeometry(QRect(mLeftSpacing, mSliderTitle->geometry().bottom() - 5, mSliderLength, 20));
	mSlider->setObjectName("DkBrightnessSlider");

	mSliderSpinBox = new QSpinBox(this);
	mSliderSpinBox->setGeometry(mSlider->geometry().right() - 45, mSliderTitle->geometry().top(), 45, 20);
	mSliderSpinBox->setMinimum(mMinVal);
	mSliderSpinBox->setMaximum(mMaxVal);
	mSliderSpinBox->setValue(mSlider->value());

	connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(updateSliderSpinBox(int)));
	connect(mSliderSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSliderVal(int)));

	mMinValLabel = new QLabel(QString::number(mMinVal), this);
	mMinValLabel->move(mLeftSpacing, mSlider->geometry().bottom());

	mMiddleValLabel = new QLabel(QString::number(mMiddleVal), this);
	mMiddleValLabel->move(mLeftSpacing + mSliderLength / 2 - 2, mSlider->geometry().bottom());

	mMaxValLabel = new QLabel(QString::number(mMaxVal), this);
	mMaxValLabel->move(mSlider->geometry().right() - 20, mSlider->geometry().bottom());

	contrast = mSlider->value();

};

DkContrast::~DkContrast() {


};

#ifdef WITH_OPENCV
// compute a LUT for a contrast change
cv::Mat DkContrast::compute(cv::Mat inLut, float val1, float val2) {

	return changeBrightnessAndContrast(inLut, val1, val2);
};
#endif

// change contrast value and redraw image
void DkContrast::redrawImage() {

	contrast = mSlider->value();
	historyData currData;
	currData.isHsv = false;
	currData.arg1 = (float)brightness;
	currData.arg2 = (float)contrast;

	if (manipulationType != manipulationBrightness && manipulationType != manipulationContrast) {

		resetSliderValues(manipulationContrast);
#ifdef WITH_OPENCV
		imgMat = DkImage::qImage2Mat(mManipDialog->getImgPreview());
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
	cv::Mat manipulationLUT = compute(tempLUT, currData.arg1, currData.arg2);
	emit updateDialogImgSignal(DkImage::mat2QImage(applyLutToImage(imgMat, manipulationLUT, currData.isHsv)));
#endif

};

// saturation widget
DkSaturation::DkSaturation(QWidget *parent, DkImageManipulationDialog *parentDialog) 
	: DkImageManipulationWidget(parent, parentDialog){

	mName = QString("DkSaturation");
	mDefaultValue = 0;

	mMinVal = -255;
	mMiddleVal = mDefaultValue;
	mMaxVal = 255;

	mSliderTitle = new QLabel(tr("Saturation"), this);
	mSliderTitle->move(mLeftSpacing, mTopSpacing);

	mSlider = new QSlider(this);
	mSlider->setMinimum(mMinVal);
	mSlider->setMaximum(mMaxVal);
	mSlider->setValue(mMiddleVal);
	mSlider->setTickInterval(255/5);
	mSlider->setOrientation(Qt::Horizontal);
	mSlider->setTickPosition(QSlider::TicksBelow);
	mSlider->setGeometry(QRect(mLeftSpacing, mSliderTitle->geometry().bottom() - 5, mSliderLength, 20));
	mSlider->setObjectName("DkSaturationSlider");

	mSliderSpinBox = new QSpinBox(this);
	mSliderSpinBox->setGeometry(mSlider->geometry().right() - 45, mSliderTitle->geometry().top(), 45, 20);
	mSliderSpinBox->setMinimum(mMinVal);
	mSliderSpinBox->setMaximum(mMaxVal);
	mSliderSpinBox->setValue(mSlider->value());

	connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(updateSliderSpinBox(int)));
	connect(mSliderSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSliderVal(int)));

	mMinValLabel = new QLabel(QString::number(mMinVal), this);
	mMinValLabel->move(mLeftSpacing, mSlider->geometry().bottom());

	mMiddleValLabel = new QLabel(QString::number(mMiddleVal), this);
	mMiddleValLabel->move(mLeftSpacing + mSliderLength / 2 - 2, mSlider->geometry().bottom());

	mMaxValLabel = new QLabel(QString::number(mMaxVal), this);
	mMaxValLabel->move(mSlider->geometry().right() - 20, mSlider->geometry().bottom());

	saturation = mSlider->value();

};

DkSaturation::~DkSaturation() {


};

#ifdef WITH_OPENCV
// compute a LUT for a saturation change
cv::Mat DkSaturation::compute(cv::Mat inLut, float val1, float val2) {

	return changeSaturationAndHue(inLut, val1, val2);
};
#endif

// change saturation value and redraw image
void DkSaturation::redrawImage() {

	saturation = mSlider->value();
	historyData currData;
	currData.isHsv = true;
	currData.arg1 = (float)saturation;
	currData.arg2 = (float)hue;

	if (manipulationType != manipulationSaturation && manipulationType != manipulationHue) {

		resetSliderValues(manipulationSaturation);
#ifdef WITH_OPENCV
		imgMat = DkImage::qImage2Mat(mManipDialog->getImgPreview());
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
	cv::Mat manipulationLUT = compute(tempLUT, currData.arg1, currData.arg2);
	emit updateDialogImgSignal(DkImage::mat2QImage(applyLutToImage(imgMat, manipulationLUT, currData.isHsv)));
#endif

};

// change saturation slider style
void DkSaturation::setSliderStyle(QString sColor) {
	
	mSlider->setStyleSheet(
		QString("QSlider::groove:horizontal {border: 1px solid #999999; height: 4px; margin: 2px 0;")
		+ QString("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ffffff, stop:1 ")+ sColor + QString(");} ")
		+ QString("QSlider::handle:horizontal {background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #d2d2d2, stop:1 #e6e6e6); border: 1px solid #5c5c5c; width: 6px; margin:-4px 0px -6px 0px ;border-radius: 3px;}"));
}

// hue widget
DkHue::DkHue(QWidget *parent, DkImageManipulationDialog *parentDialog) 
	: DkImageManipulationWidget(parent, parentDialog){

	mName = QString("DkHue");
	mDefaultValue = 0;

	mMinVal = -180;
	mMiddleVal = mDefaultValue;
	mMaxVal = 180;

	mSliderTitle = new QLabel(tr("Hue"), this);
	mSliderTitle->move(mLeftSpacing, mTopSpacing);

	mSlider = new QSlider(this);
	mSlider->setMinimum(mMinVal);
	mSlider->setMaximum(mMaxVal);
	mSlider->setValue(mMiddleVal);
	mSlider->setTickInterval(90);
	mSlider->setOrientation(Qt::Horizontal);
	mSlider->setTickPosition(QSlider::TicksBelow);
	mSlider->setGeometry(QRect(mLeftSpacing, mSliderTitle->geometry().bottom() - 5, mSliderLength, 20));
	mSlider->setObjectName("DkHueSlider");

	mSliderSpinBox = new QSpinBox(this);
	mSliderSpinBox->setGeometry(mSlider->geometry().right() - 45, mSliderTitle->geometry().top(), 45, 20);
	mSliderSpinBox->setMinimum(mMinVal);
	mSliderSpinBox->setMaximum(mMaxVal);
	mSliderSpinBox->setValue(mSlider->value());

	connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(updateSliderSpinBox(int)));
	connect(mSliderSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSliderVal(int)));

	mMinValLabel = new QLabel(QString::number(mMinVal), this);
	mMinValLabel->move(mLeftSpacing, mSlider->geometry().bottom());

	mMiddleValLabel = new QLabel(QString::number(mMiddleVal), this);
	mMiddleValLabel->move(mLeftSpacing + mSliderLength / 2 - 2, mSlider->geometry().bottom());

	mMaxValLabel = new QLabel(QString::number(mMaxVal), this);
	mMaxValLabel->move(mSlider->geometry().right() - 20, mSlider->geometry().bottom());

	// TODO: create a slider class & add a layout!!!

	hue = mSlider->value(); // / 2;

};

DkHue::~DkHue() {

	
};

#ifdef WITH_OPENCV
// compute a LUT for a hue change
cv::Mat DkHue::compute(cv::Mat inLut, float val1, float val2) {

	return changeSaturationAndHue(inLut, val1, val2);
};
#endif

// change hue value and redraw image
void DkHue::redrawImage() {

	hue = mSlider->value();
	historyData currData;
	currData.isHsv = true;
	currData.arg1 = (float)saturation;
	currData.arg2 = (float)hue;

	if (manipulationType != manipulationSaturation && manipulationType != manipulationHue) {

		resetSliderValues(manipulationHue);
#ifdef WITH_OPENCV
		imgMat = DkImage::qImage2Mat(mManipDialog->getImgPreview());
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

	setSaturationSliderColor(QColor(mHueGradientImg.pixel(hue/2 + 90, 0)).name());
#ifdef WITH_OPENCV
	cv::Mat manipulationLUT = compute(tempLUT, currData.arg1, currData.arg2);
	emit updateDialogImgSignal(DkImage::mat2QImage(applyLutToImage(imgMat, manipulationLUT, currData.isHsv)));
#endif

};

// gamma widget
DkGamma::DkGamma(QWidget *parent, DkImageManipulationDialog *parentDialog) 
	: DkImageManipulationWidget(parent, parentDialog){

	mName = QString("DkGamma");
	mDefaultValueF = 1.0f;

	mMinValD = 0.01f;
	mMiddleValD = mDefaultValueF;
	mMaxValD = 9.99f;
	mMinVal = 0;
	mMiddleVal = 99;
	mMaxVal = 199;

	for(int i = 0; i < 100; i++) mGammaSliderValues[i] = (i+1) / 100.0;
	for(int i = 0; i < 99; i++) mGammaSliderValues[100 + i] = (int)(100 * pow(10, (i+1) / 100.0)) / 100.0;
	mGammaSliderValues[199] = 9.99;

	mSliderTitle = new QLabel(tr("Gamma"), this);
	mSliderTitle->move(mLeftSpacing, mTopSpacing);

	mSlider = new QSlider(this);
	mSlider->setMinimum(mMinVal);
	mSlider->setMaximum(mMaxVal);
	mSlider->setValue(mMiddleVal);
	mSlider->setTickInterval(100);
	mSlider->setOrientation(Qt::Horizontal);
	mSlider->setTickPosition(QSlider::TicksBelow);
	mSlider->setGeometry(QRect(mLeftSpacing, mSliderTitle->geometry().bottom() - 5, mSliderLength, 20));
	mSlider->setObjectName("DkBrightnessSlider");

	mSliderSpinBoxDouble = new QDoubleSpinBox(this);
	mSliderSpinBoxDouble->setGeometry(mSlider->geometry().right() - 45, mSliderTitle->geometry().top(), 45, 20);
	mSliderSpinBoxDouble->setMinimum(mMinValD);
	mSliderSpinBoxDouble->setMaximum(mMaxValD);
	mSliderSpinBoxDouble->setValue(mMiddleValD);
	mSliderSpinBoxDouble->setSingleStep(0.01);

	connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(updateDoubleSliderSpinBox(int)));
	connect(mSliderSpinBoxDouble, SIGNAL(valueChanged(double)), this, SLOT(updateDoubleSliderVal(double)));

	mMinValLabel = new QLabel(QString::number(mMinValD), this);
	mMinValLabel->move(mLeftSpacing, mSlider->geometry().bottom());

	mMiddleValLabel = new QLabel(QString::number(mMiddleValD), this);
	mMiddleValLabel->move(mLeftSpacing + mSliderLength / 2 - 2, mSlider->geometry().bottom());

	mMaxValLabel = new QLabel(QString::number(mMaxValD), this);
	mMaxValLabel->move(mSlider->geometry().right() - 21, mSlider->geometry().bottom());

	gamma = (float)mSliderSpinBoxDouble->value();

};

DkGamma::~DkGamma() {


};

#ifdef WITH_OPENCV
// compute a LUT for a gamma change
cv::Mat DkGamma::compute(cv::Mat inLut, float val1, float) {

	return changeGamma(inLut, val1);
};
#endif

// change gamma value and redraw image
void DkGamma::redrawImage() {

	gamma = (float)mSliderSpinBoxDouble->value();
	historyData currData;
	currData.isHsv = false;
	currData.arg1 = gamma;
	currData.arg2 = 0;

	if (manipulationType != manipulationGamma) {

		resetSliderValues(manipulationGamma);
#ifdef WITH_OPENCV
		imgMat = DkImage::qImage2Mat(mManipDialog->getImgPreview());
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
	cv::Mat manipulationLUT = compute(tempLUT, currData.arg1, currData.arg2);
	emit updateDialogImgSignal(DkImage::mat2QImage(applyLutToImage(imgMat, manipulationLUT, currData.isHsv)));
#endif

};

// exposure widget
DkExposure::DkExposure(QWidget *parent, DkImageManipulationDialog *parentDialog) 
	: DkImageManipulationWidget(parent, parentDialog){

	mName = QString("DkExposure");
	mDefaultValueF = 0.0f;

	mMinValD = -6.0f;
	mMiddleValD = mDefaultValueF;
	mMaxValD = 6.0f;
	mMinVal = -600;
	mMiddleVal = 0;
	mMaxVal = 600;

	mSliderTitle = new QLabel(tr("Exposure"), this);
	mSliderTitle->move(mLeftSpacing, mTopSpacing);

	mSlider = new QSlider(this);
	mSlider->setMinimum(mMinVal);
	mSlider->setMaximum(mMaxVal);
	mSlider->setValue(mMiddleVal);
	mSlider->setTickInterval(100);
	mSlider->setOrientation(Qt::Horizontal);
	mSlider->setTickPosition(QSlider::TicksBelow);
	mSlider->setGeometry(QRect(mLeftSpacing, mSliderTitle->geometry().bottom() - 5, mSliderLength, 20));
	mSlider->setObjectName("DkBrightnessSlider");

	mSliderSpinBoxDouble = new QDoubleSpinBox(this);
	mSliderSpinBoxDouble->setGeometry(mSlider->geometry().right() - 45, mSliderTitle->geometry().top(), 45, 20);
	mSliderSpinBoxDouble->setMinimum(mMinValD);
	mSliderSpinBoxDouble->setMaximum(mMaxValD);
	mSliderSpinBoxDouble->setValue(mMiddleValD);
	mSliderSpinBoxDouble->setSingleStep(0.01);

	connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(updateDoubleSliderSpinBox(int)));
	connect(mSliderSpinBoxDouble, SIGNAL(valueChanged(double)), this, SLOT(updateDoubleSliderVal(double)));

	mMinValLabel = new QLabel(QString::number((int)mMinValD), this);
	mMinValLabel->move(mLeftSpacing, mSlider->geometry().bottom());

	mMiddleValLabel = new QLabel(QString::number((int)mMiddleValD), this);
	mMiddleValLabel->move(mLeftSpacing + mSliderLength / 2 - 2, mSlider->geometry().bottom());

	mMaxValLabel = new QLabel(QString::number((int)mMaxValD), this);
	mMaxValLabel->move(mSlider->geometry().right() - 6, mSlider->geometry().bottom());

	sExposure = (float)mSliderSpinBoxDouble->value();

};

DkExposure::~DkExposure() {


};

#ifdef WITH_OPENCV
// compute a LUT for an exposure change
cv::Mat DkExposure::compute(cv::Mat inLut, float val1, float) {

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

	sExposure = (float)mSliderSpinBoxDouble->value();
	sExposure = convertSliderValToExposure(sExposure);

	historyData currData;
	currData.isHsv = false;
	currData.arg1 = sExposure;
	currData.arg2 = 0;

	if (manipulationType != manipulationExposure) {

		resetSliderValues(manipulationExposure);
#ifdef WITH_OPENCV
		imgMat = DkImage::qImage2Mat(mManipDialog->getImgPreview());
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
	cv::Mat manipulationLUT = compute(tempLUT, currData.arg1, currData.arg2);
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

void DkUndoRedo::enableUndoButton(bool val) { 
	buttonUndo->setEnabled(val); 
}

void DkUndoRedo::enableRedoButton(bool val) { 
	buttonRedo->setEnabled(val); 
}

#ifdef WITH_OPENCV
// just a dummy function - needed because it is an virtual function
cv::Mat DkUndoRedo::compute(cv::Mat inLut, float, float) {
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

		cv::Mat imgToDisplay = origMat.clone();
		for (unsigned int i = 0; i < historyToolsVec.size(); i++) {

			cv::Mat lut = historyToolsVec[i]->compute(tempLUT, historyDataVec[i].arg1, historyDataVec[i].arg2);
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
	cv::Mat imgToDisplay = origMat.clone();
	for (unsigned int i = 0; i < historyToolsVec.size(); i++) {

		cv::Mat lut = historyToolsVec[i]->compute(tempLUT, historyDataVec[i].arg1, historyDataVec[i].arg2);
		imgToDisplay = applyLutToImage(imgToDisplay, lut, historyDataVec[i].isHsv);
	}

	if(historyDataVec.size() != historyDataVecCopy.size()) imgMat = imgToDisplay.clone();
	else prepareUndo = true;

	emit updateDialogImgSignal(DkImage::mat2QImage(imgToDisplay));
#endif

};




};
