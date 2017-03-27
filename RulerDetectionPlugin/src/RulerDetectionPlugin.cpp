/*******************************************************************************************************
 RulerDetectionPlugin.cpp

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2017 Pascal Plank

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

#include "RulerDetectionPlugin.h"


#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QUuid>
#include <QVector2D>
#include <QSharedPointer>
#include <qfiledialog.h>
#include <qmessagebox.h>

#include "opencv2/xfeatures2d/nonfree.hpp"
#include "opencv2\opencv.hpp"

#include "persistence1d.h"

#include "DkBasicLoader.h"
#include "DkImageContainer.h"
 // TODO: this include is nessesary for saving the dpi as metadata via the called loader, but it causes an include error:
 // fatal error C1083 : Cannot open include file : 'exiv2/xmpsidecar.hpp' : No such file or directory
//#include "DkMetaData.h"

#pragma warning(pop)		// no warnings from includes - end


namespace nmc {

/**
*	Constructor
**/
RulerDetectionPlugin::RulerDetectionPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);
	runIds[id_detectionSettings] = QUuid::createUuid().toString();
	runIds[id_detectRuler] = QUuid::createUuid().toString();
	//runIds[id_extractionInfo] = QUuid::createUuid().toString();
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);
	menuNames[id_detectionSettings] = tr("Settings");
	menuNames[id_detectRuler] = tr("Detect ruler (calculate dpi)");
	//menuNames[id_extractionInfo] = tr("Detection Info");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);
	statusTips[id_detectRuler] = tr("Settings for the Ruler Detection plugin");
	statusTips[id_detectRuler] = tr("Calculates the dpi via Ruler Detection");
	//statusTips[id_extractionInfo] = tr("Gives information about the detection process for better settings");
	mMenuStatusTips = statusTips.toList();

	// Load
	loadSettings(nmc::DkSettingsManager::instance().qSettings());
	saveSettings(nmc::DkSettingsManager::instance().qSettings());
}

/**
*	Destructor
**/
RulerDetectionPlugin::~RulerDetectionPlugin() {
}

/**
* Returns descriptive image for every ID
* @param plugin ID
**/
QImage RulerDetectionPlugin::image() const {

	return QImage(":/RulerDetectionPlugin/img/page-extraction123.png");
};

QString RulerDetectionPlugin::name() const {
	return "Ruler Detection Plugin";
}

QList<QAction*> RulerDetectionPlugin::createActions(QWidget* parent) {
	if (mActions.empty()) {
		for (int idx = 0; idx < id_end; idx++) {
			QAction* ca = new QAction(mMenuNames[idx], parent);
			ca->setObjectName(mMenuNames[idx]);
			ca->setStatusTip(mMenuStatusTips[idx]);
			ca->setData(mRunIDs[idx]);	// runID needed for calling function runPlugin()
			mActions.append(ca);
		}
	}
	return mActions;
}

QList<QAction*> RulerDetectionPlugin::pluginActions() const {
	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> RulerDetectionPlugin::runPlugin(
	const QString &runID,
	QSharedPointer<nmc::DkImageContainer> imgC,
	const nmc::DkSaveInfo& saveInfo,
	QSharedPointer<nmc::DkBatchInfo>& batchInfo) const {


	if (!imgC || !mRunIDs.contains(runID)) {
		QMainWindow* mainWindow = getMainWindow();
		QMessageBox::information(mainWindow, "Ruler Detection", "No image was selected.");
		return imgC;
	}

	if(runID == mRunIDs[id_detectionSettings]) {
		handleSettingsDialog();
	} else if (runID == mRunIDs[id_detectRuler]) {
		qDebug() << "VORM RULER DETECTION" << curr_template_path;
		imgC = handleRulerDetection(imgC, curr_template_path);
	//} else if (runID == mRunIDs[id_extractionInfo]) {
	//	handleDetectionInfo();
	} else {
		qWarning() << "Illegal run ID...";
	}
	return imgC;
}


//##############################################################################
//##############################################################################
//############################ SAVE AND LOAD ###################################
//##############################################################################
//##############################################################################
/**
* This method loads the settings required (for this plugin for this user)
* @param settings the settings to load
**/
void RulerDetectionPlugin::loadSettings(QSettings & settings) {

	settings.beginGroup(name());
	curr_template_path = settings.value("t_path", curr_template_path).toString();
	curr_tickDistance = settings.value("tickDist", curr_tickDistance).toString();
	curr_referencePoints = settings.value("refPoints", curr_referencePoints).toInt();
	curr_hessianThreshold = settings.value("hessianThres", curr_hessianThreshold).toInt();
	settings.endGroup();
}

/**
* This method saves the settings required for this plugin (to be accessable for future 
* logins of the current user)
* @param settings the settings to be saved
**/
void RulerDetectionPlugin::saveSettings(QSettings & settings) const {

	settings.beginGroup(name());
	settings.setValue("t_path", curr_template_path);
	settings.setValue("tickDist", curr_tickDistance);
	settings.setValue("refPoints", curr_referencePoints);
	settings.setValue("hessianThres", curr_hessianThreshold);
	settings.endGroup();
}





//##############################################################################
//##############################################################################
//############################# ACTION METHODS #################################
//##############################################################################
//##############################################################################
/**
* This method handles the Ruler Detection Settings Dialog and saves the values
**/
void RulerDetectionPlugin::handleSettingsDialog() const{
	qInfo() << "[RULER DETECTION] Settings...";
	QMainWindow* mainWindow = getMainWindow();
	RulerSettingsDialog* rulerSettingsDialog;
	if (mainWindow)
		rulerSettingsDialog = new RulerSettingsDialog(curr_template_path, curr_tickDistance, curr_referencePoints, curr_hessianThreshold, mainWindow);
	else
		rulerSettingsDialog = new RulerSettingsDialog(curr_template_path, curr_tickDistance, curr_referencePoints, curr_hessianThreshold);

	// Run settings dialog and wait until it is finished to save new settings
	rulerSettingsDialog->exec();

	// Get new settings from dialog and save them
	curr_template_path = rulerSettingsDialog->getTemplatePath();
	curr_tickDistance = rulerSettingsDialog->getTickDistance();
	curr_referencePoints = rulerSettingsDialog->getReferencePoints();
	curr_hessianThreshold = rulerSettingsDialog->getHessianThreshold();

	saveSettings(nmc::DkSettingsManager::instance().qSettings());
}

QSharedPointer<nmc::DkImageContainer> RulerDetectionPlugin::handleRulerDetection(QSharedPointer<nmc::DkImageContainer> imgC, QString templatepath) const {
	qInfo() << "[RULER DETECTION] Detect ruler and calculate dpi...";
	
	// Get image
	cv::Mat img = nmc::DkImage::qImage2Mat(imgC->image());

	// TODO: load with nomacs --> creates HeapCorruption if uses as pointer on heap, or Stack corruption as normal var
	// nmc::DkImageContainer* img_template_orig = new nmc::DkImageContainer(templatepath);
	// cv::Mat img_template = nmc::DkImage::qImage2Mat(img_template_orig->image());

	cv::Mat img_template = cv::imread(templatepath.toStdString());
	if (!img_template.data) {
		qWarning() << "[RULER DETECTION] Invalid templatefile -> change settings\n" << templatepath;
		QMainWindow* mainWindow = getMainWindow();
		QMessageBox::critical(mainWindow, "Ruler Detection", "Invalid templatefile. Change your settings.");
		return imgC;
	}

	int dpi = calculateDPI(img, img_template);

	qDebug() << "DPI: " << dpi;

	// TODO: uncomment after checking problem with includes: cannot include header to metadata, because it misses some header file itself
	// imgC->getLoader()->getMetaData()->setResolution(QVector2D(dpi, dpi));
	imgC->saveMetaData();

	

	return imgC;
}


void RulerDetectionPlugin::handleDetectionInfo() const {
	qInfo() << "[RULER DETECTION] Show detection info...";

	// TODO: show debug info-images
}




//##############################################################################
//##############################################################################
//######################### RULER DETECTION METHODS ############################
//##############################################################################
//##############################################################################
/**
* This function calculates the dpi of an image given a ruler template within the image
* @param img a RGB image in which to search for the template
* @param templ a RGB image serving as a template to search for
*
* @return returns the calculated DPI
*/
int RulerDetectionPlugin::calculateDPI(cv::Mat img, cv::Mat ruler_template) const {

	// transform image and ruler to grayscale
	cv::Mat gray_image(img.size(), CV_8UC1);
	cv::cvtColor(img, gray_image, CV_BGR2GRAY);
	cv::Mat gray_Template(ruler_template.size(), CV_8UC1);
	cv::cvtColor(ruler_template, gray_Template, CV_BGR2GRAY);
	
	// locate the ruler template
	cv::RotatedRect rectangle = locateTemplate(gray_image, gray_Template);

	// crop out the found template
	cv::Mat extractedTemplate = cropRotatedRectangle(gray_image, rectangle);

	// calculate the DPI
	int dpi = getDPIFromCroppedRuler(extractedTemplate);

	return dpi;
}

/**
* This function finds the location of a template-image within img and returns it as a fitted rotated rectangle
* @param templ a grayscale image serving as a template to search for
* @param img a grayscale image in which to search for the template
*
* @return a rotated rectangle representing a fitted bounding box of the template within img
*/
cv::RotatedRect RulerDetectionPlugin::locateTemplate(cv::Mat img, cv::Mat templ) const {
	qDebug() << "-> Locate template. Hessian Threshold =" << curr_hessianThreshold;

	cv::Ptr<cv::xfeatures2d::SURF> detector = cv::xfeatures2d::SURF::create(curr_hessianThreshold);

	qDebug() << "--- Detect keypoints...";
	std::vector<cv::KeyPoint> keypoints_object, keypoints_scene;
	detector->detect(templ, keypoints_object);
	detector->detect(img, keypoints_scene);

	qDebug() <<"--- Get descriptors...";
	cv::Ptr<cv::xfeatures2d::SURF> extractor = cv::xfeatures2d::SURF::create();

	cv::Mat descriptors_object, descriptors_scene;
	extractor->compute(templ, keypoints_object, descriptors_object);
	extractor->compute(img, keypoints_scene, descriptors_scene);

	qDebug() << "--- Match descriptors with FLANN matcher..";
	cv::FlannBasedMatcher matcher;
	std::vector< cv::DMatch > matches;
	matcher.match(descriptors_object, descriptors_scene, matches);

	qDebug() << "--- Find good matches...";
	// Calculate min and max distances between keypoints
	double max_dist = 0;
	double min_dist = 100;
	for (int i = 0; i < descriptors_object.rows; i++) {
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	// Find good matches, i.e. whose distance is less than 3*min_dist
	std::vector< cv::DMatch > good_matches;
	for (int i = 0; i < descriptors_object.rows; i++) {
		if (matches[i].distance < 3 * min_dist) {
			good_matches.push_back(matches[i]);
		}
	}

	// Find the template
	std::vector<cv::Point2f> obj;
	std::vector<cv::Point2f> scene;
	for (uint i = 0; i < good_matches.size(); i++) {
		// Get keypoints from the good matches
		obj.push_back(keypoints_object[good_matches[i].queryIdx].pt);
		scene.push_back(keypoints_scene[good_matches[i].trainIdx].pt);
	}

	qDebug() << "--- Calculate homography from best matches with RANSAC...";
	cv::Mat H = cv::findHomography(obj, scene, CV_RANSAC);

	// Get the corners from the ruler to be detected
	std::vector<cv::Point2f> ruler_corners(4);
	ruler_corners[0] = cvPoint(0, 0);
	ruler_corners[1] = cvPoint(templ.cols, 0);
	ruler_corners[2] = cvPoint(templ.cols, templ.rows);
	ruler_corners[3] = cvPoint(0, templ.rows);

	// Transform the ruler corners with the found homography to get the coords in the space of the sceneimage
	std::vector<cv::Point2f> scene_corners(4);
	cv::perspectiveTransform(ruler_corners, scene_corners, H);

	// Find best fitting rectangle
	std::vector<cv::Point> points;					// vector for 1 contour containing several points
	points.push_back(scene_corners[0]);				// fill contour with points
	points.push_back(scene_corners[1]);
	points.push_back(scene_corners[2]);
	points.push_back(scene_corners[3]);
	cv::RotatedRect rectangle = cv::minAreaRect(points);

	return rectangle;
}


/**
* This function crops out the region specified by rect in a given image
* For this, an adapted approach from http://felix.abecassis.me/2011/10/opencv-rotation-deskewing/ is used
* @param src_image the image from which to crop out a region
* @param rect a RotatedRect specifying the region to crop
*
* @return the cropped out region
*/
cv::Mat RulerDetectionPlugin::cropRotatedRectangle(cv::Mat src_image, cv::RotatedRect rect) const {
	qDebug() << "-> Crop template region...";
	cv::Mat M, rotated, cropped;

	// get rotation matrix
	M = getRotationMatrix2D(rect.center, rect.angle, 1.0);
	// perform affine transformation
	warpAffine(src_image, rotated, M, src_image.size(), cv::INTER_CUBIC);
	// crop the resulting image
	getRectSubPix(rotated, rect.size, rect.center, cropped);
	
	return cropped;
}



/**
* This function returns the approximate DPI for a cropped image of a horizonal ruler
* @param grayscale ruler with properly distinguishable ticks
*
* @return the approximate DPI of the image
*/
int RulerDetectionPlugin::getDPIFromCroppedRuler(cv::Mat img) const {
	qDebug() << "-> Calculate DPI...";

	// Extend and pad image to optimal size (to improve DFT performance)
	cv::Mat padded;
	int m = cv::getOptimalDFTSize(img.rows);
	int n = cv::getOptimalDFTSize(img.cols);
	cv::copyMakeBorder(img, padded, 0, m - img.rows, 0, n - img.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

	cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F)};
	cv::Mat complexI;
	cv::merge(planes, 2, complexI);         // Add to the expanded another plane with zeros

	qDebug() << "--- Perform DFT";
	cv::dft(complexI, complexI);

	qDebug() << "--- Processing of DFT result (swap quadrants, log-scale, etc.)";
	// Transform real and complex values to magnitude
	// => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
	cv::split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
	cv::Mat magI = cv::Mat::zeros(planes[0].size(), planes[0].type());
	cv::magnitude(planes[0], planes[1], magI);

	// Transform to log scale (M1 = log(1+M))
	magI += cv::Scalar::all(1);
	cv::log(magI, magI);

	// transform to more handable format with 0 at center pos
	magI = swapQuadrants(magI);		

	cv::normalize(magI, magI, 0, 1, CV_MINMAX);

	qDebug() << "--- Find pixels per tick";
	float pixelPerTick = getPixelPerTick(magI);

	qDebug() << "--- Transform pixel per tick into dpi via proper tick-metric (mm/cm/in)";
	
	qDebug() << "Pixel per tick: " << pixelPerTick;
	int dpi = applyMetricToPixelticks(pixelPerTick, curr_tickDistance);

	return dpi;
}


/**
* This function swapps the diagonally opposite quadrants of an image
* @param img the image for which the quadrants are swapped (ususally a Fourier image from the OpenCV DFT-method)
*
* @return the image with swapped quadrants
*/
cv::Mat RulerDetectionPlugin::swapQuadrants(cv::Mat img) const {

	// rearrange the quadrants of (Fourier) image so that the origin is at the image center
	int cx = img.cols / 2;
	int cy = img.rows / 2;

	cv::Mat q0(img, cv::Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
	cv::Mat q1(img, cv::Rect(cx, 0, cx, cy));  // Top-Right
	cv::Mat q2(img, cv::Rect(0, cy, cx, cy));  // Bottom-Left
	cv::Mat q3(img, cv::Rect(cx, cy, cx, cy)); // Bottom-Right

	cv::Mat tmp;								// swap quadrants (Top-Left with Bottom-Right)
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);								// swap quadrant (Top-Right with Bottom-Left)
	q2.copyTo(q1);
	tmp.copyTo(q2);

	return img;
}


/**
* This function processes an image of a horizontal ruler to extract the pixel per tick in the original space
* @param img fourier image of a ruler
*
* @return the pixel per tick on the ruler
*/
float RulerDetectionPlugin::getPixelPerTick(cv::Mat img) const {
	
	// Project to x-axis (= sum up each column), because peaks show as vertical streaks.
	// When projecting the streaks become 1D local maxima
	cv::Mat reduced = cv::Mat(1, img.cols, CV_64FC1);
	cv::reduce(img, reduced, 0, CV_REDUCE_SUM);

	/*
	cv::Mat repeatedTest(img.size(), CV_8UC1);
	cv::repeat(reduced, img.rows, 1, repeatedTest);
	cv::Mat normalizedTest;
	cv::normalize(repeatedTest, normalizedTest, 0, 255, cv::NORM_MINMAX, CV_8UC1);
	imshow("Repeated", normalizedTest);
	imwrite("Repeated.png", normalizedTest);
	*/

	// Put the 1D values into a vector
	std::vector<float> data;
	if (reduced.isContinuous()) {
		data.assign((float*)reduced.datastart, (float*)reduced.dataend);
	} else {
		for (int i = 0; i < reduced.rows; ++i) {
			data.insert(data.end(), (float*)reduced.ptr<uchar>(i), (float*)reduced.ptr<uchar>(i) + reduced.cols);
		}
	}

	// Analyze the signal of 1D points
	p1d::Persistence1D p;
	p.RunPersistence(data);

	// Get all extrema with a persistence larger than 5.
	std::vector<p1d::TPairedExtrema> Extrema;
	p.GetPairedExtrema(Extrema, 5);

	// Print all found pairs - pairs are sorted DESCENDING wrt. persistence.
	std::reverse(Extrema.begin(), Extrema.end());		// flip sorting order
	cv::Mat maxima = cv::Mat(1, img.cols, CV_64FC1);	// only for visualization
	maxima.setTo(cv::Scalar(0.0));
	
	// Of the strongest X maxima take the left-most and the right-most (ignore all minima)
	int minLocalMax = 99999999;
	int maxLocalMax = -99999999;
	int count = 0;
	for (std::vector<p1d::TPairedExtrema >::iterator it = Extrema.begin(); it != Extrema.end(); it++) {
		qDebug() << "Persistence: " << (*it).Persistence << " maximum index: " << (*it).MaxIndex;
		maxima.at<double>((*it).MaxIndex) = 255;

		if ((*it).MaxIndex < minLocalMax) {
			minLocalMax = (*it).MaxIndex;
		}
		if ((*it).MaxIndex > maxLocalMax) {
			maxLocalMax = (*it).MaxIndex;
		}

		count++;
		// Take only the best X points
		if (count >= curr_referencePoints) break;
	}

	// calc pixel per tick-intervall in frequency space
	float intervalSize = (float)(maxLocalMax - minLocalMax) / (float)(curr_referencePoints - 1);

	// calc pixel per tick-intervall in image-space (invers proportional with T=img-width)
	float pixelPerTick = (float)img.cols / intervalSize;

	qDebug() << "Pixel per tick: " << pixelPerTick;

	/*
	cv::Mat repeated(img.size(), CV_8UC1);
	cv::repeat(maxima, img.rows, 1, repeated);
	double min, max;
	cv::minMaxLoc(repeated, &min, &max);
	cv::Mat normalized;
	cv::normalize(repeated, normalized, 0, 255, cv::NORM_MINMAX, CV_8UC1);

	cv::Mat score;
	//cv::adaptiveThreshold(normalized, score, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 301, 0);
	score = normalized;
	cv::minMaxLoc(score, &min, &max);
	std::cout << "After Adaptivethreshold: Min: " << min << ", Max: " << max << std::endl;

	//cv::GaussianBlur(score, score, cv::Size(5,1), 3.5);

	score.convertTo(score, CV_32FC1);
	cv::normalize(score, score, 0, 1, cv::NORM_MINMAX, CV_32FC1);
	cv::imwrite("Scores.png", convertFloat32toUint8_1channel(score));
	*/
	return pixelPerTick;
}

/**
* This function takes the pixel per tick and calculates the dpi depending on the selected metric-setting (e.g. cm/mm/in)
* @param pixelPerTick how many pixel are in between two ticks
*
* @return the dpi, -1 if invalid pixelPerTick-value or unknown tickMetric
*/
int RulerDetectionPlugin::applyMetricToPixelticks(float pixelPerTick, QString tickMetric) const {

	if (pixelPerTick < 0.0f) {
		qCritical() << "Invalid PixelPerTick-Value: " << pixelPerTick;
		return -1;
	}

	int dpi = 0;
	if (tickMetric == "mm") {
		dpi = (int)(pixelPerTick * 2.54f * 10.0f + 0.5f);	// convert and round
	} else if (tickMetric == "cm") {
		dpi = (int)(pixelPerTick * 2.54f + 0.5f);			// convert and round
	} else if (tickMetric == "inch") {
		dpi = (int)(pixelPerTick + 0.5f);						// just round
	} else {
		qCritical() << "Not supported tick-metric: " << tickMetric;
		dpi = -1;
	}
	return dpi;
}

cv::Mat RulerDetectionPlugin::convertFloat32toUint8_1channel(cv::Mat image) const {
	cv::Mat convertedImg = cv::Mat::zeros(image.size(), CV_8UC1);
	image.convertTo(convertedImg, CV_8UC1, 255.0);
	return convertedImg;
}


};

