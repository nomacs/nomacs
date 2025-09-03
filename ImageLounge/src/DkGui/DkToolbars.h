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

#include "DkBaseWidgets.h"

#include <QToolBar>

class QCheckBox;
class QComboBox;
class QGraphicsOpacityEffect;
class QDoubleSpinBox;
class QPushButton;
class QColorDialog;
class QStandardItemModel;

namespace nmc
{
class DkTransferToolBar;
class DkVector;
class DkQuickAccessEdit;
class DkRectWidget;

class DkMainToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit DkMainToolBar(const QString &title, QWidget *parent = nullptr);

    void allActionsAdded(); // fast fix for now
    void setQuickAccessModel(QStandardItemModel *model);
    DkQuickAccessEdit *getQuickAccess() const;

public slots:
    void closeQuickAccess();

protected:
    void createLayout();

    DkQuickAccessEdit *mQuickAccessEdit;
};

class DkColorSlider : public DkWidget
{
    Q_OBJECT

public:
    // DkColorSlider(QWidget *parent);
    DkColorSlider(QWidget *parent, QColor color);
    DkColorSlider(QWidget *parent, qreal normedPos, QColor color, int sliderWidth);
    ~DkColorSlider() override;
    QColor getColor();
    qreal getNormedPos();
    void setNormedPos(qreal pos);
    void setActive(bool isActive);
    void updatePos(int parentWidth);

    // void paintSlider(QPainter *painter);

signals:
    void sliderMoved(DkColorSlider *sender, int dragDistX, int yPos) const;
    void sliderActivated(DkColorSlider *sender) const;
    void colorChanged(DkColorSlider *slider) const;

public slots:
    void paintEvent(QPaintEvent *event) override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    int mSliderWidth = 0, mSliderHeight = 0, mSliderHalfWidth = 0;
    bool mIsActive = false;
    int mDragStartX = 0;
    QColor mColor;
    qreal mNormedPos;
};

class DkGradient : public DkWidget
{
    Q_OBJECT

public:
    explicit DkGradient(QWidget *parent);
    ~DkGradient() override;
    QGradientStops getGradientStops();
    void insertSlider(qreal pos, QColor col = QColor());
    void reset();
    QLinearGradient getGradient();
    void setGradient(const QLinearGradient &gradient);

signals:
    void gradientChanged() const;

public slots:
    void moveSlider(DkColorSlider *sender, int dragDistX, int yPos);
    void changeColor(DkColorSlider *slider);
    void activateSlider(DkColorSlider *sender);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void init();
    void addSlider(qreal pos, QColor color);
    void clearAllSliders();
    void updateGradient();
    qreal getNormedPos(int pos);
    int getAbsolutePos(qreal pos);

    int mClickAreaHeight;
    int mDeleteSliderDist;
    QVector<DkColorSlider *> mSliders;
    bool mIsSliderDragged = false;
    QLinearGradient mGradient;
    int mSliderWidth = 0, mHalfSliderWidth = 0;

    DkColorSlider *mActiveSlider = nullptr;
    bool mIsActiveSliderExisting = false;
};

enum toolBarIcons {
    icon_toolbar_reset,
    icon_toolbar_pipette,
    icon_toolbar_save,
    icon_toolbar_end, // nothing beyond this point
};

enum toolBarActions {
    toolbar_save,
    toolbar_reset,
    toolbar_pipette,
    toolbar_end, // nothing beyond this point
};

enum imageModes {
    mode_uninitialized,
    mode_invalid_format,
    mode_gray,
    mode_rgb,
};

class DkTransferToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit DkTransferToolBar(QWidget *parent);
    ~DkTransferToolBar() override;

signals:
    void pickColorRequest(bool enabled) const;
    void colorTableChanged(QGradientStops stops) const;
    void channelChanged(int channel) const;
    void transferFunctionChanged(int channel, QGradientStops stops) const;
    void tFEnabled(bool) const;
    void gradientChanged() const;

public slots:
    void paintEvent(QPaintEvent *event) override;
    void pickColorCancelled();
    void insertSlider(qreal pos);
    void setImageMode(int mode);
    void saveGradient();
    void deleteGradientMenu(QPoint pos);
    void deleteGradient();
    void resetGradient();

protected slots:
    void applyTF();
    void pickColor(bool enabled);
    void changeChannel(int index);
    void enableTFCheckBoxClicked(int state);
    void switchGradient(int idx);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void loadSettings();
    void saveSettings();
    void updateGradientHistory();

private:
    void createIcons();
    void applyImageMode(int mode);
    void enableToolBar(bool enable);

    QCheckBox *mEnableTFCheckBox = nullptr;

    QImage mShade;
    QImage mSliderImg, mActiveSliderImg;

    QVector<QAction *> mToolBarActions;
    QVector<QIcon> mToolBarIcons;

    DkGradient *mGradient = nullptr;
    QComboBox *mChannelComboBox = nullptr;

    QComboBox *mHistoryCombo = nullptr;
    QVector<QLinearGradient> mOldGradients;

    QGraphicsOpacityEffect *mEffect = nullptr;
    int mImageMode = mode_uninitialized;
};

class DkCropToolBar : public QToolBar
{
    Q_OBJECT

public:
    enum {
        crop_icon = 0,
        pan_icon,
        cancel_icon,
        invert_icon,
        info_icon,

        icons_end,
    };

    explicit DkCropToolBar(const QString &title, QWidget *parent = nullptr);
    ~DkCropToolBar() override;

    QColor getColor()
    {
        return mBgCol;
    };

    void loadSettings();

public slots:
    void setAspectRatio(const QPointF &aRatio);
    void setRect(const QRect &r);
    void onCropActionTriggered();
    void onCancelActionTriggered();
    void onSwapActionTriggered();
    void onRatioBoxCurrentIndexChanged(const QString &text);
    void onGuideBoxCurrentIndexChanged(int idx);
    void onHorValBoxValueChanged(double val);
    void onVerValBoxValueChanged(double val);
    void onAngleBoxValueChanged(double val);
    void onBgColButtonClicked();
    void onPanActionToggled(bool checked);
    void onInvertActionToggled(bool checked);
    void onInfoActionToggled(bool checked);
    void angleChanged(double val);
    void setVisible(bool visible) override;

signals:
    void panSignal(bool checked);
    void cropSignal(bool cropToMetadata = false); // vs. crop
    void cancelSignal();
    void aspectRatio(const DkVector &diag);
    void angleSignal(double angle);
    void colorSignal(const QBrush &brush);
    void paintHint(int paintMode);
    void shadingHint(bool invert);
    void showInfo(bool show);
    void updateRectSignal(const QRect &r);

protected:
    void createLayout();
    void createIcons();
    void saveSettings();

    QComboBox *mRatioBox = nullptr;
    QComboBox *mGuideBox = nullptr;
    QAction *mInvertAction = nullptr;
    QDoubleSpinBox *mHorValBox = nullptr;
    QDoubleSpinBox *mVerValBox = nullptr;
    QDoubleSpinBox *mAngleBox = nullptr;
    QPushButton *mBgColButton = nullptr;
    QColorDialog *mColorDialog = nullptr;
    QColor mBgCol;
    QAction *mPanAction = nullptr;
    QAction *mInfoAction = nullptr;
    QCheckBox *mCbMeta = nullptr;
    DkRectWidget *mCropRect = nullptr;

    QVector<QIcon> mIcons; // needed for colorizing
};

class DllCoreExport DkToolBarManager
{
public:
    static DkToolBarManager &inst();

    // singleton
    DkToolBarManager(DkToolBarManager const &) = delete;
    void operator=(DkToolBarManager const &) = delete;

    void showDefaultToolBar(bool show, bool permanent = true);
    void showMovieToolBar(bool show);
    void show(bool show, bool permanent = false);
    void restore();
    void showToolBar(QToolBar *toolbar, bool show);
    void showToolBarsTemporarily(bool show);

    void createTransferToolBar();

    DkMainToolBar *defaultToolBar() const;
    DkTransferToolBar *transferToolBar() const;

private:
    DkToolBarManager();
    void createDefaultToolBar();

    DkMainToolBar *mToolBar = nullptr;
    QToolBar *mMovieToolBar = nullptr;
    QVector<QToolBar *> mHiddenToolBars;
    Qt::ToolBarArea mMovieToolbarArea = Qt::NoToolBarArea;

    DkTransferToolBar *mTransferToolBar = nullptr;
};

}
