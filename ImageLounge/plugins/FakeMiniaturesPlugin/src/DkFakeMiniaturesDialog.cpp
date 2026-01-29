/*******************************************************************************************************
 DkFakeMiniaturesDialog.cpp
 Created on:	04.06.2013

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

#include "DkFakeMiniaturesDialog.h"
#include "DkImageStorage.h"

#include <QColorSpace>
#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>

#define INIT_X 0
#define INIT_Y 0.7117
#define INIT_WIDTH 1
#define INIT_HEIGHT 0.1941

namespace nmp
{

#ifdef WITH_OPENCV
// I know - but I just want to get rid of these little issues...
#if (CV_MAJOR_VERSION > 3)
constexpr int DIST_C = cv::DIST_C;
#else
constexpr int DIST_C = CV_DIST_C;
#endif
#endif

/**************************************************************
 * DkFakeMiniaturesDialog: Dialog for creating fake miniatures
 ***************************************************************/
DkFakeMiniaturesDialog::DkFakeMiniaturesDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    init();
}

DkFakeMiniaturesDialog::~DkFakeMiniaturesDialog() = default;

/**
 * initializes variables
 **/
void DkFakeMiniaturesDialog::init()
{
    isOk = false;
    dialogWidth = 700;
    dialogHeight = 510;
    toolsWidth = 200;
    previewMargin = 20;
    previewWidth = dialogWidth - toolsWidth - 2 * previewMargin;
    previewHeight = dialogHeight - previewMargin * 2;

    setWindowTitle(tr("Fake Miniatures"));
    setFixedSize(dialogWidth, dialogHeight);
    createLayout();
}

/**
 * creates dialog layout and adds items to it
 **/
void DkFakeMiniaturesDialog::createLayout()
{
    // central widget - preview image
    auto *centralWidget = new QWidget(this);
    // previewLabel = new QLabel(centralWidget);
    previewLabel = new DkPreviewLabel(this, centralWidget);
    previewLabel->setGeometry(QRect(QPoint(previewMargin, previewMargin), QSize(previewWidth, previewHeight)));

    // east widget - sliders
    auto *eastWidget = new QWidget(this);
    eastWidget->setMinimumWidth(toolsWidth);
    eastWidget->setMaximumWidth(toolsWidth);
    eastWidget->setFixedHeight(previewHeight);
    eastWidget->setContentsMargins(0, 10, 10, 0);
    auto *toolsLayout = new QVBoxLayout(eastWidget);
    toolsLayout->setContentsMargins(0, 0, 0, 0);

    kernelSizeWidget = new DkKernelSize(eastWidget, this);
    saturationWidget = new DkSaturation(eastWidget, this);

    toolsLayout->addWidget(kernelSizeWidget);
    toolsLayout->addWidget(saturationWidget);

    auto *spacer = new QSpacerItem(20, 280, QSizePolicy::Minimum, QSizePolicy::Minimum);
    toolsLayout->addItem(spacer);

    // bottom widget - buttons
    // QWidget* bottomWidget = new QWidget(eastWidget);
    auto *bottomWidgetHBoxLayout = new QHBoxLayout();

    auto *buttonOk = new QPushButton(tr("&Ok"));
    connect(buttonOk, SIGNAL(clicked()), this, SLOT(okPressed()));
    auto *buttonCancel = new QPushButton(tr("&Cancel"));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(cancelPressed()));

    bottomWidgetHBoxLayout->addWidget(buttonOk);
    bottomWidgetHBoxLayout->addWidget(buttonCancel);

    toolsLayout->addLayout(bottomWidgetHBoxLayout);

    eastWidget->setLayout(toolsLayout);

    auto *dummy = new QWidget(this);
    auto *cLayout = new QHBoxLayout(dummy);
    cLayout->setContentsMargins(0, 0, 0, 0);
    // cLayout->setSpacing(0);

    cLayout->addWidget(centralWidget);
    cLayout->addWidget(eastWidget);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(dummy);
    // layout->addWidget(buttons);

    this->setSizeGripEnabled(false);
}

/**
 * scales input image for the display in the preview label
 **/
void DkFakeMiniaturesDialog::createImgPreview()
{
    if (!mImg || mImg->isNull())
        return;

    QPoint lt;
    float rW = previewWidth / (float)mImg->width();
    float rH = previewHeight / (float)mImg->height();
    rMin = (rW < rH) ? rW : rH;

    if (rMin < 1) {
        if (rW < rH)
            lt = QPoint(0, qRound((float)mImg->height() * (rH - rMin) / 2.0f));
        else {
            lt = QPoint(qRound((float)mImg->width() * (rW - rMin) / 2.0f), 0);
        }
    } else
        lt = QPoint(qRound((previewWidth - mImg->width()) / 2.0f), qRound((previewHeight - mImg->height()) / 2.0f));

    auto imgSizeScaled = QSize(mImg->size());
    if (rMin < 1)
        imgSizeScaled *= rMin;

    previewImgRect = QRect(lt, imgSizeScaled);

    previewImgRect.setTop(previewImgRect.top() + 1);
    previewImgRect.setLeft(previewImgRect.left() + 1);
    previewImgRect.setWidth(previewImgRect.width() - 1); // we have a border... correct that...
    previewImgRect.setHeight(previewImgRect.height() - 1);

    if (rMin < 1)
        scaledImg = mImg->scaled(imgSizeScaled, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    else
        scaledImg = *mImg;

    imgPreview = applyMiniaturesFilter(scaledImg,
                                       QRect(INIT_X,
                                             qRound(INIT_Y * scaledImg.height()),
                                             INIT_WIDTH * scaledImg.width(),
                                             qRound(INIT_HEIGHT * scaledImg.height())));

    previewLabel->setImgRect(previewImgRect);
}

/**
 * draws preview image onto preview label
 **/
void DkFakeMiniaturesDialog::drawImgPreview()
{
    QImage preview = QImage(previewWidth, previewHeight, imgPreview.format());
    preview.setColorSpace(imgPreview.colorSpace());
    preview.fill(Qt::transparent);
    QPainter painter(&preview);
    painter.setPen(QColor(0, 0, 0));
    painter.drawRect(0, 0, previewWidth - 1, previewHeight - 1);
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.drawImage(previewImgRect, imgPreview);
    painter.end();
    preview = nmc::DkImage::convertToColorSpaceInPlace(this, preview);
    previewLabel->setPixmap(QPixmap::fromImage(preview));
}

/**
 * applies fake miniature filter to an image
 * @param inImg image where the filter is applied
 * @param roi the rectangle that will not be blurred
 * @return image with filter applied
 **/
QImage DkFakeMiniaturesDialog::applyMiniaturesFilter(QImage inImg, QRect qRoi)
{
#ifdef WITH_OPENCV

    int kernelSize = kernelSizeWidget->getToolValue();
    if (inImg == scaledImg) {
        double diagO = sqrt(mImg->width() * mImg->width() + mImg->height() * mImg->height());
        double diagP = sqrt(scaledImg.width() * scaledImg.width() + scaledImg.height() * scaledImg.height());
        kernelSize = qRound(kernelSize * diagP / diagO);
    }

    int saturation = saturationWidget->getToolValue();
    float satFactor = saturation / 50.0f + 1;

    cv::Mat blurImg = nmc::DkImage::qImage2Mat(inImg);
    cv::Mat distImg(blurImg.size(), CV_8UC1);
    distImg = 255;
    cv::Mat roi(distImg, cv::Rect(qRoi.topLeft().x(), qRoi.topLeft().y(), qRoi.width(), qRoi.height()));
    roi.setTo(0);
    // blur plane by plane
    std::vector<cv::Mat> planes;
    cv::split(blurImg, planes);

    // dist image based on 'some' threshold
    // cv::Mat distImg;
    // cv::threshold(planes.at(0), distImg, 40, 255, THRESH_BINARY);

    cv::distanceTransform(distImg, distImg, DIST_C, 3);
    cv::normalize(distImg, distImg, 1.0f, 0.0f, cv::NORM_MINMAX);

    for (size_t idx = 0; idx < planes.size(); idx++)
        planes.at(idx) = blurPanTilt(planes.at(idx), distImg, kernelSize); // 140 is the maximal blurring kernel size
    // planes.at(idx) = distImg;

    cv::merge(planes, blurImg);
    // return (DkFakeMiniaturesDialog::mat2QImage(blurImg));

    if (satFactor > 1) {
        cv::Mat imgHsv;
        cvtColor(blurImg, imgHsv, cv::COLOR_RGB2HSV);
        std::vector<cv::Mat> imgHsvCh;
        split(imgHsv, imgHsvCh);

        for (int row = 0; row < imgHsv.rows; row++) {
            auto *ptr = imgHsvCh[1].ptr<unsigned char>(row);

            for (int col = 0; col < imgHsv.cols; col++) {
                float tmp = (float)ptr[col] * satFactor;
                if (tmp > 255.0f)
                    tmp = 255.0f;
                if (tmp < 0)
                    tmp = 0.0f;
                ptr[col] = (unsigned char)qRound(tmp);
            }
        }

        merge(imgHsvCh, imgHsv);
        cv::Mat tempImg(blurImg);
        cvtColor(imgHsv, blurImg, cv::COLOR_HSV2RGB);

        if (tempImg.type()
            == CV_8UC4) { // the retImg is always CV_8UC3, so for pics in CV_8UC4 we need to add one channel
            std::vector<cv::Mat> inImgCh;
            split(tempImg, inImgCh);
            std::vector<cv::Mat> retImgCh;
            split(blurImg, retImgCh);
            retImgCh.push_back(inImgCh[3]);
            merge(retImgCh, blurImg);
        }
    }

    return nmc::DkImage::mat2QImage(blurImg, inImg);
#else
    return inImg;
#endif
}

#ifdef WITH_OPENCV
/**
 * blur filter
 * @param src input cv::Mat
 * @param depthImg distance transform based on a roi
 * @param maxKernel maximum blur kernel size
 * @return cv::Mat blurres mat
 **/
cv::Mat DkFakeMiniaturesDialog::blurPanTilt(cv::Mat src, cv::Mat depthImg, int maxKernel)
{
    cv::Mat integralImg;

    cv::Mat blurImg(src.size(), src.depth());

    // a template function would have more 'style' here
    // const double* itgrl64Ptr = 0;

    // images with an area below 4000*4000 can be compuated using 32 bit
    cv::integral(src, integralImg, CV_32S);
    const unsigned int *itgrl32Ptr = integralImg.ptr<unsigned int>();

    for (int rIdx = 0; rIdx < src.rows; rIdx++) {
        auto *blurPtr = blurImg.ptr<unsigned char>(rIdx); // assuming unsigned char
        const float *depthPtr = depthImg.ptr<float>(rIdx);
        const unsigned char *srcPtr = src.ptr<unsigned char>(rIdx);

        for (int cIdx = 0; cIdx < src.cols; cIdx++) {
            // kernel size depends on the distance transform, the user selected
            // float dpv = depthPtr[cIdx];
            float ksf = depthPtr[cIdx] * maxKernel * 0.5f;

            // if (!ks) {
            //	blurPtr[cIdx] = srcPtr[cIdx];
            //	continue;
            // }

            int ks = qRound(ksf);
            if (ksf > 0 && ksf < 2)
                ks = 2;
            else if (ks == 0) { // early skip
                blurPtr[cIdx] = srcPtr[cIdx];
                continue;
            }

            // clip all coordinates
            int left = qMax(cIdx - ks, 0);
            int right = qMin(cIdx + ks + 1, src.cols); // note not cols-1 since integral mImg is src.cols+1
            int bottom = qMax(rIdx - ks, 0); // note top bottom is flipped since -y coords
            int top = qMin(rIdx + ks + 1, src.rows);
            int area = (right - left) * (top - bottom);

            top *= integralImg.cols;
            bottom *= integralImg.cols;

            float tmp = 0.0f;

            // compute mean kernel
            if (area && itgrl32Ptr && ks > 1)
                tmp = (float)(*(itgrl32Ptr + top + right) + *(itgrl32Ptr + bottom + left) - *(itgrl32Ptr + top + left)
                              - *(itgrl32Ptr + bottom + right))
                    / area;
            else
                tmp = srcPtr[cIdx];

            if (tmp < 0)
                tmp = 0.0f;
            if (tmp > 255)
                tmp = 255.0f;

            blurPtr[cIdx] = (uchar)qRound(tmp);
        }
    }

    return blurImg;
}
#endif

/**
 * on button ok pressed event
 **/
void DkFakeMiniaturesDialog::okPressed()
{
    isOk = true;
    this->close();
}

/**
 * on button cancel pressed event
 **/
void DkFakeMiniaturesDialog::cancelPressed()
{
    this->close();
}

/**
 * the dialog is executed and displayes
 **/
void DkFakeMiniaturesDialog::showEvent(QShowEvent *event)
{
    isOk = false;
    double diag = sqrt(mImg->width() * mImg->width() + mImg->height() * mImg->height());
    kernelSizeWidget->setToolValue(qMin(qMax(int(diag * 0.02), 5), 140));
    saturationWidget->setToolValue(2);

    QDialog::showEvent(event);
}

void DkFakeMiniaturesDialog::setImage(const QImage *img)
{
    this->mImg = img;
    createImgPreview();
    drawImgPreview();
};

/**
 * the dialog returns image after execution
 **/
QImage DkFakeMiniaturesDialog::getImage()
{
    QRect rescaledRect = previewLabel->getROI();
    rescaledRect.moveTo(rescaledRect.topLeft().x() - previewImgRect.topLeft().x(),
                        rescaledRect.topLeft().y() - previewImgRect.topLeft().y());
    if (rMin < 1) {
        rescaledRect.moveTo(qRound(rescaledRect.topLeft().x() / rMin), qRound(rescaledRect.topLeft().y() / rMin));
        rescaledRect.setWidth(qRound(rescaledRect.width() / rMin));
        rescaledRect.setHeight(qRound(rescaledRect.height() / rMin));
    }
    QRect imgRect = (*(this->mImg)).rect();
    if (rescaledRect.topLeft().x() < imgRect.topLeft().x())
        rescaledRect.topLeft().setX(imgRect.topLeft().x());
    if (rescaledRect.topLeft().y() < imgRect.topLeft().y())
        rescaledRect.topLeft().setY(imgRect.topLeft().y());
    if (rescaledRect.bottomRight().x() > imgRect.bottomRight().x())
        rescaledRect.bottomRight().setX(imgRect.bottomRight().x());
    if (rescaledRect.bottomRight().y() > imgRect.bottomRight().y())
        rescaledRect.bottomRight().setY(imgRect.bottomRight().y());

    QImage miniature = applyMiniaturesFilter(*(this->mImg), rescaledRect);
    return miniature;
};

/**
 * slot that redraws preview after slider change
 **/
void DkFakeMiniaturesDialog::redrawImgPreview()
{
    QRect rescaledRect = previewLabel->getROI();
    rescaledRect.moveTo(rescaledRect.topLeft().x() - previewImgRect.topLeft().x(),
                        rescaledRect.topLeft().y() - previewImgRect.topLeft().y());
    setImagePreview(applyMiniaturesFilter(getScaledImg(), rescaledRect));
    drawImgPreview();
};

/**************************************************************
 * DkPreviewLabel: label for displaying image preview
 ***************************************************************/
DkPreviewLabel::DkPreviewLabel(DkFakeMiniaturesDialog *parentDialog, QWidget *parent)
    : QLabel(parent)
{
    fmDialog = parentDialog;
    showROI = false;
    selectionStarted = false;
};

DkPreviewLabel::~DkPreviewLabel() = default;

void DkPreviewLabel::mousePressEvent(QMouseEvent *e)
{
    selectionStarted = true;
    QPoint pos = e->pos();
    if (pos.x() < previewImgRect.topLeft().x())
        pos.setX(previewImgRect.topLeft().x());
    if (pos.y() < previewImgRect.topLeft().y())
        pos.setY(previewImgRect.topLeft().y());
    if (pos.x() > previewImgRect.bottomRight().x())
        pos.setX(previewImgRect.bottomRight().x());
    if (pos.y() > previewImgRect.bottomRight().y())
        pos.setY(previewImgRect.bottomRight().y());
    selectionRect.setTopLeft(pos);
    selectionRect.setBottomRight(pos);
};

void DkPreviewLabel::mouseMoveEvent(QMouseEvent *e)
{
    if (selectionStarted) {
        QPoint pos = e->pos();
        if (pos.x() > previewImgRect.topLeft().x() && pos.x() < previewImgRect.bottomRight().x()
            && pos.y() > previewImgRect.topLeft().y() && pos.y() < previewImgRect.bottomRight().y()) {
            selectionRect.setBottomRight(pos);
        }
        repaint();
    }
};

void DkPreviewLabel::mouseReleaseEvent(QMouseEvent *e)
{
    selectionStarted = false;
    if (selectionRect.top() > selectionRect.bottom()) {
        int top = selectionRect.top();
        selectionRect.setTop(selectionRect.bottom());
        selectionRect.setBottom(top);
    }
    if (selectionRect.left() > selectionRect.right()) {
        int left = selectionRect.left();
        selectionRect.setLeft(selectionRect.right());
        selectionRect.setRight(left);
    }

    fmDialog->redrawImgPreview();

    QLabel::mouseReleaseEvent(e);
};

void DkPreviewLabel::enterEvent(QEnterEvent *e)
{
    showROI = true;
    repaint();
    QLabel::enterEvent(e);
};

void DkPreviewLabel::leaveEvent(QEvent *e)
{
    showROI = false;
    repaint();
    QLabel::leaveEvent(e);
};

void DkPreviewLabel::paintEvent(QPaintEvent *e)
{
    QLabel::paintEvent(e);
    if (showROI) {
        QPainter painter(this);
        painter.setPen(QPen(QBrush(QColor(0, 0, 0, 180)), 1, Qt::DashLine));
        painter.setBrush(QBrush(QColor(255, 255, 255, 120)));
        painter.drawRect(selectionRect);
    }
};

void DkPreviewLabel::setImgRect(QRect rect)
{
    previewImgRect = rect;
    selectionRect = QRect(previewImgRect.left() + INIT_X * previewImgRect.width(),
                          qRound(previewImgRect.top() + INIT_Y * previewImgRect.height()),
                          INIT_WIDTH * previewImgRect.width(),
                          qRound(INIT_HEIGHT * previewImgRect.height()));
};

/**************************************************************
 * DkFakeMiniaturesToolWidget: abstract class - all tool widgets inherit from it
 ***************************************************************/
DkFakeMiniaturesToolWidget::DkFakeMiniaturesToolWidget(QWidget *parent, DkFakeMiniaturesDialog *parentDialog)
    : QWidget(parent)
{
    this->leftSpacing = 10;
    this->topSpacing = 10;
    this->margin = 10;
    this->sliderLength = parent->minimumWidth() - 2 * leftSpacing;
    this->valueUpdated = false;

    connect(this, SIGNAL(redrawImgPreview()), parentDialog, SLOT(redrawImgPreview()));
};

DkFakeMiniaturesToolWidget::~DkFakeMiniaturesToolWidget() = default;

/**
 * slider spin box slot: update value and redraw image
 * @param changed value
 **/
void DkFakeMiniaturesToolWidget::updateSliderSpinBox(int val)
{
    if (!valueUpdated) {
        valueUpdated = true;
        this->sliderSpinBox->setValue(val);
        emit redrawImgPreview();
    } else
        valueUpdated = false;
};

/**
 * slider slot: update value and redraw image
 * @param changed value
 **/
void DkFakeMiniaturesToolWidget::updateSliderVal(int val)
{
    if (!valueUpdated) {
        valueUpdated = true;
        this->slider->setValue(val);
        emit redrawImgPreview();
    } else
        valueUpdated = false;
};

/**
 * set a new tool value
 * @param new value
 **/
void DkFakeMiniaturesToolWidget::setToolValue(int val)
{
    if (this->name.compare("DkKernelSize") == 0) {
        slider->setValue(val);
    }
    // else if (this->name.compare("DkSaturation") == 0) { contrast = (int)val; slider->setValue((int)val);}
};

/**
 * @return tool value
 **/
int DkFakeMiniaturesToolWidget::getToolValue()
{
    if (this->name.compare("DkKernelSize") == 0)
        return slider->value();
    else if (this->name.compare("DkSaturation") == 0)
        return slider->value();
    else
        return 0;
};

/**************************************************************
 * DkKernelSize: widget for changing kernel size
 ***************************************************************/
DkKernelSize::DkKernelSize(QWidget *parent, DkFakeMiniaturesDialog *parentDialog)
    : DkFakeMiniaturesToolWidget(parent, parentDialog)
{
    name = QString("DkKernelSize");
    defaultValue = 50;

    minVal = 1; // no change
    maxVal = 140; // max change
    middleVal = 70;

    sliderTitle = new QLabel(tr("Blur amount"), this);
    sliderTitle->move(leftSpacing, topSpacing);

    slider = new QSlider(this);
    slider->setMinimum(minVal);
    slider->setMaximum(maxVal);
    slider->setValue(middleVal);
    slider->setOrientation(Qt::Horizontal);
    slider->setGeometry(QRect(leftSpacing, sliderTitle->geometry().bottom() - 5, sliderLength, 20));

    slider->setStyleSheet(
        QString("QSlider::groove:horizontal {border: 1px solid #999999; height: 4px; margin: 2px 0;")
        + QString("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3c3c3c, stop:1 #c8c8c8) ")
        + QString(";} ")
        + QString("QSlider::handle:horizontal {background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #d2d2d2, "
                  "stop:1 #e6e6e6); border: "
                  "1px solid #5c5c5c; width: 6px; margin:-4px 0px -6px 0px ;border-radius: 3px;}"));

    sliderSpinBox = new QSpinBox(this);
    sliderSpinBox->setGeometry(slider->geometry().right() - 45, sliderTitle->geometry().top(), 45, 20);
    sliderSpinBox->setMinimum(minVal);
    sliderSpinBox->setMaximum(maxVal);
    sliderSpinBox->setValue(slider->value());

    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateSliderSpinBox(int)));
    connect(sliderSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSliderVal(int)));

    minValLabel = new QLabel(QString::number(minVal), this);
    minValLabel->move(leftSpacing, slider->geometry().bottom());

    middleValLabel = new QLabel(QString::number(middleVal), this);
    middleValLabel->move(leftSpacing + sliderLength / 2 - 2, slider->geometry().bottom());

    maxValLabel = new QLabel(QString::number(maxVal), this);
    maxValLabel->move(slider->geometry().right() - 20, slider->geometry().bottom());
};

DkKernelSize::~DkKernelSize() = default;

/**************************************************************
 * DkSaturation: widget for changing saturation
 ***************************************************************/
DkSaturation::DkSaturation(QWidget *parent, DkFakeMiniaturesDialog *parentDialog)
    : DkFakeMiniaturesToolWidget(parent, parentDialog)
{
    name = QString("DkSaturation");
    defaultValue = 50;

    minVal = 0;
    middleVal = 50;
    maxVal = 100;

    sliderTitle = new QLabel(tr("Saturation"), this);
    sliderTitle->move(leftSpacing, topSpacing);

    slider = new QSlider(this);
    slider->setMinimum(minVal);
    slider->setMaximum(maxVal);
    slider->setValue(middleVal);
    slider->setOrientation(Qt::Horizontal);
    slider->setGeometry(QRect(leftSpacing, sliderTitle->geometry().bottom() - 5, sliderLength, 20));

    slider->setStyleSheet(
        QString("QSlider::groove:horizontal {border: 1px solid #999999; height: 4px; margin: 2px 0;")
        + QString("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ffffff, stop:1 #00ffff);} ")
        + QString("QSlider::handle:horizontal {background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #d2d2d2, "
                  "stop:1 #e6e6e6); border: "
                  "1px solid #5c5c5c; width: 6px; margin:-4px 0px -6px 0px ;border-radius: 3px;}"));

    sliderSpinBox = new QSpinBox(this);
    sliderSpinBox->setGeometry(slider->geometry().right() - 45, sliderTitle->geometry().top(), 45, 20);
    sliderSpinBox->setMinimum(minVal);
    sliderSpinBox->setMaximum(maxVal);
    sliderSpinBox->setValue(slider->value());

    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateSliderSpinBox(int)));
    connect(sliderSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSliderVal(int)));

    minValLabel = new QLabel(QString::number(minVal), this);
    minValLabel->move(leftSpacing, slider->geometry().bottom());

    middleValLabel = new QLabel(QString::number(middleVal), this);
    middleValLabel->move(leftSpacing + sliderLength / 2 - 2, slider->geometry().bottom());

    maxValLabel = new QLabel(QString::number(maxVal), this);
    maxValLabel->move(slider->geometry().right() - 20, slider->geometry().bottom());
};

DkSaturation::~DkSaturation() = default;
};
