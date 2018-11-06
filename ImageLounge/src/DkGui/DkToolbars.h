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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QToolBar>
#include <QWidget>
#include <QObject>
#include <QCompleter>
#pragma warning(pop)		// no warnings from includes - end

#pragma warning(disable: 4251)

// Qt defines
class QCheckBox;
class QComboBox;
class QLineEdit;
class QGraphicsOpacityEffect;
class QDoubleSpinBox;
class QPushButton;
class QColorDialog;
class QStandardItemModel;

namespace nmc {

class DkTransferToolBar;
class DkVector;
class DkQuickAccess;
class DkQuickAccessEdit;
class DkRectWidget;

class DkMainToolBar : public QToolBar {
	Q_OBJECT

public:
	DkMainToolBar(const QString & title, QWidget * parent = 0);

	void allActionsAdded();	// fast fix for now
	void setQuickAccessModel(QStandardItemModel* model);
	DkQuickAccessEdit* getQuickAccess() const;

public slots:
	void closeQuickAccess();

protected:
	void createLayout();

	DkQuickAccessEdit* mQuickAccessEdit;
};

class DkColorSlider : public DkWidget {
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
	void sliderMoved(DkColorSlider *sender, int dragDistX, int yPos) const;
	void sliderActivated(DkColorSlider *sender) const;
	void colorChanged(DkColorSlider *slider) const;
				
public slots:
	virtual void paintEvent(QPaintEvent* event) override;

protected:
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
	int mSliderWidth = 0, mSliderHeight = 0, mSliderHalfWidth = 0;
	bool mIsActive = false;
	int mDragStartX = 0;
	QColor mColor;
	qreal mNormedPos;
};

class DkGradient : public DkWidget {
	Q_OBJECT

public:
	DkGradient(QWidget *parent);
	~DkGradient();
	QGradientStops getGradientStops();
	void insertSlider(qreal pos, QColor col = QColor());
	void reset();
	QLinearGradient getGradient();
	void setGradient(const QLinearGradient& gradient);

signals:
	void gradientChanged() const;
		
public slots:
	void moveSlider(DkColorSlider* sender, int dragDistX, int yPos);
	void changeColor(DkColorSlider *slider);
	void activateSlider(DkColorSlider *sender);

protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void resizeEvent(QResizeEvent * event) override;
		
private:
	void init();
	void addSlider(qreal pos, QColor color);
	void clearAllSliders();
	void updateGradient();
	qreal getNormedPos(int pos);
	int getAbsolutePos(qreal pos);

	int mClickAreaHeight;
	int mDeleteSliderDist;
	QVector<DkColorSlider*> mSliders;
	bool mIsSliderDragged = false;
	QLinearGradient mGradient;
	int mSliderWidth = 0, mHalfSliderWidth = 0;

	DkColorSlider *mActiveSlider = 0;
	bool mIsActiveSliderExisting = false;
};

enum toolBarIcons {
	icon_toolbar_reset,
	icon_toolbar_pipette,
	icon_toolbar_save,
	icon_toolbar_end,	// nothing beyond this point
};

enum toolBarActions {
	toolbar_save,
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
	void pickColorRequest(bool enabled) const;
	void colorTableChanged(QGradientStops stops) const;
	void channelChanged(int channel) const;
	void transferFunctionChanged(int channel, QGradientStops stops) const;
	void tFEnabled(bool) const;
	void gradientChanged() const;

public slots:
	virtual void paintEvent(QPaintEvent* event) override;
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
	virtual void resizeEvent(QResizeEvent * event) override;
	void loadSettings();
	void saveSettings();
	void updateGradientHistory();

private:
	void createIcons();
	void applyImageMode(int mode);
	void enableToolBar(bool enable);
	
	QCheckBox *mEnableTFCheckBox = 0;
		
	QImage mShade;
	QImage mSliderImg, mActiveSliderImg;

	QVector<QAction *> mToolBarActions;
	QVector<QIcon> mToolBarIcons;
		
	DkGradient *mGradient = 0;
	QComboBox *mChannelComboBox = 0;

	QComboBox* mHistoryCombo = 0;
	QVector<QLinearGradient> mOldGradients;

	QGraphicsOpacityEffect *mEffect = 0;
	int mImageMode = mode_uninitialized;

};

class DkCropToolBar : public QToolBar {
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

	DkCropToolBar(const QString & title, QWidget * parent = 0);
	virtual ~DkCropToolBar();

	QColor getColor() {
		return mBgCol;
	};

	void loadSettings();

public slots:
	void setAspectRatio(const QPointF& aRatio);
	void setRect(const QRect& r);
	void on_cropAction_triggered();
	void on_cancelAction_triggered();
	void on_swapAction_triggered();
	void on_ratioBox_currentIndexChanged(const QString& text);
	void on_guideBox_currentIndexChanged(int idx);
	void on_horValBox_valueChanged(double val);
	void on_verValBox_valueChanged(double val);
	void on_angleBox_valueChanged(double val);
	void on_bgColButton_clicked();
	void on_panAction_toggled(bool checked);
	void on_invertAction_toggled(bool checked);
	void on_infoAction_toggled(bool checked);
	void angleChanged(double val);
	virtual void setVisible(bool visible) override;

signals:
	void panSignal(bool checked);
	void cropSignal(bool cropToMetadata = false);	// vs. crop
	void cancelSignal();
	void aspectRatio(const DkVector& diag);
	void angleSignal(double angle);
	void colorSignal(const QBrush& brush);
	void paintHint(int paintMode);
	void shadingHint(bool invert);
	void showInfo(bool show);
	void updateRectSignal(const QRect& r);

protected:
	void createLayout();
	void createIcons();
	void saveSettings();

	QComboBox* mRatioBox = 0;
	QComboBox* mGuideBox = 0;
	QAction* mInvertAction = 0;
	QDoubleSpinBox* mHorValBox = 0;
	QDoubleSpinBox* mVerValBox = 0;
	QDoubleSpinBox* mAngleBox = 0;
	QPushButton* mBgColButton = 0;
	QColorDialog* mColorDialog = 0;
	QColor mBgCol;
	QAction* mPanAction = 0;
	QAction* mInfoAction = 0;
	QCheckBox* mCbMeta = 0;
	DkRectWidget* mCropRect = 0;

	QVector<QIcon> mIcons;		// needed for colorizing
};

class DllCoreExport DkToolBarManager {

public:
	static DkToolBarManager& inst();

	// singleton
	DkToolBarManager(DkToolBarManager const&) = delete;
	void operator=(DkToolBarManager const&) = delete;

	void showDefaultToolBar(bool show, bool permanent = true);
	void showMovieToolBar(bool show);
	void show(bool show, bool permanent = false);
	void restore();
	void showToolBar(QToolBar* toolbar, bool show);
	void showToolBarsTemporarily(bool show);

	void createTransferToolBar();

	DkMainToolBar* defaultToolBar() const;
	DkTransferToolBar* transferToolBar() const;

private:
	DkToolBarManager();
	void createDefaultToolBar();

	DkMainToolBar* mToolBar = 0;
	QToolBar* mMovieToolBar = 0;
	QVector<QToolBar*> mHiddenToolBars;
	Qt::ToolBarArea mMovieToolbarArea = Qt::NoToolBarArea;

	DkTransferToolBar* mTransferToolBar = 0;

};


}
