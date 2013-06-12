/*******************************************************************************************************
 DkTransferToolBar.h
 Created on:	13.02.2012
 
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

#pragma once

#include <QtGui/QToolBar>
#include <QtGui/QWidget>
#include <QObject>
#include <QtGui/QPainter>
#include <QtGui/QLinearGradient>
#include <QtGui/QImage>
#include <QtGui/QPainterPath>
#include <QDebug>
#include <QtGui/QMouseEvent>
#include <QtGui/QColorDialog>
#include <QtGui/QColor>
#include <QtGui/QGradientStops>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLayout>
#include <QtGui/QIcon>
#include <QtGui/QAction>

#include <QtGui/QGridLayout>
#include <QtGui/QGraphicsOpacityEffect>

namespace nmc {

class DkTransferToolBar;

class DkColorSlider : public QWidget {
	Q_OBJECT
	
	
	public:
		//DkColorSlider(QWidget *parent);
		DkColorSlider(QWidget *parent, QColor color);
		DkColorSlider(QWidget *parent, qreal normedPos, QColor color, int sliderWidth);
		~DkColorSlider();
		QColor getColor();
		qreal getNormedPos();
		void setNormedPos(qreal pos);
		void setActive(bool isActive);
		void updatePos(int parentWidth);

		//void paintSlider(QPainter *painter);

	signals:
		void sliderMoved(DkColorSlider *sender, int dragDistX, int yPos);
		void sliderActivated(DkColorSlider *sender);
		void colorChanged(DkColorSlider *slider);
				
	public slots:
		virtual void paintEvent(QPaintEvent* event);

	protected:
		virtual void mousePressEvent(QMouseEvent *event);
		virtual void mouseMoveEvent(QMouseEvent *event);
		virtual void mouseDoubleClickEvent(QMouseEvent *event);

	private:
		int sliderWidth, sliderHeight, sliderHalfWidth;
		bool isActive;
		int dragStartX;
		QColor color;
		qreal normedPos;
				
};

class DkGradient : public QWidget {
	Q_OBJECT

	public:
		DkGradient(QWidget *parent);
		~DkGradient();
		QGradientStops getGradientStops();
		void insertSlider(qreal pos);
		void reset();

	signals:
		void gradientChanged();
		
	public slots:
		virtual void paintEvent(QPaintEvent* event);
		//virtual void setEnabled(bool enabled);
		void moveSlider(DkColorSlider* sender, int dragDistX, int yPos);
		void changeColor(DkColorSlider *slider);
		void activateSlider(DkColorSlider *sender);

	protected:
		virtual void mousePressEvent(QMouseEvent *event);
		virtual void mouseReleaseEvent(QMouseEvent *event);
		virtual void resizeEvent ( QResizeEvent * event );
		
	private:
		void init();
		void addSlider(qreal pos, QColor color);
		void updateGradient();
		qreal getNormedPos(int pos);
		int getAbsolutePos(qreal pos);

		int clickAreaHeight;
		int deleteSliderDist;
		QVector<DkColorSlider*> sliders;
		bool isSliderDragged;
		QLinearGradient gradient;
		int sliderWidth, halfSliderWidth;

		DkColorSlider *activeSlider;
		bool isActiveSliderExisting;
};

enum toolBarIcons {
	icon_toolbar_reset,
	icon_toolbar_pipette,
	icon_toolbar_end,	// nothing beyond this point
};

enum toolBarActions {
	toolbar_reset,
	toolbar_pipette,
	toolbar_end,	// nothing beyond this point
};

enum imageModes {
	mode_uninitialized,
	mode_invalid_format,
	mode_gray,
	mode_rgb,
};

class DkTransferToolBar : public QToolBar {

	Q_OBJECT

	public:
		DkTransferToolBar(QWidget *parent);
		~DkTransferToolBar();
		

	signals:
		void pickColorRequest();
		void colorTableChanged(QGradientStops stops);
		void channelChanged(int channel);
		void transferFunctionChanged(int channel, QGradientStops stops);
		void tFEnabled(bool);
		

	public slots:
		virtual void paintEvent(QPaintEvent* event);
		void insertSlider(qreal pos);
		void setImageMode(int mode);
		

	private slots:
		void applyTF();
		void pickColor();
		void changeChannel(int index);
		void enableTFCheckBoxClicked(int state);
		void reset();

	protected:
		virtual void resizeEvent ( QResizeEvent * event );


	private:
		void createIcons();
		void applyImageMode(int mode);
		void enableToolBar(bool enable);
		QCheckBox *enableTFCheckBox;
		
		QImage m_shade;
		QImage sliderImg, activeSliderImg;

		QVector<QAction *> toolBarActions;
		QVector<QIcon> toolBarIcons;
		
		DkGradient *gradient;
		QComboBox *channelComboBox;

		QGraphicsOpacityEffect *effect;
		int imageMode;

};
};
