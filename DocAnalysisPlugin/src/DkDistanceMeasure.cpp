/*******************************************************************************************************
 DkDistanceMeasure.cpp
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

#include "DkDistanceMeasure.h"
#include "DkMetaData.h"

#include <QVector2D>

namespace nmc {

/**
* Constructor for the distance measure tool.
**/
DkDistanceMeasure::DkDistanceMeasure() {

	dist_cm = 0.0;
	dist_inch = 5.0;
	points[0] = QPoint();
	points[1] = QPoint();
	snap = false;

}

DkDistanceMeasure::~DkDistanceMeasure() {
	
}

void DkDistanceMeasure::setMetaData(QSharedPointer<DkMetaDataT> metaData) {

	this->metaData = metaData;

}

/**
* Add a point to be considered for distance measuring. Called when user clicked.
* Note: Maximum two points are considred.
* If a second point is handed to the functionk, it starts calculating the distance
* @param point Point clicked by the user from which/until the distance shall be measured
* \sa calculateStartEndDistance()
**/
void DkDistanceMeasure::setPoint(QPoint point) {

	if(points[0].isNull())
		points[0] = point;
	else {
		if(!snap) {
			points[1] = point;
		} else {
			// use snapped point
			points[1] = performSnapping(points[0], point);
		}
		calculateStartEndDistance();
	}
}

/**
* Uses a previously set point (the start point) and calculates the distance from it to the current Point.
* Called when moving the mouse.
* @param point The new current point
* \sa getStartPoint() calculateStartEndDistance()
**/
void DkDistanceMeasure::setCurPoint(QPoint point) {
	
	if(!snap) {
		curPoint = point;
	} else {
		// snapping is active -> snap to 45 degree line
		curPoint = performSnapping(getStartPoint(), point);
	}

	calculateStartEndDistance();
}

/**
* Resets the start and the end point
**/
void DkDistanceMeasure::resetPoints() {

	points[0] = QPoint();
	points[1] = QPoint();
	curPoint = QPoint();
}

/**
* Calculates the distance between the two set points considering the resolution
* retrieved by the given metadata.
* The distance is calculated between the start point and end point.
* If the end point is not yet available, then the current point (curPoint) is used.
* \sa getDistanceInCm() getDistanceInInch()
**/
void DkDistanceMeasure::calculateStartEndDistance() {

	if (points[0].isNull()) return;

	// get the image resolution for distance calculation
	x_res = 72;		// markus: 72 dpi is the default value assumed
	y_res = 72;

	// >DIR: get metadata resolution if available [21.10.2014 markus]
	if (metaData) {

		QVector2D res = metaData->getResolution();
		x_res = res.x();
		y_res = res.y();
	}

	float length_x_inch, length_y_inch;

	if (!points[1].isNull()) curPoint = points[1];
	
	length_x_inch = abs(curPoint.x() - points[0].x()) / x_res;
	length_y_inch = abs(curPoint.y() - points[0].y()) / y_res;

	dist_inch = sqrt(length_x_inch * length_x_inch + length_y_inch * length_y_inch);
	dist_cm = dist_inch * 2.54;
}

/**
* Checks if start and end points have both been set
* @returns true if both points have been set already
**/
bool DkDistanceMeasure::hastStartAndEndPoint() {

	if(!points[0].isNull() && !points[1].isNull())
		return true;
	else
		return false;
}

/**
* Check if a start point has been set
* @returns true if the point has been set
**/
bool DkDistanceMeasure::hasStartPoint() {

	if(points[0].isNull())
		return false;
	else
		return true;
}

/**
* Set a flag to indicate that the tool shall snap to the nearest line at a 45 degree angle
**/
void DkDistanceMeasure::setSnapping(bool snap) {
	this->snap = snap;
}

/**
* Check if snapping is active
* @returns true if snap = true
**/
bool DkDistanceMeasure::isSnapping() {
	return snap;
}

/**
* Perform the actual snapping given a start and endpoint. Snaps to the
* nearest 45 degree line.
* @returns The point after snapping the end point/current point to the nearest 45 degree line
**/
QPoint DkDistanceMeasure::performSnapping(QPoint start, QPoint end) {
	if(start.y() == end.y() || start.x() == end.x()) {
		// on "x-Axis" or "y-Axis"
		return end;
	}

	float alpha;
	QPoint snapPoint(end);

	alpha = atan((abs(start.y() - end.y()) / (float)abs(end.x() - start.x())));
	
	if(alpha < PI/8) {
		snapPoint.setY(start.y());
	} else if (alpha > 3*PI/8) {
		snapPoint.setX(start.x());
	} else {
		int dy = abs(start.y() - end.y());
		int dx = abs(start.x() - end.x());

		// check which quadrant
		if (end.y() < start.y()) {
			// first or second quadrant
			if(end.x() > start.x()) {
				// first quadrant
				if(dx > dy) {
					snapPoint.setY(end.y() - (dx - dy));
				} else {
					snapPoint.setX(end.x() + (dy - dx));
				}
			} else {
				// second quadrant
				if(dx > dy) {
					snapPoint.setY(end.y() - (dx - dy));
				} else {
					snapPoint.setX(end.x() - (dy - dx));
				}
			}
		} else {
			// third or fourth quadrant
			if(end.x() > start.x()) {
				// fourth quadrant
				if(dx > dy) {
					snapPoint.setY(end.y() + (dx - dy));
				} else {
					snapPoint.setX(end.x() + (dy - dx));
				}
			} else {
				// third quadrant
				if(dx > dy) {
					snapPoint.setY(end.y() + (dx - dy));
				} else {
					snapPoint.setX(end.x() - (dy - dx));
				}
			}
		}
	}

	return snapPoint;
}

};