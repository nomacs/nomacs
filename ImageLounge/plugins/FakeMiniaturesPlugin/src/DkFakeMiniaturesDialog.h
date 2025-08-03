/*******************************************************************************************************
 DkFakeMiniaturesDialog.h
 Created on:	29.04.2013

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2012 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2012 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2012 Florian Kleber <florian@nomacs.org>

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

#pragma warning(push, 0) // no warnings from includes - begin

#include <QBoxLayout>
#include <QDialog>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QWidget>

// OpenCV
#ifdef WITH_OPENCV

#ifdef Q_WS_WIN
#pragma warning(disable : 4996)
#endif

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
#endif

#pragma warning(pop, 0) // no warnings from includes - end

namespace nmp
{

class DkPreviewLabel;
class DkKernelSize;
class DkSaturation;

class DkFakeMiniaturesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DkFakeMiniaturesDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~DkFakeMiniaturesDialog() override;
    bool wasOkPressed()
    {
        return isOk;
    };
    void setImage(const QImage *img);
    void setImagePreview(QImage img)
    {
        imgPreview = img;
    };
    QImage getImage();
    QImage applyMiniaturesFilter(QImage inImg, QRect qRoi);
    QImage getScaledImg()
    {
        return scaledImg;
    };
    void drawImgPreview();

public slots:
    void redrawImgPreview();

protected slots:
    void okPressed();
    void cancelPressed();

protected:
    bool isOk;
    const QImage *mImg;
    QImage imgPreview;
    int dialogWidth;
    int dialogHeight;
    QRect previewImgRect;
    DkPreviewLabel *previewLabel;
    QImage scaledImg;
    float rMin;
    DkKernelSize *kernelSizeWidget;
    DkSaturation *saturationWidget;

    int previewWidth;
    int previewHeight;
    int toolsWidth;
    int previewMargin;

    void init();
    void createLayout();
    void showEvent(QShowEvent *event) override;
    void createImgPreview();

#ifdef WITH_OPENCV
    Mat blurPanTilt(Mat src, Mat depthImg, int maxKernel);

    /**
     * Converts a QImage to a Mat
     * @param mImg formats supported: ARGB32 | RGB32 | RGB888 | Indexed8
     * @return cv::Mat the corresponding Mat
     **/
    static Mat qImage2Mat(const QImage img)
    {
        Mat mat2;
        QImage cImg; // must be initialized here!	(otherwise the data is lost before clone())

        if (img.format() == QImage::Format_ARGB32 || img.format() == QImage::Format_RGB32) {
            mat2 = Mat(img.height(), img.width(), CV_8UC4, (uchar *)img.bits(), img.bytesPerLine());
            // qDebug() << "ARGB32 or RGB32";
        } else if (img.format() == QImage::Format_RGB888) {
            mat2 = Mat(img.height(), img.width(), CV_8UC3, (uchar *)img.bits(), img.bytesPerLine());
            // qDebug() << "RGB888";
        } else if (img.format() == QImage::Format_Indexed8) {
            mat2 = Mat(img.height(), img.width(), CV_8UC1, (uchar *)img.bits(), img.bytesPerLine());
            // qDebug() << "indexed...";
        } else {
            // qDebug() << "image flag: " << mImg.format();
            cImg = img.convertToFormat(QImage::Format_ARGB32);
            mat2 = Mat(cImg.height(), cImg.width(), CV_8UC4, (uchar *)cImg.bits(), cImg.bytesPerLine());
            // qDebug() << "I need to convert the QImage to ARGB32";
        }

        mat2 = mat2.clone(); // we need to own the pointer

        return mat2;
    }

    /**
     * Converts a cv::Mat to a QImage.
     * @param mImg supported formats CV8UC1 | CV_8UC3 | CV_8UC4
     * @return QImage the corresponding QImage
     **/
    static QImage mat2QImage(Mat img)
    {
        QImage qImg;

        // since Mat header is copied, a new buffer should be allocated (check this!)
        if (img.depth() == CV_32F)
            img.convertTo(img, CV_8U, 255);

        if (img.type() == CV_8UC1) {
            qImg = QImage(img.data,
                          (int)img.cols,
                          (int)img.rows,
                          (int)img.step,
                          QImage::Format_Indexed8); // opencv uses size_t if for scaling in x64 applications
            // Mat tmp;
            // cvtColor(mImg, tmp, CV_GRAY2RGB);	// Qt does not support writing to index8 images
            // mImg = tmp;
        }
        if (img.type() == CV_8UC3) {
            // cv::cvtColor(mImg, mImg, CV_RGB2BGR);
            qImg = QImage(img.data, (int)img.cols, (int)img.rows, (int)img.step, QImage::Format_RGB888);
        }
        if (img.type() == CV_8UC4) {
            qImg = QImage(img.data, (int)img.cols, (int)img.rows, (int)img.step, QImage::Format_ARGB32);
        }

        qImg = qImg.copy();

        return qImg;
    }
#endif
};

class DkPreviewLabel : public QLabel
{
    Q_OBJECT

public:
    explicit DkPreviewLabel(DkFakeMiniaturesDialog *parentDialog, QWidget *parent = nullptr);
    ~DkPreviewLabel() override;
    void setImgRect(QRect rect);
    QRect getROI()
    {
        return selectionRect;
    };

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;

private:
    bool selectionStarted;
    bool showROI;
    QRect selectionRect;
    QRect previewImgRect;
    DkFakeMiniaturesDialog *fmDialog;
};

class DkFakeMiniaturesToolWidget : public QWidget
{
    Q_OBJECT

public:
    DkFakeMiniaturesToolWidget(QWidget *parent, DkFakeMiniaturesDialog *parentDialog);
    ~DkFakeMiniaturesToolWidget() override;

    void setToolValue(int val);
    int getToolValue();

    int getDefaultValue()
    {
        return defaultValue;
    };

protected:
    QSpinBox *sliderSpinBox;
    QSlider *slider;
    QLabel *sliderTitle;
    QLabel *minValLabel;
    QLabel *maxValLabel;
    QLabel *middleValLabel;
    int leftSpacing;
    int topSpacing;
    int sliderLength;
    int margin;
    int minVal;
    int maxVal;
    int middleVal;
    bool valueUpdated;
    QString name;
    int defaultValue;

protected slots:

    void updateSliderSpinBox(int val);
    void updateSliderVal(int val);

signals:
    void redrawImgPreview();
};

class DkKernelSize : public DkFakeMiniaturesToolWidget
{
    Q_OBJECT

public:
    DkKernelSize(QWidget *parent, DkFakeMiniaturesDialog *parentDialog);
    ~DkKernelSize() override;
};

class DkSaturation : public DkFakeMiniaturesToolWidget
{
    Q_OBJECT

public:
    DkSaturation(QWidget *parent, DkFakeMiniaturesDialog *parentDialog);
    ~DkSaturation() override;
};

};