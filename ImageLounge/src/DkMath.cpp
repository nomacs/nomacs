/*******************************************************************************************************
 DkMath.cpp
 Created on:	22.03.2010
 
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

#include "DkMath.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QCursor>
#include <QTransform>
#pragma warning(pop)		// no warnings from includes - end


#ifndef WITH_OPENCV
int cvRound(float num) {

	return (int) (num+0.5f);
}

int cvCeil(float num) {

	return (int) (num+1.0f);
}

int cvFloor(float num) {

	return (int) num;
}

#endif

namespace nmc {

// DkRotatingRect --------------------------------------------------------------------
DkRotatingRect::DkRotatingRect(QRectF rect) {

	if (rect.isEmpty()) {

		for (int idx = 0; idx < 4; idx++)
			this->rect.push_back(QPointF());
	}
	else
		this->rect = rect;

}

DkRotatingRect::~DkRotatingRect() {}

bool DkRotatingRect::isEmpty() const {

	if (rect.size() < 4)
		return true;

	QPointF lp = rect[0]; 
	for (int idx = 1; idx < rect.size(); idx++) {

		if (lp != rect[idx]) {
			return false;
		}
		lp = rect[idx];
	}
	return true;
}

void DkRotatingRect::setAllCorners(QPointF &p) {

	for (int idx = 0; idx < rect.size(); idx++)
		rect[idx] = p;

}

DkVector DkRotatingRect::getDiagonal(int cIdx) const {

	DkVector c0 = rect[cIdx % 4];
	DkVector c2 = rect[(cIdx+2) % 4];

	return c2 - c0;
}

QCursor DkRotatingRect::cpCursor(int idx) {

	double angle = 0;

	if (idx >= 0 && idx < 4) {

		// this seems a bit complicated...
		// however the points are not necessarily stored in clockwise order...
		DkVector e1 = rect[(idx+1) % 4] - rect[idx];
		DkVector e2 = rect[(idx+3) % rect.size()] - rect[idx];
		e1.normalize();
		e2.normalize();
		DkVector rv = e1-e2;
		rv = rv.normalVec();
		angle = rv.angle();
	}
	else {
		DkVector edge = rect[(idx+1) % 4] - rect[idx % 4];
		angle = edge.normalVec().angle();	// the angle of the normal vector
	}

	angle = DkMath::normAngleRad(angle, -CV_PI/8.0, 7.0*CV_PI/8.0);

	if (angle > 5.0*CV_PI/8.0)
		return QCursor(Qt::SizeBDiagCursor);
	else if (angle > 3.0*CV_PI/8.0)
		return QCursor(Qt::SizeVerCursor);
	else if (angle > CV_PI/8.0)
		return QCursor(Qt::SizeFDiagCursor);
	else
		return QCursor(Qt::SizeHorCursor);

}

void DkRotatingRect::updateCorner(int cIdx, QPointF nC, DkVector oldDiag) {

	// index does not exist
	if (cIdx < 0 || cIdx >= rect.size()*2)
		return;

	if (rect[(cIdx+1) % 4] == rect[(cIdx+3) % 4]) {
		QPointF oC = rect[(cIdx+2) % 4];	// opposite corner
		rect[cIdx] = nC;
		rect[(cIdx+1) % 4] = QPointF(nC.x(), oC.y());
		rect[(cIdx+3) % 4] = QPointF(oC.x(), nC.y());
	}
	// these indices indicate the control points on edges
	else if (cIdx >= 4 && cIdx < 8) {

		DkVector c0 = rect[cIdx % 4];
		DkVector n = (rect[(cIdx+1) % 4] - c0).normalVec();
		n.normalize();

		// compute the offset vector
		DkVector oV = n * n.scalarProduct(nC-c0);

		rect[cIdx % 4] = (rect[cIdx % 4] + oV).getQPointF();
		rect[(cIdx+1) % 4] = (rect[(cIdx+1) % 4] + oV).getQPointF();
	}
	else {

		// we have to update the n-1 and n+1 corner
		DkVector cN = nC;
		DkVector c0 = rect[cIdx];
		DkVector c1 = rect[(cIdx+1) % 4];
		DkVector c2 = rect[(cIdx+2) % 4];
		DkVector c3 = rect[(cIdx+3) % 4];

		if (!oldDiag.isEmpty()) {
			DkVector dN = oldDiag.normalVec();
			dN.normalize();

			float d = dN*(cN-c2);
			cN += (dN*-d);
		}

		// new diagonal
		float diagLength = (c2-cN).norm();
		float diagAngle = (float)(c2-cN).angle();

		// compute the idx-1 corner
		float c1Angle = (float)(c1-c0).angle();
		float newLength = cos(c1Angle - diagAngle)*diagLength;
		DkVector nc1 = DkVector((newLength), 0);
		nc1.rotate(-c1Angle);

		// compute the idx-3 corner
		float c3Angle = (float)(c3-c0).angle();
		newLength = cos(c3Angle - diagAngle)*diagLength;
		DkVector nc3 = DkVector((newLength), 0);
		nc3.rotate(-c3Angle);

		rect[(cIdx+1) % 4] = (nc1+cN).getQPointF();			
		rect[(cIdx+3) % 4] = (nc3+cN).getQPointF();
		rect[cIdx] = cN.getQPointF();
	}
}

const QPolygonF& DkRotatingRect::getPoly() const {

	return rect;
}

void DkRotatingRect::setPoly(QPolygonF &poly) {

	rect = poly;
}

QPolygonF DkRotatingRect::getClosedPoly() {

	if (rect.isEmpty())
		return QPolygonF();

	QPolygonF closedPoly = rect;
	closedPoly.push_back(closedPoly[0]);

	return closedPoly;
}

QPointF DkRotatingRect::getCenter() const {

	if (rect.empty())
		return QPointF();

	DkVector c1 = rect[0];
	DkVector c2 = rect[2];

	return ((c2-c1)*0.5f + c1).getQPointF();
}

void DkRotatingRect::setCenter(const QPointF& center) {

	if (rect.empty())
		return;

	DkVector diff = getCenter() - center;

	for (int idx = 0; idx < rect.size(); idx++) {

		rect[idx] = rect[idx] - diff.getQPointF();
	}
}

double DkRotatingRect::getAngle() const {

	// default upper left corner is 0
	DkVector xV = rect[3] - rect[0];
	return xV.angle();
}

float DkRotatingRect::getAngleDeg() const {

	float sAngle = (float)(getAngle()*DK_RAD2DEG);

	while (sAngle > 90)
		sAngle -= 180;
	while (sAngle < -90)
		sAngle += 180;

	sAngle = qRound(sAngle*100)/100.0f;	// round to 2 digits

	return sAngle;
}

void DkRotatingRect::getTransform(QTransform& tForm, QPointF& size) const {

	if (rect.size() < 4)
		return;

	// default upper left corner is 0
	DkVector xV = DkVector(rect[3] - rect[0]).round();
	DkVector yV = DkVector(rect[1] - rect[0]).round();

	QPointF ul = QPointF(qRound(rect[0].x()), qRound(rect[0].y()));
	size = QPointF(xV.norm(), yV.norm());

	qDebug() << xV.getQPointF();
	qDebug() << "size: " << size;


	double angle = xV.angle();
	angle = DkMath::normAngleRad(angle, -CV_PI, CV_PI);

	if (abs(angle) > DBL_EPSILON)
		qDebug() << "angle is > eps...";

	// switch width/height for /\ and \/ quadrants
	if (abs(angle) > CV_PI*0.25 && abs(angle) < CV_PI*0.75) {
		float x = (float)size.x();
		size.setX(size.y());
		size.setY(x);
	}

	// invariance -> user does not want to make a difference between an upside down rect
	if (angle > CV_PI*0.25 && angle < CV_PI*0.75) {
		angle -= CV_PI*0.5;
		ul = rect[1];
	}
	else if (angle > -CV_PI*0.75 && angle < -CV_PI*0.25) {
		angle += CV_PI*0.5;
		ul = rect[3];
	}
	else if (angle >= CV_PI*0.75 || angle <= -CV_PI*0.75) {
		angle += CV_PI;
		ul = rect[2];
	}

	tForm.rotateRadians(-angle);
	tForm.translate(qRound(-ul.x()), qRound(-ul.y()));	// round guarantees that pixels are not interpolated

}

std::ostream& DkRotatingRect::put(std::ostream& s) {

	s << "DkRotatingRect: ";
	for (int idx = 0; idx < rect.size(); idx++) {
		DkVector vec = DkVector(rect[idx]);
		s << vec << ", ";
	}

	return s;
}

}