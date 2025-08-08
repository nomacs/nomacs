/*******************************************************************************************************
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

 related links:
 [1] https://nomacs.org/
 [2] https://github.com/nomacs/
 [3] http://download.nomacs.org
 *******************************************************************************************************/

#pragma once

#include "DkBaseWidgets.h"

#include <QWidget>

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

// Qt defines
class QSlider;
class QLabel;
class QSpinBox;
class QDoubleSpinBox;
class QColorDialog;
class QPushButton;
class QMenu;
class QLineEdit;

namespace nmc
{

// nomacs defines

class DllCoreExport DkSlider : public DkWidget
{
    Q_OBJECT

public:
    explicit DkSlider(QString title = "", QWidget *parent = nullptr);

    QSlider *getSlider() const;
    void setMinimum(int minValue);
    void setMaximum(int maxValue);
    void setTickInterval(int ticValue);
    int value() const;
    void setFocus(Qt::FocusReason reason);

public slots:
    void setValue(int value);

signals:
    void sliderMoved(int value);
    void valueChanged(int value);

protected:
    void createLayout();

    QLabel *titleLabel;
    QLabel *minValLabel;
    QLabel *maxValLabel;
    QSlider *slider;
    QSpinBox *sliderBox;
};

class DllCoreExport DkDoubleSlider : public DkWidget
{
    Q_OBJECT

public:
    explicit DkDoubleSlider(const QString &title = "", QWidget *parent = nullptr);

    QSlider *getSlider() const;
    void setMinimum(double minValue);
    void setMaximum(double maxValue);
    void setCenterValue(double center);
    void setTickInterval(double ticValue);
    double value() const;
    void setFocus(Qt::FocusReason reason);
    void setSliderInverted(bool inverted);

public slots:
    void setValue(double value);
    void setIntValue(int value);

signals:
    void sliderMoved(double value);
    void valueChanged(double value);

protected:
    void createLayout();
    int map(double val) const;
    double mapInv(int val) const;

    QLabel *mTitleLabel;
    QSlider *mSlider;
    QDoubleSpinBox *mSliderBox;
    bool mSliderInverted = false;
    double mCenter = 0;
};

class DllCoreExport DkColorChooser : public DkWidget
{
    Q_OBJECT

public:
    explicit DkColorChooser(QColor defaultColor = QColor(),
                            QString text = "Color",
                            QWidget *parent = nullptr,
                            Qt::WindowFlags flags = Qt::WindowFlags());
    ~DkColorChooser() override = default;

    void setColor(const QColor &color); // will not emit
    void setDefaultColor(const QColor &color); // will not emit
    void enableAlpha(bool enable = true);

protected slots:
    void onResetButtonClicked();
    void onColorButtonClicked();
    void onColorDialogAccepted();

signals:
    void colorReset(); // clicked reset button; color may not have changed
    void colorAccepted(); // clicked accept/ok button; color may not have changed
    void colorChanged(const QColor &color); // user changed the color for any reason

protected:
    QColorDialog *mColorDialog = nullptr;
    QPushButton *mColorButton = nullptr;
    QPushButton *mResetButton = nullptr;

    QColor mColor, mDefaultColor;
    const QString mText;

    void createLayout();
};

class DllCoreExport DkColorEdit : public DkWidget
{
    Q_OBJECT

public:
    explicit DkColorEdit(const QColor &col = QColor(), QWidget *parent = nullptr);

    void setColor(const QColor &col);
    QColor color() const;

signals:
    void newColor(const QColor &col);

public slots:
    void colorChanged();
    void hashChanged(const QString &name);
    void hashEditFinished();

protected:
    void createLayout();

    enum cols {
        r = 0,
        g,
        b,

        c_end
    };

    QVector<QSpinBox *> mColBoxes;
    QLineEdit *mColHash;
    QColor mColor;
};

class DllCoreExport DkColorPane : public DkWidget
{
    Q_OBJECT

public:
    explicit DkColorPane(QWidget *parent = nullptr);

    QColor color() const;
    double hue() const;

signals:
    void colorSelected(const QColor &col) const;

public slots:
    void setHue(int hue);
    void setColor(const QColor &col);

protected:
    void paintEvent(QPaintEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *me) override;
    void mouseReleaseEvent(QMouseEvent *me) override;
    void mousePressEvent(QMouseEvent *me) override;
    void resizeEvent(QResizeEvent *re) override;

    QPoint color2Pos(const QColor &col) const;
    QColor pos2Color(const QPoint &pos) const;
    QColor ipl(const QColor &c0, const QColor &c1, double alpha) const;
    void setPos(const QPoint &pos);

    double brightness(const QColor &col) const;

    QColor mColor = QColor(255, 0, 0);
    QPoint mPos = QPoint(0, 0);
};

class DllCoreExport DkColorPicker : public DkWidget
{
    Q_OBJECT

public:
    explicit DkColorPicker(QWidget *parent = nullptr);

    QColor color() const;

signals:
    void colorSelected(const QColor &col);

public slots:
    void setColor(const QColor &col);
    void showMenu(const QPoint &pos = QPoint());

protected:
    void contextMenuEvent(QContextMenuEvent *cme) override;
    void createLayout();

    DkColorPane *mColorPane = nullptr;
    QLabel *mColorPreview = nullptr;
    QMenu *mContextMenu = nullptr;
    DkColorEdit *mColorEdit = nullptr;
};

class DllCoreExport DkRectWidget : public DkWidget
{
    Q_OBJECT

public:
    explicit DkRectWidget(const QRect &r = QRect(), QWidget *parent = nullptr);

    QRect rect() const;

public slots:
    void setRect(const QRect &r);
    void setSizeOnly(bool sizeOnly);
    void updateRect();

signals:
    void updateRectSignal(const QRect &r) const;

protected:
    void createLayout();

    enum {
        crop_x = 0,
        crop_y,
        crop_width,
        crop_height,

        crop_end
    };

    QVector<QSpinBox *> mSpCropRect;
    QLabel *mCropXLabel;
    QLabel *mCropYLabel;
    QLabel *mCropWLabel;
    QLabel *mCropHLabel;
};

}
