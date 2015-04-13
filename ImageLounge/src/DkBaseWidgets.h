/*******************************************************************************************************
 DkBaseWidgets.h
 Created on:	11.02.2015
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2015 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2015 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2015 Florian Kleber <florian@nomacs.org>

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
#include <QWidget>
#include <QLabel>
#include <QDockWidget>
#include <QScrollArea>
#pragma warning(pop)	// no warnings from includes - end

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
class QBitArray;
class QGraphicsOpacityEffect;

namespace nmc {

class DllExport DkWidget : public QWidget {
	Q_OBJECT

public:
	DkWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	void registerAction(QAction* action);
	void block(bool blocked);
	void setDisplaySettings(QBitArray* displayBits);
	bool getCurrentDisplaySetting();
	bool isHiding() const;

	enum {
		pos_west,
		pos_north,
		pos_east,
		pos_south,
		pos_dock_hor,
		pos_dock_ver,

		pos_end,
	};

signals:
	void visibleSignal(bool visible);

public slots:
	virtual void show(bool saveSetting = true);
	virtual void hide(bool saveSetting = true);
	virtual void setVisible(bool visible, bool saveSetting = true);

	void animateOpacityUp();
	void animateOpacityDown();

protected:

	QColor bgCol;
	bool blocked;
	bool hiding;
	bool showing;

	QGraphicsOpacityEffect* opacityEffect;
	QBitArray* displaySettingsBits;

	// functions
	void init();
	void paintEvent(QPaintEvent *event);
};

class DllExport DkLabel : public QLabel {
	Q_OBJECT

public:
	DkLabel(QWidget* parent = 0, const QString& text = QString());
	virtual ~DkLabel();

	virtual void showTimed(int time = 3000);
	virtual void setText(const QString& msg, int time = 3000);
	QString getText();
	void setFontSize(int fontSize);
	void stop();
	void setFixedWidth(int fixedWidth);

	void block(bool blocked) {
		this->blocked = blocked;
	};
	
	void setMargin(const QPoint& margin) {
		this->margin = margin;
		updateStyleSheet();
	};

public slots:
	virtual void hide();

protected:
	QWidget* parent;
	QTimer* timer;
	QString text;
	QColor textCol;
	int defaultTime;
	int fontSize;
	int time;
	int fixedWidth;
	QPoint margin;
	bool blocked;
	QColor bgCol;

	virtual void init();
	virtual void paintEvent(QPaintEvent *event);
	virtual void draw(QPainter* painter);

	// for my children...
	virtual void drawBackground(QPainter*) {};
	virtual void setTextToLabel();
	virtual void updateStyleSheet();
};

class DkLabelBg : public DkLabel {
	Q_OBJECT

public:
	DkLabelBg(QWidget* parent = 0, const QString& text = QString());
	virtual ~DkLabelBg() {};

protected:
	virtual void updateStyleSheet();
};

/**
 * This label fakes the DkWidget behavior.
 * (allows for registering actions + fades in and out)
 * we need this class too, since we cannot derive from DkLabel & DkWidget
 * at the same time -> both have QObject as common base class.
 **/
class DkFadeLabel : public DkLabel {
	Q_OBJECT

public:
	DkFadeLabel(QWidget* parent = 0, const QString& text = QString());

	void block(bool blocked);
	void registerAction(QAction* action);
	void setDisplaySettings(QBitArray* displayBits);
	bool getCurrentDisplaySetting();

signals:
	void visibleSignal(bool visible);

public slots:
	virtual void show(bool saveSetting = true);
	virtual void hide(bool saveSetting = true);
	virtual void setVisible(bool visible, bool saveSetting = true);

protected slots:
	void animateOpacityUp();
	void animateOpacityDown();

protected:

	bool hiding;
	bool showing;
	QBitArray* displaySettingsBits;

	QGraphicsOpacityEffect *opacityEffect;

	// functions
	void init();

};

class DkDockWidget : public QDockWidget {
	Q_OBJECT

public:
	DkDockWidget(const QString& title, QWidget* parent = 0, Qt::WindowFlags flags = 0 );
	~DkDockWidget();

	void registerAction(QAction* action);
	void setDisplaySettings(QBitArray* displayBits);
	bool getCurrentDisplaySetting() const;
	static bool testDisplaySettings(const QBitArray& displaySettingsBits);
	Qt::DockWidgetArea getDockLocationSettings(const Qt::DockWidgetArea& defaultArea) const;

public slots:
	virtual void setVisible(bool visible, bool saveSetting = true);

signals:
	void visibleSignal(bool visible);

protected:
	virtual void closeEvent(QCloseEvent* event);

	QBitArray* displaySettingsBits;

};

class DkResizableScrollArea : public QScrollArea {
	Q_OBJECT

public:
	DkResizableScrollArea(QWidget * parent = 0);

	void updateSize();

	virtual QSize sizeHint() const;
	virtual QSize minimumSizeHint() const;

protected:
	bool eventFilter(QObject * o, QEvent * e);

};

}
