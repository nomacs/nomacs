#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QFile>
#include <QFileInfo>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QElapsedTimer>
#include <QPushButton>
#include <QFileDialog>
#include <QSettings>
#include <QDir>
#include <QSlider>
#include <QStyleOptionSlider>
#include <QToolTip>

#include <opencv2/opencv.hpp>
#include "DkImageStorage.h"
#include "DkBasicLoader.h"

namespace nmc {

	// a slider that dynmically displays the current value in a tooltip.
	// https://stackoverflow.com/questions/18383885/qslider-show-min-max-and-current-value
	class SbIntensitySlider : public QSlider {
		Q_OBJECT
	public:
		SbIntensitySlider(QWidget* parent = 0) : QSlider(parent) {}
		SbIntensitySlider(Qt::Orientation orientation, QWidget* parent = 0) : QSlider(orientation, parent) {}


	protected:
		virtual void sliderChange(SliderChange change)
		{
			QSlider::sliderChange(change);

			if (change == QAbstractSlider::SliderValueChange)
			{
				QStyleOptionSlider opt;
				initStyleOption(&opt);

				QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
				QPoint bottomRightCorner = sr.bottomLeft();

				QToolTip::showText(mapToGlobal(QPoint(bottomRightCorner.x(), bottomRightCorner.y())), QString("%1%").arg(value()), this);
			}
		}
	};



	// manages editing of a single color channel
	class SbChannelWidget : public QWidget {
		Q_OBJECT

	public:
		enum Channel {
			R, G, B
		};

		static const int THUMB_MAX_SIZE = 150;

		SbChannelWidget(Channel c, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
		~SbChannelWidget();
		cv::Mat getImg();													// return the channel content
		void setImg(cv::Mat _img = cv::Mat(), QString _name="");			// "override" from outside. call with empty mat --> reset. also resets intensity slider.

		const Channel c;													// so that this channel knows which one it is

	protected:
		static const int INT_SLIDER_MIN = 0;
		static const int INT_SLIDER_MAX = 200;
		static const int INT_SLIDER_INIT = 100;

		cv::Mat img;								// the channel content
		QPushButton* thumbnail;
		QLabel* filenameLabel;
		SbIntensitySlider* intSlider;

		void loadImage(QString file = "");			// load file with DkBasicLoader, convert to grayscale, set as img, emit imageChanged()
		void buildUI();								// setup and connect UI elements
		void updateThumbnail();						// update channel thumbnail using img

		void dropEvent(QDropEvent* event) override;				// try loading the dropped item as an image
		void dragEnterEvent(QDragEnterEvent* event) override;	// accept everything that has urls

	public slots:
		void onPushButtonInvert();		// triggers updates, actually inverts img
		void onClickThumbnail();		// open file dialog
		void onIntensityChange();		// triggers updates, does NOT allter img

	signals:
		void imageChanged(int channel);	// whenever there is an update (actual image change, inversion, intensity)
		void newAlpha(QImage alpha);	// is emitted upon image loading
	};


};