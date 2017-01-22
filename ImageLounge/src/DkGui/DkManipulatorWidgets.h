/*******************************************************************************************************
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2016 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2016 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2016 Florian Kleber <florian@nomacs.org>

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

 related links:
 [1] http://www.nomacs.org/
 [2] https://github.com/nomacs/
 [3] http://download.nomacs.org
 *******************************************************************************************************/

#pragma once

#include "DkBaseWidgets.h"
#include "DkImageContainer.h"
#include "DkManipulatorsIpl.h"

#pragma warning(push, 0)	// no warnings from includes

#pragma warning(pop)

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport Q_DECL_IMPORT
#endif
#endif

// Qt defines

namespace nmc {

// nomacs defines

	class DkBaseManipulatorWidget : public DkWidget {
	Q_OBJECT

public:
	DkBaseManipulatorWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget* parent = 0);

	QSharedPointer<DkBaseManipulatorExt> baseManipulator() const;

private:
	QSharedPointer<DkBaseManipulatorExt> mBaseManipulator;
};

class DkTinyPlanetWidget : public DkBaseManipulatorWidget {
	Q_OBJECT

public:
	DkTinyPlanetWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget* parent = 0);

	QSharedPointer<DkTinyPlanetManipulator> manipulator() const;

public slots:
	void on_scaleSlider_valueChanged(int val);
	void on_angleSlider_valueChanged(int val);
	void on_invertBox_toggled(bool val);

private:
	void createLayout();
};

class DkUnsharpMaskWidget : public DkBaseManipulatorWidget {
	Q_OBJECT

public:
	DkUnsharpMaskWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget* parent = 0);

	QSharedPointer<DkUnsharpMaskManipulator> manipulator() const;

public slots:
	void on_sigmaSlider_valueChanged(int val);
	void on_amountSlider_valueChanged(int val);

private:
	void createLayout();

};

class DkRotateWidget : public DkBaseManipulatorWidget {
	Q_OBJECT

public:
	DkRotateWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget* parent = 0);

	QSharedPointer<DkRotateManipulator> manipulator() const;

public slots:
	void on_angleSlider_valueChanged(int val);

private:
	void createLayout();

};

class DkHueWidget : public DkBaseManipulatorWidget {
	Q_OBJECT

public:
	DkHueWidget(QSharedPointer<DkBaseManipulatorExt> manipulator, QWidget* parent = 0);

	QSharedPointer<DkHueManipulator> manipulator() const;

public slots:
	void on_hueSlider_valueChanged(int val);
	void on_satSlider_valueChanged(int val);
	void on_brightnessSlider_valueChanged(int val);

private:
	void createLayout();
};

// dock --------------------------------------------------------------------
class DkManipulatorWidget : public DkWidget {
	Q_OBJECT

public:
	DkManipulatorWidget(QWidget* parent = 0);

	void setImage(QSharedPointer<DkImageContainerT> imgC);

public slots:
	void selectManipulator();

private:
	void createLayout();
	QImage scaledPreview(const QImage& img) const;

	QVector<DkBaseManipulatorWidget*> mWidgets;

	QSharedPointer<DkImageContainerT> mImgC;
	QLabel* mPreview = 0;
	int mMaxPreview = 300;
};

class DkEditDock : public DkDockWidget {
	Q_OBJECT

public:
	DkEditDock(const QString& title, QWidget* parent = 0, Qt::WindowFlags flags = 0);

public slots:
	void setImage(QSharedPointer<DkImageContainerT> imgC);

protected:
	void createLayout();

	//void writeSettings();
	//void readSettings();

	DkManipulatorWidget* mMplWidget = 0;

};


}