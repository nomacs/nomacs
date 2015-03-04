/*******************************************************************************************************
 DkThresholdPlugin.h
 Created on:	20.05.2014

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

#include <QObject>
#include <QtPlugin>
#include <QImage>
#include <QStringList>
#include <QString>
#include <QMessageBox>
#include <QAction>
#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>
#include <QToolBar>
#include <QMainWindow>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QListWidget>
#include <QSlider>
#include <QPushButton>
#include <QMouseEvent>

#include "DkPluginInterface.h"
#include "DkNoMacs.h"
#include "DkSettings.h"
#include "DkUtils.h"
#include "DkBaseViewPort.h"
#include "DkImageStorage.h"

namespace nmc {

class DkThresholdViewPort;
class DkThresholdToolBar;

class DkThresholdPlugin : public QObject, DkViewPortInterface {
    Q_OBJECT
    Q_INTERFACES(nmc::DkViewPortInterface)

public:

	DkThresholdPlugin();
	~DkThresholdPlugin();

	QString pluginID() const;
    QString pluginName() const;
    QString pluginDescription() const;
    QImage pluginDescriptionImage() const;
    QString pluginVersion() const;

    QStringList runID() const;
    QString pluginMenuName(const QString &runID = QString()) const;
    QString pluginStatusTip(const QString &runID = QString()) const;
    QImage runPlugin(const QString &runID = QString(), const QImage &image = QImage()) const;
	DkPluginViewPort* getViewPort();
	void deleteViewPort();

protected:
	DkPluginViewPort* viewport;

protected slots:
	void viewportDestroyed();
};

class DkThresholdViewPort : public DkPluginViewPort {
	Q_OBJECT

public:

	DkThresholdViewPort(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~DkThresholdViewPort();

	bool isCanceled();
	QImage getThresholdedImage(bool thrEnabled);
	QImage getOriginalImage();

public slots:
	void setPanning(bool checked);
	void applyChangesAndClose();
	void discardChangesAndClose();
	virtual void setVisible(bool visible);
	void setThrValue(int val);
	void setThrValueUpper(int val);
	void calculateAutoThreshold();
	void setThrChannel(int val);
	void setThrEnabled(bool enabled);

protected:

	enum {
		channel_gray = 0,
		channel_red,
		channel_green,
		channel_blue,

		channel_end,
	};

	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent*event);
	void paintEvent(QPaintEvent *event);
	virtual void init();

	bool cancelTriggered;
	bool panning;
	DkThresholdToolBar* thresholdToolbar;
	QCursor defaultCursor;
	int thrChannel;
	int thrValue;
	int thrValueUpper;
	bool thrEnabled;
	QImage origImg;
	bool origImgSet;
};


class DkThresholdToolBar : public QToolBar {
	Q_OBJECT


public:

	enum {
		apply_icon = 0,
		cancel_icon,
		pan_icon,

		icons_end,
	};

	DkThresholdToolBar(const QString & title, QWidget * parent = 0);
	virtual ~DkThresholdToolBar();

	void disableColorChannels();
	void setThrValue(int val);


public slots:
	void on_applyAction_triggered();
	void on_cancelAction_triggered();
	void on_panAction_toggled(bool checked);
	void on_thrValBox_valueChanged(int val);
	void on_thrValUpperBox_valueChanged(int val);
	void on_thrChannelBox_currentIndexChanged(int val);
	void on_thrEnableBox_stateChanged(int val);
	void on_autoThrButton_clicked();
	virtual void setVisible(bool visible);
	void setBoxMinimumValue(int val);

signals:
	void applySignal();
	void cancelSignal();
	void thrValSignal(int val);
	void thrValUpperSignal(int val);
	void calculateAutoThresholdSignal();
	void thrChannelSignal(int val);
	void thrEnabledSignal(bool enabled);
	void panSignal(bool checked);

protected:
	void createLayout();
	void createIcons();

	QSpinBox* thrValBox;
	QSpinBox* thrValUpperBox;
	QSlider* thrValSlider;
	QComboBox* thrChannelBox;
	QCheckBox* thrEnableBox;
	QListWidget* thrChannelBoxContents;
	QPushButton* autoThrButton;

	QAction* panAction;
	QVector<QIcon> icons;		// needed for colorizing

};


};
