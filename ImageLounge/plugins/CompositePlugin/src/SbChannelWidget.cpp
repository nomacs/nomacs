#include "SbChannelWidget.h"

#include <cstring>

namespace nmc
{
SbChannelWidget::SbChannelWidget(Channel channel, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , mChannel(channel)
{
    setAcceptDrops(true);
    buildUI();
    setImg();
}

SbChannelWidget::~SbChannelWidget() = default;

cv::Mat SbChannelWidget::getImg()
{
    return mImg * (mIntSlider->value() / 100.0);
}

void SbChannelWidget::setImg(cv::Mat img, QString name)
{
    mImg = img;
    updateThumbnail();
    mFilenameLabel->setText(name);
    mIntSlider->setValue(kSliderInit);
}

void SbChannelWidget::loadImage(QString file)
{
    // load image content for channel
    DkBasicLoader bl;
    bool couldLoad = bl.loadGeneral(file);

    if (couldLoad) {
        QImage qImg = bl.image();
        if (qImg.hasAlphaChannel()) {
            const QImage alpha_tmp = qImg.convertToFormat(QImage::Format_Alpha8);
            QImage aimage(alpha_tmp.size(), QImage::Format_Indexed8);

            aimage.setColorCount(256);
            for (int i = 0; i < 256; ++i)
                aimage.setColor(i, qRgb(i, i, i));

            for (int y = 0; y < aimage.height(); y++) {
                std::memcpy(aimage.scanLine(y), alpha_tmp.constScanLine(y), aimage.width());
            }

            // emit newAlpha(qImg.alphaChannel());		//aparently alphaChannel() is obsolete, however I don't know
            // what else to use emit newAlpha(qImg.convertToFormat(QImage::Format_Alpha8));		//because that doesn't
            // seem to work as expected..
            emit newAlpha(aimage);
        } else {
            emit newAlpha(QImage());
        }

        // this is optional; markus says it makes the grayscale image nicer
        qImg = DkImage::grayscaleImage(qImg);

        mImg = DkImage::qImage2Mat(qImg);
        cv::cvtColor(mImg, mImg, CV_RGB2GRAY);

        updateThumbnail();
        QFileInfo fi(file);
        mFilenameLabel->setText(fi.fileName());

        emit imageChanged(mChannel);
    } else {
        qDebug() << "could not load:" << file;
    }
}

void SbChannelWidget::buildUI()
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    mThumbnail = new QPushButton();
    mThumbnail->setFlat(true);
    mThumbnail->setIconSize(QSize(kThumbMaxSize, kThumbMaxSize));
    connect(mThumbnail, SIGNAL(released()), this, SLOT(onClickThumbnail()));
    mFilenameLabel = new QLabel();

    auto *controlsLayout = new QHBoxLayout();
    auto *invertButton = new QPushButton("invert");
    connect(invertButton, SIGNAL(released()), this, SLOT(onPushButtonInvert()));
    mIntSlider = new SbIntensitySlider(Qt::Orientation::Horizontal);
    mIntSlider->setMinimum(kSliderMin);
    mIntSlider->setMaximum(kSliderMax);
    mIntSlider->setSingleStep(1);
    mIntSlider->setValue(kSliderInit);
    mIntSlider->setTickInterval(50);
    mIntSlider->setTickPosition(QSlider::TickPosition::TicksBelow);
    mIntSlider->setToolTip("adjust intensity");
    connect(mIntSlider, SIGNAL(sliderReleased()), this, SLOT(onIntensityChange()));
    controlsLayout->addWidget(invertButton);
    controlsLayout->addWidget(mIntSlider);

    outerLayout->addWidget(mThumbnail);
    outerLayout->addWidget(mFilenameLabel);
    outerLayout->addLayout(controlsLayout);

    this->setMaximumWidth(kThumbMaxSize + 50);
}
void SbChannelWidget::updateThumbnail()
{
    cv::Mat imgScaled;
    if (mImg.empty()) {
        // set image to solid color
        imgScaled = cv::Mat::ones(cv::Size(kThumbMaxSize, kThumbMaxSize), CV_8UC1) * 255;
    } else {
        int s = std::max(mImg.rows, mImg.cols);
        double f = (double)kThumbMaxSize / (double)s;
        cv::resize(mImg, imgScaled, cv::Size(), f, f);
    }

    cv::Mat black = cv::Mat::zeros(cv::Size(imgScaled.cols, imgScaled.rows), imgScaled.type());
    cv::Mat channels[3] = {black, black, black};
    channels[mChannel] = imgScaled; // *(intSlider->value() / 100.0);
    if (!mImg.empty())
        channels[mChannel] *= (mIntSlider->value() / 100.0);

    cv::Mat imgTinted;
    cv::merge(channels, 3, imgTinted);

    QImage qimg = DkImage::mat2QImage(imgTinted);
    QPixmap pxm = QPixmap::fromImage(qimg);
    mThumbnail->setIcon(pxm);
}
void SbChannelWidget::dropEvent(QDropEvent *event)
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

void SbChannelWidget::dragEnterEvent(QDragEnterEvent *event)
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
    if (!mImg.empty()) {
        updateThumbnail();
        emit(imageChanged(mChannel));
    }
}

void SbChannelWidget::onPushButtonInvert()
{
    if (!mImg.empty()) {
        mImg = 255 - mImg;
        updateThumbnail();
        emit imageChanged(mChannel);
    }
}
};
