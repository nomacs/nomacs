#include "SbChannelWidget.h"

#include "DkTimer.h"

namespace nmc {
	SbChannelWidget::SbChannelWidget(Channel c, QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f), c(c)
	{
		setAcceptDrops(true); 
		buildUI();
		setImg();
	}

	SbChannelWidget::~SbChannelWidget()
	{
	}

	cv::Mat SbChannelWidget::getImg()
	{
		return img * (intSlider->value()/100.0);
	}

	void SbChannelWidget::setImg(cv::Mat _img, QString _name)
	{
		img = _img;
		updateThumbnail();
		filenameLabel->setText(_name);
		intSlider->setValue(INT_SLIDER_INIT);
	}


	void SbChannelWidget::loadImage(QString file) {

		//load image content for channel
		DkBasicLoader bl;
		bool couldLoad = bl.loadGeneral(file);

		if (couldLoad) {
			QImage qImg = bl.image();
			if (qImg.hasAlphaChannel()) {
				emit newAlpha(qImg.alphaChannel());		//aparently alphaChannel() is obsolete, however I don't know what else to use
				//emit newAlpha(qImg.convertToFormat(QImage::Format_Alpha8));		//because that doesn't seem to work as expected..
			}
			else {
				emit newAlpha(QImage());
			}
			
			//this is optional; markus says it makes the grayscale image nicer
			qImg = DkImage::grayscaleImage(qImg);

			img = DkImage::qImage2Mat(qImg);
			cv::cvtColor(img, img, CV_RGB2GRAY);

			updateThumbnail();
			QFileInfo fi(file);
			filenameLabel->setText(fi.fileName());
			
			emit imageChanged(c);
		}
		else {
			qDebug() << "could not load:" << file;
		}

	}

	void SbChannelWidget::buildUI()
	{
		QVBoxLayout* outerLayout = new QVBoxLayout(this);
		outerLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

		thumbnail = new QPushButton();
		thumbnail->setFlat(true);
		thumbnail->setIconSize(QSize(THUMB_MAX_SIZE, THUMB_MAX_SIZE));
		connect(thumbnail, SIGNAL(released()), this, SLOT(onClickThumbnail()));
		filenameLabel = new QLabel();
		
		QHBoxLayout* controlsLayout = new QHBoxLayout();
		QPushButton* invertButton = new QPushButton("invert");
		connect(invertButton, SIGNAL(released()), this, SLOT(onPushButtonInvert()));
		intSlider = new SbIntensitySlider(Qt::Orientation::Horizontal);
		intSlider->setMinimum(INT_SLIDER_MIN);
		intSlider->setMaximum(INT_SLIDER_MAX);
		intSlider->setSingleStep(1);
		intSlider->setValue(INT_SLIDER_INIT);
		intSlider->setTickInterval(50);
		intSlider->setTickPosition(QSlider::TickPosition::TicksBelow);
		intSlider->setToolTip("adjust intensity");
		connect(intSlider, SIGNAL(sliderReleased()), this, SLOT(onIntensityChange()));
		controlsLayout->addWidget(invertButton);
		controlsLayout->addWidget(intSlider);

		outerLayout->addWidget(thumbnail);
		outerLayout->addWidget(filenameLabel);
		outerLayout->addLayout(controlsLayout);

		this->setMaximumWidth(THUMB_MAX_SIZE+50);
	}
	void SbChannelWidget::updateThumbnail()
	{
		cv::Mat imgScaled;
		if (img.empty()) {
			//set image to solid color
			imgScaled = cv::Mat::ones(cv::Size(THUMB_MAX_SIZE, THUMB_MAX_SIZE), CV_8UC1) * 255;
		}	
		else {
			int s = std::max(img.rows, img.cols);
			double f = (double)THUMB_MAX_SIZE / (double)s;
			cv::resize(img, imgScaled, cv::Size(), f, f);
		}
		
		cv::Mat black = cv::Mat::zeros(cv::Size(imgScaled.cols, imgScaled.rows), imgScaled.type());
		cv::Mat channels[3] = { black, black, black };
		channels[c] = imgScaled;// *(intSlider->value() / 100.0);
		if(!img.empty())
			channels[c] *= (intSlider->value() / 100.0);

		cv::Mat imgTinted;
		cv::merge(channels, 3, imgTinted);

		QImage qimg = DkImage::mat2QImage(imgTinted);
		QPixmap pxm = QPixmap::fromImage(qimg);
		thumbnail->setIcon(pxm);
	}
	void SbChannelWidget::dropEvent(QDropEvent* event)
	{
		qDebug() << "dropped something on the channel widget!";
		QList<QUrl> urls = event->mimeData()->urls();
		if (urls.length() < 1)
			return;
		if (urls.length() > 1)
			qDebug() << "you droped more than 1 files. I am taking the first one I find..";
		QUrl url = urls[0];
		QString s = url.toLocalFile();
		loadImage(s);
	}

	void SbChannelWidget::dragEnterEvent(QDragEnterEvent* event)
	{
		if (event->mimeData()->hasUrls()) {
			event->acceptProposedAction();
		}
	}

	void SbChannelWidget::onClickThumbnail()
	{
		const QString FILE_LOC_KEY = "comp_last_file_loc";
		QSettings settings; 


		QString fileName = QFileDialog::getOpenFileName(this, "Open Image File", settings.value(FILE_LOC_KEY).toString());
		if (!fileName.isEmpty()) {
			QFileInfo finf(fileName);
			settings.setValue(FILE_LOC_KEY, QDir().absoluteFilePath(fileName));
			loadImage(fileName);
		}
	}

	void SbChannelWidget::onIntensityChange()
	{
		qDebug() << "intensity changed";
		if (!img.empty()) {
			updateThumbnail();
			emit(imageChanged(c));
		}
	}

	void SbChannelWidget::onPushButtonInvert() {
		if (!img.empty()) {
			img = 255 - img;
			updateThumbnail();
			emit imageChanged(c);
		}
	}
};