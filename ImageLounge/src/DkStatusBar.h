/*******************************************************************************************************
 DkStatusBar.h
 Created on:	12.01.2016
 
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

 *******************************************************************************************************/

#pragma once

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QStatusBar>
#pragma warning(pop)

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport
#endif
#endif

// Qt defines
class QLabel;

namespace nmc {


class DkStatusBar : public QStatusBar {
	Q_OBJECT

public:
	DkStatusBar(QWidget* parent = 0);
	~DkStatusBar() {};

	enum StatusLabel {
		status_pixel_info,	// the first is special (left)

		status_zoom_info,
		status_filenumber_info,
		status_filesize_info,
		status_time_info,

		status_end,
	};

	void setMessage(const QString& msg, StatusLabel which = status_pixel_info);

protected:

	void createLayout();

	QVector<QLabel*> mLabels;
};

class DkStatusBarManager {

public:
	static DkStatusBarManager& instance();

	DkStatusBar* statusbar();
	void setMessage(const QString& msg, DkStatusBar::StatusLabel which = DkStatusBar::status_pixel_info);

private:
	DkStatusBarManager();

	DkStatusBar* mStatusBar = 0;
};

};
