/*******************************************************************************************************
 DkLineDetection.cpp
 Created on:	04.06.2012

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

#include "DkLineDetection.h"
#include <time.h>
#include <QPushButton>
#include <QMessageBox>

namespace nmp {

// class: DkLineDetection start

/**
* Creates a new instance of the tool for detecting lines within an image
* with default parameters
**/

bool SetWindow(int Width, int Height) 
    { 
    _COORD coord; 
    coord.X = Width; 
    coord.Y = Height; 

    _SMALL_RECT Rect; 
    Rect.Top = 0; 
    Rect.Left = 0; 
    Rect.Bottom = Height - 1; 
    Rect.Right = Width - 1; 

    // Get handle of the standard output 
    HANDLE Handle = GetStdHandle(STD_OUTPUT_HANDLE); 
    if (Handle == NULL) 
        { 
        std::cout<<"Failure in getting the handle\n"<<GetLastError(); 
        return FALSE; 
        } 
     
    // Set screen buffer size to that specified in coord 
    if(!SetConsoleScreenBufferSize(Handle, coord)) 
        { 
        std::cout<<"Failure in setting buffer size\n"<<GetLastError(); 
        return FALSE; 
        } 

    // Set the window size to that specified in Rect 
    if(!SetConsoleWindowInfo(Handle, TRUE, &Rect)) 
        { 
        std::cout<<"Failure in setting window size\n"<<GetLastError(); 
        return FALSE; 
        } 

    return TRUE; 
} 


DkLineDetection::DkLineDetection() {
	
	params.stripeLength = 300;
	params.halfStripeLength = (int)(params.stripeLength/2);
	params.sigma = 6;
	params.nonExtremaKernelSize = 70;
	params.maxThresh = 0.15f;
	params.alpha = 0.50f;
	params.optimizeImage = true;
	params.boxFilterSizeX = 70;
	params.boxFilterSizeY = 70;
	params.rescale = 1.0f;

	hasLines = false;
	recalc = true;

	debug = false;

	SetWindow(1024, 900);
}

DkLineDetection::~DkLineDetection() {

}


void DkLineDetection::debugOutputMat(cv::Mat *mat, std::string message) {
	
	//return;
	std::cout << "Matrix: "<< message << std::endl << " "  << *mat << std::endl << std::endl;
	/*std::cout << "Matrix: " << message << std::endl;
	uchar *ptr;
	for (int i=0; i < (*mat).rows; i++)
	{
		for (int j=0; j <(*mat).cols; j++)
		{
			ptr = (*mat).data + (*mat).step * i;
			std::cout << (int)(ptr[j]) << " ; ";
		}
		std::cout << std::endl;
	}*/
}

/**
* Set the parameters for text line detection and optimization.
* @param stripeWidth The width of the vertical stripes to calculate the local projection profile
* @param nonExtrKernelSize The kernel size for non-extrema suppression oft the LLP
* @param optimize Flag to declare if the optimization algorithm shall be run
* @param sobelX 1 means to enable Sobel-filtering in x-direction for edge detection during optimization
* @param sobelY 1 means to enable Sobel-filtering in y-direction for edge detection during optimization
* @param sobelKernelSize Size of the sobel kernel for optimization in case sobelX or sobelY is enabled 
* @param boxFilterSizeX Width of the box filter to blur the edge images
* @param boxFilterSizeY Height of the box filter to blur the edge images
* @param removeShort A flag to specifiy if very short text lines should be removed from the output
**/
void DkLineDetection::setParameters(int stripeWidth, int nonExtrKernelSize, bool optimize,
									bool sobelX, bool sobelY, int sobelKernelSize,
									int boxFilterSizeX, int boxFilterSizeY, int removeShort/*, float rescale*/) {
	
	if(stripeWidth == params.stripeLength && nonExtrKernelSize == params.nonExtremaKernelSize) {
		recalc = false;
	} else {
		recalc = true;
	}
	
	params.stripeLength = stripeWidth;
	params.halfStripeLength = (int)(params.stripeLength/2);
	params.nonExtremaKernelSize = nonExtrKernelSize;
	params.optimizeImage = optimize;
	// set the optimization parameters
	params.sobelFilterX = sobelX ? 1 : 0;
	params.sobelFilterY = sobelY ? 1 : 0;
	params.sobelFilterSize = sobelKernelSize;
	params.boxFilterSizeX = boxFilterSizeX;
	params.boxFilterSizeY = boxFilterSizeY;
	params.removeShort = removeShort;
	//params.rescale = rescale;
}

/**
* DEBUG
* @returns The type of the image as string
**/
std::string DkLineDetection::getImageType(int number)
{
    // find type
    int imgTypeInt = number%8;
    std::string imgTypeString;

    switch (imgTypeInt)
    {
        case 0:
            imgTypeString = "8U";
            break;
        case 1:
            imgTypeString = "8S";
            break;
        case 2:
            imgTypeString = "16U";
            break;
        case 3:
            imgTypeString = "16S";
            break;
        case 4:
            imgTypeString = "32S";
            break;
        case 5:
            imgTypeString = "32F";
            break;
        case 6:
            imgTypeString = "64F";
            break;
        default:
            break;
    }

    // find channel
    int channel = (number/8) + 1;

    std::stringstream type;
    type<<"CV_"<<imgTypeString<<"C"<<channel;

    return type.str();
}

/**
* Sets a new image for which the lines shall be calculated.
* Note: calculation must be started extra.
* @param image The new image
* \sa startLineDetection()
**/
void DkLineDetection::setImage(cv::Mat image) {
	
	this->image = image.clone();
	// check if image is greyscale
	if(this->image.channels() > 1)
		cv::cvtColor(this->image, this->image, CV_BGR2GRAY);


	this->image.convertTo(this->image, CV_64F);
	// noramlize image to range 0...1
	this->image = this->image / 255;
	//std::cout << "Image Type: " << this->getImageType(this->image.type()) << std::endl;

	lpp_image = cv::Mat::zeros(image.rows, image.cols, CV_64F);
	lowerTextLines = cv::Mat::zeros(image.rows, image.cols, CV_8UC1);
	upperTextLines = cv::Mat::zeros(image.rows, image.cols, CV_8UC1);

	hasLines = false;
	recalc = true;
	//std::cout << "LPP Image Type: " << this->getImageType(lpp_image.type()) << std::endl;
}

/**
* @returns The currently set image
**/
cv::Mat DkLineDetection::getImage() {
	return image;
}

/**
* Creates Qt images out of the Open CV matrix text line masks by creating
* images with alpha channel.
* \sa bottomLines topLines
**/
void DkLineDetection::createTextLineImages() {

	std::vector<cv::Mat> ImgChannels;
	cv::Mat imgUC4;
	
	// bottom text lines
	ImgChannels.push_back(lowerTextLines.clone()*0);
	ImgChannels.push_back(lowerTextLines.clone());
	ImgChannels.push_back(lowerTextLines.clone()*0);
	ImgChannels.push_back(lowerTextLines.clone()*params.alpha);

	cv::merge(ImgChannels, imgUC4);

	bottomLines = nmc::DkImage::mat2QImage(imgUC4);

	// top text lines
	ImgChannels.clear();
	ImgChannels.push_back(upperTextLines.clone()*0);
	ImgChannels.push_back(upperTextLines.clone()*0);
	ImgChannels.push_back(upperTextLines.clone());
	ImgChannels.push_back(upperTextLines.clone()*params.alpha);

	cv::merge(ImgChannels, imgUC4);

	topLines = nmc::DkImage::mat2QImage(imgUC4);
}

/**
* Starts the line detection algorithm and optimization and saves the
* results as class variables
* The steps are:
* 1. Calculate a local projection profile
* 2. Find the local minimas in each projection profile
* 3. If optimization enabled: Optimize both line images
* \sa calcLocalProjectionProfile() findLocalMinima() optimizeLineImg(cv::Mat *segLineImg, cv::Mat *lowertextLineImg, cv::Mat *uppertextLineImg)
**/
void DkLineDetection::startLineDetection() {

	// only do the hole recalculation if values have changed
	if(recalc) {

		// reset the images
		lpp_image = cv::Mat::zeros(image.rows, image.cols, CV_64F);
		lowerTextLines = cv::Mat::zeros(image.rows, image.cols, CV_8UC1);
		upperTextLines = cv::Mat::zeros(image.rows, image.cols, CV_8UC1);
		hasLines = false;

		// first, calculate the LPP for the image
		calcLocalProjectionProfile();
		if (debug) {
			debugOutputMat(&image, "image");
		}

		if (debug) {
			debugOutputMat(&lpp_image, "lpp_image");
		}

		// find the local minima for each projection profile
		findLocalMinima();

		/*cv::namedWindow( "Step 2: local minima and non-extrema", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
		cv::imshow( "Step 2: local minima and non-extrema", lowerTextLines);*/

		// make lines a little thicker (for better visualisation)
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
		cv::dilate(lowerTextLines, lowerTextLines, element);
		lowerTextLines = lowerTextLines > 0;

		cv::dilate(upperTextLines, upperTextLines, element);
		upperTextLines = upperTextLines > 0;

		// save the original calculated versions extra, to make optimization possible
		// without having to recalculate everything
		basicLowerTextLines = lowerTextLines.clone();
		basicUpperTextLines = upperTextLines.clone();
	}

	// optimize the line image (clear borders)
	if (params.optimizeImage) {
		//const clock_t begin_time = clock();
		
		lowerTextLines = basicLowerTextLines.clone();
		upperTextLines = basicUpperTextLines.clone();
		// optimize the text line images
		optimizeLineImg(&image, &lowerTextLines, &upperTextLines);

		//std::cout << "Duration for optimization: " << float( clock () - begin_time ) /  CLOCKS_PER_SEC << std::endl;

	}

	// create a Qt image with transparency
	createTextLineImages();

	/*cv::namedWindow( "Step 3: Optimization", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
	cv::imshow( "Step 3: Optimization", lowerTextLines);*/


	// check if lines have been found
	double maxValLower, maxValUpper;
	minMaxIdx(lowerTextLines, 0, &maxValLower);
	minMaxIdx(upperTextLines, 0, &maxValUpper);
	if(maxValLower > 0 || maxValUpper > 0) {
		hasLines = true;
	} else {
		hasLines = false;
	}
}

/**
* Calculates the local projection profile of the current image using stripeLength.
* It is calculated by summing up the pixel values along each row within each stripe.
* \sa parameters::stripeLength
**/
void DkLineDetection::calcLocalProjectionProfile() {

	// calc LPP for the first stripe
	for(int i = 0; i < params.stripeLength; i++) {

		add(lpp_image.col(params.halfStripeLength), image.col(i), lpp_image.col(params.halfStripeLength));
	}


	// now add most right column and subract most left - dynamic programming
	for(int i = params.halfStripeLength+1; i < image.cols-params.halfStripeLength; i++) {

		subtract(image.col(i+params.halfStripeLength), image.col(i-params.halfStripeLength-1), lpp_image.col(i));
		add(lpp_image.col(i-1), lpp_image.col(i), lpp_image.col(i));
	}
}

/**
* Finds local minimas and maximas of the local projection profile by filtering it with a
* 1D gaussian derivative filter. Each column is then treated as a "histogram". The local maxima and
* minima within this histogram refer to the points on the upper respectively lower text lines.
* A Non-Extrema Suppression is performed to limit the number of line points.
* \sa parameters::sigma
* \sa nonExtremaSuppression(cv::Mat *histogram, cv::Mat *maxima, cv::Mat *minima)
**/
void DkLineDetection::findLocalMinima() {

	/** filter with 1D gaussian derivative kernel **/
	//  create gaussian derivative kernel
	int s = (int)(3*params.sigma+0.5)*2+1;
	cv::Mat kernel = cv::Mat::zeros(1, s, CV_64F);
	// get pointer to matrix row
	double* row = kernel.ptr<double>(0);
	int col=0;
	for (int x = (int)(-s/2 - 0.5); x <= (int) (s/2); x++) {
		// evaluate gaussian derivative at x
		row[col] = (double) -x*exp(-(x*x)/(2*(params.sigma*params.sigma)))/(params.sigma*params.sigma);
		col++;
	}
	kernel = kernel.reshape(0,s);

	cv::Mat filtered;
	// filter with kernel
	cv::filter2D(lpp_image, filtered, CV_64FC1, kernel, cv::Point(-1,-1), 0.0, cv::BORDER_REPLICATE); 

	if (debug) {
			debugOutputMat(&filtered, "lpp_image filtered with gaussian derivative");

			// >DIR: these are no-go dependencies ( [21.10.2014 markus]
			// in order to display debug output - you can either draw them in the viewport or save these images
			// DkBasicLoader is pretty handy for saving... -> if you open another nomacs instance, images are automatically updated on writing
			//cv::namedWindow( "lpp_image filtered with gaussian derivative", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
			//cv::imshow( "lpp_image filtered with gaussian derivative", filtered);
	}

	cv::Mat histogram, extrHist, maxima, minima;
	cv::Mat diffkernel = (cv::Mat_<double>(2,1) << -1, 1);
	cv::Mat sxkernel = (cv::Mat_<double>(2,1) << 1, -1);
	/*
	if (debug)
			debugOutputMat(&diffkernel, "diffkernel");

	if (debug)
			debugOutputMat(&sxkernel, "sxkernel");

	// DEBUG DEBUG: try filtering without loop
	cv::filter2D(filtered, extrHist, CV_64FC1, diffkernel, cv::Point(0,0), 0.0, cv::BORDER_REPLICATE);

	if (debug) {
			debugOutputMat(&extrHist, "filtered filtered with diffkernel");

			// >DIR: these are no-go dependencies ( [21.10.2014 markus]
			// in order to display debug output - you can either draw them in the viewport or save these images
			// DkBasicLoader is pretty handy for saving... -> if you open another nomacs instance, images are automatically updated on writing

			//cv::namedWindow( "filtered filtered with diffkernel", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
			//cv::imshow( "filtered filtered with diffkernel", extrHist);
	}

	extrHist.setTo(-1, extrHist < 0);
	extrHist.setTo(1, extrHist > 0);

	if (debug) {
			debugOutputMat(&extrHist, "-1 to 1 matrix for extrHist < 0 > 0");

			// >DIR: these are no-go dependencies ( [21.10.2014 markus]
			// in order to display debug output - you can either draw them in the viewport or save these images
			// DkBasicLoader is pretty handy for saving... -> if you open another nomacs instance, images are automatically updated on writing

			//cv::namedWindow( "-1 to 1 matrix for extrHist < 0 > 0", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
			//cv::imshow( "-1 to 1 matrix for extrHist < 0 > 0", extrHist);
	}


	cv::filter2D(extrHist, extrHist, CV_64FC1, sxkernel, cv::Point(0,1), 0.0, cv::BORDER_CONSTANT);

	if (debug) {
			debugOutputMat(&extrHist, "filtered with sx kernel");

			// >DIR: these are no-go dependencies ( [21.10.2014 markus]
			// in order to display debug output - you can either draw them in the viewport or save these images
			// DkBasicLoader is pretty handy for saving... -> if you open another nomacs instance, images are automatically updated on writing

			//cv::namedWindow( "filtered with sx kernel", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
			//cv::imshow( "filtered with sx kernel", extrHist);
	}

	// now find local maxima and minima
	maxima = extrHist > 0;
	minima = extrHist < 0;*/


	//cv::Mat lower, upper;
	//const clock_t begin_time_n = clock();

	/*
	//if (true) {
		nonExtremaSuppression2D(&filtered, &maxima, &minima);
		minima.copyTo(lowerTextLines);
		maxima.copyTo(upperTextLines);
		// debug
		lower = lowerTextLines.clone();
		upper = upperTextLines.clone();
	std::cout << "Duration for 2D approach: " << float( clock () - begin_time_n ) /  CLOCKS_PER_SEC << std::endl;

	const clock_t begin_time_o = clock();
	//} else { // old 1d approach*/
	for(int i=0; i<filtered.cols; i++) {
		
		// compute extrema in "histogram"
		histogram = filtered.col(i);
		cv::filter2D(histogram, extrHist, CV_64FC1, diffkernel, cv::Point(0,0), 0.0, cv::BORDER_REPLICATE);

		extrHist.setTo(-1, extrHist < 0);
		extrHist.setTo(1, extrHist > 0);

		cv::filter2D(extrHist, extrHist, CV_64FC1, sxkernel, cv::Point(0,1), 0.0, cv::BORDER_CONSTANT);

		// now find local maxima and minima
		maxima = extrHist > 0;
		minima = extrHist < 0;

		// maxima and minima are the same as before... no need to recalc all of those steps

		nonExtremaSuppression(&histogram, &maxima, &minima);

		minima.copyTo(lowerTextLines.col(i));
		maxima.copyTo(upperTextLines.col(i));

	}
	//std::cout << "Duration for 1D approach: " <<  float( clock () - begin_time_o ) /  CLOCKS_PER_SEC << std::endl;

	//	}
	//bool equal = compareMat(lower, lowerTextLines, "diff lower");
	//bool equals = compareMat(upper, upperTextLines, "diff upper");
	// DEBUG
	//lower.copyTo(lowerTextLines);
	//upper.copyTo(upperTextLines);
	if (debug) {
			debugOutputMat(&lowerTextLines, "lowerTextLines");

			// >DIR: these are no-go dependencies ( [21.10.2014 markus]
			// in order to display debug output - you can either draw them in the viewport or save these images
			// DkBasicLoader is pretty handy for saving... -> if you open another nomacs instance, images are automatically updated on writing

			//cv::namedWindow( "lowerTextLines", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
			//cv::imshow( "lowerTextLines", lowerTextLines);
	}

	if (debug)
			debugOutputMat(&upperTextLines, "upperTextLines");
}

bool DkLineDetection::compareMat(cv::Mat in1, cv::Mat in2, std::string text) {
    cv::Mat diff;

	if (in1.type() != in2.type()) {
		std::cout << "Matrix types not the same!!!" << std::endl;
		return false;
	}

    cv::compare(in1, in2, diff, cv::CMP_NE);

	// >DIR: these are no-go dependencies ( [21.10.2014 markus]
	// in order to display debug output - you can either draw them in the viewport or save these images
	// DkBasicLoader is pretty handy for saving... -> if you open another nomacs instance, images are automatically updated on writing

	//cv::namedWindow(text, CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
	//cv::imshow(text, diff);

	int count = cv::countNonZero(diff);
	std::cout << count << " pixels differ... " << std::endl;
    return count == 0;
}

/**
* Performs a non Extrema Suppression and therefore limits the number of text line points. The bigger
* the kernel for the suppression, the lesser text lines might be detected.
* Also ensures that only one minimum is between to maximas.
* \sa parameters::nonExtremaKernelSize parameters::maxThresh
**/
void DkLineDetection::nonExtremaSuppression2D(cv::Mat *filtered, cv::Mat *maxima, cv::Mat *minima) {
	
	// handle maximum ( = upper text lines)
	cv::Mat masked = cv::Mat::zeros(filtered->rows, filtered->cols, filtered->type());
	// mask image to keep only max values
	filtered->copyTo(masked, *maxima);
	if (debug)
		debugOutputMat(&masked, "masked with maxima");
	
	// dilate with non extrema kernel (uses max value of neighborhood as dialtion value)
	cv::Mat dilatedMask;
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, params.nonExtremaKernelSize));
	cv::dilate(masked, dilatedMask, element, cv::Point(-1, -1));

	if (debug)
		debugOutputMat(&dilatedMask, "dilated maximum mask");

	// set values to 0 which are below the given max threshold
	dilatedMask.setTo(0, dilatedMask < params.maxThresh);
	// shrink again: keep only values which really are the dilated max value
	// set 0 values to -1 to prevent from having equality there
	dilatedMask.setTo(-1, dilatedMask == 0);
	
	cv::compare(masked, dilatedMask, *maxima, cv::CMP_EQ);
	if (debug)
		debugOutputMat(maxima, "real max");

	// handle minimum ( = lower text lines)
	masked.setTo(0);
	dilatedMask.setTo(0);
	// mask image to keep only minima values
	filtered->copyTo(masked, *minima);
	if (debug)
		debugOutputMat(&masked, "masked with minima");

	// convert matrix so that minimum values are transformed into positive maximum values
	masked = masked * (-1);
	// add minimum val to all matrix elements to convert to positive values (> 0)
	double minVal;
	cv::minMaxIdx(masked, &minVal, 0);
	masked = masked + abs(minVal) + 0.1;
	// again: keep only minimum values
	masked.setTo(0, *minima == 0);

	if (debug)
		debugOutputMat(&masked, "masked + minimum value");

	// no matrix is ready to be dilated (which dilates maximum value)
	cv::dilate(masked, dilatedMask, element, cv::Point(-1, -1));

	if( debug)
		debugOutputMat(&dilatedMask, "dilated minimum mask");

	// set values to 0 which are below the given max threshold
	//dilatedMask.setTo(0, dilatedMask < params.maxThresh);
	// shrink again: keep only values which really are the dilated max value
	// set 0 values to -1 to prevent from having equality there
	dilatedMask.setTo(-1, dilatedMask == 0);
	
	cv::compare(masked, dilatedMask, *minima, cv::CMP_EQ);
	if( debug)
		debugOutputMat(minima, "real min");

	// optimization, alternate minima and maxima rows
	/*cv::Mat combined = cv::Mat::zeros(minima->rows, minima->cols, CV_64FC1);
	combined.setTo(-1, *minima > 0);
	if (debug)
		debugOutputMat(&combined, "copied");
	
	combined.setTo(1, *maxima > 0);

	if (debug)
		debugOutputMat(&combined, "combined lines -1  +1");

	element = cv::Mat::ones(params.nonExtremaKernelSize, 1, CV_64FC1);
	if (debug)
		debugOutputMat(&element, "filter kernel");
	
	cv::filter2D(combined, combined, CV_64FC1, element, cv::Point(-1,-1), 0.0, BORDER_CONSTANT);

	if (debug)
		debugOutputMat(&combined, "filtered combined");*/

	// debug
	/*cv::Mat test;
	test = cv::abs(combined);
	test = combined > 2;

	cv::namedWindow( "TEST", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
	cv::imshow( "TEST", test);*/
	/*
	// optimization: only one minimum between two maxima
	cv::Mat max_indices, min_indices;
	max_indices = maxima > 0;
    min_indices = minima > 0; 
	//cv::findNonZero(*maxima, max_indices);
	//cv::findNonZero(*minima, min_indices);

	for(int i=0; i<max_indices.rows-1; i++) {

		// get maxima interval
		idx1 = (max_indices.at<Point>(i)).y;
		idx2 = (max_indices.at<Point>(i+1)).y;
	
		// find min-index values between two consecutive maximas
		std::vector<int> min_between;
		for(int j=0; j<min_indices.rows; j++) {

			int idx_val = (int)min_indices.at<int>(j,1);
			
			if(idx_val > idx1 && idx_val < idx2) {
				min_between.push_back(idx_val);
			}	
		}
		// more than one minimum between two consecutive maximas?
		if(min_between.size() <= 1) continue;
		
		// make sure only one minimum between two maxima
		int newMin = (int) (std::accumulate(min_between.begin(),min_between.end(), 0.0)/min_between.size()+0.5f);
		
		for(int j=idx1; j<=idx2; j++) {

			 minima->at<uchar>(j,0) = (int) 0;
		}
		minima->at<uchar>(newMin,0) = (int) 255;
		
	}*/

}

/**
* Performs a non Extrema Suppression and therefore limits the number of text line points. The bigger
* the kernel for the suppression, the lesser text lines might be detected.
* Also ensures that only one minimum is between to maximas.
* \sa parameters::nonExtremaKernelSize parameters::maxThresh
**/
void DkLineDetection::nonExtremaSuppression(cv::Mat *histogram, cv::Mat *maxima, cv::Mat *minima) {

	uchar *row_max, *row_min;
	int idx1, idx2;
	cv::Mat neighborhood;

	for(int i=0; i<histogram->rows; i++) {

		// get pointer to matrix rows
		row_max = maxima->data + maxima->step * i; 
		row_min = minima->data + minima->step * i;

		// check if columns element represent a max or min
		if((int)row_max[0] == 0 && (int)row_min[0] == 0) {
			// nothing to do here
			continue;
		}

		// calculate index for neighborhood
		idx1 = i - (int)params.nonExtremaKernelSize/2;
		idx2 = i + (int)params.nonExtremaKernelSize/2;
		// check bounds
		if(idx1 < 0) idx1 = 0;
		if(idx2 >= histogram->rows) idx2 = histogram->rows - 1;
		
		// extract neighborhood
		neighborhood = (*histogram)(cv::Range(idx1,idx2+1),cv::Range(0,1));
		// extract the current value
		double hist_val = histogram->at<double>(i,0);

		if((int)row_max[0] != 0) {
			
			// is a maximum, check for other maximums in neighborhood
			double maxVal;
			minMaxIdx(neighborhood, 0, &maxVal);

			if (maxVal > hist_val || hist_val < params.maxThresh) {
				row_max[0] = (int) 0;
			}

		} else if ((int)row_min[0] != 0) {

			// is a minimum, check for other minimums in neighborhood
			double minVal;
			minMaxIdx(neighborhood, &minVal, 0);

			if (minVal < hist_val) {
				row_min[0] = (int) 0;
			}
		}
		
	}
	/*
	cv::Mat max_indices, min_indices;
	max_indices = *maxima > 0;
    min_indices = *minima > 0; 
	
	for(int i=0; i<max_indices.rows-1; i++) {

		// get maxima interval
		idx1 = (max_indices.at<Point>(i, 0)).y;
		idx2 = (max_indices.at<Point>(i+1, 0)).y;
		
		// find min-index values between two consecutive maximas
		std::vector<int> min_between;
		for(int j=0; j<min_indices.rows; j++) {

			int idx_val = (int)min_indices.at<int>(j,1);
			
			if(idx_val > idx1 && idx_val < idx2) {
				min_between.push_back(idx_val);
			}	
		}
		// more than one minimum between two consecutive maximas?
		if(min_between.size() <= 1) continue;
		
		// make sure only one minimum between two maxima
		int newMin = (int) (std::accumulate(min_between.begin(),min_between.end(), 0.0)/min_between.size()+0.5f);
		
		for(int j=idx1; j<=idx2; j++) {

			 minima->at<uchar>(j,0) = (int) 0;
		}
		minima->at<uchar>(newMin,0) = (int) 255;
		
	}*/
}

/**
* Optimizes the basic lower and upper text line images.
* Uses Sobel filtering for edge detection (in x- and y-direction).
* Performs a mean filtering on the edge images to create blurred images.
* Performs Otsu thresholding to eliminate weak edges.
* Text lines are only considered where the algorithm recognized vertical and horizontal edges
* (which should correspond to text regions).
* This procedure should eliminate most of the text lines detected at image corners.
* @param segLineImg Pointer to the original image used for line detection
* @param lowertextLineImg Pointer to the basic calculated lower text line img mask (as CV_8UC1)
* @param uppertextLineImg Pointer to the basic calculated upper text line img (as CV_8UC1)
**/
void DkLineDetection::optimizeLineImg(cv::Mat *segLineImg,cv::Mat *lowertextLineImg, cv::Mat*uppertextLineImg) {
	
	// estimate text regions
	cv::Mat intImg = cv::Mat(segLineImg->rows+1, segLineImg->cols+1, CV_64FC1);

	cv::Mat filtered_gradX(segLineImg->rows, segLineImg->cols, segLineImg->type());
	cv::Mat filtered_gradY(segLineImg->rows, segLineImg->cols, segLineImg->type());
	if(params.sobelFilterX)
		cv::Sobel(*segLineImg, filtered_gradX, segLineImg->depth(), params.sobelFilterX, /*params.sobelFilterY*/0, params.sobelFilterSize);
	if(params.sobelFilterY)
		cv::Sobel(*segLineImg, filtered_gradY, segLineImg->depth(), /*params.sobelFilterX*/0, params.sobelFilterY, params.sobelFilterSize);

	/*cv::namedWindow( "Sobel - X", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
	cv::imshow( "Sobel - X", filtered_gradX);*/

	if(params.sobelFilterX) {
		filtered_gradX = cv::abs(filtered_gradX);
		normalize(filtered_gradX, filtered_gradX, 1.0f, 0.0f, cv::NORM_MINMAX);
	}
	if(params.sobelFilterY) {
		filtered_gradY = cv::abs(filtered_gradY);
		normalize(filtered_gradY, filtered_gradY, 1.0f, 0.0f, cv::NORM_MINMAX);
	}
	/*cv::namedWindow( "normalized sobel", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
	cv::imshow( "normalized sobel", filtered_gradX);*/

	// create integral image and do mean filtering
	if(params.sobelFilterX) {
		integral(filtered_gradX, intImg);
		filtered_gradX = DkLineDetection::convolveIntegralImage(intImg, cvCeil(params.boxFilterSizeX*params.rescale), cvCeil(params.boxFilterSizeY*params.rescale), DkLineDetection::DK_BORDER_ZERO);
		normalize(filtered_gradX, filtered_gradX, 255, 0, cv::NORM_MINMAX);
		
		if (filtered_gradX.depth() != CV_8UC1)
			filtered_gradX.convertTo(filtered_gradX, CV_8UC1);
	}
	if(params.sobelFilterY) {
		integral(filtered_gradY, intImg);
		filtered_gradY = DkLineDetection::convolveIntegralImage(intImg, cvCeil(params.boxFilterSizeX*params.rescale), cvCeil(params.boxFilterSizeY*params.rescale), DkLineDetection::DK_BORDER_ZERO);
		normalize(filtered_gradY, filtered_gradY, 255, 0, cv::NORM_MINMAX);

		if (filtered_gradY.depth() != CV_8UC1)
			filtered_gradY.convertTo(filtered_gradY, CV_8UC1);
	}

	
	// DEBUG
	/*
	cv::namedWindow( "convolved X", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
	cv::imshow( "convolved X", filtered_gradX);

	cv::namedWindow( "convolved Y", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
	cv::imshow( "convolved Y", filtered_gradY);
	*/
	// threshold the filtered sobel image using otsu
	if(params.sobelFilterX) {
		cv::threshold(filtered_gradX, filtered_gradX, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		filtered_gradX.convertTo(filtered_gradX, CV_32FC1);
		normalize(filtered_gradX, filtered_gradX, 1.0f, 0.0f, cv::NORM_MINMAX);
	}
	if(params.sobelFilterY) {
		cv::threshold(filtered_gradY, filtered_gradY, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		filtered_gradY.convertTo(filtered_gradY, CV_32FC1);
		normalize(filtered_gradY, filtered_gradY, 1.0f, 0.0f, cv::NORM_MINMAX);
	}

	/*
	cv::namedWindow( "otsu X", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
	cv::imshow( "otsu X", filtered_gradX);
	cv::namedWindow( "otsu Y", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
	cv::imshow( "otsu Y", filtered_gradY);
	*/
	// create binary images
	lowertextLineImg->convertTo(*lowertextLineImg, CV_32FC1, 1.0f/255.0f);
	uppertextLineImg->convertTo(*uppertextLineImg, CV_32FC1, 1.0f/255.0f);

	
	if (params.sobelFilterX || params.sobelFilterY) {
		cv::Mat mixed;
		if(params.sobelFilterX && params.sobelFilterY) {
			mixed = filtered_gradY.mul(filtered_gradX);
		} else if (params.sobelFilterX) {
			mixed = filtered_gradX;
		} else {
			mixed = filtered_gradY;
		}

		/*
		cv::namedWindow( "otsu mixed", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
		cv::imshow( "otsu mixed", mixed);
		*/
		cv::morphologyEx(mixed, mixed, cv::MORPH_DILATE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(20,20/*15, 15*/)));
		/*
		cv::namedWindow( "otsu mixed closed", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
		cv::imshow( "otsu mixed closed", mixed);
		*/
		*lowertextLineImg = lowertextLineImg->mul(mixed);
		*uppertextLineImg = uppertextLineImg->mul(mixed);
	}

	

	/*cv::namedWindow( "before removing short text lines lower", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
	cv::imshow( "before removing short text lines lower", *lowertextLineImg);
	*/

	if (params.removeShort) {
		*lowertextLineImg = removeShortLines(*lowertextLineImg, 250);
		*uppertextLineImg = removeShortLines(*uppertextLineImg, 250);
	}
	/*
	cv::namedWindow( "after removing short text lines lower", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL );
	cv::imshow( "after removing short text lines lower", *lowertextLineImg);
	*/
	if (lowertextLineImg->depth() != CV_8UC1)
		lowertextLineImg->convertTo(*lowertextLineImg, CV_8UC1, 255);

	if (uppertextLineImg->depth() != CV_8UC1)
		uppertextLineImg->convertTo(*uppertextLineImg, CV_8UC1, 255);

	
}

/**
* checsk img for text lines shorter than a certain length an removes them
* @param img The binary text line image to check
* @param minLength 
*/
cv::Mat DkLineDetection::removeShortLines(cv::Mat img, int minLength) {

    if (img.channels() > 1)
        img.convertTo(img, CV_RGB2GRAY);

    if (img.type() != CV_8UC1)
        img.convertTo(img, CV_8UC1, 255);

    img = img > 0;

    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    std::vector<std::vector<cv::Point>>::iterator contour = contours.begin();

    cv::Scalar color(255);

    //Scalar color( 0, 0, 255 );
    cv::Mat dst = cv::Mat::zeros(img.size(), CV_8UC1);
	int idx = 0;
    for (; contour != contours.end(); ++contour) {

        double extend = fabs(contourArea(cv::Mat(*contour)));

        if (extend >= minLength)
            drawContours( dst, contours, idx, color, CV_FILLED, 8, hierarchy );

		idx ++;
    }

    return dst;

} 

// integral text estimation


/**
 * Convolves an integral image by means of box filters.
 * This functions applies box filtering. It is specifically useful for the computation
 * of image sums, mean filtering and standard deviation with big kernel sizes.
 * @param src an integral image CV_64FC1
 * @param kernelSize the box filter's size
 * @param norm if DK_BORDER_ZERO an image sum is computed, if DK_BORDER_FLIP a mean filtering is applied.
 * @return the convolved image CV_32FC1
 **/
cv::Mat DkLineDetection::convolveIntegralImage(const cv::Mat src, const int kernelSizeX, const int kernelSizeY, const int norm = DK_BORDER_ZERO) {

	//if (src.channels() > 1) {
	//	std::string msg = "the image needs to have 1 channel, but it has: " + 
	//		DkUtils::stringify(src.channels());
	//	throw DkMatException(msg, __LINE__, __FILE__);
	//}

	//if (src.type() != CV_64FC1) {
	//	std::string msg = "the image needs to be CV_64FC1, it is: " + 
	//		DkUtils::getMatInfo(src);
	//	throw DkMatException(msg, __LINE__, __FILE__);
	//}

	//int ksY = (kernelSizeY != 0) ? kernelSizeY : kernelSizeX;	// make squared kernel

	cv::Mat dst = cv::Mat(src.rows-1, src.cols-1, CV_32FC1);
	
	int halfKRows = (kernelSizeY < dst.rows) ? cvFloor((float)kernelSizeY*0.5)+1 : cvFloor((float)(dst.rows-1)*0.5)-1;
	int halfKCols = (kernelSizeX < dst.cols) ? cvFloor((float)kernelSizeX*0.5)+1 : cvFloor((float)(dst.cols-1)*0.5)-1;

	// if the image dimension (rows, cols) <= 2
	if (halfKRows <= 0 || halfKCols <= 0) {
		dst.setTo(0);
		return dst;
	}

	// pointer for all corners
	const double* llc = src.ptr<double>();
	const double* lrc = src.ptr<double>();
	const double* ulc = src.ptr<double>();
	const double* urc = src.ptr<double>();
	const double* origin = src.ptr<double>();
	const double* lastRow = src.ptr<double>();

	float *dstPtr = dst.ptr<float>();
	
	// initial positions
	lrc += halfKCols;
	ulc += halfKRows*src.cols;
	urc += halfKRows*src.cols+halfKCols;
	lastRow += (src.rows-1)*src.cols;

	// area computation
	float rs = (float)halfKRows;
	float cs = (float)halfKCols;
	float area = rs*cs;
	
	for (int row = 0; row < dst.rows; row++) {

		for (int col = 0; col < dst.cols; col++) {

			// filter operation
			if (norm == DK_BORDER_ZERO)
				*dstPtr = (float)(*urc-*ulc-*lrc+*llc);
			else if (norm == DK_BORDER_FLIP) {
				*dstPtr = (float)((*urc-*ulc-*lrc+*llc)/area);
			}
			
			// do not change the left corners if we are near the left border
			if (col >= halfKCols-1) {
				llc++; ulc++;
			}
			// but recompute the filter area near the border
			else if (norm == DK_BORDER_FLIP) {
				cs++;
				area = rs*cs;
			}

			// do not change the right corners if we are near the right border
			if (col < dst.cols-halfKCols) {
				lrc++; urc++;
			}
			else if (norm == DK_BORDER_FLIP && col != dst.cols-1) {
				cs--;
				area = rs*cs;
			}

			dstPtr++;
		}

		// ok, flip to next row
		llc = ++lrc;
		ulc = ++urc;
		lrc += halfKCols;
		urc += halfKCols;

		if (row < halfKRows-1) {
			llc = origin;
			lrc = origin+halfKCols;
			if (norm == DK_BORDER_FLIP) {
				rs++;
				area = rs*cs;
			}
		}

		if (row >= dst.rows-halfKRows) {
			ulc = lastRow;
			urc = lastRow+halfKCols;
			if (norm == DK_BORDER_FLIP) {
				rs--;
				area = rs*cs;
			}
		}
	}
	
	//DkUtils::printDebug(DK_DEBUG_INFO, "convolving integral image in: %s\n", dt.getTotal().c_str());
	//normalize(dst, dst, 1, 0, NORM_MINMAX);

	return dst;
}




// class: DkLineDetection end

// class: DkLineDetectionDialog start

/**
* creates a new instance of the dialog responsible for setting up and executing the line detection
* calculations
**/
DkLineDetectionDialog::DkLineDetectionDialog(DkLineDetection *lineDetector, QSharedPointer<nmc::DkMetaDataT> metadata,
											 QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	this->lineDetector = lineDetector;
	this->margin = 10;
	this->metaData = metaData;
	init();
}

DkLineDetectionDialog::~DkLineDetectionDialog() {

}

/**
* Initiliazes the dialog and creates its layout
**/
void DkLineDetectionDialog::init() {

	dialogWidth = 300; //700;
	dialogHeight = 240;//160; //560;

	setWindowTitle(tr("Line Detection Settings"));
	//setFixedSize(dialogWidth, dialogHeight);
	this->setBaseSize(dialogWidth, dialogHeight);
	createLayout();
}

/**
* Layouting of the dialog with default parameters
**/
void DkLineDetectionDialog::createLayout() {
	
	// widgets for the different settings
	QWidget *centralWidget = new QWidget(this);
	QGridLayout* centralWidgetGridLayout = new QGridLayout(centralWidget);

	QLabel *labelStripeLength = new QLabel(tr("Word length in Pixel:"), centralWidget);
	labelStripeLength->move(margin, margin);
	spinnerStripeLength = new QSpinBox(centralWidget);
	spinnerStripeLength->setMinimumWidth(50);

	connect(spinnerStripeLength, SIGNAL(valueChanged(int)), this, SLOT(stripeLengthSliderValChanged(int)));

	QLabel *labelStripeLengthCM = new QLabel(tr("                       or cm:"), centralWidget);
	labelStripeLengthCM->move(margin, margin);
	spinnerStripeLengthCM = new QDoubleSpinBox(centralWidget);
	spinnerStripeLengthCM->setMinimumWidth(50);
	spinnerStripeLengthCM->setSingleStep(0.1);

	connect(spinnerStripeLengthCM, SIGNAL(valueChanged(double)), this, SLOT(stripeLengthSliderValChangedCM(double)));

	QLabel *labelNonExtKernelSize = new QLabel(tr("Line height in Pixel:"), centralWidget);	
	spinnerNonExtKernelSize = new QSpinBox(centralWidget);

	connect(spinnerNonExtKernelSize, SIGNAL(valueChanged(int)), this, SLOT(lineHeightSliderValChanged(int)));

	QLabel *labelNonExtKernelSizeCM = new QLabel(tr("                     or cm:"), centralWidget);	
	spinnerNonExtKernelSizeCM = new QDoubleSpinBox(centralWidget);
	spinnerNonExtKernelSizeCM->setSingleStep(0.1);

	connect(spinnerNonExtKernelSizeCM, SIGNAL(valueChanged(double)), this, SLOT(lineHeightSliderValChangedCM(double)));

	QLabel *labelOptimize = new QLabel(tr("Optimize line image:"), centralWidget);
	labelOptimize->setToolTip("Optimizes the line image by removing noise on the borders");
	checkOptimize = new QCheckBox(centralWidget);

	QLabel *labelSobelFilterX = new QLabel("    SobelFilterX:", centralWidget);
	checkSobelX = new QCheckBox(centralWidget);

	QLabel *labelSobelFilterY = new QLabel("    SobelFilterY:", centralWidget);
	checkSobelY = new QCheckBox(centralWidget);

	QLabel *labelSobelFilterSize = new QLabel("    SobelFilterSize:", centralWidget);
	comboBoxSobelSize = new QComboBox(centralWidget);
	comboBoxSobelSize->addItem("3", 3);
	comboBoxSobelSize->addItem("5", 5);
	comboBoxSobelSize->addItem("7", 7);

	QLabel *labelFilterSizeX = new QLabel("    BoxFilterSizeX:", centralWidget);
	spinnerFilterSizeX = new QSpinBox(centralWidget);

	QLabel *labelFilterSizeY = new QLabel("    BoxFilterSizeY:", centralWidget);
	spinnerFilterSizeY = new QSpinBox(centralWidget);

	QLabel *labelRemoveShort = new QLabel("    Remove Short Lines:", centralWidget);
	checkRemoveShort = new QCheckBox(centralWidget);
	

	/*QLabel *labelRescale = new QLabel("    rescale:", centralWidget);
	spinnerRescale = new QDoubleSpinBox(centralWidget);
	spinnerRescale->setDecimals(1);
	spinnerRescale->setSingleStep(0.1);*/

	centralWidgetGridLayout->addWidget(labelStripeLength, 1, 1);
	centralWidgetGridLayout->addWidget(spinnerStripeLength, 1, 2);
	centralWidgetGridLayout->addWidget(labelStripeLengthCM, 2, 1);
	centralWidgetGridLayout->addWidget(spinnerStripeLengthCM, 2, 2);
	centralWidgetGridLayout->addWidget(labelNonExtKernelSize, 3, 1);
	centralWidgetGridLayout->addWidget(spinnerNonExtKernelSize, 3, 2);
	centralWidgetGridLayout->addWidget(labelNonExtKernelSizeCM, 4, 1);
	centralWidgetGridLayout->addWidget(spinnerNonExtKernelSizeCM, 4, 2);
	centralWidgetGridLayout->addWidget(labelOptimize, 5, 1);
	centralWidgetGridLayout->addWidget(checkOptimize, 5, 2);
	centralWidgetGridLayout->addWidget(labelSobelFilterX, 6, 1);
	centralWidgetGridLayout->addWidget(checkSobelX, 6, 2);
	centralWidgetGridLayout->addWidget(labelSobelFilterY, 7, 1);
	centralWidgetGridLayout->addWidget(checkSobelY, 7, 2);
	centralWidgetGridLayout->addWidget(labelSobelFilterSize, 8, 1);
	centralWidgetGridLayout->addWidget(comboBoxSobelSize, 8, 2);
	centralWidgetGridLayout->addWidget(labelFilterSizeX, 9, 1);
	centralWidgetGridLayout->addWidget(spinnerFilterSizeX, 9, 2);
	centralWidgetGridLayout->addWidget(labelFilterSizeY, 10, 1);
	centralWidgetGridLayout->addWidget(spinnerFilterSizeY, 10, 2);
	centralWidgetGridLayout->addWidget(labelRemoveShort, 11, 1);
	centralWidgetGridLayout->addWidget(checkRemoveShort, 11, 2);
	/*centralWidgetGridLayout->addWidget(labelRescale, 6, 1);
	centralWidgetGridLayout->addWidget(spinnerRescale, 6, 2);*/
	
	// bottom widget - buttons	
	QWidget* bottomWidget = new QWidget(this);
	QHBoxLayout* bottomWidgetHBoxLayout = new QHBoxLayout(bottomWidget);

	QPushButton* buttonSave = new QPushButton(tr("&Detect Lines"));
	buttonSave->setDefault(true);
	connect(buttonSave, SIGNAL(clicked()), this, SLOT(detectLinesPressed()));
	QPushButton* buttonCancel = new QPushButton(tr("&Cancel"));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(cancelPressed()));

	QSpacerItem* spacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Expanding);
	
	bottomWidgetHBoxLayout->addItem(spacer);
	bottomWidgetHBoxLayout->addWidget(buttonSave);
	bottomWidgetHBoxLayout->addWidget(buttonCancel);	
	
	BorderLayout* borderLayout = new BorderLayout;
	borderLayout->addWidget(centralWidget, BorderLayout::Center);
	borderLayout->addWidget(bottomWidget, BorderLayout::South);
	this->setSizeGripEnabled(false);

	this->setLayout(borderLayout);

	setDefaultConfiguration();

	// connect optimization settings to check box
	connect(checkOptimize, SIGNAL(stateChanged(int)), this, SLOT(enableOptimizationSettings(int)));
}

void DkLineDetectionDialog::showEvent(QShowEvent *event) {

	oldOptimizeImage = checkOptimize->isChecked();
	oldSobelFilterX = checkSobelX->isChecked();
	oldSobelFilterY = checkSobelY->isChecked();

	oldStripeLength = spinnerStripeLength->value();
	oldNonExtremaKernelSize = spinnerNonExtKernelSize->value();
	
	oldSobelFilterSize = comboBoxSobelSize->currentText().toInt();
	oldBoxFilterSizeX = spinnerFilterSizeX->value();
	oldBoxFilterSizeY = spinnerFilterSizeY->value();

	oldRemoveShort = checkRemoveShort->isChecked();
}

/**
* Makes optimization settings clickable in the dialog
**/
void DkLineDetectionDialog::enableOptimizationSettings(int checked) {
	
	if(checked == 0) {
		checkSobelX->setEnabled(false);
		checkSobelY->setEnabled(false);
		comboBoxSobelSize->setEnabled(false);
		spinnerFilterSizeX->setEnabled(false);
		spinnerFilterSizeY->setEnabled(false);
		checkRemoveShort->setEnabled(false);
		//spinnerRescale->setEnabled(false);
	} else {
		checkSobelX->setEnabled(true);
		checkSobelY->setEnabled(true);
		comboBoxSobelSize->setEnabled(true);
		spinnerFilterSizeX->setEnabled(true);
		spinnerFilterSizeY->setEnabled(true);
		checkRemoveShort->setEnabled(true);
		//spinnerRescale->setEnabled(true);
	}
}

/**
* Sets the default values for the input fields according to the current image
*/
void DkLineDetectionDialog::setDefaultConfiguration() {
	cv::Mat img = lineDetector->getImage();
	int defaultStripeLength = (int)(img.cols/7); // 300
	int defaultNonExtrKernelSize = (int) (img.rows/50); //70
	int defaultFilterSizeX = 70 < img.cols ? 70 : img.cols;
	int defaultFilterSizeY = 50 < img.rows ? 50 : img.rows;
	//float defaultRescale = 1.0f;


	spinnerStripeLength->setMaximum(img.cols);
	spinnerStripeLength->setMinimum(2);
	spinnerStripeLength->setValue(defaultStripeLength);

	spinnerNonExtKernelSize->setMaximum(img.cols);
	spinnerNonExtKernelSize->setMinimum(2);
	spinnerNonExtKernelSize->setValue(defaultNonExtrKernelSize);

	// optimize parameters
	checkOptimize->setChecked(true);

	checkSobelX->setChecked(true);
	checkSobelY->setChecked(false);

	spinnerFilterSizeX->setMaximum(img.cols);
	spinnerFilterSizeX->setMinimum(3);
	spinnerFilterSizeX->setValue(defaultFilterSizeX);

	spinnerFilterSizeY->setMaximum(img.rows);
	spinnerFilterSizeY->setMinimum(3);
	spinnerFilterSizeY->setValue(defaultFilterSizeY);

	checkRemoveShort->setChecked(true);

	/*spinnerRescale->setMaximum(2.0);
	spinnerRescale->setMinimum(1.0);
	spinnerRescale->setValue(defaultRescale);*/
}

void DkLineDetectionDialog::setMetaData(QSharedPointer<nmc::DkMetaDataT> metadata) {

	this->metaData = metaData;
}

/**
* Closes the dialog.
**/
void DkLineDetectionDialog::cancelPressed() {

	// reset the values
	spinnerStripeLength->setValue(oldStripeLength);
	spinnerNonExtKernelSize->setValue(oldNonExtremaKernelSize);
	checkOptimize->setChecked(oldOptimizeImage);
	checkSobelX->setChecked(oldSobelFilterX ? true : false);
	checkSobelY->setChecked(oldSobelFilterY ? true : false);

	spinnerFilterSizeX->setValue(oldBoxFilterSizeX);
	spinnerFilterSizeY->setValue(oldBoxFilterSizeY);

	checkRemoveShort->setChecked(oldRemoveShort ? true : false);

	int index = comboBoxSobelSize->findData(oldSobelFilterSize);
	if ( index != -1 )
		comboBoxSobelSize->setCurrentIndex(index);

	this->close();
}

/**
* User signals to start the line detection algorithm.
* \sa DkLineDetection::startLineDetection()
**/
void DkLineDetectionDialog::detectLinesPressed() {

	//start calculation with current settings
	int sobelFilterSize = comboBoxSobelSize->currentText().toInt();

	lineDetector->setParameters(spinnerStripeLength->value(), 
		spinnerNonExtKernelSize->value(), 
		checkOptimize->isChecked(),
		checkSobelX->isChecked(),
		checkSobelY->isChecked(),
		sobelFilterSize,
		spinnerFilterSizeX->value(), 
		spinnerFilterSizeY->value(),
		checkRemoveShort->isChecked()
		/*,spinnerRescale->value()*/);

	lineDetector->startLineDetection();

	if(!lineDetector->hasTextLines()) {
		QMessageBox infoDialog(this);
		infoDialog.setWindowTitle(tr("No text lines"));
		infoDialog.setText(tr("No text lines were detected in the image"));
		infoDialog.setIcon(QMessageBox::Information);
		infoDialog.setStandardButtons(QMessageBox::Ok);
		infoDialog.show();
		infoDialog.exec();
	}
	// finished - now close dialog
	this->close();
}

/**
* Receives the changed value of the spinner which is the strip length in pixel,
* converts them to cm and sets the other slider to this value
**/
void DkLineDetectionDialog::stripeLengthSliderValChanged(int val) {
	if (changingStripeSlider) {
		changingStripeSlider = false;
		return;
	}
	changingStripeSlider = true;
	// get the image resolution for distance calculation
	float x_res = 72;		// markus: 72 dpi is the default value assumed

	// >DIR: get metadata resolution if available [21.10.2014 markus]
	if (metaData) {
		QVector2D res = metaData->getResolution();
		x_res = res.x();
	}
	// convert into cm and put into corresponding spinner
	float length_inch;

	length_inch = val / x_res;

	float length_cm = length_inch * 2.54;
	spinnerStripeLengthCM->setValue(length_cm);
}

/**
* Receives the changed value of the spinner which is the strip length in cm,
* converts them to pixel and sets the other slider to this value
**/
void DkLineDetectionDialog::stripeLengthSliderValChangedCM(double val) {
	if (changingStripeSlider) {
		changingStripeSlider = false;
		return;
	}
	changingStripeSlider = true;
	// get the image resolution for distance calculation
	float x_res = 72;		// markus: 72 dpi is the default value assumed

	// >DIR: get metadata resolution if available [21.10.2014 markus]
	if (metaData) {
		QVector2D res = metaData->getResolution();
		x_res = res.x();
	}
	// convert into cm and put into corresponding spinner
	float length_pixel;

	length_pixel = (val/2.54) * x_res;

	spinnerStripeLength->setValue(length_pixel);
}

/**
* Receives the changed value of the spinner which is the strip length in pixel,
* converts them to cm and sets the other slider to this value
**/
void DkLineDetectionDialog::lineHeightSliderValChanged(int val) {
	if (changingLineHeightSlider) {
		changingLineHeightSlider = false;
		return;
	}
	changingLineHeightSlider = true;
	// get the image resolution for distance calculation
	float y_res = 72;		// markus: 72 dpi is the default value assumed

	// >DIR: get metadata resolution if available [21.10.2014 markus]
	if (metaData) {
		QVector2D res = metaData->getResolution();
		y_res = res.y();
	}
	// convert into cm and put into corresponding spinner
	float length_inch;

	length_inch = val / y_res;

	float length_cm = length_inch * 2.54;
	spinnerNonExtKernelSizeCM->setValue(length_cm);
}

/**
* Receives the changed value of the spinner which is the strip length in cm,
* converts them to pixel and sets the other slider to this value
**/
void DkLineDetectionDialog::lineHeightSliderValChangedCM(double val) {
	if (changingLineHeightSlider) {
		changingLineHeightSlider = false;
		return;
	}
	changingLineHeightSlider = true;
	// get the image resolution for distance calculation
	float y_res = 72;		// markus: 72 dpi is the default value assumed

	// >DIR: get metadata resolution if available [21.10.2014 markus]
	if (metaData) {
		QVector2D res = metaData->getResolution();
		y_res = res.y();
	}
	// convert into cm and put into corresponding spinner
	float length_pixel;

	length_pixel = (val/2.54) * y_res;

	spinnerNonExtKernelSize->setValue(length_pixel);
}




// class: DkLineDetectionDialog end

};