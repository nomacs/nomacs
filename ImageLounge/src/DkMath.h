/*******************************************************************************************************
 DkMath.h
 Created on:	25.02.2010
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011 Florian Kleber <florian@nomacs.org>

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

#include <cmath>
//#include <cfloat>
//#include <algorithm>
//#include <vector>
//#include <list>
//#include <QSize>
//#include <QRect>
//#include <QPoint>
//#include <QPointF>
//
//#include "DkError.h"


#ifdef WITH_OPENCV
#include <opencv/cv.h>
using namespace cv;
#else

//#define int64 long long;
#define CV_PI 3.141592653589793238462643383279

int cvRound(float num);
int cvCeil(float num);
int cvFloor(float num);

#endif

#define DK_DEG2RAD	0.017453292519943
#define DK_RAD2DEG 	57.295779513082323

// no min max macros for windows...
#undef min
#undef max

/** 
 * Provides useful mathematical functions.
 **/
class DkMath {

public:
	
	/** 
	 * Divides the integer by 2.
	 * @param val the integer value.
	 * @return the half integer (floor(val)).
	 **/
	static int halfInt(int val) {
		return (val >> 1);
	}

	/**
	 * Computes a fast square root.
	 * @param val the value for which the root will be computed.
	 * @return the approximated square root of the value val.
	 **/
	static float fastSqrt(const float val) {

		long sqrtVal = *(long *) &val;

		//sqrtVal -= 1L<<23;	// Remove IEEE bias from exponent (-2^23)
		sqrtVal -= 127L<<23;
		// sqrtVal is now an approximation to logbase2(val)
		sqrtVal = sqrtVal>>1; // divide by 2
		//sqrtVal += 1L<<23;	// restore the IEEE bias from the exponent (+2^23)
		sqrtVal += 127L<<23;

		return *(float *) &sqrtVal;
	}

	/**
	 * Computes a fast inverse square root.
	 * @param x the value to be computed.
	 * @return the inverse square root of x.
	 **/
	static float invSqrt (float x) {
		float xhalf = 0.5f*x;
		int i = *(int*)&x;
		i = 0x5f3759df - (i>>1);
		x = *(float*)&i;
		x = x*(1.5f - xhalf*x*x);
		return x;
	}

	/**
	 * Returns the greatest common divisor (GGT).
	 * Where a must be greater than b.
	 * @param a the greater number.
	 * @param b the smaller number.
	 * @return int the greatest common divisor.
	 **/ 
	static int gcd(int a, int b) {
		// zu deutsch: ggt

		if (b==0)
			return a;
		else
			return gcd(b, a%b);
	}

	/**
	 * Computes the normalized angle in radians.
	 * The normalized angle is in this case defined as the
	 * corresponding angle within [0 pi].
	 * @param angle an angle in radians.
	 * @return the normalized angle in radians within [0 pi].
	 **/
	static double normAngleRad(double angle) {

		// this could be a bottleneck
		if (abs(angle) > 1000)
			return angle;

		while (angle < 0)
			angle += 2*CV_PI;
		while (angle >= 2*CV_PI)
			angle -= 2*CV_PI;

		return angle;
	}

	/**
	 * Computes the normalized angle within startIvl and endIvl.
	 * @param angle the angle in radians.
	 * @param startIvl the interval's lower bound.
	 * @param endIvl the interval's upper bound.
	 * @return the angle within [startIvl endIvl)
	 **/
	static double normAngleRad(double angle, double startIvl, double endIvl) {

		// this could be a bottleneck
		if (abs(angle) > 1000)
			return angle;

		while(angle <= startIvl)
			angle += endIvl-startIvl;
		while(angle > endIvl)
			angle -= endIvl-startIvl;

		return angle;
	}

	/**
	 * Computes the normalized angle within startIvl and endIvl.
	 * @param angle the angle in radians.
	 * @param startIvl the interval's lower bound.
	 * @param endIvl the interval's upper bound.
	 * @return the angle within [startIvl endIvl)
	 **/
	static float normAngleRad(float angle, float startIvl, float endIvl) {

		// this could be a bottleneck
		if (abs(angle) > 1000)
			return angle;

		while(angle <= startIvl)
			angle += endIvl-startIvl;
		while(angle > endIvl)
			angle -= endIvl-startIvl;

		return angle;
	}

	/**
	 * Computes the normalized angle in radians.
	 * The normalized angle is in this case defined as the
	 * corresponding angle within [0 pi].
	 * @param angle an angle in radians.
	 * @return the normalized angle in radians within [0 pi].
	 **/
	static float normAngleRad(float angle) {

		// this could be a bottleneck
		if (abs(angle) > 1000)
			return angle;

		while (angle < 0)
			angle += 2*(float)CV_PI;
		while (angle >= 2.0*CV_PI)
			angle -= 2*(float)CV_PI;

		return angle;
	}

	static double distAngle(const double angle1, const double angle2) {

		double nAngle1 = normAngleRad(angle1);
		double nAngle2 = normAngleRad(angle2);

		double angle = abs(nAngle1 - nAngle2);

		return (angle > CV_PI) ? 2*CV_PI - angle : angle;
	}

	/**
	 * Check if a number is a power of two.
	 * @param ps a positive integer
	 * @return true if ps is a power of two.
	 **/
	static bool isPowerOfTwo(unsigned int ps) {

		// count the bit set, see: http://tekpool.wordpress.com/category/bit-count/
		unsigned int bC;

		bC = ps - ((ps >> 1) & 033333333333) - ((ps >> 2) & 011111111111);
		bC = ((bC + (bC >> 3)) & 030707070707) % 63;

		return bC == 1;
	}

	/**
	 * Returns the next power of two.
	 * @param val a number for which the next power of two needs to be computed.
	 * @return the next power of two for val.
	 **/
	static int getNextPowerOfTwo(int val) {

		int pt = 1;
		while (val > pt)
			pt = pt << 1;	// *2

		return pt;
	}

	/**
	 * Returns the value of f(x,sigma) where f is a gaussian.
	 * @param sigma of the gaussian distribution.
	 * @param x param of the gaussian.
	 * @return f(x,sigma) .
	 **/
	static float getGaussian(float sigma, float x) {

		return 1/sqrt(2*(float)CV_PI*sigma*sigma) * exp(-(x*x)/(2*sigma*sigma));
	}

	template <typename numFmt>
	static numFmt sq(numFmt x) {
		
		return x*x;
	}

	template <typename numFmt>
	static double log2(numFmt x) {

		return log((double)x)/log(2.0);
	}


};
