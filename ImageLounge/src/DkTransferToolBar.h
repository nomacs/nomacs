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
		void setImageMode(bool isGrayScale);
		

	private slots:
		void applyTF();
		void pickColor();
		void changeChannel(int index);
		void enableTF(int state);
		void reset();

	protected:
		//void mousePressEvent(QMouseEvent *event);

	private:
		void createIcons();
		void applyImageMode(int mode);
		QPushButton *applyTFButton, *pickColorButton;
		QCheckBox *enableTFCheckBox;
		int gradX, gradY, gradWidth, gradHeight;
		
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