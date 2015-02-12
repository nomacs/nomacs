/*******************************************************************************************************
 DkDistanceMeasure.h
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

#include <qmath.h>
#include <QPoint>

#include "DkWidgets.h"

namespace nmc {

class DkMetaDataT;

#define PI 3.14159265
/**
* Main class for measuring the distance between two points within the image (either in cm or inch)
* based on the set image resolution in x and y direction.
**/
class DkDistanceMeasure {

public:
	DkDistanceMeasure();
	~DkDistanceMeasure();

	void setPoint(QPoint point);
	void setCurPoint(QPoint point);
	void resetPoints();
	float getDistanceInCm() { return dist_cm; };
	float getDistanceInInch() { return dist_inch; };
	bool hastStartAndEndPoint();
	bool hasStartPoint();
	void setSnapping(bool snap);
	bool isSnapping();
	void setMetaData(QSharedPointer<DkMetaDataT> metaData);
	
	QPoint getStartPoint() { return points[0]; };
	QPoint getEndPoint() { return points[1]; };
	QPoint getCurPoint() { return curPoint; };

private:
	QSharedPointer<DkMetaDataT> metaData; /**< metadata containing the image resolution **/
	QPoint points[2]; /**< start and end point **/
	QPoint curPoint; /**< The current point when moving the mouse **/
	float dist_cm; /**< The current distance in cm **/ 
	float dist_inch; /**< The current distance in inch **/
	float x_res; /**< Image resolution in x direction **/
	float y_res; /**< Image resolution in y direction **/
	bool snap; /**< flag to indicate if snapping to 45 degree lines should be performed **/

	void calculateStartEndDistance();
	QPoint performSnapping(QPoint start, QPoint end);
};
};
