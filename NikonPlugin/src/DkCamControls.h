/*******************************************************************************************************
 DkPaintPlugin.h
 Created on:	14.07.2013

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

#include <QObject>
#include <QtPlugin>
#include <QImage>
#include <QStringList>
#include <QString>
#include <QMessageBox>
#include <QAction>
#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>
#include <QToolbar>
#include <QMainWindow>
#include <QColorDialog>
#include <QSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QListWidgetItem>
#include <QDialogButtonBox>
#include <QLabel>
#include <QProgressBar>
#include <QApplication>
#include <QInputDialog>

#include <QDockWidget>

#include <functional>
#include <memory>
#include <set>

#include "DkPluginInterface.h"
#include "DkSettings.h"
#include "DkUtils.h"
#include "DkBaseViewport.h"
#include "DkImageStorage.h"
#include "MaidFacade.h"
#include "DkNoMacs.h"

namespace nmc {

class DkPaintToolBar;
class ConnectDeviceDialog;
class OpenDeviceProgressDialog;
class OpenDeviceThread;

class DkCamControls : public QDockWidget {
	Q_OBJECT

public:
	DkCamControls(MaidFacade* maidFacade, const QString& title, QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~DkCamControls();

	void capabilityValueChanged(unsigned int capId);
	void setVisible(bool visible);
	bool isConnected();
	bool isLiveViewActive();
	bool isShootActive();

signals:
	void statusChanged();
	void updateImage(QImage image);
	void loadFile(QFileInfo file);
	void closeSignal();

public slots:
	void connectDevice();
	void onAutoFocus();
	void onShoot();
	void onShootAf();
	void onAcquireStart();
	void onShootFinished();
	void onLiveView();
	void onDeviceOpened();
	void onOpenDeviceError();
	void stopActivities();
	void onUpdateAcquireProgress(unsigned int done, unsigned int total);

protected slots:
	void stateUpdate();
	void updateLiveViewImage();
	void onComboActivated(int);
	void onProfilesComboIndexChanged(int);
	void onExposureModeActivated(int index);
	void arrangeLayout(Qt::DockWidgetArea location = Qt::DockWidgetArea::NoDockWidgetArea);
	void loadProfile();
	void saveProfile();
	void deleteProfile();
	void newProfile();
	void onSaveNamesCheckBoxChanged(int state);

protected:
	struct Profile {
		QString name;
		bool lensAttached;
		QString exposureMode;
		QString aperture;
		QString sensitivity;
		QString shutterSpeed;
	};

	void showEvent(QShowEvent *event);
	void closeEvent(QCloseEvent *event);
	void resizeEvent(QResizeEvent *event);
	void closeMaid();

	void createLayout();
	void updateLensAttachedLabel(bool attached);
	void updateAutoIsoLabel();
	void updateUiValues();
	void updateExposureModeDependentUiValues();
	void updateAperture();
	void updateSensitivity();
	void updateShutterSpeed();
	void updateExposureMode();
	void updateProfilesUi();
	void setConnected(bool connected);
	void closeDeviceAndSetState();
	void shoot(bool withAf = false);
	void setAperture(const int index, int fallback = -1);
	void setShutterSpeed(const int index, int fallback = -1);
	void setSensitivity(const int index, int fallback = -1);
	void setExposureMode(const int index, int fallback = -1);
	void setCameraComboBoxValue(QComboBox* comboBox, std::function<bool(size_t)> setCameraValue, std::function<void()> onSuccess, const int index, int fallback = -1);
	Profile createProfileFromCurrent(const QString& name);
	void writeProfiles();
	void readProfiles();
	void addProfilesComboItem(const Profile& p);
	void updateWidgetSize();

	static const int stateRefreshInterval;
	static const int liveViewImageInterval;
	static const int horizontalItemSpacing;
	static const QString profilesFileName;
	MaidFacade* maidFacade;
	bool connected;
	bool liveViewActive;
	bool shootActive;
	std::unique_ptr<ConnectDeviceDialog> connectDeviceDialog;
	std::unique_ptr<OpenDeviceProgressDialog> openDeviceProgressDialog;
	std::unique_ptr<OpenDeviceThread> openDeviceThread;
	std::unique_ptr<QTimer> stateUpdateTimer;
	std::unique_ptr<QTimer> liveViewTimer;
	std::set<uint32_t> deviceIds;
	std::pair<uint32_t, bool> connectedDeviceId;
	QList<Profile> profiles;

	QWidget* widget;
	QWidget* filePathWidget;
	QGroupBox* profilesGroup;
	QGroupBox* mainGroup;
	QGroupBox* optionsGroup;
	QBoxLayout* mainLayout;
	QBoxLayout* optionsLayout;
	QBoxLayout* outerLayout;
	QHBoxLayout* profilesLayout;
	QLabel* lensAttachedLabel;
	QLabel* autoIsoLabel;
	QLabel* filePathLabel;
	QCheckBox* saveNamesCheckBox;
	QCheckBox* openImagesCheckBox;
	QComboBox* profilesCombo;
	QComboBox* exposureModeCombo;
	QComboBox* isoCombo;
	QComboBox* apertureCombo;
	QComboBox* shutterSpeedCombo;
	QPushButton* connectButton;
	QPushButton* liveViewButton;
	QPushButton* afButton;
	QPushButton* shootButton;
	QPushButton* shootAfButton;
	QPushButton* newProfileButton;
	QPushButton* deleteProfileButton;
	QPushButton* loadProfileButton;
	QPushButton* saveProfileButton;
	QHBoxLayout* connectionLayout;
	QHBoxLayout* exposureModeLayout;
	QHBoxLayout* apertureLayout;
	QHBoxLayout* isoLayout;
	QHBoxLayout* shutterSpeedLayout;
	QHBoxLayout* buttonsLayout;
	QSpacerItem* boxFillerV;
	QSpacerItem* boxFillerH;
	QProgressBar* acquireProgressBar;

};

class OpenDeviceProgressDialog : public QProgressDialog {
public:
	OpenDeviceProgressDialog(QWidget* parent);
	~OpenDeviceProgressDialog() {}

protected:
	void closeEvent(QCloseEvent* e);
};

class ConnectDeviceDialog : public QDialog {
	Q_OBJECT

public:
	ConnectDeviceDialog(MaidFacade* maidFacade, QWidget *parent = 0);
	virtual ~ConnectDeviceDialog() {}

	std::pair<uint32_t, bool> getSelectedId();
	void updateDevicesList(std::set<uint32_t> deviceIds);

private:
	class DeviceListWidgetItem : public QListWidgetItem {
	public:
		DeviceListWidgetItem(const QString& text, unsigned long id) 
			: QListWidgetItem(text), id(id) {}

		unsigned long getId() {
			return id;
		}
	private:
		unsigned long id;
	};

	void createLayout();

	MaidFacade* maidFacade;
	QVBoxLayout* verticalLayout;
	QListWidget* devicesListWidget;
	QHBoxLayout* hboxLayout;
	QSpacerItem* spacerItem;
	QDialogButtonBox* buttonBox;
};

class OpenDeviceThread : public QThread {
	Q_OBJECT

public:
	OpenDeviceThread(MaidFacade *maidFacade, ULONG deviceId);
	~OpenDeviceThread() {}

signals:
	void error();

protected:
	void run();

private:
	MaidFacade *maidFacade;
	ULONG deviceId;
};

};