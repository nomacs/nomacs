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

#pragma warning(push, 0)	// no warnings from includes
#include <QWidget>
#pragma warning(pop)

#ifndef DllLoaderExport
#ifdef DK_LOADER_DLL_EXPORT
#define DllLoaderExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllLoaderExport Q_DECL_IMPORT
#else
#define DllLoaderExport
#endif
#endif

// Qt defines
class QSlider;
class QLabel;
class QSpinBox;
class QColorDialog;
class QPushButton;

namespace nmc {

// nomacs defines

class DllLoaderExport DkSlider : public QWidget {
	Q_OBJECT

public:
	DkSlider(QString title = "", QWidget* parent = 0);

	QSlider* getSlider() const;
	void setMinimum(int minValue);
	void setMaximum(int maxValue);
	void setTickInterval(int ticValue);
	int value() const;
	void setFocus(Qt::FocusReason reason);

	public slots:
	void setValue(int value);

signals:
	void sliderMoved(int value);
	void valueChanged(int value);

protected:
	void createLayout();

	QLabel* titleLabel;
	QLabel* minValLabel;
	QLabel* maxValLabel;
	QSlider* slider;
	QSpinBox* sliderBox;
};

class DllLoaderExport DkColorChooser : public QWidget {
	Q_OBJECT

public:
	DkColorChooser(QColor defaultColor = QColor(), QString text = "Color", QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~DkColorChooser() {};

	void setColor(const QColor& color);
	void setColor(QColor* color);
	QColor getColor();
	bool isAccept() const;
	void enableAlpha(bool enable = true);

	public slots:
	void on_resetButton_clicked();
	void on_colorButton_clicked();
	void on_colorDialog_accepted();

signals:
	void resetClicked();
	void accepted();

protected:
	QColorDialog* colorDialog = 0;
	QPushButton* colorButton = 0;

	QColor defaultColor;
	QColor* mSettingColor = 0;
	QString mText;
	bool mAccepted = false;

	void init();

};

}