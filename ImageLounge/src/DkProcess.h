/*******************************************************************************************************
 DkProcess.h
 Created on:	27.12.2014
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2014 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2014 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2014 Florian Kleber <florian@nomacs.org>

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

#pragma once;

#include "DkImageContainer.h"

namespace nmc {

class DkAbstractBatch {

public:
	DkAbstractBatch() {};

	virtual void setProperties(...) {};
	virtual bool compute(QSharedPointer<DkImageContainer> container, QStringList& logStrings) const;
	virtual bool compute(QImage& img, QStringList& logStrings) const {return true;};
	virtual QString name() const {return "Abstract Batch";};
};

class DkResizeBatch : public DkAbstractBatch {

public:
	DkResizeBatch();

	virtual void setProperties(float scaleFactor, int iplMethod = DkImage::ipl_area, bool correctGamma = false);
	virtual bool compute(QImage& img, QStringList& logStrings) const;
	virtual QString name() const;

protected:
	float scaleFactor;
	int iplMethod;
	bool correctGamma;

};

class DkBatchProcess {

public:

	enum {
		mode_overwrite,
		mode_skip_existing,

		mode_end
	};

	DkBatchProcess(const QFileInfo& fileInfoIn = QFileInfo(), const QFileInfo& fileInfoOut = QFileInfo());

	void setProcessChain(const QVector<DkAbstractBatch*> processes);
	void setMode(int mode);
	void compute();	// do the work
	QStringList getLog() const;

protected:
	QFileInfo fileInfoIn;
	QFileInfo fileInfoOut;
	int mode;
	int compression;

	QVector<DkAbstractBatch*> processFunctions;
	QStringList logStrings;

	bool process();
	bool deleteExisting();
	bool copyFile();
	bool renameFile();
};

}