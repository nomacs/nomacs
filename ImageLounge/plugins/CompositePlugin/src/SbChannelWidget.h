#pragma once

#include <QColorSpace>
#include <QImage>
#include <QSlider>
#include <QStyleOptionSlider>
#include <QToolTip>
#include <QVBoxLayout>
#include <QWidget>

#include "DkImageStorage.h"

class QLabel;
class QPushButton;

namespace nmc
{

// a slider that dynmically displays the current value in a tooltip.
// https://stackoverflow.com/questions/18383885/qslider-show-min-max-and-current-value
class SbIntensitySlider : public QSlider
{
    Q_OBJECT
public:
    explicit SbIntensitySlider(QWidget *parent = nullptr)
        : QSlider(parent)
    {
    }
    explicit SbIntensitySlider(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QSlider(orientation, parent)
    {
    }

protected:
    void sliderChange(SliderChange change) override
    {
        QSlider::sliderChange(change);

        if (change == QAbstractSlider::SliderValueChange) {
            QStyleOptionSlider opt;
            initStyleOption(&opt);

            QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
            QPoint bottomRightCorner = sr.bottomLeft();

            QToolTip::showText(mapToGlobal(QPoint(bottomRightCorner.x(), bottomRightCorner.y())),
                               QString("%1%").arg(value()),
                               this);
        }
    }
};

// manages editing of a single color channel
class SbChannelWidget : public QWidget
{
    Q_OBJECT

public:
    enum Channel {
        R,
        G,
        B
    };

    static constexpr int kThumbMaxSize = 150;

    explicit SbChannelWidget(Channel channel, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~SbChannelWidget() override;
    cv::Mat getImg(); // return the channel content
    void setImg(
        cv::Mat img = cv::Mat(),
        QString name = ""); // "override" from outside. call with empty mat --> reset. also resets intensity slider.

    const Channel mChannel; // so that this channel knows which one it is
    QImage srcFormat() const
    {
        return mSrcFormat;
    } // for conversion back to QImage
protected:
    static constexpr int kSliderMin = 0;
    static constexpr int kSliderMax = 200;
    static constexpr int kSliderInit = 100;

    QImage mSrcFormat;
    cv::Mat mImg; // the channel content
    QPushButton *mThumbnail;
    QLabel *mFilenameLabel;
    SbIntensitySlider *mIntSlider;

    void loadImage(
        QString file = ""); // load file with DkBasicLoader, convert to grayscale, set as img, emit imageChanged()
    void buildUI(); // setup and connect UI elements
    void updateThumbnail(); // update channel thumbnail using img

    void dropEvent(QDropEvent *event) override; // try loading the dropped item as an image
    void dragEnterEvent(QDragEnterEvent *event) override; // accept everything that has urls

public slots:
    void onPushButtonInvert(); // triggers updates, actually inverts img
    void onClickThumbnail(); // open file dialog
    void onIntensityChange(); // triggers updates, does NOT allter img

signals:
    void imageChanged(int channel); // whenever there is an update (actual image change, inversion, intensity)
    void newAlpha(QImage alpha); // is emitted upon image loading
};

};
