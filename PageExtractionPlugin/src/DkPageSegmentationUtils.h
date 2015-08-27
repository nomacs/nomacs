/*******************************************************************************************************
 DkPageSegmentation.h

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2015 Markus Diem <markus@nomacs.org>

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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <opencv2/core/core.hpp>
#include <QString>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

/**
* A simple 2D vector class.
*/
class DkVector {

public:

	union {
		float x;		/**< the vector's x-coordinate*/
		float width;	/**< the vector's x-coordinate*/
		float r;		/**< radius for log-polar coordinates or red channel*/
		float h;		/**< hue channel*/
	};

	union {
		float y;		/**< the vector's y-coordinate*/
		float height;	/**< the vector's y-coordinate*/
		float theta;	/**< angle for log-polar coordinates*/
		float g;		/**< green channel*/
		float l;		/**< luminance channel*/
	};

	/** 
	* Default constructor.
	**/
	DkVector() : x(0), y(0) {
		empty = true;
	};

	/** 
	* Initializes an object.
	* @param x the vector's x-coordinate.
	* @param y the vector's y-coordinate.
	**/
	DkVector(float x, float y) {
		this->x = x;
		this->y = y;
		empty = false;
	};

	/**
	* Initializes an object by means of the OpenCV size.
	* @param s the size.
	**/
	DkVector(cv::Size s) {
		this->width  = (float)s.width;
		this->height = (float)s.height;
		empty = false;
	};

	/**
	* Initializes a Vector by means of a OpenCV Point.
	* @param p the point
	**/
	DkVector(cv::Point2f p) {
		this->x = p.x;
		this->y = p.y;
		empty = false;
	};

	/**
	* Initializes a Vector by means of a OpenCV Point.
	* @param p the point
	**/
	DkVector(cv::Point p) {
		this->x = (float)p.x;
		this->y = (float)p.y;
		empty = false;
	};


	/** 
	* Default destructor.
	**/
	virtual ~DkVector() {};

	QString toString() const {
		QString msg;
		msg += "<" + QString::number(x) + ", " + QString::number(y) + ">";
		return msg;
	};

	/**
	* Compares two vectors.
	* @return true if both vectors have the same coordinates
	*/
	virtual bool operator== (const DkVector &vec) const {

		return (this->x == vec.x && this->y == vec.y);
	};

	/**
	* Compares two vectors.
	* @return true if both either the x or y coordinates of both
	* vectors are not the same.
	*/
	virtual bool operator!= (const DkVector &vec) const {

		return (this->x != vec.x || this->y != vec.y);
	};

	/**
	* Decides which vector is smaller.
	* If y is < vec.y the function returns true.
	* Solely if y == vec.y the x coordinates are compared.
	* @param vec the vector to compare this instance with.
	* @return true if y < vec.y or y == vec.y && x < vec.y.
	**/
	virtual bool operator< (const DkVector &vec) const {

		if (y != vec.y)
			return y < vec.y;
		else
			return x < vec.x;
	};

	/**  
	* Adds vector to the current vector.
	* @param vec the vector to be added
	*/
	virtual void operator+= (const DkVector &vec) {

		this->x += vec.x;
		this->y += vec.y;
	};

	/** 
	* Adds a scalar to the current vector.
	* @param scalar the scalar which should be added
	*/
	virtual void operator+= (const float &scalar) {

		this->x += scalar;
		this->y += scalar;
	};

	/** 
	* Computes the direction vector between this vector and vec.
	* Computes the direction vector pointing to the current vector
	* and replacing it.
	*/
	virtual void operator-= (const DkVector &vec) {

		this->x -= vec.x;
		this->y -= vec.y;
	};

	/** 
	* Subtracts a scalar from the current vector.
	* @param scalar the scalar which should be subtracted.
	*/
	virtual void operator-= (const float &scalar) {

		this->x -= scalar;
		this->y -= scalar;
	};

	/** 
	* Scalar product.
	* @param vec a vector which should be considered for the scalar product.
	* @return the scalar product of vec and the current vector.
	*/ 
	virtual float operator* (const DkVector &vec) const {

		return this->x*vec.x + this->y*vec.y;
	};

	/** 
	* Scalar multiplication.
	* @param scalar a scalar.
	*/
	virtual void operator*= (const float scalar) {

		this->x *= scalar;
		this->y *= scalar;
	};

	/** 
	* Scalar division.
	* @param scalar a scalar.
	*/
	virtual void operator/= (const float scalar) {
		this->x /= scalar;
		this->y /= scalar;
	};

	// friends ----------------

	/** 
	* Adds a vector to the current vector.
	* @param vec the vector which should be added
	* @return the addition of the current and the given vector.
	*/
	friend DkVector operator+ (const DkVector &vec, const DkVector &vec2) {

		return DkVector(vec.x+vec2.x, vec.y+vec2.y);
	};

	/** 
	* Adds a scalar to the current vector.
	* @param scalar the scalar which should be added
	* @return the addition of the current vector and the scalar given.
	*/
	friend DkVector operator+ (const DkVector &vec, const float &scalar) {

		return DkVector(vec.x+scalar, vec.y+scalar);
	};

	/** 
	* Adds a scalar to the current vector.
	* @param scalar the scalar which should be added
	* @return the addition of the current vector and the scalar given.
	*/
	friend DkVector operator+ (const float &scalar, const DkVector &vec) {

		return DkVector(vec.x+scalar, vec.y+scalar);
	};

	/** 
	* Computes the direction vector between the given vector and vec.
	* The direction vector C is computed by means of: C = B-A
	* where B is the current vector.
	* @param vec the basis vector A.
	* @return a direction vector that points from @param vec to the 
	* current vector.
	*/
	friend DkVector operator- (const DkVector &vec, const DkVector &vec2) {

		return DkVector(vec.x-vec2.x, vec.y-vec2.y);
	};

	/** 
	* Subtracts a scalar from the current vector.
	* @param scalar the scalar which should be subtracted.
	* @return the subtraction of the current vector and the scalar given.
	*/
	friend DkVector operator- (const DkVector vec, const float &scalar) {

		return DkVector(vec.x-scalar, vec.y-scalar);
	};

	/** 
	* Subtracts the vector from a scalar.
	* @param scalar the scalar which should be subtracted.
	* @return the subtraction of the current vector and the scalar given.
	*/
	friend DkVector operator- (const float &scalar, const DkVector vec) {

		return DkVector(scalar-vec.x, scalar-vec.y);
	};

	/** 
	* Scalar multiplication.
	* @param scalar a scalar.
	* @return the current vector multiplied by a scalar.
	*/
	friend DkVector operator* (const DkVector& vec, const float scalar) {

		return DkVector(vec.x*scalar, vec.y*scalar);
	};

	/** 
	* Scalar multiplication.
	* @param scalar a scalar.
	* @return the current vector multiplied by a scalar.
	*/
	friend DkVector operator* (const float &scalar, const DkVector& vec) {

		return DkVector(vec.x*scalar, vec.y*scalar);
	};

	/** 
	* Scalar division.
	* @param vec a vector which shall be divided.
	* @param scalar a scalar.
	* @return the current vector divided by a scalar.
	*/
	friend DkVector operator/ (const DkVector &vec, const float &scalar) {

		return DkVector(vec.x/scalar, vec.y/scalar);
	};

	/** 
	* Scalar division.
	* @param scalar a scalar.
	* @param vec a vector which shall be divided.
	* @return the current vector divided by a scalar.
	*/
	friend DkVector operator/ (const float &scalar, const DkVector &vec) {

		return DkVector(scalar/vec.x, scalar/vec.y);
	};

	/** 
	* Vector division.
	* @param vec1 the numerator.
	* @param vec2 the denominator.
	* @return vec1 divided by vec2.
	*/
	friend DkVector operator/ (const DkVector &vec1, const DkVector &vec2) {

		return DkVector(vec1.x/vec2.x, vec1.y/vec2.y);
	};

	/**
	* Writes the vector r coordinates to the outputstream s.
	* @param s the outputstream
	* @param r the vector
	* @return friend std::ostream& the modified outputstream
	**/ 
	friend std::ostream& operator<<(std::ostream& s, const DkVector& r) {

		return r.put(s);
	};

	/**
	* Writes the vector coordinates to the stream s.
	* @param s the output stream
	* @return std::ostream& the output stream with the coordinates.
	**/ 
	virtual std::ostream& put(std::ostream& s) const {

		return s << "[" << x << ", " << y << "]";
	};


	bool isEmpty() const {

		return empty && x == 0 && y == 0;
	};

	/**
	* Returns the largest coordinate.
	* @return float the largest coordinate
	**/ 
	virtual float maxCoord() {

		return std::max(x, y);
	};

	/**
	* Returns the largest coordinate.
	* @return float the largest coordinate.
	**/ 
	virtual float minCoord() {

		return std::min(x, y);
	};

	/**
	* Creates a new vector having the
	* maximum coordinates of both vectors.
	* Thus: n.x = max(this.x, vec.x).
	* @param vec the second vector.
	* @return a vector having the maximum 
	* coordinates of both vectors.
	**/
	virtual DkVector getMaxVec(const DkVector& vec) const {

		return DkVector(std::max(x, vec.x), std::max(y, vec.y));
	}

	/**
	* Creates a new vector having the
	* minimum coordinates of both vectors.
	* Thus: n.x = min(this.x, vec.x).
	* @param vec the second vector.
	* @return a vector having the minimum
	* coordinates of both vectors.
	**/
	virtual DkVector getMinVec(const DkVector& vec) const{

		return DkVector(std::min(x, vec.x), std::min(y, vec.y));
	}


	/**
	* Creates a new vector having the
	* maximum coordinates of both vectors.
	* Thus: n.x = max(this.x, vec.x).
	* @param vec the second vector.
	**/
	virtual void maxVec(const DkVector& vec) {

		x = std::max(x, vec.x);
		y = std::max(y, vec.y);
	}

	/**
	* Creates a new vector having the
	* minimum coordinates of both vectors.
	* Thus: n.x = min(this.x, vec.x).
	* @param vec the second vector.
	**/
	virtual void minVec(const DkVector vec) {

		x = std::min(x, vec.x);
		y = std::min(y, vec.y);
	}

	/**
	* Swaps the coordinates of a vector.
	**/
	void swap() {
		float xtmp = x;
		x = y;
		y = xtmp;
	}

	/**
	* Returns the vector's angle in radians.
	* The angle is computed by: atan2(y,x).
	* @return the vector's angle in radians.
	**/
	double angle() {
		return atan2(y, x);
	};

	/**
	* Rotates the vector by a specified angle in radians.
	* The rotation matrix is: R(-theta) = [cos sin; -sin cos]
	* @param angle the rotation angle in radians.
	**/
	void rotate(double angle) {

		float xtmp = x;
		x = (float) ( xtmp*cos(angle)+y*sin(angle));
		y = (float) (-xtmp*sin(angle)+y*cos(angle));
	};


	void computeTransformed(DkVector centerR, DkVector centerO, double angle) {

		*this -= centerR;
		rotate(angle);
		*this += centerO;
	};


	/**
	* Computes the absolute value of both coordinates.
	**/
	virtual void abs() {

		x = fabs(x);
		y = fabs(y);
	};

	virtual void floor() {
		x = (float)cvFloor(x);
		y = (float)cvFloor(y);
	};

	virtual void ceil() {
		x = (float)cvCeil(x);
		y = (float)cvCeil(y);
	};

	virtual void round() {
		x = (float)cvRound(x);
		y = (float)cvRound(y);
	};

	/**
	* Clips the vector's coordinates to the bounds given.
	* @param maxBound the maximum bound.
	* @param minBound the minimum bound.
	**/
	virtual void clipTo(float maxBound = 1.0f, float minBound = 0.0f) {

		if (minBound > maxBound) {
			//std::cout << "[DkVector] maxBound < minBound: " << maxBound <<  " < " << minBound << std::endl;
			return;
		}

		if (x > maxBound)		x = maxBound;
		else if (x < minBound)	x = minBound;
		if (y > maxBound)		y = maxBound;
		else if (y < minBound)	y = minBound;
	};

	/**
	* Clips the vector's coordinates to the bounds given.
	* @param maxBound the maximum bound.
	* @param minBound the minimum bound.
	**/
	virtual void clipTo(const DkVector& maxBound) {

		if (maxBound.x < 0  || maxBound.y < 0) {

			DkVector nonConst = maxBound;
			//std::cout << "[WARNING] clipTo maxBound < 0: " << nonConst << std::endl;
			return;
		}

		maxVec(DkVector(0.0f,0.0f));
		minVec(maxBound);
	};

	/**
	* Convert DkVector to cv::Point.
	* @return a cv::Point having the vector's coordinates.
	**/
	virtual cv::Point getCvPoint32f() const {

		return cv::Point_<float>(x, y);
	};

	/**
	* Convert DkVector to cv::Point.
	* The vectors coordinates are rounded.
	* @return a cv::Point having the vector's coordinates.
	**/
	virtual cv::Point getCvPoint() const {

		return cv::Point(cvRound(x), cvRound(y));
	};

	/**
	* Convert DkVector to cv::Size.
	* The vector coordinates are rounded.
	* @return a cv::Size having the vector's coordinates.
	**/
	cv::Size getCvSize() const {

		return cv::Size(cvRound(width), cvRound(height));
	}


	/** 
	* Normal vector.
	* @return a vector which is normal to the current vector
	* (rotated by 90° counter clockwise).
	*/
	DkVector normalVec() const {

		return DkVector(-y, x);
	};

	/** 
	* The vector norm.
	* @return the vector norm of the current vector.
	*/
	virtual float norm() const {

		return sqrt(this->x*this->x + this->y*this->y);
	}

	/**
	* Convenience function.
	* @return float the vector norm (norm()).
	**/ 
	virtual float length() const {
		return norm();
	}

	/** 
	* Normalizes the vector.
	* After normalization the vector's magnitude is |v| = 1
	*/
	virtual void normalize() {
		float n = norm();
		x /= n; 
		y /= n;
	};

	///** 
	// * Returns the normalized vector.
	// * After normalization the vector's magnitude is |v| = 1
	// * @return the normalized vector.
	// */
	//virtual DkVector getNormalized() const {
	//	float n = norm();

	//	return DkVector(x/n, y/n);
	//};


	/** 
	* Returns the angle between two vectors
	*  @param vec vector
	*  @return the angle between two vectors
	*/
	double angle(const DkVector &vec) const {
		return acos(cosv(vec));
	};

	double cosv(const DkVector& vec) const {
		return (this->x*vec.x + this->y*vec.y) / (sqrt(this->x*this->x + this->y*this->y)*sqrt(vec.x*vec.x + vec.y*vec.y));
	};

	/** Returns euclidean distance between two vectors
	*  @param vec vector
	*  @return the euclidean distance
	*/
	virtual float euclideanDistance(const DkVector &vec) const {
		return sqrt((this->x - vec.x)*(this->x - vec.x) + (this->y - vec.y)*(this->y - vec.y));
	};


	/** 
	* Scalar product.
	* @param vec a vector which should be considered for the scalar product.
	* @return the scalar product of vec and the current vector.
	*/ 
	virtual float scalarProduct(const DkVector& vec) const {

		return this->x*vec.x + this->y*vec.y;
	};

	virtual float vectorProduct(const DkVector& vec) const {

		return x*vec.y - y*vec.x;
	};

	/**
	* Multiplies thee coordinates.
	* @param vec a vector to multiply this vector with.
	* @return DkVector the multiplied vector
	**/ 
	virtual DkVector mul(const DkVector& vec) const {

		return DkVector(x*vec.x, y*vec.y);
	}

	/** 
	* Slope of a line connecting two vectors. 
	* start point is the actual vector, end point the parameter vector
	* @param vec a vector which should be considered for the slope.
	* @return the slope between the two points.
	*/ 
	float slope(DkVector vec) {
		return (vec.x - this->x) != 0 ? (vec.y - this->y) / (vec.x - this->x) : FLT_MAX;
	}

private:
	bool empty;

};

/**
* Box class DK_CORE_API, defines a non-skewed rectangle e.g. Bounding Box
**/
class DkBox {

public:

	/**
	* Default constructor.
	* All values are initialized with zero.
	**/
	DkBox() : uc(), lc() {};

	/**
	* Constructor.
	* @param uc the upper left corner of the box.
	* @param size the size of the box.
	**/
	DkBox(DkVector uc, DkVector size) {

		this->uc = uc;
		this->lc = uc+size;

		//if (size.width < 0 || size.height < 0)
			//std::cout << "the size is < 0: " << size << std::endl;
	};
	/**
	* Constructor.
	* @param x value of the upper left corner.
	* @param y value of the upper left corner.
	* @param width of the box.
	* @param height of the box.
	**/
	DkBox(float x, float y, float width, float height) {

		DkVector size = DkVector(width, height);

		uc = DkVector(x,y);
		lc = uc+size;

		//if (size.width < 0 || size.height < 0)
			//std::cout << "the size is < 0: " << size << std::endl;

	};
	/**
	* Constructor.
	* @param r box as rect with upperleft corner and width and height.
	**/
	DkBox(cv::Rect r) {

		DkVector size((float)r.width, (float)r.height);

		uc.x = (float)r.x;
		uc.y = (float)r.y;

		lc = uc+size;

		//if (size.width < 0 || size.height < 0)
		//	std::cout << "the size is < 0: " << size << std::endl;

	};

	/**
	* Constructor.
	* @param b box as DkBox.
	**/
	DkBox(const DkBox &b) {

		this->uc = b.uc;
		this->lc = b.uc + b.size();

		//if (size().width < 0 || size().height < 0)
		//	std::cout << "the size is < 0: " << size() << std::endl;
	}
	/**
	* Default destructor.
	**/
	~DkBox() {};

	void getStorageBuffer(char** buffer, size_t& length) const {


		size_t newBufferLength = length + 4*sizeof(float);
		char* newStream = new char[newBufferLength];

		if (*buffer) {

			// copy old stream & clean it
			memcpy(newStream, *buffer, length);
			delete *buffer;
		}

		float* newFStream = (float*)newStream;

		int pos = 0;
		newFStream[pos] = uc.x; pos++;
		newFStream[pos] = uc.y; pos++;
		newFStream[pos] = lc.x; pos++;
		newFStream[pos] = lc.y; pos++;

		*buffer = newStream;
		length = newBufferLength;
	}

	const char* setSorageBuffer(const char* buffer) {

		const float* fBuffer = (const float*)buffer;
		int pos = 0;
		uc.x = fBuffer[pos]; pos++;
		uc.y = fBuffer[pos]; pos++;
		lc.x = fBuffer[pos]; pos++;
		lc.y = fBuffer[pos]; pos++;

		return buffer+sizeof(float)*pos;	// update buffer position
	}

	//friend std::ostream& operator<<(std::ostream& s, DkBox& b) - original
	friend std::ostream& operator<<(std::ostream& s, DkBox b) {

		// this makes the operator<< virtual (stroustrup)
		return s << b.toString().toStdString();
	};

	void moveBy(const DkVector& dxy) {

		uc += dxy;
		lc += dxy;
	};

	bool isEmpty() const {


		return uc.isEmpty() && lc.isEmpty();
	};

	/**
	* Returns the box as opencv Rect.
	* @return a box as opencv Rect.
	**/
	cv::Rect getCvRect() const {

		return cv::Rect(cvRound(uc.x), cvRound(uc.y), cvRound(size().width), cvRound(size().height));
	}

	static DkBox contour2BBox(const std::vector<std::vector<cv::Point> >& pts) {

		if (pts.empty())
			return DkBox();

		// TODO: write this in dk style
		int ux = INT_MAX, uy = INT_MAX;
		int lx = 0, ly = 0;

		for (int cIdx = 0; cIdx < (int)pts.size(); cIdx++) {

			const std::vector<cv::Point>& cont = pts[cIdx];

			for (int idx = 0; idx < (int)cont.size(); idx++) {

				cv::Point p = cont[idx];

				if (p.x < ux)
					ux = p.x;
				if (p.x > lx)
					lx = p.x;
				if (p.y < uy)
					uy = p.y;
				if (p.y > ly)
					ly = p.y;
			}
		}
		DkBox rect((float)ux, (float)uy, (float)lx-ux, (float)ly-uy);

		return rect;
	}

	/**
	* Enlarges the box by the given offset, and the upperleft corner is recalculated.
	* @param offset by which the box is expanded.
	**/
	void expand(float offset) {

		uc -= (offset*0.5f);
	}

	/**
	* Clips the box according the vector s (the box is only clipped but not expanded).
	* @param s the clip vector.
	**/
	void clip(DkVector s) {

		uc.round();
		lc.round();

		uc.clipTo(s);
		lc.clipTo(s);

		//if (lc.x > s.x || lc.y > s.y)
		//	mout << "I did not clip..." << dkendl;
	};

	bool within(const DkVector& p) const {

		return (p.x >= uc.x && p.x < lc.x && 
			p.y >= uc.y && p.y < lc.y);
	};

	DkVector center() const {
		return uc + size() * 0.5f;
	};

	void scaleAboutCenter(float s) {

		DkVector c = center();

		uc = DkVector(uc-c)*s+c;
		lc = DkVector(lc-c)*s+c;
	};

	/**
	* Returns the x value of the upper left corner.
	* @return x value in pixel of the upperleft corner.
	**/
	int getX() const {
		return cvRound(uc.x);
	};
	/**
	* Returns the y value of the upper left corner.
	* @return y value in pixel of the upperleft corner.
	**/
	int getY() const {
		return cvRound(uc.y);
	};
	/**
	* Returns the width of the box.
	* @return the width in pixel of the box.
	**/
	int getWidth() const {
		return cvRound(lc.x-uc.x);
	};
	/**
	* Returns the width of the box.
	* @return float the width in pixel fo the box.
	**/ 
	float getWidthF() const {
		return lc.x-uc.x;
	};
	/**
	* Returns the height of the box.
	* @return the height in pixel of the box.
	**/
	int getHeight() const {
		return cvRound(lc.y-uc.y);
	};
	/**
	* Returns the height of the box as float
	* @return float height in pixel of the box.
	**/ 
	float getHeightF() const {
		return lc.y-uc.y;
	};
	/**
	* Returns the size of the box.
	* @return size of the box as opencv Size.
	**/
	cv::Size getSize() const {
		return cv::Size(getWidth(), getHeight());
	};

	DkVector size() const {

		return lc-uc;
	};

	void setSize(DkVector size) {

		lc = uc+size;
	};

	float area() const {

		DkVector s = size();
		return s.width*s.height;
	};

	float intersectArea(const DkBox& box) const {

		DkVector tmp1 = lc.getMaxVec(box.lc);
		DkVector tmp2 = uc.getMaxVec(box.uc);

		// no intersection?
		if (lc.x < uc.x || lc.y < lc.y)
			return 0;

		tmp1 = tmp2-tmp1;

		return tmp1.width*tmp1.height;
	};

	QString toString() const {

		QString msg =	"\n upper corner: " + uc.toString();
		msg +=				"\n size:         " + size().toString();

		return msg;
	};

	DkVector uc;		/**< upper left corner of the box **/
	DkVector lc;		/**< lower right corner of the box **/
};

/**
* A simple point class DK_CORE_API.
* This class DK_CORE_API is needed for a fast computation
* of the polygon overlap.
**/
class DkIPoint {

public:
	int x;
	int y;

	DkIPoint() : x(0), y(0) {};

	DkIPoint(int x, int y) {
		this->x = x;
		this->y = y;
	};
};


/**
* A simple vertex class DK_CORE_API.
* This class DK_CORE_API is needed for a fast computation
* of the polygon overlap.
**/
class DkVertex {

public:
	DkIPoint ip;
	DkIPoint rx;
	DkIPoint ry;
	int in;

	DkVertex() {};

	DkVertex (DkIPoint ip, DkIPoint rx, DkIPoint ry) {
		this->ip = ip;
		this->rx = rx;
		this->ry = ry;
		in = 0;
	};
};



class DkIntersectPoly {

	// this class DK_CORE_API is based on a method proposed by norman hardy
	// see: http://www.cap-lore.com/MathPhys/IP/aip.c

public:

	DkIntersectPoly();
	DkIntersectPoly(std::vector<DkVector> vecA, std::vector<DkVector> vecB);

	double compute();

private:

	std::vector<DkVector> vecA;
	std::vector<DkVector> vecB;
	int64 interArea;
	DkVector maxRange;
	DkVector minRange;
	DkVector scale;
	float gamut;

	void inness(std::vector<DkVertex> ipA, std::vector<DkVertex> ipB);
	void cross(DkVertex a, DkVertex b, DkVertex c, DkVertex d, double a1, double a2, double a3, double a4);
	void cntrib(int fx, int fy, int tx, int ty, int w);
	int64 area(DkIPoint a, DkIPoint p, DkIPoint q);
	bool ovl(DkIPoint p, DkIPoint q);
	void getVertices(const std::vector<DkVector>& vec, std::vector<DkVertex> *ip, int noise);
	void computeBoundingBox(std::vector<DkVector> vec, DkVector *minRange, DkVector *maxRange);
};

// data class
class DkPolyRect {

public:
	//DkPolyRect(DkVector p1, DkVector p2, DkVector p3, DkVector p4);
	DkPolyRect(const std::vector<cv::Point>& pts = std::vector<cv::Point>());
	DkPolyRect(const std::vector<DkVector>& pts);

	bool empty() const;
	double getMaxCosine() const { return maxCosine; };
	void draw(cv::Mat& img, const cv::Scalar& col = cv::Scalar(0, 100, 255)) const;
	std::vector<cv::Point> toCvPoints() const;
	std::vector<DkVector> getCorners() const;
	DkBox getBBox() const;
	double intersectArea(const DkPolyRect& pr) const;
	double getArea();
	double getAreaConst() const;
	void scale(float s);
	void scaleCenter(float s);
	bool inside(const DkVector& vec) const;
	float maxSide() const;
	DkVector center() const;
	static bool compArea(const DkPolyRect& pl, const DkPolyRect& pr);

protected:
	std::vector<DkVector> pts;
	double maxCosine;
	double area;

	void toDkVectors(const std::vector<cv::Point>& pts, std::vector<DkVector>& dkPts) const;
	void computeMaxCosine();
};

};
