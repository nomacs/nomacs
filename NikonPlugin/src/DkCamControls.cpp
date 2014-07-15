/*******************************************************************************************************
 DkPaintPlugin.cpp
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

#include "DkNikonPlugin.h"

#include "MaidError.h"

namespace nmc {

const int DkCamControls::stateRefreshInterval = 1000; // in ms
const int DkCamControls::liveViewImageInterval = 100;
const int DkCamControls::horizontalItemSpacing = 10;
const QString DkCamControls::profilesFileName = "cameraProfiles.txt";

DkCamControls::DkCamControls(MaidFacade* maidFacade, const QString& title, QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */) 
	: QDockWidget(title, parent, flags), maidFacade(maidFacade), connected(false), mainLayout(nullptr), liveViewActive(false), shootActive(false) {

	setObjectName("DkCamControls");
	createLayout();
	setConnected(false);
	updateUiValues();
	maidFacade->setCapValueChangeCallback([&] (unsigned int cap) { capabilityValueChanged(cap); });

	stateUpdateTimer.reset(new QTimer(this));
	connect(stateUpdateTimer.get(), SIGNAL(timeout()), this, SLOT(stateUpdate()));

	liveViewTimer.reset(new QTimer(this));
	connect(liveViewTimer.get(), SIGNAL(timeout()), this, SLOT(updateLiveViewImage()));
}

DkCamControls::~DkCamControls() {

	closeMaid();
}

void DkCamControls::createLayout() {
	widget = new QWidget();

	QWidget* controlGroup = new QWidget(this);
	QHBoxLayout* controlsLayout = new QHBoxLayout(controlGroup); 
	connectButton = new QPushButton(tr("Connect"), controlGroup);
	connectButton->setEnabled(maidFacade->isInitialized());
	connectButton->setChecked(false);
	connectButton->setStatusTip(tr("Connect to a Camera"));
	connect(connectButton, SIGNAL(clicked()), this, SLOT(connectDevice()));

	liveViewButton = new QPushButton(tr("Start Live View"), controlGroup);
	liveViewButton->setStatusTip(tr("Toggles Live View"));
	connect(liveViewButton, SIGNAL(clicked()), this, SLOT(onLiveView()));

	controlsLayout->addWidget(connectButton);
	controlsLayout->addWidget(liveViewButton);

	connectionLayout = new QHBoxLayout();
	lensAttachedLabel = new QLabel(tr("No lens attached"));
	autoIsoLabel = new QLabel(tr("Auto-ISO is activated"));
	autoIsoLabel->setToolTip(tr("The selected ISO value will not be used. Deactivate Auto-ISO in the camera menu."));
	acquireProgressBar = new QProgressBar();
	updateLensAttachedLabel(false);
	updateAutoIsoLabel();
	connectionLayout->addWidget(lensAttachedLabel);
	connectionLayout->addWidget(autoIsoLabel);
	connectionLayout->addWidget(acquireProgressBar);
	acquireProgressBar->setVisible(false);
	acquireProgressBar->setMinimum(0);

	QWidget* exposureModeWidget = new QWidget();
	exposureModeLayout = new QHBoxLayout();
	QLabel* exposureModeLabel = new QLabel(tr("Exposure Mode"));
	exposureModeCombo = new QComboBox();
	exposureModeCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	exposureModeCombo->setObjectName("exposureModeCombo");
	exposureModeCombo->setAccessibleName(tr("Exposure Mode"));
	exposureModeLayout->addWidget(exposureModeLabel);
	exposureModeLayout->addWidget(exposureModeCombo);
	exposureModeWidget->setLayout(exposureModeLayout);

	QWidget* apertureWidget = new QWidget();
	apertureLayout = new QHBoxLayout();
	QLabel* apertureLabel = new QLabel(tr("Aperture"));
	apertureCombo = new QComboBox();
	apertureCombo->setObjectName("apertureCombo");
	apertureCombo->setAccessibleName(tr("Aperture"));
	apertureLayout->addWidget(apertureLabel);
	apertureLayout->addWidget(apertureCombo);
	apertureWidget->setLayout(apertureLayout);

	QWidget* isoWidget = new QWidget();
	isoLayout = new QHBoxLayout();
	QLabel* isoLabel = new QLabel(tr("Sensitivity (ISO)"));
	isoCombo = new QComboBox();
	isoCombo->setObjectName("isoCombo");
	isoCombo->setAccessibleName(tr("ISO Sensitivity"));
	isoLayout->addWidget(isoLabel);
	isoLayout->addWidget(isoCombo);
	isoWidget->setLayout(isoLayout);

	QWidget* shutterSpeedWidget = new QWidget();
	shutterSpeedLayout = new QHBoxLayout();
	QLabel* shutterSpeedLabel = new QLabel(tr("Shutter Speed"));
	shutterSpeedCombo = new QComboBox();
	shutterSpeedCombo->setObjectName("shutterSpeedCombo");
	shutterSpeedCombo->setAccessibleName(tr("Shutter Speed"));
	shutterSpeedLayout->addWidget(shutterSpeedLabel);
	shutterSpeedLayout->addWidget(shutterSpeedCombo);
	shutterSpeedWidget->setLayout(shutterSpeedLayout);

	buttonsLayout = new QHBoxLayout();
	afButton = new QPushButton(tr("AF"));
	afButton->setToolTip(tr("Auto-Focus"));
	shootButton = new QPushButton(tr("Shoot"));
	shootAfButton = new QPushButton(tr("Shoot with AF"));
	buttonsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
	buttonsLayout->addWidget(afButton);
	buttonsLayout->addWidget(shootButton);
	buttonsLayout->addWidget(shootAfButton);
	buttonsLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

	boxFillerV = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
	boxFillerH = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	mainGroup = new QGroupBox(tr("Camera options"));
	mainGroup->setFlat(true);
	mainLayout = new QVBoxLayout();
	mainLayout->addWidget(exposureModeWidget);
	mainLayout->addWidget(apertureWidget);
	mainLayout->addWidget(isoWidget);
	mainLayout->addWidget(shutterSpeedWidget);
	mainLayout->addLayout(buttonsLayout);
	mainLayout->addLayout(connectionLayout);
	mainGroup->setLayout(mainLayout);

	// profiles

	profilesCombo = new QComboBox();
	newProfileButton = new QPushButton();
	newProfileButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder));
	newProfileButton->adjustSize();
	newProfileButton->setToolTip(tr("Create a new Profile for the current settings"));
	deleteProfileButton = new QPushButton();
	deleteProfileButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogDiscardButton));
	deleteProfileButton->adjustSize();
	deleteProfileButton->setToolTip(tr("Delete the selected profile"));
	//loadProfileButton = new QPushButton();
	//loadProfileButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton));
	//loadProfileButton->adjustSize();
	//loadProfileButton->setToolTip(tr("Load and apply the selected profile"));
	saveProfileButton = new QPushButton();
	saveProfileButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
	saveProfileButton->adjustSize();
	saveProfileButton->setToolTip(tr("Save the current settings for the selected profile"));

	profilesGroup = new QGroupBox(tr("Profiles"));
	profilesGroup->setFlat(true);
	profilesLayout = new QHBoxLayout();
	profilesLayout->addWidget(profilesCombo);
	//profilesLayout->addWidget(loadProfileButton);
	profilesLayout->addSpacerItem(new QSpacerItem(10, 1, QSizePolicy::Minimum, QSizePolicy::Minimum));
	profilesLayout->addWidget(saveProfileButton);
	profilesLayout->addWidget(newProfileButton);
	profilesLayout->addWidget(deleteProfileButton);
	profilesLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
	profilesGroup->setLayout(profilesLayout);

	// options group
	
	filePathWidget = new QWidget();
	QLayout* filePathLayout = new QHBoxLayout();
	filePathLabel = new QLabel();
	filePathLabel->setObjectName("filePathLabel");
	filePathLabel->setAccessibleName(tr("Save path"));
	filePathLayout->addWidget(new QLabel(tr("Save path")));
	filePathLayout->addWidget(filePathLabel);
	filePathWidget->setLayout(filePathLayout);
	filePathWidget->setEnabled(false);

	saveNamesCheckBox = new QCheckBox(tr("Name files automatically"));
	saveNamesCheckBox->setChecked(true);
	saveNamesCheckBox->setEnabled(false);

	openImagesCheckBox = new QCheckBox(tr("Load and display images after shooting"));
	openImagesCheckBox->setChecked(true);
	openImagesCheckBox->setEnabled(false);

	optionsGroup = new QGroupBox(tr("Options"));
	optionsGroup->setFlat(true);
	optionsLayout = new QVBoxLayout();
	optionsLayout->addWidget(filePathWidget);
	optionsLayout->addWidget(saveNamesCheckBox);
	optionsLayout->addWidget(openImagesCheckBox);
	optionsGroup->setLayout(optionsLayout);

	// .

	outerLayout = new QVBoxLayout();
	outerLayout->addWidget(controlGroup);
	outerLayout->addWidget(profilesGroup);
	outerLayout->addWidget(mainGroup);
	outerLayout->addWidget(optionsGroup);
	outerLayout->addSpacerItem(boxFillerV);

	widget->setLayout(outerLayout);
	setWidget(widget);

	updateProfilesUi();

	// connect signals
	connect(afButton, SIGNAL(clicked()), this, SLOT(onAutoFocus()));
	connect(shootButton, SIGNAL(clicked()), this, SLOT(onShoot()));
	connect(shootAfButton, SIGNAL(clicked()), this, SLOT(onShootAf()));
	//connect(loadProfileButton, SIGNAL(clicked()), this, SLOT(loadProfile()));
	connect(saveProfileButton, SIGNAL(clicked()), this, SLOT(saveProfile()));
	connect(newProfileButton, SIGNAL(clicked()), this, SLOT(newProfile()));
	connect(deleteProfileButton, SIGNAL(clicked()), this, SLOT(deleteProfile()));
	connect(profilesCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onProfilesComboIndexChanged(int)));
	connect(saveNamesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onSaveNamesCheckBoxChanged(int)));
	connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(arrangeLayout(Qt::DockWidgetArea)));
	connect(this, SIGNAL(topLevelChanged(bool)), this, SLOT(arrangeLayout()));
	// maidFacade signals
	connect(maidFacade, SIGNAL(acquireStart()), this, SLOT(onAcquireStart()));
	connect(maidFacade, SIGNAL(shootAndAcquireFinished()), this, SLOT(onShootFinished()));
	connect(maidFacade, SIGNAL(updateAcquireProgress(unsigned int, unsigned int)), this, SLOT(onUpdateAcquireProgress(unsigned int, unsigned int)));

	readProfiles();
}

void DkCamControls::arrangeLayout(Qt::DockWidgetArea location) {
	if (location == Qt::DockWidgetArea::TopDockWidgetArea || location == Qt::DockWidgetArea::BottomDockWidgetArea) {
		mainLayout->setDirection(QBoxLayout::Direction::LeftToRight);
	} else { //if (location == Qt::DockWidgetArea::LeftDockWidgetArea || location == Qt::DockWidgetArea::RightDockWidgetArea) {
		mainLayout->setDirection(QBoxLayout::Direction::TopToBottom);
	}
}

void DkCamControls::connectDevice() {
	
	if (connected) {
		closeDeviceAndSetState();
		return;
	}

	connectDeviceDialog.reset(new ConnectDeviceDialog(maidFacade, this));
	stateUpdate(); // avoid delay
	if (connectDeviceDialog->exec() == QDialog::Accepted && connectDeviceDialog->getSelectedId().second) {
		try {
			openDeviceProgressDialog.reset(new OpenDeviceProgressDialog(this));

			connectedDeviceId = connectDeviceDialog->getSelectedId();
			openDeviceThread.reset(new OpenDeviceThread(maidFacade, connectedDeviceId.first));
			connect(openDeviceThread.get(), SIGNAL(finished()), this, SLOT(onDeviceOpened()));
			connect(openDeviceThread.get(), SIGNAL(error()), this, SLOT(onOpenDeviceError()));
			openDeviceThread->start();
			openDeviceProgressDialog->exec();
		} catch (Maid::MaidError e) {
			onOpenDeviceError();
		}
	}
}

void DkCamControls::onDeviceOpened() {
	
	if (openDeviceProgressDialog) {
		openDeviceProgressDialog->cancel();
	}

	try {
		if (!maidFacade->isSourceAlive()) {
			return;
		}

		if (!maidFacade->checkCameraType()) {
			QMessageBox dialog(this);
			dialog.setIcon(QMessageBox::Warning);
			dialog.setText(tr("This program is not compatible with the selected camera model."));
			dialog.show();
			dialog.exec();

			//qDebug() << tr("This program is not compatible with the selected camera model.");
			closeDeviceAndSetState();
			return;
		}

		setConnected(true);
		updateUiValues();
	} catch (Maid::MaidError e) {
		qDebug() << tr("[ERROR] after opening device");
	}
}

void DkCamControls::onOpenDeviceError() {
	QMessageBox dialog(this);
	dialog.setIcon(QMessageBox::Warning);
	dialog.setText(tr("The source could not be opened"));
	dialog.show();
	dialog.exec();

	//qDebug() << tr("The source could not be opened");
}

void DkCamControls::onSaveNamesCheckBoxChanged(int state) {
	maidFacade->setAutoSaveNaming(state != 0);
}

void DkCamControls::setConnected(bool newValue) {
	connected = newValue;

	if (!connected) {
		connectButton->setText(tr("Connect"));
		connectButton->setChecked(false);
	}
	else {
		connectButton->setText(tr("Disconnect"));
		connectButton->setChecked(true);
	}

	emit statusChanged();
}

bool DkCamControls::isConnected() {
	return connected;
}

bool DkCamControls::isLiveViewActive() {
	return liveViewActive;
}

bool DkCamControls::isShootActive() {
	return shootActive;
}

void DkCamControls::stateUpdate() {
	auto prevDeviceIds = deviceIds;
	bool isSourceAlive = false;
	try {
		deviceIds = maidFacade->listDevices();
	} catch (Maid::MaidError) {
		qDebug() << "listing devices failed";
	}

	if (connected && connectedDeviceId.second) {
		try {
			isSourceAlive = maidFacade->isSourceAlive();
		} catch (Maid::MaidError) {
			qDebug() << "could not read whether source is alive";
			isSourceAlive = false;
		}

		// device disconnected?
		if (deviceIds.find(connectedDeviceId.first) != deviceIds.end() && !isSourceAlive) {
			closeDeviceAndSetState();
		} else {
			// update live view status
			bool newLiveViewStatus;
			try {
				newLiveViewStatus = maidFacade->isLiveViewActive();
			} catch (Maid::MaidError) {
				// reading capability did not work, camera may have been disconnected
				newLiveViewStatus = false;
			}
			if (newLiveViewStatus != liveViewActive) {
				liveViewActive = newLiveViewStatus;

				// start or stop transmitting and displaying images
				if (liveViewActive) {
					liveViewTimer->start(liveViewImageInterval);
				} else {
					liveViewTimer->stop();
				}

				updateUiValues();
				emit statusChanged();
			}
		}
	}

	if (deviceIds.size() != prevDeviceIds.size() || 
		!std::equal(deviceIds.begin(), deviceIds.end(), prevDeviceIds.begin())) {

		if (connected && connectedDeviceId.second) {
			if (deviceIds.find(connectedDeviceId.first) == deviceIds.end()) {
				closeDeviceAndSetState();
			}
		}	
	}

	// update gui list
	if (connectDeviceDialog) {
		connectDeviceDialog->updateDevicesList(deviceIds);
	}
}

void DkCamControls::updateLiveViewImage() {
	stateUpdate();

	try {
		// TODO: get the plugin viewport here?!
		//viewport->setImage(maidFacade->getLiveViewImage());
		emit updateImage(maidFacade->getLiveViewImage());
		qDebug() << "live view image updated...";

	} catch (Maid::MaidError) {
		// do nothing
	}
}

void DkCamControls::closeDeviceAndSetState() {
	bool lvActive = false;
	try {
		lvActive = maidFacade->isLiveViewActive();
	} catch (Maid::MaidError) {
		lvActive = false;
	}

	if (lvActive) {
		maidFacade->toggleLiveView();
		liveViewTimer->stop();
		
		// TODO: get the plugin viewport here?!
		//viewport->setImage(QImage());
		//emit updateImage(QImage());	// is setting an empty image here intentional?
	}

	try {
		maidFacade->closeSource();
	} catch (Maid::MaidError) {
		// there is probably nothing left to close
	}
	connectedDeviceId.second = false;
	setConnected(false);
	updateUiValues();
}

void DkCamControls::stopActivities() {
	if (stateUpdateTimer) {
		stateUpdateTimer->stop();
	}
	if (liveViewTimer) {
		liveViewTimer->stop();
	}
}

void DkCamControls::showEvent(QShowEvent *event) {
	stateUpdateTimer->start(stateRefreshInterval);
}

void DkCamControls::closeEvent(QCloseEvent* event) {

	closeMaid();
	emit closeSignal();

}

void DkCamControls::closeMaid() {

	// something to do here?
	if (maidFacade->isClosed())
		return;

	//if (connected && maidFacade->isLiveViewActive())
	//	onLiveView();

	stopActivities();

	try {
		maidFacade->closeEverything();
	} catch (Maid::MaidError) {
		// hopefully nothing left to close
		qDebug() << "unable to close maid objects and module";
	}
}

void DkCamControls::resizeEvent(QResizeEvent *event) {
	updateWidgetSize();
	QWidget::resizeEvent(event);
}

void DkCamControls::setVisible(bool visible) {
	if (!visible) {
		stopActivities();
	}
	QDockWidget::setVisible(visible);
}

void DkCamControls::updateWidgetSize() {
	QString savePath = maidFacade->getCurrentSavePath();
	if (savePath.isEmpty()) {
		filePathLabel->setText(tr("-"));
	} else {
		QFontMetricsF fontMetrics(filePathLabel->font());
		filePathLabel->setText(fontMetrics.elidedText(savePath, Qt::TextElideMode::ElideLeft, exposureModeCombo->width()));
		filePathLabel->setToolTip(savePath);
	}
}

void DkCamControls::updateLensAttachedLabel(bool attached) {
	if (attached || !connected) {
		lensAttachedLabel->setVisible(false);
	} else {
		lensAttachedLabel->setVisible(true);
	}
}

void DkCamControls::updateAutoIsoLabel() {
	if (!connected) {
		autoIsoLabel->setVisible(false);
	} else {
		try {
			autoIsoLabel->setVisible(maidFacade->isAutoIso());
		} catch (Maid::MaidError) {
			qDebug() << "could not read auto iso (IsoControl) value";
		}
	}
}


void DkCamControls::updateUiValues() {
	// exposure mode first
	updateExposureMode();
	updateExposureModeDependentUiValues();
	updateAutoIsoLabel();

	filePathWidget->setEnabled(connected);
	saveNamesCheckBox->setEnabled(connected);
	openImagesCheckBox->setEnabled(connected);

	if (liveViewActive) {
		shootAfButton->setEnabled(false);
		liveViewButton->setText(tr("Stop Live View"));

		afButton->setEnabled(true);
	} else {
		mainGroup->setEnabled(connected);
		profilesGroup->setEnabled(connected);
		shootAfButton->setEnabled(connected);
		shootButton->setEnabled(connected);
		afButton->setEnabled(connected);
		liveViewButton->setEnabled(connected);
		liveViewButton->setText(tr("Start Live View"));
	}
}

void DkCamControls::updateExposureModeDependentUiValues() {
	updateAperture();
	updateSensitivity();
	updateShutterSpeed();
}

void DkCamControls::capabilityValueChanged(uint32_t capId) {
	switch (capId) {
	case kNkMAIDCapability_Sensitivity:
		updateSensitivity();
		break;
	case kNkMAIDCapability_Aperture:
		updateAperture();
		break;
	case kNkMAIDCapability_ShutterSpeed:
		updateShutterSpeed();
		break;
	case kNkMAIDCapability_ExposureMode:
		updateExposureMode();
		updateExposureModeDependentUiValues();
		break;
	case kNkMAIDCapability_IsoControl:
		updateAutoIsoLabel();
		break;
	}
}

void DkCamControls::onComboActivated(int index) {
	
	if (QObject::sender() == apertureCombo) {
		MaidFacade::MaybeStringValues aperture = maidFacade->getAperture();
		if (aperture.second && index != aperture.first.currentValue) {
			setAperture(apertureCombo->currentIndex(), aperture.first.currentValue);
		}
	} else if (QObject::sender() == isoCombo) {
		MaidFacade::MaybeStringValues sensitivity = maidFacade->getSensitivity();
		if (sensitivity.second && index != sensitivity.first.currentValue) {
			setSensitivity(isoCombo->currentIndex(), sensitivity.first.currentValue);
		}
	} else if (QObject::sender() == shutterSpeedCombo) {
		MaidFacade::MaybeStringValues shutterSpeed = maidFacade->getShutterSpeed();
		if (shutterSpeed.second && index != shutterSpeed.first.currentValue) {
			setShutterSpeed(shutterSpeedCombo->currentIndex(), shutterSpeed.first.currentValue);
		}
	}
}

void DkCamControls::onProfilesComboIndexChanged(int index) {
	
	updateProfilesUi();
}

void DkCamControls::onExposureModeActivated(int index) {

	if (index <= -1) {
		return;
	}

	MaidFacade::MaybeUnsignedValues exposureMode = maidFacade->getExposureMode();
	if (exposureMode.second && index != exposureMode.first.currentValue) {
		setExposureMode(index, exposureMode.first.currentValue);
	}
}

void DkCamControls::setExposureMode(const int index, int fallback) {
	setCameraComboBoxValue(exposureModeCombo, 
		[&] (size_t v) { return maidFacade->setExposureMode(v); }, 
		[&] () { updateExposureModeDependentUiValues(); }, 
		index, 
		fallback);
}

void DkCamControls::setAperture(const int index, int fallback) {
	setCameraComboBoxValue(apertureCombo, 
		[&] (size_t v) { return maidFacade->setAperture(v); }, 
		[&] () {}, 
		index, 
		fallback);
}

void DkCamControls::setShutterSpeed(const int index, int fallback) {
	setCameraComboBoxValue(shutterSpeedCombo, 
		[&] (size_t v) { return maidFacade->setShutterSpeed(v); }, 
		[&] () {}, 
		index, 
		fallback);
}

void DkCamControls::setSensitivity(const int index, int fallback) {
	setCameraComboBoxValue(isoCombo, 
		[&] (size_t v) { return maidFacade->setSensitivity(v); }, 
		[&] () {}, 
		index, 
		fallback);
}

void DkCamControls::setCameraComboBoxValue(QComboBox* comboBox, std::function<bool(size_t)> setCameraValue, std::function<void()> onSuccess, const int index, int fallback) {
	if (index == -1) {
		return;
	}
	if (fallback == -1) {
		fallback = apertureCombo->currentIndex();
	}

	bool r = false;
	try {
		r = setCameraValue(index);
	} catch (Maid::MaidError) {
		r = false;
	}

	if (r) {
		if (comboBox->currentIndex() != index) {
			comboBox->setCurrentIndex(index);
		}
		onSuccess();
	} else {
		QMessageBox dialog(this);
		dialog.setIcon(QMessageBox::Warning);
		dialog.setText(tr("Value could not be set: ") + comboBox->accessibleName());
		dialog.show();
		dialog.exec();

		//qDebug() << tr("Aperture value could not be set");
		comboBox->setCurrentIndex(fallback);
	}
}

void DkCamControls::updateAperture() {
	
	if (!connected) {
		apertureCombo->setEnabled(false);
		return;
	}

	MaidFacade::MaybeStringValues aperture;
	MaidFacade::MaybeUnsignedValues exposureMode;
	try {
		aperture = maidFacade->readAperture();
		exposureMode = maidFacade->getExposureMode();
	} catch (Maid::MaidError) {
		qDebug() << "error reading aperture";
	}

	apertureCombo->clear();
	if (aperture.second && exposureMode.second) {
		auto valueData = maidFacade->toQStringList(aperture.first);
		const QStringList& values = valueData.first;
		const size_t& currentIndex = valueData.second;
		apertureCombo->insertItems(0, values);
		apertureCombo->setCurrentIndex(currentIndex);

		if (exposureMode.first.currentValue != kNkMAIDExposureMode_SpeedPriority &&
				exposureMode.first.currentValue != kNkMAIDExposureMode_Program) {

			connect(apertureCombo, SIGNAL(activated(int)), this, SLOT(onComboActivated(int)));
			apertureCombo->setEnabled(true);
		}
	}

	if (!aperture.second || !exposureMode.second ||
			exposureMode.first.currentValue == kNkMAIDExposureMode_SpeedPriority ||
			exposureMode.first.currentValue == kNkMAIDExposureMode_Program) {

		disconnect(apertureCombo, SIGNAL(activated(int)), this, SLOT(onComboActivated(int)));
		apertureCombo->setEnabled(false);
	}
}

void DkCamControls::updateSensitivity() {
	if (!connected) {
		isoCombo->setEnabled(false);
		return;
	}

	MaidFacade::MaybeStringValues sensitivity;
	try {
		sensitivity = maidFacade->readSensitivity();
	} catch (Maid::MaidError) {
		qDebug() << "error reading ISO sensitivity";
	}

	isoCombo->clear();
	if (sensitivity.second) {
		auto valueData = maidFacade->toQStringList(sensitivity.first);
		const QStringList& values = valueData.first;
		const size_t& currentIndex = valueData.second;
		isoCombo->insertItems(0, values);
		isoCombo->setCurrentIndex(currentIndex);

		connect(isoCombo, SIGNAL(activated(int)), this, SLOT(onComboActivated(int)));
		isoCombo->setEnabled(true);
	} else {
		disconnect(isoCombo, SIGNAL(activated(int)), this, SLOT(onComboActivated(int)));
		isoCombo->setEnabled(false);
	}
}

void DkCamControls::updateShutterSpeed() {
	if (!connected) {
		shutterSpeedCombo->setEnabled(false);
		return;
	}

	MaidFacade::MaybeStringValues shutterSpeed;
	MaidFacade::MaybeUnsignedValues exposureMode;
	try {
		shutterSpeed = maidFacade->readShutterSpeed();
		exposureMode = maidFacade->getExposureMode();
	} catch (Maid::MaidError) {
		qDebug() << "error reading shutter speed or exposure mode";
	};

	shutterSpeedCombo->clear();
	if (shutterSpeed.second && exposureMode.second) {
		auto valueData = maidFacade->toQStringList(shutterSpeed.first);
		const QStringList& values = valueData.first;
		const size_t& currentIndex = valueData.second;
		shutterSpeedCombo->insertItems(0, values);
		shutterSpeedCombo->setCurrentIndex(currentIndex);

		if (exposureMode.first.currentValue != kNkMAIDExposureMode_AperturePriority &&
				exposureMode.first.currentValue != kNkMAIDExposureMode_Program) {

			connect(shutterSpeedCombo, SIGNAL(activated(int)), this, SLOT(onComboActivated(int)));
			shutterSpeedCombo->setEnabled(true);
		}
	}

	if (!shutterSpeed.second || !exposureMode.second ||
			exposureMode.first.currentValue == kNkMAIDExposureMode_AperturePriority ||
			exposureMode.first.currentValue == kNkMAIDExposureMode_Program) {

		disconnect(shutterSpeedCombo, SIGNAL(activated(int)), this, SLOT(onComboActivated(int)));
		shutterSpeedCombo->setEnabled(false);
	}
}

void DkCamControls::updateExposureMode() {
	if (!connected) {
		exposureModeCombo->setEnabled(false);
		return;
	}

	MaidFacade::MaybeUnsignedValues exposureMode;
	try {
		exposureMode = maidFacade->readExposureMode();
	} catch (Maid::MaidError) {
		qDebug() << "error reading exposure mode";
	}
	
	if (exposureMode.second) {
		exposureModeCombo->clear();
		for (auto v : exposureMode.first.values) {
			switch (v) {
			case kNkMAIDExposureMode_Program:
				exposureModeCombo->addItem(tr("[P] Program Mode"));
				break;
			case kNkMAIDExposureMode_AperturePriority:
				exposureModeCombo->addItem(tr("[A] Aperture Priority"));
				break;
			case kNkMAIDExposureMode_SpeedPriority:
				exposureModeCombo->addItem(tr("[S] Speed Priority"));
				break;
			case kNkMAIDExposureMode_Manual:
				exposureModeCombo->addItem(tr("[M] Manual"));
				break;
			}
		}
		exposureModeCombo->setCurrentIndex(exposureMode.first.currentValue);

		// update lens state
		updateLensAttachedLabel(maidFacade->isLensAttached());

		connect(exposureModeCombo, SIGNAL(activated(int)), this, SLOT(onExposureModeActivated(int)));
		exposureModeCombo->setEnabled(true);
	} else {
		disconnect(exposureModeCombo, SIGNAL(activated(int)), this, SLOT(onExposureModeActivated(int)));
		exposureModeCombo->setEnabled(false);
	}
}

void DkCamControls::updateProfilesUi() {
	if (profilesCombo->currentIndex() == -1 || profilesCombo->count() == 0) {
		//loadProfileButton->setEnabled(false);
		saveProfileButton->setEnabled(false);
		deleteProfileButton->setEnabled(false);
	} else {
		//loadProfileButton->setEnabled(true);
		saveProfileButton->setEnabled(true);
		deleteProfileButton->setEnabled(true);
	}
	loadProfile();

	newProfileButton->setEnabled(true);
}

void DkCamControls::onAutoFocus() {
	try {
		maidFacade->autoFocus();
	} catch (Maid::MaidError) {
		qDebug() << tr("error during auto-focus");
	}
}

void DkCamControls::onShoot() {
	shoot(false);
}

void DkCamControls::onShootAf() {
	shoot(true);
}

void DkCamControls::shoot(bool withAf) {
	
	try {
		maidFacade->shoot(withAf);
	} catch (Maid::MaidError e) {
		QMessageBox dialog(this);
		dialog.setIcon(QMessageBox::Warning);
		dialog.setText(tr("Could not capture image with AF"));
		dialog.show();
		dialog.exec();

		//qDebug() << tr("Could not capture image");
	}
}

void DkCamControls::onAcquireStart() {
	mainGroup->setEnabled(false);
	profilesGroup->setEnabled(false);
	liveViewButton->setEnabled(false);

	shootActive = true;
	emit statusChanged();
	qApp->processEvents();
}

void DkCamControls::onShootFinished() {
	shootActive = false;
	mainGroup->setEnabled(true);
	profilesGroup->setEnabled(true);
	liveViewButton->setEnabled(true);
	acquireProgressBar->setVisible(false);
	emit statusChanged();

	QString savePath = maidFacade->getCurrentSavePath();
	updateWidgetSize();
	filePathWidget->setEnabled(!savePath.isEmpty());

	if (openImagesCheckBox->isChecked()) {
		emit loadFile(maidFacade->getLastFileInfo());
	}

}

void DkCamControls::onUpdateAcquireProgress(unsigned int done, unsigned int total) {
	if (!acquireProgressBar->isVisible()) {
		acquireProgressBar->setVisible(true);
		acquireProgressBar->setMaximum(total);
	}

	acquireProgressBar->setValue(done);
}

void DkCamControls::onLiveView() {
	try {
		maidFacade->toggleLiveView();
		stateUpdate();

	} catch (Maid::MaidError e) {
		QMessageBox dialog(this);
		dialog.setIcon(QMessageBox::Warning);
		dialog.setText(tr("Could not start/stop live view"));
		dialog.show();
		dialog.exec();

		//qDebug() << tr("Could not capture image with AF");
	}
}

// profiles

void DkCamControls::loadProfile() {

	if (profilesCombo->currentIndex() >= profiles.size() || profilesCombo->currentIndex() < 0)
		return;

	const Profile& p = profiles.at(profilesCombo->currentIndex());
	const QString unequalItemsText = tr("Could not apply profile because a value from the profile was not available");
	QString errorText;

	if (!maidFacade->isSourceAlive())
		return;

	bool lensAttached = maidFacade->isLensAttached();
	if (p.lensAttached != lensAttached || exposureModeCombo->findText(p.exposureMode) == -1) {
		if (lensAttached) {
			errorText = tr("Could not apply profile because it was made without the lens attached");
		} else {
			errorText = tr("Could not apply profile because it was made with the lens attached");
		}
	} 

	if (!errorText.isEmpty()) {
		QMessageBox dialog(this);
		dialog.setIcon(QMessageBox::Warning);
		dialog.setText(errorText);
		dialog.show();
		dialog.exec();
		return;
	}

	// the exposure mode has to be set first, it determines the values for the other settings
	const int prevExposureModeIndex = exposureModeCombo->currentIndex();
	setExposureMode(exposureModeCombo->findText(p.exposureMode));
 
	const int apertureIndex = apertureCombo->findText(p.aperture);
	const int sensitivityIndex = isoCombo->findText(p.sensitivity);
	const int shutterSpeedIndex = shutterSpeedCombo->findText(p.shutterSpeed);

	if (apertureIndex == -1 && !p.aperture.isEmpty() 
		|| sensitivityIndex == -1 && !p.sensitivity.isEmpty() 
		|| shutterSpeedIndex == -1 && !p.shutterSpeed.isEmpty()) {

		errorText = unequalItemsText;
	}

	if (!errorText.isEmpty()) {
		setExposureMode(prevExposureModeIndex);

		QMessageBox dialog(this);
		dialog.setIcon(QMessageBox::Warning);
		dialog.setText(errorText);
		dialog.show();
		dialog.exec();
		return;
	}

	if (!p.aperture.isEmpty()) {
		setAperture(apertureIndex);
	}
	if (!p.sensitivity.isEmpty()) {
		setSensitivity(sensitivityIndex);
	}
	if (!p.shutterSpeed.isEmpty()) {
		setShutterSpeed(shutterSpeedIndex);
	}
	
}

void DkCamControls::saveProfile() {
	const int currentIndex = profilesCombo->currentIndex();
	if (currentIndex > -1) {
		Profile p = createProfileFromCurrent(profiles.at(currentIndex).name);
		profiles.replace(currentIndex, p);
		writeProfiles();
	}
}

void DkCamControls::deleteProfile() {
	
	const int& currentIndex = profilesCombo->currentIndex();
	if (currentIndex > -1 && profiles.size() > 0) {
		auto answer = QMessageBox::question(
			this, 
			tr("Delete profile"), 
			tr("Do you really want to delete the selected profile?"), 
			QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No)
			);
		if (answer != QMessageBox::Yes) {
			return;
		}

		profiles.erase(profiles.begin() + currentIndex);
		profilesCombo->removeItem(currentIndex);

		updateProfilesUi();
	}
}

void DkCamControls::newProfile() {
	const QString name = QInputDialog::getText(this, tr("New profile"), tr("Enter a name for the new profile"), QLineEdit::Normal);
	if (name.isEmpty()) {
		return;
	}

	Profile p = createProfileFromCurrent(name);
	profiles.append(p);

	writeProfiles();
	
	addProfilesComboItem(p);
	profilesCombo->setCurrentIndex(profiles.size() - 1);
}

void DkCamControls::addProfilesComboItem(const Profile& p) {
	QString name = p.name;
	if (!p.lensAttached) {
		name = "(" + name + ")";
	}
	profilesCombo->addItem(name);
}

DkCamControls::Profile DkCamControls::createProfileFromCurrent(const QString& name) {
	Profile p;
	p.name = name;

	auto setProfileValue = [&] (QString& field, QComboBox* comboBox) {
		if (comboBox->isEnabled()) {
			field = comboBox->currentText();
		} else {
			field = "";
		}
	};

	p.lensAttached = maidFacade->isLensAttached();
	setProfileValue(p.exposureMode, exposureModeCombo);
	setProfileValue(p.aperture, apertureCombo);
	setProfileValue(p.sensitivity, isoCombo);
	setProfileValue(p.shutterSpeed, shutterSpeedCombo);

	return p;
}

/**
 * Write the profiles to a file
 */
void DkCamControls::writeProfiles() {
	QFile file(profilesFileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox dialog(this);
		dialog.setIcon(QMessageBox::Warning);
		dialog.setText(tr("The profiles file could not be opened for writing."));
		dialog.show();
		dialog.exec();
		qDebug() << profilesFileName << " could not be opened for writing.";
		return;
	}

	QTextStream stream(&file);
	for (const Profile& p : profiles) {
		QStringList list;
		list
			<< p.name
			<< QString::number(p.lensAttached)
			<< p.exposureMode
			<< p.aperture
			<< p.sensitivity
			<< p.shutterSpeed;

		stream << list.join(";") << "\n";
	}

	file.close();
}

/**
 * Reads the profiles from the file and updates the combo box
 */
void DkCamControls::readProfiles() {
	QFile file(profilesFileName);
	if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
		QMessageBox dialog(this);
		dialog.setIcon(QMessageBox::Warning);
		dialog.setText(tr("The profiles file could not be opened for reading."));
		dialog.show();
		dialog.exec();
		qDebug() << profilesFileName << " could not be opened for reading.";
		return;
	}

	QStringList fields;
	QTextStream in(&file);
	profiles.clear();
	profilesCombo->clear();
	int i = 0;
	while (!in.atEnd()) {
		Profile p;
		i = 0;
		
		fields = in.readLine().split(";");
		p.name = fields.at(i);
		p.lensAttached = fields.at(++i).toInt();
		p.exposureMode = fields.at(++i);
		p.aperture = fields.at(++i);
		p.sensitivity = fields.at(++i);
		p.shutterSpeed = fields.at(++i);

		profiles.append(p);
		addProfilesComboItem(p);
	}

	profilesCombo->setCurrentIndex(-1);
	file.close();
}

ConnectDeviceDialog::ConnectDeviceDialog(MaidFacade* maidFacade, QWidget* parent)
	: QDialog(parent), maidFacade(maidFacade) {

		createLayout();
}

void ConnectDeviceDialog::createLayout() {
	//setObjectName(QStringLiteral("ConnectDeviceDialog"));
	setWindowTitle(tr("nomacs - Select Device"));

	setWindowModality(Qt::WindowModal);
	resize(400, 300);
	verticalLayout = new QVBoxLayout();
	//verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
	devicesListWidget = new QListWidget();
	//devicesListWidget->setObjectName(QStringLiteral("devicesListWidget"));

	verticalLayout->addWidget(devicesListWidget);

	hboxLayout = new QHBoxLayout();
	hboxLayout->setSpacing(6);
	//hboxLayout->setObjectName(QStringLiteral("hboxLayout"));
	hboxLayout->setContentsMargins(0, 0, 0, 0);
	spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

	hboxLayout->addItem(spacerItem);
	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	hboxLayout->addWidget(buttonBox);

	verticalLayout->addLayout(hboxLayout);

	setLayout(verticalLayout);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

std::pair<uint32_t, bool> ConnectDeviceDialog::getSelectedId() {
	auto selectedItems = devicesListWidget->selectedItems();
	std::pair<uint32_t, bool> v;
	if (selectedItems.isEmpty()) {
		v.first = -1;
		v.second = false;
	} else {
		v.first = static_cast<DeviceListWidgetItem*>(selectedItems.first())->getId();
		v.second = true;
	}

	return v;
}

void ConnectDeviceDialog::updateDevicesList(std::set<uint32_t> deviceIds) {
	if (deviceIds.size() != devicesListWidget->count()) {
		devicesListWidget->clear();

		if (deviceIds.size() > 0) {
			for (uint32_t deviceId : deviceIds) {
				devicesListWidget->addItem(new DeviceListWidgetItem(QString("Nikon D4 #%1").arg(deviceId), deviceId));
			}

			devicesListWidget->item(0)->setSelected(true);
		}
	}
}

OpenDeviceThread::OpenDeviceThread(MaidFacade *maidFacade, ULONG deviceId) 
	: maidFacade(maidFacade), deviceId(deviceId) {
}

void OpenDeviceThread::run() {
	try {
		maidFacade->openSource(deviceId);
	} catch (Maid::MaidError) {
		emit error();
	}
}

OpenDeviceProgressDialog::OpenDeviceProgressDialog(QWidget* parent)
	: QProgressDialog(tr("Opening Device. This can take up to several minutes."), tr("Cancel"), 0, 0, parent) {

		setModal(true);
		QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
		cancelButton->setDisabled(true);
		setCancelButton(cancelButton);

		QProgressBar* progressBar = new QProgressBar(this);
		progressBar->setTextVisible(false);
		setBar(progressBar);
		setMinimum(0);
		setMaximum(0);

		setWindowTitle(tr("Connecting Device"));
}

void OpenDeviceProgressDialog::closeEvent(QCloseEvent* e) {
	if (e->spontaneous()) {
		// prevent the user from closing the window, opening a device can _not_ be canceled
		e->ignore();
	} else {
		QProgressDialog::closeEvent(e);
	}
}

};
