/*******************************************************************************************************
 DkFotojiffy.h
 Created on:	14.09.2014
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2014 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2014 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2014 Florian Kleber <florian@nomacs.org>

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

#include <QDockWidget>
#include <QString>
#include <QWidget>
#include <QLabel>
#include <QVector>
#include <QVBoxLayout>
#include <QFileSystemWatcher>
#include <QGraphicsOpacityEffect>
#include <QStackedLayout>
#include <QFileDialog>

#include "DkUtils.h"
#include "DkImageContainer.h"
#include "DkWidgets.h"

namespace nmc {

class DkSocialConfirmDialog : public DkWidget {
	Q_OBJECT

public:
	DkSocialConfirmDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);

public slots:
	void show();
	void toggleShow();

signals:
	void saveImageSignal(QFileInfo path);

protected slots:
	void confirmToggled(bool checked);
	void on_okButton_clicked();
	void on_cancelButton_clicked();

protected:
	QLabel* infoText;
	QPushButton* okButton;
	QPushButton* cancelButton;
	QPushButton* checkBox;

	void createLayout();
	void paintEvent(QPaintEvent *event);
};

class DkQrCodeConfirmDialog : public DkWidget {
	Q_OBJECT

public:
	DkQrCodeConfirmDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);

public slots:
	void changeImage();
	void toggleShow();

protected:
	QLabel* infoText;
	QLabel* urlText;
	QLabel* imgLabel;
	QMenu* cm;

	void createLayout();
	void paintEvent(QPaintEvent *event);
	void contextMenuEvent(QContextMenuEvent *event);
};

class DkSocialButton : public QLabel {
	Q_OBJECT

public:
	enum mode {
		facebook,
		qrcode,
		reset_strip,

		mode_end
	};

	DkSocialButton(int mode, QWidget* parent = 0, Qt::WFlags flags = 0);

	void registerAction(QAction* action) {
		connect(this, SIGNAL(visibleSignal(bool)), action, SLOT(setChecked(bool)));
	};

	void setDisplaySettings(QBitArray* displayBits) {
		displaySettingsBits = displayBits;
	};

	bool getCurrentDisplaySetting() {

		if (!displaySettingsBits)
			return false;

		if (DkSettings::app.currentAppMode < 0 || DkSettings::app.currentAppMode >= displaySettingsBits->size()) {
			qDebug() << "[WARNING] illegal app mode: " << DkSettings::app.currentAppMode;
			return false;
		}

		return displaySettingsBits->testBit(DkSettings::app.currentAppMode);
	};

	bool isHiding() const {
		return hiding;
	};


signals:
	void visibleSignal(bool visible);
	void showConfirmDialogSignal();

public slots:
	virtual void show();
	virtual void hide();
	virtual void setVisible(bool visible);

	void animateOpacityUp();
	void animateOpacityDown();
	void changeImage();

protected:
	bool hiding;
	bool showing;
	int mode;

	QGraphicsOpacityEffect* opacityEffect;
	QBitArray* displaySettingsBits;
	DkSocialConfirmDialog* confirmDialog;

	QMenu* cm;

	// functions
	void init();
	void contextMenuEvent(QContextMenuEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
};

class DllExport DkRotateImageLabel : public QLabel {
	Q_OBJECT

public:
	DkRotateImageLabel(const QString& imgPath, QWidget* parent = 0, Qt::WindowFlags f = 0);

public slots:
	void setVisible(bool visible);
	void updateAnimation();

protected:
	void paintEvent(QPaintEvent *event);

	QTimer animationTimer;
	QPixmap pm;
	int animationAngle;
};

class DllExport DkStripFinished : public QLabel {
	Q_OBJECT

public:
	DkStripFinished(QWidget* parent = 0);

signals:
	void showStripSignal();
	void startNewStripSignal();

protected:
	void createLayout();

};

//class DllExport DkFotojiffyButton : public QPushButton {
//
//public:
//	DkFotojiffyButton(const QIcon& icon, const QString& objectName, const QWidget* parent = 0);
//
//protected:
//	void contextMenuEvent(QContextMenuEvent *event);
//	void loadSettings();
//	void saveSettings();
//
//	// hide constructors
//	DkFotojiffyButton(QWidget* parent = 0);
//	DkFotojiffyButton(const QString& text, QWidget* parent = 0);
//	//DkFotojiffyButton(const QIcon& icon, const QString& text, QWidget* parent = 0);
//
//};

class DllExport DkStripDock : public QDockWidget {
	Q_OBJECT

public:
	DkStripDock(const QString& title, QWidget* parent = 0, Qt::WindowFlags flags = 0 );

	void setFullScreen(bool fullscreen);
	void restartWatcher();

public slots:
	void directoryChanged(QString dirPath);
	void updateImage();
	void startNewStrip();
	//void waitForScriptFinished();
	void fadeOut();
	void fadeIn();
	void copyImagesToTemp();
	void resetStrip();
	void updateLabels();

signals:
	void loadLastImageSignal();

protected:
	void createLayout();
	void resetLabels();
	void updateTitleMissing(int num);
	//void showGenerating(bool show = true);
	void clearImages();

	QVector<QSharedPointer<DkImageContainerT> > stripImages;
	QVector<QLabel*> labels;
	QLabel* titleLabel;
	QWidget* stripBackground;
	DkWidget* resetWidget;
	QLabel* bgWidget;
	QWidget* waitContainer;
	QFileSystemWatcher* dirWatcher;
	QGraphicsOpacityEffect* opacityEffect;
	QGraphicsOpacityEffect* waitOpacityEffect;

	DkStripFinished* stripFinished;
	//QStackedLayout* stackedLayout;
	QTimer waitForScriptTimer;
	QTimer fadeOutTimer;
	QTimer fadeInTimer;
	QTimer reloadImgTimer;
	float stripOpacity;
	bool finishStrip;
};

};