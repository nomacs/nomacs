/*******************************************************************************************************
 DkImage.h
 Created on:	21.04.2011
 
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

#include "DkDialog.h"
#include "DkNoMacs.h"
#include "DkImageStorage.h"
#include "DkSettings.h"
#include "DkBaseViewPort.h"
#include "DkTimer.h"
#include "DkWidgets.h"
#include "DkImage.h"

#ifdef WIN32
#include <winsock2.h>	// needed since libraw 0.16
#endif

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QWidget>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QSlider>
#include <QColorDialog>
#include <QPushButton>
#include <QBoxLayout>
#include <QCheckBox>
#include <QFileInfo>
#include <QTableView>
#include <QCompleter>
#include <QMainWindow>
#include <QDialogButtonBox>
#include <QStandardItemModel>
#include <QItemEditorFactory>
#include <QHeaderView>
#include <QTreeView>
#include <QMimeData>
#include <QStringListModel>
#include <QListView>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QProgressDialog>
#include <QTextEdit>
#include <QApplication>
#include <QInputDialog>
#include <QFileDialog>
#include <QPageSetupDialog>
#include <QPrintDialog>
#include <QToolBar>
#include <QFormLayout>
#include <QProgressBar>
#include <QFuture>
#include <QtConcurrentRun>
#include <QMouseEvent>
#include <QAction>
#include <QMessageBox>
#include <QToolButton>
#include <QComboBox>
#include <qmath.h>

// quazip
#ifdef WITH_QUAZIP
#include <quazip/JlCompress.h>
#endif

#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

// DkSplashScreen --------------------------------------------------------------------
DkSplashScreen::DkSplashScreen(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	QPixmap img(":/nomacs/img/splash-screen.png");
	setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	setMouseTracking(true);
    
#ifdef Q_WS_MAC
    setObjectName("DkSplashScreenMac");
#else
	setObjectName("DkSplashScreen");
	setAttribute(Qt::WA_TranslucentBackground);
#endif

	imgLabel = new QLabel(this, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	imgLabel->setObjectName("DkSplashInfoLabel");
	imgLabel->setMouseTracking(true);
	imgLabel->setScaledContents(true);
	imgLabel->setPixmap(img);
	imgLabel->setFixedSize(600, 474);
	imgLabel->show();

	setFixedSize(imgLabel->size());

	//// create exit shortcuts
	//QShortcut* escExit = new QShortcut(Qt::Key_Escape, this);
	//QObject::connect(escExit, SIGNAL(activated()), this, SLOT(close()));

	//QPushButton* exitButton = new QPushButton(tr("Close"));
	//exitButton->setFlat(true);

	exitButton = new QPushButton(tr("CLOSE"), this);
	exitButton->setObjectName("cancelButtonSplash");
	exitButton->setFlat(true);
	exitButton->setIcon(QIcon(DkImage::colorizePixmap(QPixmap(":/nomacs/img/cancel2.png"), QColor(0,0,0,200), 1.0f)));
	exitButton->setToolTip(tr("Close (ESC)"));
	exitButton->setShortcut(QKeySequence(Qt::Key_Escape));
	exitButton->move(10, 435);
	exitButton->hide();
	connect(exitButton, SIGNAL(clicked()), this, SLOT(close()))	;

	// set the text
	text = 
		QString("Flo was here und wünscht<br>" 
		"Stefan fiel Spaß während<br>" 
		"Markus rockt... <br><br>" 

		"<a href=\"http://www.nomacs.org\">www.nomacs.org</a><br>"
		"<a href=\"mailto:developers@nomacs.org\">developers@nomacs.org</a><br><br>" 

		"This program is licensed under GNU General Public License v3<br>"
		"&#169; Markus Diem, Stefan Fiel and Florian Kleber, 2011-2014<br><br>"

		"Press [ESC] to exit");

	textLabel = new QLabel(this, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	textLabel->setObjectName("DkSplashInfoLabel");
	textLabel->setMouseTracking(true);
	textLabel->setScaledContents(true);
	textLabel->setTextFormat(Qt::RichText);
	textLabel->setText(text);
	textLabel->move(131, 280);
	textLabel->setOpenExternalLinks(true);
	
	//textLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
	
	QLabel* versionLabel = new QLabel(this, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	versionLabel->setObjectName("DkSplashInfoLabel");
	versionLabel->setTextFormat(Qt::RichText);

	QString platform = "";
#ifdef _WIN64
	platform = " [x64] ";
#elif defined _WIN32
	platform = " [x86] ";
#endif

	if (!DkSettings::isPortable())
		qDebug() << "nomacs is not portable: " << DkSettings::getSettingsFile().absoluteFilePath();

	versionLabel->setText("Version: " + QApplication::applicationVersion() + platform +  "<br>" +
#ifdef WITH_LIBRAW
		"RAW support: Yes"
#else
		"RAW support: No"
#endif  		
		+ (DkSettings::isPortable() ? tr("<br>Portable") : "")
		);

	versionLabel->move(360, 280);
	versionLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

	showTimer = new QTimer(this);
	showTimer->setInterval(5000);
	showTimer->setSingleShot(true);
	connect(showTimer, SIGNAL(timeout()), exitButton, SLOT(hide()));
	
	qDebug() << "splash screen created...";
}

void DkSplashScreen::mousePressEvent(QMouseEvent* event) {

	setCursor(Qt::ClosedHandCursor);
	mouseGrab = event->globalPos();
	QDialog::mousePressEvent(event);
}

void DkSplashScreen::mouseMoveEvent(QMouseEvent *event) {

	if (event->buttons() == Qt::LeftButton) {
		move(pos()-(mouseGrab-event->globalPos()));
		mouseGrab = event->globalPos();
		qDebug() << "moving...";
	}
	else
		setCursor(Qt::OpenHandCursor);
	showClose();
	QDialog::mouseMoveEvent(event);
}

void DkSplashScreen::mouseReleaseEvent(QMouseEvent *event) {

	setCursor(Qt::OpenHandCursor);
	showClose();
	QDialog::mouseReleaseEvent(event);
}

void DkSplashScreen::showClose() {

	exitButton->show();
	showTimer->start();
}

// file validator --------------------------------------------------------------------
DkFileValidator::DkFileValidator(QString lastFile, QObject * parent) : QValidator(parent) {

	this->lastFile = lastFile;
}

void DkFileValidator::fixup(QString& input) const {

	if(!QFileInfo(input).exists())
		input = lastFile;
}

QValidator::State DkFileValidator::validate(QString& input, int&) const {

	if (QFileInfo(input).exists())
		return QValidator::Acceptable;
	else
		return QValidator::Intermediate;
}

// train dialog --------------------------------------------------------------------
DkTrainDialog::DkTrainDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	setWindowTitle(tr("Add New Image Format"));
	createLayout();
	setFixedSize(340, 400);		// due to the baseViewport we need fixed sized dialogs : (
	setAcceptDrops(true);
}

void DkTrainDialog::createLayout() {

	// first row
	QLabel* newImageLabel = new QLabel(tr("Load New Image Format"), this);
	pathEdit = new QLineEdit(this);
	pathEdit->setValidator(&fileValidator);
	pathEdit->setObjectName("DkWarningEdit");
	connect(pathEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
	connect(pathEdit, SIGNAL(editingFinished()), this, SLOT(loadFile()));

	QPushButton* openButton = new QPushButton(tr("&Browse"), this);
	connect(openButton, SIGNAL(pressed()), this, SLOT(openFile()));

	feedbackLabel = new QLabel("", this);
	feedbackLabel->setObjectName("DkDecentInfo");

	// shows the image if it could be loaded
	viewport = new DkBaseViewPort(this);
	viewport->setForceFastRendering(true);
	viewport->setPanControl(QPointF(0.0f, 0.0f));

	// buttons
	buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&Add"));
	buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QWidget* trainWidget = new QWidget();
	QGridLayout* gdLayout = new QGridLayout(trainWidget);
	gdLayout->addWidget(newImageLabel, 0, 0);
	gdLayout->addWidget(pathEdit, 1, 0);
	gdLayout->addWidget(openButton, 1, 1);
	gdLayout->addWidget(feedbackLabel, 2, 0, 1, 2);
	gdLayout->addWidget(viewport, 3, 0, 1, 2);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(trainWidget);
	layout->addWidget(buttons);
}

void DkTrainDialog::textChanged(QString text) {
	
	if (QFileInfo(text).exists())
		pathEdit->setProperty("warning", false);
	else
		pathEdit->setProperty("warning", false);

	pathEdit->style()->unpolish(pathEdit);
	pathEdit->style()->polish(pathEdit);
	pathEdit->update();
}

void DkTrainDialog::openFile() {

	// load system default open dialog
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open Image"),
		cFile.absolutePath(), tr("All Files (*.*)"));

	if (QFileInfo(filePath).exists()) {
		pathEdit->setText(filePath);
		loadFile(filePath);
	}
}

void DkTrainDialog::userFeedback(const QString& msg, bool error) {

	if (!error)
		feedbackLabel->setProperty("warning", false);
	else
		feedbackLabel->setProperty("warning", true);

	feedbackLabel->setText(msg);
	feedbackLabel->style()->unpolish(feedbackLabel);
	feedbackLabel->style()->polish(feedbackLabel);
	feedbackLabel->update();
}

void DkTrainDialog::loadFile(QString filePath) {

	if (filePath.isEmpty() && !pathEdit->text().isEmpty())
		filePath = pathEdit->text();
	else if (filePath.isEmpty())
		return;

	QFileInfo fileInfo(filePath);
	if (!fileInfo.exists() || acceptedFile.absoluteFilePath() == fileInfo.absoluteFilePath())
		return;	// error message?!

	// update validator
	fileValidator.setLastFile(filePath);

	DkBasicLoader basicLoader;
	basicLoader.setTraining(true);

	// TODO: archives cannot be trained currently
	bool imgLoaded = basicLoader.loadGeneral(fileInfo, true);

	if (!imgLoaded) {
		viewport->setImage(QImage());	// remove the image
		acceptedFile = QFileInfo();
		userFeedback(tr("Sorry, currently we don't support: *.%1 files").arg(fileInfo.suffix()), true);
		return;
	}

	if (DkSettings::app.fileFilters.join(" ").contains(fileInfo.suffix(), Qt::CaseInsensitive)) {
		userFeedback(tr("*.%1 is already supported.").arg(fileInfo.suffix()), false);
		imgLoaded = false;
	}
	else
		userFeedback(tr("*.%1 is supported.").arg(fileInfo.suffix()), false);

	viewport->setImage(basicLoader.image());
	acceptedFile = fileInfo;

	// try loading the file
	// if loaded !
	buttons->button(QDialogButtonBox::Ok)->setEnabled(imgLoaded);
}

void DkTrainDialog::accept() {

	// add the extension to user filters
	if (!DkSettings::app.fileFilters.join(" ").contains(acceptedFile.suffix(), Qt::CaseInsensitive)) {

		QString name = QInputDialog::getText(this, "Format Name", tr("Please name the new format:"), QLineEdit::Normal, "Your File Format");
		QString tag = name + " (*." + acceptedFile.suffix() + ")";

		// load user filters
		QSettings& settings = Settings::instance().getSettings();
		QStringList userFilters = settings.value("ResourceSettings/userFilters", QStringList()).toStringList();
		userFilters.append(tag);
		settings.setValue("ResourceSettings/userFilters", userFilters);
		DkSettings::app.openFilters.append(tag);
		DkSettings::app.fileFilters.append("*." + acceptedFile.suffix());
		DkSettings::app.browseFilters += acceptedFile.suffix();
	}

	QDialog::accept();
}

void DkTrainDialog::dropEvent(QDropEvent *event) {

	if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
		QUrl url = event->mimeData()->urls().at(0);
		qDebug() << "dropping: " << url;
		url = url.toLocalFile();

		pathEdit->setText(url.toString());
		loadFile();
	}
}

void DkTrainDialog::dragEnterEvent(QDragEnterEvent *event) {

	if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();
		QFileInfo file = QFileInfo(url.toString());

		if (file.exists())
			event->acceptProposedAction();
	}

}



// DkAppManager --------------------------------------------------------------------
DkAppManager::DkAppManager(QWidget* parent) : QObject(parent) {
	
	defaultNames.resize(app_idx_end);
	defaultNames[app_photohsop]		= "PhotoshopAction";
	defaultNames[app_picasa]		= "PicasaAction";
	defaultNames[app_picasa_viewer] = "PicasaViewerAction";
	defaultNames[app_irfan_view]	= "IrfanViewAction";
	defaultNames[app_explorer]		= "ExplorerAction";

	this->parent = parent;
	loadSettings();
	findDefaultSoftware();

	for (int idx = 0; idx < apps.size(); idx++) {
		assignIcon(apps.at(idx));
		connect(apps.at(idx), SIGNAL(triggered()), this, SLOT(openTriggered()));
	}
}

DkAppManager::~DkAppManager() {

	saveSettings();
}

void DkAppManager::saveSettings() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup("DkAppManager");
	// clear it first
	settings.remove("Apps");
	
	settings.beginWriteArray("Apps");

	for (int idx = 0; idx < apps.size(); idx++) {
		settings.setArrayIndex(idx);
		settings.setValue("appName", apps.at(idx)->text());
		settings.setValue("appPath", apps.at(idx)->toolTip());
		settings.setValue("objectName", apps.at(idx)->objectName());
	}
	settings.endArray();
	settings.endGroup();
}

void DkAppManager::loadSettings() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup("DkAppManager");
	
	int size = settings.beginReadArray("Apps");
	
	for (int idx = 0; idx < size; idx++) {
		settings.setArrayIndex(idx);
		QAction* action = new QAction(parent);
		action->setText(settings.value("appName", "").toString());
		action->setToolTip(settings.value("appPath", "").toString());
		action->setObjectName(settings.value("objectName", "").toString());

		if (QFileInfo(action->toolTip()).exists() && !action->text().isEmpty())
			apps.append(action);
		else
			qDebug() << "could not locate: " << action->toolTip();

	}
	settings.endArray();
	settings.endGroup();
}

QVector<QAction* >& DkAppManager::getActions() {

	//for (int idx = 0; idx < apps.size(); idx++)
	//	qDebug() << "returning action: " << apps[idx]->text();

	return apps;
}

void DkAppManager::setActions(QVector<QAction* > actions) {
	
	apps = actions;
	saveSettings();
}

QAction* DkAppManager::createAction(QString filePath) {

	QFileInfo file(filePath);
	if (!file.exists())
		return 0;

	QAction* newApp = new QAction(file.baseName(), parent);
	newApp->setToolTip(QDir::fromNativeSeparators(file.filePath()));
	assignIcon(newApp);
	connect(newApp, SIGNAL(triggered()), this, SLOT(openTriggered()));

	return newApp;
}

QAction* DkAppManager::findAction(QString appPath) {

	for (int idx = 0; idx < apps.size(); idx++) {

		if (apps.at(idx)->toolTip() == appPath)
			return apps.at(idx);
	}

	return 0;
}

void DkAppManager::findDefaultSoftware() {
		
	QString appPath;

	// Photoshop
	if (!containsApp(apps, defaultNames[app_photohsop])) {
		appPath = searchForSoftware("Adobe", "Photoshop", "ApplicationPath");
		if (!appPath.isEmpty()) {
			QAction* a = new QAction(QObject::tr("&Photoshop"), parent);
			a->setToolTip(QDir::fromNativeSeparators(appPath));
			a->setObjectName(defaultNames[app_photohsop]);
			apps.append(a);
		}
	}

	if (!containsApp(apps, defaultNames[app_picasa])) {
		// Picasa
		appPath = searchForSoftware("Google", "Picasa", "Directory");
		if (!appPath.isEmpty()) {
			QAction* a = new QAction(QObject::tr("Pic&asa"), parent);
			a->setToolTip(QDir::fromNativeSeparators(appPath));
			a->setObjectName(defaultNames[app_picasa]);
			apps.append(a);
		}
	}

	if (!containsApp(apps, defaultNames[app_picasa_viewer])) {
		// Picasa Photo Viewer
		appPath = searchForSoftware("Google", "Picasa", "Directory", "PicasaPhotoViewer.exe");
		if (!appPath.isEmpty()) {
			QAction* a = new QAction(QObject::tr("Picasa Ph&oto Viewer"), parent);
			a->setToolTip(QDir::fromNativeSeparators(appPath));
			a->setObjectName(defaultNames[app_picasa_viewer]);
			apps.append(a);
		}
	}

	if (!containsApp(apps, defaultNames[app_irfan_view])) {
		// IrfanView
		appPath = searchForSoftware("IrfanView", "shell");
		if (!appPath.isEmpty()) {
			QAction* a = new QAction(QObject::tr("&IrfanView"), parent);
			a->setToolTip(QDir::fromNativeSeparators(appPath));
			a->setObjectName(defaultNames[app_irfan_view]);
			apps.append(a);
		}
	}

	if (!containsApp(apps, defaultNames[app_explorer])) {
		appPath = "C:/Windows/explorer.exe";
		if (QFileInfo(appPath).exists()) {
			QAction* a = new QAction(QObject::tr("&Explorer"), parent);
			a->setToolTip(QDir::fromNativeSeparators(appPath));
			a->setObjectName(defaultNames[app_explorer]);
			apps.append(a);
		}
	}
}

bool DkAppManager::containsApp(QVector<QAction* > apps, QString appName) {

	for (int idx = 0; idx < apps.size(); idx++)
		if (apps.at(idx)->objectName() == appName)
			return true;

	return false;
}

void DkAppManager::assignIcon(QAction* app) {

#ifdef WIN32

//#include <windows.h>

	if (!app) {
		qDebug() << "SERIOUS problem here, I should assign an icon to a NULL pointer action";
		return;
	}

	QFileInfo file = app->toolTip();
	
	if (!file.exists())
		return;

	// icon extraction should take between 2ms and 13ms
	QPixmap appIcon;
	QString winPath = QDir::toNativeSeparators(file.absoluteFilePath());

	WCHAR* wDirName = new WCHAR[winPath.length()+1];

	// CMakeLists.txt:
	// if compile error that toWCharArray is not recognized:
	// in msvc: Project Properties -> C/C++ -> Language -> Treat WChar_t as built-in type: set to No (/Zc:wchar_t-)
	wDirName = (WCHAR*)winPath.utf16();
	wDirName[winPath.length()] = L'\0';	// append null character

	int nIcons = ExtractIconExW(wDirName, 0, NULL, NULL, 0);

	if (!nIcons)
		return;

	HICON largeIcon;
	HICON smallIcon;
	ExtractIconExW(wDirName, 0, &largeIcon, &smallIcon, 1);

	if (nIcons != 0 && largeIcon != NULL)
		appIcon = DkImage::fromWinHICON(smallIcon);

	DestroyIcon(largeIcon);
	DestroyIcon(smallIcon);

	app->setIcon(appIcon);

#endif

}

QString DkAppManager::searchForSoftware(QString organization, QString application, QString pathKey, QString exeName) {

	qDebug() << "searching for: " << organization;

	// locate the settings entry
	QSettings softwareSettings(QSettings::UserScope, organization, application);
	QStringList keys = softwareSettings.allKeys();

	QString appPath;

	for (int idx = 0; idx < keys.length(); idx++) {

		// find the path
		if (keys[idx].contains(pathKey)) {
			appPath = softwareSettings.value(keys[idx]).toString();
			break;
		}
	}

	// if we did not find it -> return
	if (appPath.isEmpty())
		return appPath;

	if (exeName.isEmpty()) {

		// locate the exe
		QDir appFile = appPath.replace("\"", "");	// the string must not have extra quotes
		QFileInfoList apps = appFile.entryInfoList(QStringList() << "*.exe");

		for (int idx = 0; idx < apps.size(); idx++) {

			if (apps[idx].fileName().contains(application)) {
				appPath = apps[idx].absoluteFilePath();
				break;
			}
		}
	}
	else
		appPath = QFileInfo(appPath, exeName).absoluteFilePath();	// for correct separators

	return appPath;
}

void DkAppManager::openTriggered() {

	QAction* a = static_cast<QAction*>(QObject::sender());

	if (a)
		openFileSignal(a);
}

// DkAppManagerDialog --------------------------------------------------------------------
DkAppManagerDialog::DkAppManagerDialog(DkAppManager* manager /* = 0 */, QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */) : QDialog(parent, flags) {

	this->manager = manager;
	this->setWindowTitle(tr("Manage Applications"));
	createLayout();
}

void DkAppManagerDialog::createLayout() {

	QVector<QAction* > appActions = manager->getActions();

	model = new QStandardItemModel(this);
	for (int rIdx = 0; rIdx < appActions.size(); rIdx++)
		model->appendRow(getItems(appActions.at(rIdx)));

	appTableView = new QTableView(this);
	appTableView->setModel(model);
	appTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	appTableView->verticalHeader()->hide();
	appTableView->horizontalHeader()->hide();
	//appTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	appTableView->setShowGrid(false);
	appTableView->resizeColumnsToContents();
	appTableView->resizeRowsToContents();
	appTableView->setWordWrap(false);

	QPushButton* runButton = new QPushButton(tr("&Run"), this);
	runButton->setObjectName("runButton");

	QPushButton* addButton = new QPushButton(tr("&Add"), this);
	addButton->setObjectName("addButton");

	QPushButton* deleteButton = new QPushButton(tr("&Delete"), this);
	deleteButton->setObjectName("deleteButton");
	deleteButton->setShortcut(QKeySequence::Delete);

	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
	buttons->addButton(runButton, QDialogButtonBox::ActionRole);
	buttons->addButton(addButton, QDialogButtonBox::ActionRole);
	buttons->addButton(deleteButton, QDialogButtonBox::ActionRole);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(appTableView);
	layout->addWidget(buttons);
	QMetaObject::connectSlotsByName(this);
}

QList<QStandardItem* > DkAppManagerDialog::getItems(QAction* action) {

	QList<QStandardItem* > items;
	QStandardItem* item = new QStandardItem(action->icon(), action->text().remove("&"));
	//item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
	items.append(item);
	item = new QStandardItem(action->toolTip());
	item->setFlags(Qt::ItemIsSelectable);
	items.append(item);

	return items;
}

void DkAppManagerDialog::on_addButton_clicked() {

	// load system default open dialog
	QString appFilter;
	QString defaultPath;
#ifdef WIN32
	appFilter += tr("Executable Files (*.exe);;");
	defaultPath = getenv("PROGRAMFILES");
#elif QT_VERSION < 0x050000
	defaultPath = QDesktopServices::storageLocation(QDesktopServices::ApplicationsLocation); // retrieves startmenu on windows?!
#else
	defaultPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);	// still retrieves startmenu on windows
#endif

	QString filePath = QFileDialog::getOpenFileName(this, tr("Open Application"),
		defaultPath, 
		appFilter);

	if (filePath.isEmpty())
		return;

	QAction* newApp = manager->createAction(filePath);

	if (newApp)
		model->appendRow(getItems(newApp));

}

void DkAppManagerDialog::on_deleteButton_clicked() {

	QModelIndexList selRows = appTableView->selectionModel()->selectedRows();

	while (!selRows.isEmpty()) {
		model->removeRows(selRows.last().row(), 1);
		selRows.removeLast();
	}
}

void DkAppManagerDialog::on_runButton_clicked() {

	accept();

	QItemSelectionModel* sel = appTableView->selectionModel();

	if (!sel->hasSelection() && !manager->getActions().isEmpty())
		emit openWithSignal(manager->getActions().first());
	
	else if (!manager->getActions().isEmpty()) {

		QModelIndexList rows = sel->selectedRows();

		for (int idx = 0; idx < rows.size(); idx++) {
			emit openWithSignal(manager->getActions().at(rows.at(idx).row()));
		}
	}

}

void DkAppManagerDialog::accept() {

	QVector<QAction* > apps;

	for (int idx = 0; idx < model->rowCount(); idx++) {

		QString filePath = model->item(idx, 1)->text();
		QString name = model->item(idx, 0)->text();
		QAction* action = manager->findAction(filePath);

		if (!action)
			action = manager->createAction(filePath);
		// obviously I cannot create this action - should we tell user?
		if (!action)	
			continue;

		if (name != action->text().remove("&"))
			action->setText(name);

		qDebug() << "pushing back: " << action->text();

		apps.append(action);
	}

	manager->setActions(apps);

	QDialog::accept();
}

// DkSearchDialaog --------------------------------------------------------------------
DkSearchDialog::DkSearchDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	init();
}

void DkSearchDialog::init() {

	setObjectName("DkSearchDialog");
	setWindowTitle(tr("Find & Filter"));

	endMessage = tr("Load All");
	allDisplayed = true;
	isFilterPressed = false;

	QVBoxLayout* layout = new QVBoxLayout(this);

	QCompleter* history = new QCompleter(DkSettings::global.searchHistory, this);
	history->setCompletionMode(QCompleter::InlineCompletion);
	searchBar = new QLineEdit();
	searchBar->setObjectName("searchBar");
	searchBar->setToolTip(tr("Type a search word or a regular expression"));
	searchBar->setCompleter(history);

	stringModel = new QStringListModel(this);

	resultListView = new QListView(this);
	resultListView->setObjectName("resultListView");
	resultListView->setModel(stringModel);
	resultListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	resultListView->setSelectionMode(QAbstractItemView::SingleSelection);

	//// TODO: add cursor down - cursor up action
	//QAction* focusAction = new QAction(tr("Focus Action"), searchBar);
	//focusAction->setShortcut(Qt::Key_Down);
	//connect(focusAction, SIGNAL(triggered()), resultListView, SLOT(/*createSLOT*/));

	// buttons
	buttons.resize(button_end);
	buttons[find_button] = new QPushButton(tr("F&ind"), this);
	buttons[find_button]->setObjectName("okButton");
	//buttons[find_button]->setDefault(true);

	buttons[filter_button] = new QPushButton(tr("&Filter"), this);
	buttons[filter_button]->setObjectName("filterButton");

	buttons[cancel_button] = new QPushButton(tr("&Cancel"), this);
	buttons[cancel_button]->setObjectName("cancelButton");

	QWidget* buttonWidget = new QWidget();
	QBoxLayout* buttonLayout = new QBoxLayout(QBoxLayout::RightToLeft, buttonWidget);
	buttonLayout->addWidget(buttons[cancel_button]);
	buttonLayout->addWidget(buttons[filter_button]);
	buttonLayout->addWidget(buttons[find_button]);

	layout->addWidget(searchBar);
	layout->addWidget(resultListView);
	layout->addWidget(buttonWidget);

	searchBar->setFocus(Qt::MouseFocusReason);

	QMetaObject::connectSlotsByName(this);
}

void DkSearchDialog::setFiles(QStringList fileList) {
	this->fileList = fileList;
	this->resultList = fileList;
	stringModel->setStringList(makeViewable(fileList));
}

void DkSearchDialog::setPath(QDir path) {
	this->path = path;
}

bool DkSearchDialog::filterPressed() {
	return isFilterPressed;
}

void DkSearchDialog::on_searchBar_textChanged(const QString& text) {

	qDebug() << " you wrote: " << text;

	DkTimer dt;

	if (text == currentSearch)
		return;
	
	// white space is the magic thingy
	QStringList queries = text.split(" ");
	
	if (queries.size() == 1) {
		// if characters are added, use the result list -> speed-up
		// I think we can't do that anymore for the sake of list view cropping...
		resultList = (!text.contains(currentSearch) || !allDisplayed) ? fileList.filter(text, Qt::CaseInsensitive) : resultList.filter(text, Qt::CaseInsensitive);
	}
	else {
		resultList = fileList;
		for (int idx = 0; idx < queries.size(); idx++) {
			resultList = resultList.filter(queries[idx], Qt::CaseInsensitive);
			qDebug() << "query: " << queries[idx];
		}
	}

	// if string match returns nothing -> try a regexp
	if (resultList.empty()) {
		QRegExp regExp(text);
		resultList = fileList.filter(regExp);

		if (resultList.empty()) {
			regExp.setPatternSyntax(QRegExp::Wildcard);
			resultList = fileList.filter(regExp);
		}
	}

	qDebug() << "searching takes: " << dt.getTotal();
	currentSearch = text;

	if (resultList.empty()) {
		QStringList answerList;
		answerList.append(tr("No Matching Items"));
		stringModel->setStringList(answerList);

		resultListView->setProperty("empty", true);
		buttons[filter_button]->setEnabled(false);
		buttons[find_button]->setEnabled(false);
		//cancelButton->setFocus();
	}
	else {
		buttons[filter_button]->setEnabled(true);
		buttons[find_button]->setEnabled(true);
		stringModel->setStringList(makeViewable(resultList));
		resultListView->selectionModel()->setCurrentIndex(stringModel->index(0, 0), QItemSelectionModel::SelectCurrent);
		resultListView->setProperty("empty", false);
	}

	resultListView->style()->unpolish(resultListView);
	resultListView->style()->polish(resultListView);
	resultListView->update();

	qDebug() << "searching takes (total): " << dt.getTotal();
}

void DkSearchDialog::on_resultListView_doubleClicked(const QModelIndex& modelIndex) {

	if (modelIndex.data().toString() == endMessage) {
		stringModel->setStringList(makeViewable(resultList, true));
		return;
	}

	emit loadFileSignal(QFileInfo(path, modelIndex.data().toString()));
	close();
}

void DkSearchDialog::on_resultListView_clicked(const QModelIndex& modelIndex) {

	if (modelIndex.data().toString() == endMessage)
		stringModel->setStringList(makeViewable(resultList, true));
}

void DkSearchDialog::on_okButton_pressed() {

	if (resultListView->selectionModel()->currentIndex().data().toString() == endMessage) {
		stringModel->setStringList(makeViewable(resultList, true));
		return;
	}

	updateHistory();

	// ok load the selected file
	QString fileName = resultListView->selectionModel()->currentIndex().data().toString();
	qDebug() << "opening filename: " << fileName;

	if (!fileName.isEmpty())
		emit loadFileSignal(QFileInfo(path, fileName));
	accept();
}

void DkSearchDialog::on_filterButton_pressed() {
	filterSignal(currentSearch.split(" "));
	isFilterPressed = true;
	done(filter_button);
}

void DkSearchDialog::on_cancelButtonSplash_pressed() {

	reject();
}

void DkSearchDialog::setDefaultButton(int defaultButton /* = find_button */) {

	for (int idx = 0; idx < buttons.size(); idx++)
		buttons[idx]->setAutoDefault(defaultButton == idx);
}

void DkSearchDialog::updateHistory() {
	
	DkSettings::global.searchHistory.append(currentSearch);

	// keep the history small
	if (DkSettings::global.searchHistory.size() > 50)
		DkSettings::global.searchHistory.pop_front();

	//QCompleter* history = new QCompleter(DkSettings::global.searchHistory);
	//searchBar->setCompleter(history);
}

QStringList DkSearchDialog::makeViewable(const QStringList& resultList, bool forceAll) {
	
	QStringList answerList;
	
	// if size > 1000 it gets slow -> cut at 1000 and make an entry for 'expand'
	if (!forceAll && resultList.size() > 1000) {

		for (int idx = 0; idx < 1000; idx++)
			answerList.append(resultList[idx]);
		answerList.append(endMessage);

		allDisplayed = false;
	}
	else {
		allDisplayed = true;
		answerList = resultList;
	}

	return answerList;
}

// DkResizeDialog --------------------------------------------------------------------
DkResizeDialog::DkResizeDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	init();
}

void DkResizeDialog::accept() {
	saveSettings();

	QDialog::accept();
}

void DkResizeDialog::saveSettings() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup(objectName());

	settings.setValue("ResampleMethod", resampleBox->currentIndex());
	settings.setValue("Resample", resampleCheck->isChecked());
	settings.setValue("CorrectGamma", gammaCorrection->isChecked());

	if (sizeBox->currentIndex() == size_percent) {
		settings.setValue("Width", wPixelEdit->value());
		settings.setValue("Height", hPixelEdit->value());
	}
	else {
		settings.setValue("Width", 0);
		settings.setValue("Height", 0);
	}
	settings.endGroup();
}


void DkResizeDialog::loadSettings() {

	qDebug() << "loading new settings...";

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup(objectName());

	resampleBox->setCurrentIndex(settings.value("ResampleMethod", ipl_cubic).toInt());
	resampleCheck->setChecked(settings.value("Resample", true).toBool());
	gammaCorrection->setChecked(settings.value("CorrectGamma", true).toBool());

	if (settings.value("Width", 0).toDouble() != 0) {

		double w = settings.value("Width", 0).toDouble();
		double h = settings.value("Height", 0).toDouble();

		qDebug() << "width loaded from settings: " << w;

		if (w != h) {
			lockButton->setChecked(false);
			lockButtonDim->setChecked(false);
		}
		sizeBox->setCurrentIndex(size_percent);

		wPixelEdit->setValue(w);
		hPixelEdit->setValue(h);
		
		updateWidth();
		updateHeight();
	}
	settings.endGroup();

}

void DkResizeDialog::init() {

	setObjectName("DkResizeDialog");
	leftSpacing = 40;
	margin = 10;
	exifDpi = 72;

	unitFactor.resize(unit_end);
	unitFactor.insert(unit_cm, 1.0f);
	unitFactor.insert(unit_mm, 10.0f);
	unitFactor.insert(unit_inch, 1.0f/2.54f);

	resFactor.resize(res_end);
	resFactor.insert(res_ppi, 2.54f);
	resFactor.insert(res_ppc, 1.0f);

	setWindowTitle(tr("Resize Image"));
	//setFixedSize(600, 512);
	createLayout();
	initBoxes();

	wPixelEdit->setFocus(Qt::ActiveWindowFocusReason);

	QMetaObject::connectSlotsByName(this);
}

void DkResizeDialog::createLayout() {

	// central widget
	centralWidget = new QWidget(this);

	// preview
	int minPx = 1;
	int maxPx = 100000;
	double minWidth = 0.001;
	double maxWidth = 500000;
	int decimals = 2;

	QLabel* origLabelText = new QLabel(tr("Original"));
	origLabelText->setAlignment(Qt::AlignHCenter);
	QLabel* newLabel = new QLabel(tr("New"));
	newLabel->setAlignment(Qt::AlignHCenter);

	// shows the original image
	origView = new DkBaseViewPort(this);
	origView->setForceFastRendering(true);
	origView->setPanControl(QPointF(0.0f, 0.0f));
	connect(origView, SIGNAL(imageUpdated()), this, SLOT(drawPreview()));

	//// maybe we should report this: 
	//// if a stylesheet (with border) is set, the var
	//// cornerPaintingRect in QAbstractScrollArea (which we don't even need : )
	//// is invalid which blocks re-paints unless the widget gets a focus...
	//origView->setStyleSheet("QViewPort{border: 1px solid #888;}");

	// shows the preview
	previewLabel = new QLabel(this);
	previewLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	//previewLabel->setScaledContents(true);
	previewLabel->setMinimumHeight(100);
	//previewLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);
	//previewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	//previewLabel->setStyleSheet("QLabel{border: 1px solid #888;}");

	//previewLayout->addWidget(origLabelText, 0, 0);
	//previewLayout->addWidget(newLabel, 0, 1);
	//previewLayout->addWidget(origView, 1, 0);
	//previewLayout->addWidget(previewLabel, 1, 1);

	// all text dialogs...
	QDoubleValidator* doubleValidator = new QDoubleValidator(1, 1000000, 2, 0);
	doubleValidator->setRange(0, 100, 2);

	QWidget* resizeBoxes = new QWidget(this);
	resizeBoxes->setGeometry(QRect(QPoint(leftSpacing, 300), QSize(400, 170)));

	QGridLayout* gridLayout = new QGridLayout(resizeBoxes);

	QLabel* wPixelLabel = new QLabel(tr("Width: "));
	wPixelLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	wPixelEdit = new DkSelectAllDoubleSpinBox();
	wPixelEdit->setObjectName("wPixelEdit");
	wPixelEdit->setRange(minPx, maxPx);
	wPixelEdit->setDecimals(0);

	lockButton = new DkButton(QIcon(":/nomacs/img/lock.png"), QIcon(":/nomacs/img/lock-unlocked.png"), "lock");
	lockButton->setFixedSize(QSize(16,16));
	lockButton->setObjectName("lockButton");
	lockButton->setCheckable(true);
	lockButton->setChecked(true);

	QLabel* hPixelLabel = new QLabel(tr("Height: "));
	hPixelLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	hPixelEdit = new DkSelectAllDoubleSpinBox();
	hPixelEdit->setObjectName("hPixelEdit");
	hPixelEdit->setRange(minPx, maxPx);
	hPixelEdit->setDecimals(0);

	sizeBox = new QComboBox();
	QStringList sizeList;
	sizeList.insert(size_pixel, "pixel");
	sizeList.insert(size_percent, "%");
	sizeBox->addItems(sizeList);
	sizeBox->setObjectName("sizeBox");

	// first row
	int rIdx = 0;
	gridLayout->addWidget(wPixelLabel, 0, rIdx);
	gridLayout->addWidget(wPixelEdit, 0, ++rIdx);
	gridLayout->addWidget(lockButton, 0, ++rIdx);
	gridLayout->addWidget(hPixelLabel, 0, ++rIdx);
	gridLayout->addWidget(hPixelEdit, 0, ++rIdx);
	gridLayout->addWidget(sizeBox, 0, ++rIdx);

	// Document dimensions
	QLabel* widthLabel = new QLabel(tr("Width: "));
	widthLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	widthEdit = new DkSelectAllDoubleSpinBox();
	widthEdit->setObjectName("widthEdit");
	widthEdit->setRange(minWidth, maxWidth);
	widthEdit->setDecimals(decimals);


	lockButtonDim = new DkButton(QIcon(":/nomacs/img/lock.png"), QIcon(":/nomacs/img/lock-unlocked.png"), "lock");
	//lockButtonDim->setIcon(QIcon(":/nomacs/img/lock.png"));
	lockButtonDim->setFixedSize(QSize(16,16));
	lockButtonDim->setObjectName("lockButtonDim");
	lockButtonDim->setCheckable(true);
	lockButtonDim->setChecked(true);

	QLabel* heightLabel = new QLabel(tr("Height: "));
	heightLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	heightEdit = new DkSelectAllDoubleSpinBox();
	heightEdit->setObjectName("heightEdit");
	heightEdit->setRange(minWidth, maxWidth);
	heightEdit->setDecimals(decimals);

	unitBox = new QComboBox();
	QStringList unitList;
	unitList.insert(unit_cm, "cm");
	unitList.insert(unit_mm, "mm");
	unitList.insert(unit_inch, "inch");
	//unitList.insert(unit_percent, "%");
	unitBox->addItems(unitList);
	unitBox->setObjectName("unitBox");

	// second row
	rIdx = 0;
	gridLayout->addWidget(widthLabel, 1, rIdx);
	gridLayout->addWidget(widthEdit, 1, ++rIdx);
	gridLayout->addWidget(lockButtonDim, 1, ++rIdx);
	gridLayout->addWidget(heightLabel, 1, ++rIdx);
	gridLayout->addWidget(heightEdit, 1, ++rIdx);
	gridLayout->addWidget(unitBox, 1, ++rIdx);

	// resolution
	QLabel* resolutionLabel = new QLabel(tr("Resolution: "));
	resolutionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	resolutionEdit = new DkSelectAllDoubleSpinBox();
	resolutionEdit->setObjectName("resolutionEdit");
	resolutionEdit->setRange(minWidth, maxWidth);
	resolutionEdit->setDecimals(decimals);

	resUnitBox = new QComboBox();
	QStringList resUnitList;
	resUnitList.insert(res_ppi, "pixel/inch");
	resUnitList.insert(res_ppc, "pixel/cm");
	resUnitBox->addItems(resUnitList);
	resUnitBox->setObjectName("resUnitBox");

	// third row
	rIdx = 0;
	gridLayout->addWidget(resolutionLabel, 2, rIdx);
	gridLayout->addWidget(resolutionEdit, 2, ++rIdx);
	gridLayout->addWidget(resUnitBox, 2, ++rIdx, 1, 2);

	// resample
	resampleCheck = new QCheckBox(tr("Resample Image:"));
	resampleCheck->setChecked(true);
	resampleCheck->setObjectName("resampleCheck");

	// TODO: disable items if no opencv is available
	resampleBox = new QComboBox();
	QStringList resampleList;
	resampleList.insert(ipl_nearest, tr("Nearest Neighbor"));
	resampleList.insert(ipl_area, tr("Area (best for downscaling)"));
	resampleList.insert(ipl_linear, tr("Linear"));
	resampleList.insert(ipl_cubic, tr("Bicubic (4x4 pixel interpolation)"));
	resampleList.insert(ipl_lanczos, tr("Lanczos (8x8 pixel interpolation)"));
	resampleBox->addItems(resampleList);
	resampleBox->setObjectName("resampleBox");
	resampleBox->setCurrentIndex(ipl_cubic);

	// last two rows
	gridLayout->addWidget(resampleCheck, 3, 1, 1, 3);
	gridLayout->addWidget(resampleBox, 4, 1, 1, 3);

	gammaCorrection = new QCheckBox(tr("Gamma Correction"));
	gammaCorrection->setObjectName("gammaCorrection");
	gammaCorrection->setChecked(false);	// default: false since gamma might destroy soft gradients

	gridLayout->addWidget(gammaCorrection, 5, 1, 1, 3);

	// add stretch
	gridLayout->setColumnStretch(6, 1);

	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QGridLayout* layout = new QGridLayout(this);
	layout->setColumnStretch(0,1);
	layout->setColumnStretch(1,1);

	layout->addWidget(origLabelText, 0, 0);
	layout->addWidget(newLabel, 0, 1);
	layout->addWidget(origView, 1, 0);
	layout->addWidget(previewLabel, 1, 1);
	layout->addWidget(resizeBoxes, 2, 0, 1, 2, Qt::AlignLeft);
	//layout->addStretch();
	layout->addWidget(buttons, 3, 0, 1, 2, Qt::AlignRight);
	
	adjustSize();
	//show();
}

void DkResizeDialog::initBoxes(bool updateSettings) {

	if (img.isNull())
		return;

	if (sizeBox->currentIndex() == size_pixel) {
		wPixelEdit->setValue(img.width());
		hPixelEdit->setValue(img.height());
	}
	else {
		wPixelEdit->setValue(100);
		hPixelEdit->setValue(100);
	}

	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	float width = (float)img.width()/exifDpi * units;
	widthEdit->setValue(width);

	float height = (float)img.height()/exifDpi * units;
	heightEdit->setValue(height);

	if (updateSettings)
		loadSettings();
}

void DkResizeDialog::setImage(QImage img) {
	this->img = img;
	initBoxes(true);
	updateSnippets();
	drawPreview();
	wPixelEdit->selectAll();
}

QImage DkResizeDialog::getResizedImage() {

	return resizeImg(img, false);
}

void DkResizeDialog::setExifDpi(float exifDpi) {

	this->exifDpi = exifDpi;
	resolutionEdit->setValue(exifDpi);
}

float DkResizeDialog::getExifDpi() {
	return exifDpi;
}

bool DkResizeDialog::resample() {
	return resampleCheck->isChecked();
}

void DkResizeDialog::updateWidth() {

	float pWidth = (float)wPixelEdit->text().toDouble();

	if (sizeBox->currentIndex() == size_percent)
		pWidth = (float)qRound(pWidth/100 * img.width()); 

	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	float width = pWidth/exifDpi * units;
	widthEdit->setValue(width);
}

void DkResizeDialog::updateHeight() {

	float pHeight = (float)hPixelEdit->text().toDouble();

	if (sizeBox->currentIndex() == size_percent)
		pHeight = (float)qRound(pHeight/100 * img.height()); 

	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	float height = pHeight/exifDpi * units;
	heightEdit->setValue(height);
}

void DkResizeDialog::updateResolution() {

	qDebug() << "updating resolution...";
	float wPixel = (float)wPixelEdit->text().toDouble();
	float width = (float)widthEdit->text().toDouble();

	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	float resolution = wPixel/width * units;
	resolutionEdit->setValue(resolution);
}

void DkResizeDialog::updatePixelHeight() {

	float height = (float)heightEdit->text().toDouble();

	// *1000/10 is for more beautiful values
	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	float pixelHeight = (sizeBox->currentIndex() != size_percent) ? qRound(height*exifDpi / units) : qRound(1000.0f*height*exifDpi / (units * img.height()))/10.0f;

	hPixelEdit->setValue(pixelHeight);
}

void DkResizeDialog::updatePixelWidth() {

	float width = (float)widthEdit->text().toDouble();

	float units = resFactor.at(resUnitBox->currentIndex()) * unitFactor.at(unitBox->currentIndex());
	float pixelWidth = (sizeBox->currentIndex() != size_percent) ? qRound(width*exifDpi / units) : qRound(1000.0f*width*exifDpi / (units * img.width()))/10.0f;
	wPixelEdit->setValue(pixelWidth);
}

void DkResizeDialog::on_lockButtonDim_clicked() {

	lockButton->setChecked(lockButtonDim->isChecked());
	if (!lockButtonDim->isChecked())
		return;

	initBoxes();
	drawPreview();
}

void DkResizeDialog::on_lockButton_clicked() {

	lockButtonDim->setChecked(lockButton->isChecked());

	if (!lockButton->isChecked())
		return;

	initBoxes();
	drawPreview();
}

void DkResizeDialog::on_wPixelEdit_valueChanged(QString text) {

	if (!wPixelEdit->hasFocus())
		return;

	updateWidth();

	if (!lockButton->isChecked()) {
		drawPreview();
		return;
	}

	int newHeight = (sizeBox->currentIndex() != size_percent) ? qRound((float)text.toInt()/(float)img.width() * img.height()) : qRound(text.toFloat());
	hPixelEdit->setValue(newHeight);
	updateHeight();
	drawPreview();
}

void DkResizeDialog::on_hPixelEdit_valueChanged(QString text) {

	if(!hPixelEdit->hasFocus())
		return;
	
	updateHeight();

	if (!lockButton->isChecked()) {
		drawPreview();
		return;
	}

	int newWidth = (sizeBox->currentIndex() != size_percent) ? qRound((float)text.toInt()/(float)img.height() * (float)img.width()) : qRound(text.toFloat());
	wPixelEdit->setValue(newWidth);
	updateWidth();
	drawPreview();
}

void DkResizeDialog::on_widthEdit_valueChanged(QString text) {

	if (!widthEdit->hasFocus())
		return;

	if (resampleCheck->isChecked()) 
		updatePixelWidth();

	if (!lockButton->isChecked()) {
		drawPreview();
		return;
	}

	heightEdit->setValue(text.toFloat()/(float)img.width() * (float)img.height());

	if (resampleCheck->isChecked()) 
		updatePixelHeight();

	if (!resampleCheck->isChecked())
		updateResolution();

	drawPreview();
}

void DkResizeDialog::on_heightEdit_valueChanged(QString text) {

	if (!heightEdit->hasFocus())
		return;

	if (resampleCheck->isChecked()) 
		updatePixelHeight();

	if (!lockButton->isChecked()) {
		drawPreview();
		return;
	}

	widthEdit->setValue(text.toFloat()/(float)img.height() * (float)img.width());

	if (resampleCheck->isChecked()) 
		updatePixelWidth();

	if (!resampleCheck->isChecked())
		updateResolution();
	drawPreview();
}

void DkResizeDialog::on_resolutionEdit_valueChanged(QString text) {

	exifDpi = (float)text.toDouble();

	if (!resolutionEdit->hasFocus())
		return;

	updatePixelWidth();
	updatePixelHeight();

	if (resampleCheck->isChecked()) {
		drawPreview();
		return;
	}

	initBoxes();
}


void DkResizeDialog::on_unitBox_currentIndexChanged(int) {

	updateHeight();
	updateWidth();
	//initBoxes();
}

void DkResizeDialog::on_sizeBox_currentIndexChanged(int idx) {

	if (idx == size_pixel) {
		wPixelEdit->setDecimals(0);
		hPixelEdit->setDecimals(0);
	}
	else {
		wPixelEdit->setDecimals(2);
		hPixelEdit->setDecimals(2);
	}

	updatePixelHeight();
	updatePixelWidth();
}

void DkResizeDialog::on_resUnitBox_currentIndexChanged(int) {

	updateResolution();
	//initBoxes();
}

void DkResizeDialog::on_resampleCheck_clicked() {

	resampleBox->setEnabled(resampleCheck->isChecked());
	wPixelEdit->setEnabled(resampleCheck->isChecked());
	hPixelEdit->setEnabled(resampleCheck->isChecked());

	if (!resampleCheck->isChecked()) {
		lockButton->setChecked(true);
		lockButtonDim->setChecked(true);
		initBoxes();
	}
	else
		drawPreview();
}

void DkResizeDialog::on_gammaCorrection_clicked() {

	drawPreview();	// diem: just update
}

void DkResizeDialog::on_resampleBox_currentIndexChanged(int) {
	drawPreview();
}

void DkResizeDialog::updateSnippets() {

	if (img.isNull() /*|| !isVisible()*/)
		return;

	//// fix layout issues - sorry
	//origView->setFixedWidth(width()*0.5f-30);
	//previewLabel->setFixedWidth(origView->width()-2);
	//origView->setFixedHeight(width()*0.5f-30);
	//previewLabel->setFixedHeight(width()*0.5f-30);


	origView->setImage(img);
	origView->fullView();
	origView->zoomConstraints(origView->get100Factor());

	qDebug() << "zoom constraint: " << origView->get100Factor();

	//QSize s = QSize(width()-2*leftSpacing-10, width()-2*leftSpacing-10);
	//s *= 0.5;
	//origImg = QImage(s, QImage::Format_ARGB32);
	//origImg.fill(Qt::transparent);
	//QRect imgRect = QRect(QPoint(img.width()*0.5-origImg.width()*0.5, img.height()*0.5-origImg.height()*0.5), origImg.size());

	//QPainter painter(&origImg);
	//painter.setBackgroundMode(Qt::TransparentMode);
	//painter.drawImage(QRect(QPoint(), origImg.size()), img, imgRect;)
	//painter.end();

}

void DkResizeDialog::drawPreview() {

	if (img.isNull() || !isVisible()) 
		return;

	newImg = origView->getCurrentImageRegion();

	// TODO: thread here!
	QImage img = resizeImg(newImg);

	// TODO: is there a better way of mapping the pixels? (ipl here introduces artifacts that are not in the final image)
	img = img.scaled(previewLabel->size(), Qt::KeepAspectRatio, Qt::FastTransformation);
	previewLabel->setPixmap(QPixmap::fromImage(img));
}

QImage DkResizeDialog::resizeImg(QImage img, bool silent) {

	if (img.isNull())
		return img;

	QSize newSize;

	if (sizeBox->currentIndex() == size_percent)
		newSize = QSize(qRound(wPixelEdit->text().toFloat()/100.0f * this->img.width()), qRound(hPixelEdit->text().toFloat()/100.0f * this->img.height()));
	else
		newSize = QSize(wPixelEdit->text().toInt(), hPixelEdit->text().toInt());

	QSize imgSize = this->img.size();

	qDebug() << "new size: " << newSize;

	// nothing to do
	if (this->img.size() == newSize)
		return img;

	if (this->img.size() != img.size()) {
		// compute relative size
		float relWidth = (float)newSize.width()/(float)imgSize.width();
		float relHeight = (float)newSize.height()/(float)imgSize.height();

		qDebug() << "relative size: " << newSize;

		newSize = QSize(qRound(img.width()*relWidth), qRound(img.height()*relHeight));
	}

	if (newSize.width() < wPixelEdit->minimum() || newSize.width() > wPixelEdit->maximum() || 
		newSize.height() < hPixelEdit->minimum() || newSize.height() > hPixelEdit->maximum()) {

		if (!silent) {
			QMessageBox errorDialog(this);
			errorDialog.setIcon(QMessageBox::Critical);
			errorDialog.setText(tr("Sorry, but the image size %1 x %2 is illegal.").arg(newSize.width()).arg(newSize.height()));
			errorDialog.show();
			errorDialog.exec();
		}
	}

	QImage rImg = DkImage::resizeImage(img, newSize, 1.0f, resampleBox->currentIndex(), gammaCorrection->isChecked());

	if (rImg.isNull() && !silent) {
		qDebug() << "image size: " << newSize;
		QMessageBox errorDialog(this);
		errorDialog.setIcon(QMessageBox::Critical);
		errorDialog.setText(tr("Sorry, the image is too large: %1").arg(DkImage::getBufferSize(newSize, 32)));
		errorDialog.show();
		errorDialog.exec();
	}

	return rImg;
}

// DkShortcutDelegate --------------------------------------------------------------------
DkShortcutDelegate::DkShortcutDelegate(QObject* parent) : QItemDelegate(parent) {
	item = 0;
}

QWidget* DkShortcutDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
	
	QWidget* w = QItemDelegate::createEditor(parent, option, index);
	connect(w, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
	connect(w, SIGNAL(editingFinished()), this, SLOT(textChanged()));

	return w;
}

bool DkShortcutDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) {

	item = index.internalPointer();

	return QItemDelegate::editorEvent(event, model, option, index);
}

void DkShortcutDelegate::textChanged(QString text) {
	emit checkDuplicateSignal(text, item);
}

// DkShortcutEditor --------------------------------------------------------------------
DkShortcutEditor::DkShortcutEditor(QWidget *widget) : QLineEdit(widget) {

	installEventFilter(this);
}

QKeySequence DkShortcutEditor::shortcut() const {
	return QKeySequence(text());
}

void DkShortcutEditor::setShortcut(const QKeySequence shortcut) {
	this->ks = shortcut;
}

bool DkShortcutEditor::eventFilter(QObject *obj, QEvent *event) {

	// TODO: we somehow need to filter events such as ALT+F4 too
	if (event->type() == QEvent::KeyRelease) {
		
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		
		if (keyEvent->key() == Qt::Key_Control ||
			keyEvent->key() == Qt::Key_Shift ||
			keyEvent->key() == Qt::Key_Alt ||
			keyEvent->key() == Qt::Key_Meta)
			return true;

		int ksi = keyEvent->key();

		if (keyEvent->modifiers() & Qt::ShiftModifier)
			ksi += Qt::SHIFT;
		if (keyEvent->modifiers() & Qt::AltModifier)
			ksi += Qt::ALT;
		if (keyEvent->modifiers() & Qt::ControlModifier)
			ksi += Qt::CTRL;
		if (keyEvent->modifiers() & Qt::MetaModifier)
			ksi += Qt::META;

		QKeySequence ks(ksi);
		setText(ks.toString());

		qDebug() << "eating the event...";

		return true;
	}
	else if (event->type() == QEvent::KeyPress) {	// filter keypresses to avoid any dialog action on shortcut edit
		return true;
	}
	//else if (event->type() == QEvent::ShortcutOverride)
	//	return true;

	return QLineEdit::eventFilter(obj, event);
}


//void DkShortcutEditor::keyPressEvent(QKeyEvent *event) {
//
//}
//
//void DkShortcutEditor::keyReleaseEvent(QKeyEvent* event) {
//	
//	if (event->key() == Qt::Key_Control ||
//		event->key() == Qt::Key_Shift ||
//		event->key() == Qt::Key_Alt ||
//		event->key() == Qt::Key_Meta)
//		return;
//
//	int ksi = event->key();
//
//	if (event->modifiers() & Qt::ShiftModifier)
//		ksi += Qt::SHIFT;
//	if (event->modifiers() & Qt::AltModifier)
//		ksi += Qt::ALT;
//	if (event->modifiers() & Qt::ControlModifier)
//		ksi += Qt::CTRL;
//	if (event->modifiers() & Qt::MetaModifier)
//		ksi += Qt::META;
//
//	QKeySequence ks(ksi);
//	setText(ks.toString());
//}

// DkShortcutsModel --------------------------------------------------------------------
DkShortcutsModel::DkShortcutsModel(QObject* parent) : QAbstractTableModel(parent) {

	// create root
	QVector<QVariant> rootData;
	rootData << tr("Name") << tr("Shortcut");

	rootItem = new TreeItem(rootData);

}

DkShortcutsModel::~DkShortcutsModel() {
	delete rootItem;
}

//DkShortcutsModel::DkShortcutsModel(QVector<QPair<QString, QKeySequence> > actions, QObject *parent) : QAbstractTableModel(parent) {
//	this->actions = actions;
//}

QModelIndex DkShortcutsModel::index(int row, int column, const QModelIndex &parent) const {
	
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	TreeItem *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	TreeItem *childItem = parentItem->child(row);

	//qDebug() << " creating index for: " << childItem->data(0) << " row: " << row;
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex DkShortcutsModel::parent(const QModelIndex &index) const {
	
	if (!index.isValid())
		return QModelIndex();

	TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
	TreeItem *parentItem = childItem->parent();

	if (parentItem == rootItem)
		return QModelIndex();

	//qDebug() << "creating index for: " << childItem->data(0);

	return createIndex(parentItem->row(), 0, parentItem);
}

int DkShortcutsModel::rowCount(const QModelIndex& parent) const {

	TreeItem *parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	return parentItem->childCount();
}

int DkShortcutsModel::columnCount(const QModelIndex& parent) const {

	if (parent.isValid())
		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	else
		return rootItem->columnCount();
	//return 2;
}

QVariant DkShortcutsModel::data(const QModelIndex& index, int role) const {

	if (!index.isValid()) {
		qDebug() << "invalid row: " << index.row();
		return QVariant();
	}

	//if (index.row() > rowCount())
	//	return QVariant();

	//if (index.column() > columnCount())
	//	return QVariant();

	if (role == Qt::DisplayRole || role == Qt::EditRole) {

		TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
		//qDebug() << "returning: " << item->data(0) << "row: " << index.row();

		return item->data(index.column());
	}

	return QVariant();
}


QVariant DkShortcutsModel::headerData(int section, Qt::Orientation orientation, int role) const {

	if (orientation != Qt::Horizontal || role != Qt::DisplayRole) 
		return QVariant();

	return rootItem->data(section);
} 

bool DkShortcutsModel::setData(const QModelIndex& index, const QVariant& value, int role) {

	if (!index.isValid() || role != Qt::EditRole)
		return false;

	if (index.column() == 1) {

		QKeySequence ks = value.value<QKeySequence>();
		if (index.column() == 1) {
			TreeItem* duplicate = rootItem->find(ks, index.column());
			if (duplicate) duplicate->setData(QKeySequence(), index.column());
			if (!duplicate) qDebug() << ks << " no duplicate found...";
		}
		
		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
		item->setData(ks, index.column());

	}
	else {
		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
		item->setData(value, index.column());
	}

	emit dataChanged(index, index);
	return true;
}

Qt::ItemFlags DkShortcutsModel::flags(const QModelIndex& index) const {

	if (!index.isValid())
		return Qt::ItemIsEditable;

	//// no editing on root items
	//if (item->parent() == rootItem)
	//	return QAbstractTableModel::flags(index);

	Qt::ItemFlags flags;

	if (index.column() == 0)
		flags = QAbstractTableModel::flags(index);
	if (index.column() == 1)
		flags = QAbstractTableModel::flags(index) | Qt::ItemIsEditable;

	return flags;
}

void DkShortcutsModel::addDataActions(QVector<QAction*> actions, QString name) {

	// create root
	QVector<QVariant> menuData;
	menuData << name;

	TreeItem* menuItem = new TreeItem(menuData, rootItem);

	for (int idx = 0; idx < actions.size(); idx++) {

		// skip NULL actions - this should never happen!
		if (actions[idx]->text().isNull()) {
			qDebug() << "NULL Action detected when creating shortcuts...";
			continue;
		}

		QString text = actions[idx]->text();
		text.remove("&");

		QVector<QVariant> actionData;
		actionData << text << actions[idx]->shortcut();

		TreeItem* dataItem = new TreeItem(actionData, menuItem);
		menuItem->appendChild(dataItem);
	}

	rootItem->appendChild(menuItem);
	this->actions.append(actions);
	//qDebug() << "menu item has: " << menuItem->childCount();

}

void DkShortcutsModel::checkDuplicate(QString text, void* item) {

	if (text.isEmpty()) {
		emit duplicateSignal("");
		return;
	}

	QKeySequence ks(text);

	TreeItem* duplicate = rootItem->find(QKeySequence(text), 1);

	if (duplicate == item)
		return;

	if (duplicate && duplicate->parent()) {
		emit duplicateSignal(tr("%1 already used by %2 > %3\nPress ESC to undo changes")
				.arg(duplicate->data(1).toString())
				.arg(duplicate->parent()->data(0).toString())
				.arg(duplicate->data(0).toString()));
	}
	else if (duplicate) {
		emit duplicateSignal(tr("%1 already used by %2\nPress ESC to undo changes")
			.arg(duplicate->data(1).toString())
			.arg(duplicate->data(0).toString()));
	}
	else 
		emit duplicateSignal("");
}

void DkShortcutsModel::resetActions() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup("CustomShortcuts");

	for (int pIdx = 0; pIdx < actions.size(); pIdx++) {
		
		QVector<QAction*> cActions = actions.at(pIdx);
		
		for (int idx = 0; idx < cActions.size(); idx++) {
			QString val = settings.value(cActions[idx]->text(), "no-shortcut").toString();

			if (val != "no-shortcut") {
				cActions[idx]->setShortcut(QKeySequence());
			}
		}
	}

	settings.endGroup();
}

void DkShortcutsModel::saveActions() {

	if (!rootItem)
		return;

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup("CustomShortcuts");

	// loop all menu entries
	for (int idx = 0; idx < rootItem->childCount(); idx++) {

		TreeItem* cMenu = rootItem->child(idx);
		QVector<QAction*> cActions = actions.at(idx);

		// loop all action entries
		for (int mIdx = 0; mIdx < cMenu->childCount(); mIdx++) {

			TreeItem* cItem = cMenu->child(mIdx);
			QKeySequence ks = cItem->data(1).value<QKeySequence>();

			// if empty try to restore
			if (ks.isEmpty() && !rootItem->find(cActions.at(mIdx)->shortcut(), 1))
				continue;

			if (cActions.at(mIdx)->shortcut() != ks) {

				if (cActions.at(mIdx)->text().isEmpty()) {
					qDebug() << "empty action detected! shortcut is: " << ks;
					continue;
				}

				cActions.at(mIdx)->setShortcut(ks);		// assign new shortcut
				settings.setValue(cActions.at(mIdx)->text(), ks.toString());	// note this works as long as you don't change the language!
			}
		}
	}
	settings.endGroup();

}

// DkShortcutsDialog --------------------------------------------------------------------
DkShortcutsDialog::DkShortcutsDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

	createLayout();
}

void DkShortcutsDialog::createLayout() {

	setWindowTitle(tr("Keyboard Shortcuts"));

	QVBoxLayout* layout = new QVBoxLayout(this);

	// register our special shortcut editor
	QItemEditorFactory *factory = new QItemEditorFactory;

	QItemEditorCreatorBase *shortcutListCreator =
		new QStandardItemEditorCreator<DkShortcutEditor>();

	factory->registerEditor(QVariant::KeySequence, shortcutListCreator);

	QItemEditorFactory::setDefaultFactory(factory);

	// create our beautiful shortcut view
	model = new DkShortcutsModel();
	
	DkShortcutDelegate* scDelegate = new DkShortcutDelegate(this);

	treeView = new QTreeView(this);
	treeView->setModel(model);
	treeView->setItemDelegate(scDelegate);
	treeView->setAlternatingRowColors(true);
	treeView->setIndentation(8);

	notificationLabel = new QLabel(this);
	notificationLabel->setObjectName("DkDecentInfo");
	notificationLabel->setProperty("warning", true);
	//notificationLabel->setTextFormat(Qt::)

	defaultButton = new QPushButton(tr("Set to &Default"), this);
	defaultButton->setToolTip(tr("Removes All Custom Shortcuts"));
	connect(defaultButton, SIGNAL(clicked()), this, SLOT(defaultButtonClicked()));

	connect(scDelegate, SIGNAL(checkDuplicateSignal(QString, void*)), model, SLOT(checkDuplicate(QString, void*)));
	connect(model, SIGNAL(duplicateSignal(QString)), notificationLabel, SLOT(setText(QString)));

	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	buttons->addButton(defaultButton, QDialogButtonBox::ActionRole);
	
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	layout->addWidget(treeView);
	layout->addWidget(notificationLabel);
	//layout->addSpacing()
	layout->addWidget(buttons);
}

void DkShortcutsDialog::addActions(const QVector<QAction*>& actions, const QString& name) {

	QString cleanName = name;
	cleanName.remove("&");
	model->addDataActions(actions, cleanName);

}

void DkShortcutsDialog::contextMenu(const QPoint&) {

}

void DkShortcutsDialog::defaultButtonClicked() {

	if (model) model->resetActions();

	QSettings& settings = Settings::instance().getSettings();
	settings.remove("CustomShortcuts");

	QDialog::reject();
}

void DkShortcutsDialog::accept() {

	// assign shortcuts & save them if they are changed
	if (model) model->saveActions();

	QDialog::accept();
}

// DkTextDialog --------------------------------------------------------------------
DkTextDialog::DkTextDialog(QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */) : QDialog(parent, flags) {

	setWindowTitle(tr("Text Editor"));
	createLayout();
}

void DkTextDialog::createLayout() {

	textEdit = new QTextEdit(this);

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	buttons->button(QDialogButtonBox::Ok)->setDefault(true);	// ok is auto-default
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&Save"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Close"));

	connect(buttons, SIGNAL(accepted()), this, SLOT(save()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	// dialog layout
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(textEdit);
	layout->addWidget(buttons);
}

void DkTextDialog::setText(const QStringList& text) {
	textEdit->setText(text.join("\n"));
}

void DkTextDialog::save() {

	QStringList folders = DkSettings::global.recentFolders;
	QString savePath = QDir::rootPath();

	if (folders.size() > 0)
		savePath = folders.first();

	QString saveFilters("Text File (*.txt);;All Files (*.*)");

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Text File"),
		savePath, saveFilters);

	if (fileName.isEmpty())
		return;

	QFile file(fileName);
	
	if (file.open(QIODevice::WriteOnly)) {
		QTextStream stream(&file);
		stream << textEdit->toPlainText();
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("Could not save: %1\n%2").arg(fileName).arg(file.errorString()));
		return;
	}

	file.close();
	accept();
}

// DkUpdateDialog --------------------------------------------------------------------
DkUpdateDialog::DkUpdateDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {
	init();
}

void DkUpdateDialog::init() {
	createLayout();

	connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(okButton, SIGNAL(clicked()), this, SLOT(okButtonClicked()));
}

void DkUpdateDialog::createLayout() {
	setFixedWidth(300);
	setFixedHeight(150);
	setWindowTitle(tr("nomacs updater"));

	QGridLayout* gridlayout = new QGridLayout;
	upperLabel = new QLabel;
	upperLabel->setOpenExternalLinks(true);

	QWidget* lowerWidget = new QWidget;
	QHBoxLayout* hbox = new QHBoxLayout;
	okButton = new QPushButton(tr("Install Now"));
	cancelButton = new QPushButton(tr("Cancel"));
	hbox->addStretch();
	hbox->addWidget(okButton);
	hbox->addWidget(cancelButton);
	lowerWidget->setLayout(hbox);

	gridlayout->addWidget(upperLabel, 0, 0);
	gridlayout->addWidget(lowerWidget, 1, 0);
	
	setLayout(gridlayout);

}

void DkUpdateDialog::okButtonClicked() {
	emit startUpdate();
	close();
}

// DkPrintPreviewDialog --------------------------------------------------------------------

DkPrintPreviewDialog::DkPrintPreviewDialog(QImage img, float dpi, QPrinter* printer, QWidget* parent, Qt::WindowFlags flags) 
		: QMainWindow(parent, flags) {
	this->img = img;
	this->printer = printer;
	this->dpi = dpi;
	this->origdpi = dpi;
	printDialog = 0;
	imgTransform = QTransform();
	init();
	scaleImage();
}

void DkPrintPreviewDialog::setImage(QImage img, float dpi) {
	this->img = img;
	this->dpi = dpi;
	imgTransform = QTransform();
	scaleImage();
}

void DkPrintPreviewDialog::scaleImage() {
	QRectF rect = printer->pageRect();
	qreal scaleFactor;
	QSizeF paperSize = printer->paperSize(QPrinter::Inch);
	QRectF pageRectInch = printer->pageRect(QPrinter::Inch);

	// scale image to fit on paper
	if (rect.width()/img.width() < rect.height()/img.height()) {
		scaleFactor = rect.width()/(img.width()+FLT_EPSILON);		
	} else {
		scaleFactor = rect.height()/(img.height()+FLT_EPSILON);
	}

	float inchW = (float)printer->pageRect(QPrinter::Inch).width();
	float pxW = (float)printer->pageRect().width();
	dpi = (pxW/inchW)/(float)scaleFactor;


	qDebug() << "img:" << img.size();
	qDebug() << "paperInch:" << paperSize;
	qDebug() << "rect:" << rect;
	qDebug() << "rectInch:" << pageRectInch;
	qDebug() << "scaleFactor:" << scaleFactor;
	qDebug() << "dpi:" << dpi;
	qDebug() << "origDpi:" << origdpi;

	// use at least 150 dpi as default if image has less then 150
	if ((origdpi < 150 || dpi < 150) && scaleFactor > 1) {
		dpi = 150;
		scaleFactor = (pxW/inchW)/dpi;
	}

	imgTransform.scale(scaleFactor, scaleFactor);

	dpiFactor->lineEdit()->setText(QString().sprintf("%.0f", dpi)+dpiEditorSuffix);
	centerImage();
	updateZoomFactor();
}

void DkPrintPreviewDialog::init() {
	
	if (!printer) {
#ifdef WIN32
		printer = new QPrinter(QPrinter::HighResolution);
#else
		printer = new QPrinter;
#endif
	}
	
	preview = new DkPrintPreviewWidget(printer, this);
	connect(preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(paintRequested(QPrinter*)));
	connect(preview, SIGNAL(zoomChanged()), this, SLOT(updateZoomFactor()));
	
	createIcons();
	setup_Actions();
	createLayout();
	setMinimumHeight(600);
	setMinimumWidth(800);

}

void DkPrintPreviewDialog::createIcons() {

	icons.resize(print_end);

	icons[print_fit_width]	= QIcon(":/nomacs/img/fit-width.png");
	icons[print_fit_page]	= QIcon(":/nomacs/img/zoomReset.png");
	icons[print_zoom_in]	= QIcon(":/nomacs/img/zoom-in.png");
	icons[print_zoom_out]	= QIcon(":/nomacs/img/zoom-out.png");
	icons[print_reset_dpi]	= QIcon(":/nomacs/img/zoom100.png");
	icons[print_landscape]	= QIcon(":/nomacs/img/landscape.png");
	icons[print_portrait]	= QIcon(":/nomacs/img/portrait.png");
	icons[print_setup]		= QIcon(":/nomacs/img/print-setup.png");
	icons[print_printer]	= QIcon(":/nomacs/img/printer.png");

	if (!DkSettings::display.defaultIconColor) {
		// now colorize all icons
		for (int idx = 0; idx < icons.size(); idx++)
			icons[idx].addPixmap(DkImage::colorizePixmap(icons[idx].pixmap(100), DkSettings::display.iconColor));
	}
}

void DkPrintPreviewDialog::setup_Actions() {
	
	fitGroup = new QActionGroup(this);
	
	fitWidthAction = fitGroup->addAction(icons[print_fit_width], tr("Fit width"));
	fitPageAction = fitGroup->addAction(icons[print_fit_page], tr("Fit page"));
	fitWidthAction->setObjectName(QLatin1String("fitWidthAction"));
	fitPageAction->setObjectName(QLatin1String("fitPageAction"));
	fitWidthAction->setCheckable(true);
	fitPageAction->setCheckable(true);
	//setIcon(fitWidthAction, QLatin1String("fit-width"));
	//setIcon(fitPageAction, QLatin1String("fit-page"));
	QObject::connect(fitGroup, SIGNAL(triggered(QAction*)), this, SLOT(fitImage(QAction*)));

	// Zoom
	zoomGroup = new QActionGroup(this);
	
	zoomInAction = zoomGroup->addAction(icons[print_zoom_in], tr("Zoom in"));
	zoomInAction->setShortcut(Qt::Key_Plus);
	//preview->addAction(zoomInAction);
	//addAction(zoomInAction);
	//zoomInAction->setShortcut(QKeySequence::AddTab);
		//addAction(zoomInAction);
	zoomOutAction = zoomGroup->addAction(icons[print_zoom_out], tr("Zoom out"));
	zoomOutAction->setShortcut(QKeySequence(Qt::Key_Minus));
	//addAction(zoomOutAction);
	//preview->addAction(zoomOutAction);
	//setIcon(zoomInAction, QLatin1String("zoom-in"));
	//setIcon(zoomOutAction, QLatin1String("zoom-out"));

	// Portrait/Landscape
	orientationGroup = new QActionGroup(this);
	portraitAction = orientationGroup->addAction(icons[print_portrait], tr("Portrait"));
	landscapeAction = orientationGroup->addAction(icons[print_landscape], tr("Landscape"));
	portraitAction->setCheckable(true);
	landscapeAction->setCheckable(true);
	//setIcon(portraitAction, QLatin1String("layout-portrait"));
	//setIcon(landscapeAction, QLatin1String("layout-landscape"));
	QObject::connect(portraitAction, SIGNAL(triggered(bool)), preview, SLOT(setPortraitOrientation()));
	QObject::connect(portraitAction, SIGNAL(triggered(bool)), this, SLOT(centerImage()));
	QObject::connect(landscapeAction, SIGNAL(triggered(bool)), preview, SLOT(setLandscapeOrientation()));
	QObject::connect(landscapeAction, SIGNAL(triggered(bool)), this, SLOT(centerImage()));


	// Print
	printerGroup = new QActionGroup(this);
	printAction = printerGroup->addAction(icons[print_printer], tr("Print"));
	pageSetupAction = printerGroup->addAction(icons[print_setup], tr("Page setup"));
	//setIcon(printAction, QLatin1String("print"));
	//setIcon(pageSetupAction, QLatin1String("page-setup"));
	QObject::connect(printAction, SIGNAL(triggered(bool)), this, SLOT(print()));
	QObject::connect(pageSetupAction, SIGNAL(triggered(bool)), this, SLOT(pageSetup()));

	dpiGroup = new QActionGroup(this);
	resetDpiAction = dpiGroup->addAction(icons[print_reset_dpi], tr("Reset dpi"));
	//setIcon(resetDpiAction, QLatin1String("fit-width"));
	QObject::connect(resetDpiAction, SIGNAL(triggered(bool)), this, SLOT(resetDpi()));

}

void DkPrintPreviewDialog::createLayout() {

	zoomFactor = new QComboBox;
	zoomFactor->setEditable(true);
	zoomFactor->setMinimumContentsLength(7);
	zoomFactor->setInsertPolicy(QComboBox::NoInsert);
	QLineEdit *zoomEditor = new QLineEdit;
	zoomEditor->setValidator(new DkPrintPreviewValidator("%", 1,1000, 1, zoomEditor));
	zoomFactor->setLineEdit(zoomEditor);
	static const short factorsX2[] = { 25, 50, 100, 200, 250, 300, 400, 800, 1600 };
	for (int i = 0; i < int(sizeof(factorsX2) / sizeof(factorsX2[0])); ++i)
		zoomFactor->addItem(QString::number(factorsX2[i] / 2.0)+"%");
	QObject::connect(zoomFactor->lineEdit(), SIGNAL(editingFinished()), this, SLOT(zoomFactorChanged()));
	QObject::connect(zoomFactor, SIGNAL(currentIndexChanged(int)), this, SLOT(zoomFactorChanged()));

	QString zoomTip = tr("keep ALT key pressed to zoom with the mouse wheel");
	zoomFactor->setToolTip(zoomTip);
	zoomEditor->setToolTip(zoomTip);
	zoomOutAction->setToolTip(zoomTip);
	zoomInAction->setToolTip(zoomTip);

	// dpi selection
	dpiFactor = new QComboBox;
	dpiFactor->setEditable(true);
	dpiFactor->setMinimumContentsLength(5);
	dpiFactor->setInsertPolicy(QComboBox::NoInsert);
	QLineEdit* dpiEditor = new QLineEdit;
	dpiEditorSuffix = " dpi";
	dpiEditor->setValidator(new DkPrintPreviewValidator(dpiEditorSuffix, 1,1000, 1, zoomEditor));
	dpiFactor->setLineEdit(dpiEditor);
	static const short dpiFactors[] = {72, 150, 300, 600};
	for (int i = 0; i < int(sizeof(dpiFactors) / sizeof(dpiFactors[0])); i++)
		dpiFactor->addItem(QString::number(dpiFactors[i])+dpiEditorSuffix);
	QObject::connect(dpiFactor->lineEdit(), SIGNAL(editingFinished()), this, SLOT(dpiFactorChanged()));
	QObject::connect(dpiFactor, SIGNAL(currentIndexChanged(int)), this, SLOT(dpiFactorChanged()));

	QToolBar *toolbar = new QToolBar(tr("Print Preview"), this);
	toolbar->addAction(fitWidthAction);
	toolbar->addAction(fitPageAction);
	toolbar->addSeparator();
	toolbar->addWidget(zoomFactor);
	toolbar->addAction(zoomInAction);
	toolbar->addAction(zoomOutAction);
	toolbar->addSeparator();
	toolbar->addWidget(dpiFactor);
	toolbar->addAction(resetDpiAction);
	toolbar->addSeparator();
	toolbar->addAction(portraitAction);
	toolbar->addAction(landscapeAction);
	toolbar->addSeparator();
	//toolbar->addAction(firstPageAction);
	//toolbar->addAction(prevPageAction);
	//toolbar->addSeparator();
	toolbar->addAction(pageSetupAction);
	toolbar->addAction(printAction);

	if (DkSettings::display.toolbarGradient)
		toolbar->setObjectName("toolbarWithGradient");

	if (DkSettings::display.smallIcons)
		toolbar->setIconSize(QSize(16, 16));
	else
		toolbar->setIconSize(QSize(32, 32));


	// Cannot use the actions' triggered signal here, since it doesn't autorepeat
	QToolButton *zoomInButton = static_cast<QToolButton *>(toolbar->widgetForAction(zoomInAction));
	QToolButton *zoomOutButton = static_cast<QToolButton *>(toolbar->widgetForAction(zoomOutAction));
	zoomInButton->setAutoRepeat(true);
	zoomInButton->setAutoRepeatInterval(200);
	zoomInButton->setAutoRepeatDelay(200);
	zoomOutButton->setAutoRepeat(true);
	zoomOutButton->setAutoRepeatInterval(200);
	zoomOutButton->setAutoRepeatDelay(200);
	QObject::connect(zoomInButton, SIGNAL(clicked()), this, SLOT(zoomIn()));
	QObject::connect(zoomOutButton, SIGNAL(clicked()), this, SLOT(zoomOut()));


	this->addToolBar(toolbar);

	this->setCentralWidget(preview);
}

void DkPrintPreviewDialog::setIcon(QAction* action, const QLatin1String &name) {
	QLatin1String imagePrefix(":/trolltech/dialogs/qprintpreviewdialog/images/");
	QIcon icon;
	icon.addFile(imagePrefix + name + QLatin1String("-24.png"), QSize(24, 24));
	icon.addFile(imagePrefix + name + QLatin1String("-32.png"), QSize(32, 32));
	action->setIcon(icon);
}

void DkPrintPreviewDialog::paintRequested(QPrinter* printer) {
	QPainter painter(printer);
	QRect rect = painter.viewport();
	QSize size = img.size();
	painter.setWorldTransform(imgTransform);
	painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
	painter.setWindow(img.rect());
	painter.drawImage(0, 0, img);

	painter.end();

}

void DkPrintPreviewDialog::fitImage(QAction* action) {
	setFitting(true);
	if (action == fitPageAction)
		preview->fitInView();
	else
		preview->fitToWidth();
	updateZoomFactor();
}

bool DkPrintPreviewDialog::isFitting() {
	return (fitGroup->isExclusive() && (fitWidthAction->isChecked() || fitPageAction->isChecked()));
}

void DkPrintPreviewDialog::centerImage() {
	QRect imgRect = img.rect();
	QRectF transRect = imgTransform.mapRect(imgRect);
	qreal xtrans = 0, ytrans = 0;
	xtrans = ((printer->pageRect().width() - transRect.width())/2);
	ytrans = (printer->pageRect().height() - transRect.height())/2;

	imgTransform.translate(-imgTransform.dx()/(imgTransform.m11()+DBL_EPSILON), -imgTransform.dy()/(imgTransform.m22()+DBL_EPSILON)); // reset old transformation

	imgTransform.translate(xtrans/(imgTransform.m11()+DBL_EPSILON), ytrans/(imgTransform.m22()+DBL_EPSILON));
	preview->updatePreview();
}

void DkPrintPreviewDialog::setFitting(bool on) {
	if (isFitting() == on)
		return;
	fitGroup->setExclusive(on);
	if (on) {
		QAction* action = fitWidthAction->isChecked() ? fitWidthAction : fitPageAction;
		action->setChecked(true);
		if (fitGroup->checkedAction() != action) {
			// work around exclusitivity problem
			fitGroup->removeAction(action);
			fitGroup->addAction(action);
		}
	} else {
		fitWidthAction->setChecked(false);
		fitPageAction->setChecked(false);
	}
}

void DkPrintPreviewDialog::zoomIn() {
	setFitting(false);
	preview->zoomIn();
	updateZoomFactor();
}

void DkPrintPreviewDialog::zoomOut() {
	setFitting(false);
	preview->zoomOut();
	updateZoomFactor();
}

void DkPrintPreviewDialog::zoomFactorChanged() {
	QString text = zoomFactor->lineEdit()->text();
	bool ok;
	qreal factor = text.remove(QLatin1Char('%')).toFloat(&ok);
	factor = qMax(qreal(1.0), qMin(qreal(1000.0), factor));
	if (ok) {
		preview->setZoomFactor(factor/100.0);
		zoomFactor->setEditText(QString::fromLatin1("%1%").arg(factor));
		setFitting(false);
		updateZoomFactor();
	}
	updateZoomFactor();
}

void DkPrintPreviewDialog::updateZoomFactor()
{
	zoomFactor->lineEdit()->setText(QString().sprintf("%.1f%%", preview->zoomFactor()*100));
}

void DkPrintPreviewDialog::dpiFactorChanged() {
	
	QString text = dpiFactor->lineEdit()->text();
	bool ok;
	qreal factor = text.remove(dpiEditorSuffix).toFloat(&ok);
	if (ok) {
		imgTransform.reset();

		float inchW = (float)printer->pageRect(QPrinter::Inch).width();
		float pxW = (float)printer->pageRect().width();
		float scaleFactor = (float)((pxW/inchW)/factor);

		imgTransform.scale(scaleFactor, scaleFactor);

	}
	centerImage();
	preview->updatePreview();
}

void DkPrintPreviewDialog::updateDpiFactor(qreal dpi) {
	dpiFactor->lineEdit()->setText(QString().sprintf("%.0f", dpi)+dpiEditorSuffix);
}

void DkPrintPreviewDialog::resetDpi() {
	updateDpiFactor(origdpi);
	dpiFactorChanged();
}

void DkPrintPreviewDialog::pageSetup() {
	QPageSetupDialog pageSetup(printer, this);
	if (pageSetup.exec() == QDialog::Accepted) {
		// update possible orientation changes
		if (preview->orientation() == QPrinter::Portrait) {
			portraitAction->setChecked(true);
			preview->setPortraitOrientation();
		}else {
			landscapeAction->setChecked(true);
			preview->setLandscapeOrientation();
		}
		centerImage();
	}
}

void DkPrintPreviewDialog::print() {
//#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
//	if (printer->outputFormat() != QPrinter::NativeFormat) {
//		QString title;
//		QString suffix;
//		if (printer->outputFormat() == QPrinter::PdfFormat) {
//			title = QCoreApplication::translate("QPrintPreviewDialog", "Export to PDF");
//			suffix = QLatin1String(".pdf");
//		} else {
//			title = QCoreApplication::translate("QPrintPreviewDialog", "Export to PostScript");
//			suffix = QLatin1String(".ps");
//		}
//		QString fileName = QFileDialog::getSaveFileName(this, title, printer->outputFileName(),
//			QLatin1Char('*') + suffix);
//		if (!fileName.isEmpty()) {
//			if (QFileInfo(fileName).suffix().isEmpty())
//				fileName.append(suffix);
//			printer->setOutputFileName(fileName);
//		}
//		if (!printer->outputFileName().isEmpty())
//			preview->print();
//		this->accept();
//		return;
//	}
//#endif

	if (!printDialog)
		printDialog = new QPrintDialog(printer, this);
	if (printDialog->exec() == QDialog::Accepted) {
		preview->print();
		this->close();
	}
}

// DkPrintPreviewWidget --------------------------------------------------------------------
DkPrintPreviewWidget::DkPrintPreviewWidget(QPrinter* printer, QWidget* parent, Qt::WindowFlags flags) : QPrintPreviewWidget(printer, parent, flags) {
	// do nothing atm - bis zum bankomat
}

//void DkPrintPreviewWidget::paintEvent(QPaintEvent * event) {
//	qDebug() << "paintEvent";
//	QPrintPreviewWidget::paintEvent(event);
//}

void DkPrintPreviewWidget::wheelEvent(QWheelEvent *event) {

	if (event->modifiers() != Qt::AltModifier) {
		QPrintPreviewWidget::wheelEvent(event);
		return;
	}


	qreal delta = event->delta();
	if (DkSettings::display.invertZoom)
		delta *= -1;
	if (event->delta() > 0)
		zoomIn();
	else
		zoomOut();
	emit zoomChanged();

	QPrintPreviewWidget::wheelEvent(event);	
}

// DkOpacityDialog --------------------------------------------------------------------
DkOpacityDialog::DkOpacityDialog(QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f) {

	createLayout();
}

void DkOpacityDialog::createLayout() {

	QVBoxLayout* layout = new QVBoxLayout(this);

	slider = new DkSlider(tr("Window Opacity"), this);
	slider->setMinimum(5);

	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	layout->addWidget(slider);
	layout->addWidget(buttons);
}

int DkOpacityDialog::value() const {
	return slider->value();
}

// DkExportTiffDialog --------------------------------------------------------------------
DkExportTiffDialog::DkExportTiffDialog(QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QDialog(parent, f) {

	setWindowTitle(tr("Export Multi-Page TIFF"));
	createLayout();
	//setFixedSize(340, 400);		// due to the baseViewport we need fixed sized dialogs : (
	setAcceptDrops(true);
	processing = false;

	connect(this, SIGNAL(updateImage(QImage)), viewport, SLOT(setImage(QImage)));
	connect(&watcher, SIGNAL(finished()), this, SLOT(processingFinished()));
	connect(this, SIGNAL(infoMessage(QString)), msgLabel, SLOT(setText(QString)));
	connect(this, SIGNAL(updateProgress(int)), progress, SLOT(setValue(int)));
	QMetaObject::connectSlotsByName(this);
}

void DkExportTiffDialog::dropEvent(QDropEvent *event) {

	if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();

		setFile(url.toString());
	}
}

void DkExportTiffDialog::dragEnterEvent(QDragEnterEvent *event) {

	if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();
		QFileInfo file = QFileInfo(url.toString());

		if (file.exists() && file.suffix().indexOf(QRegExp("tif"), Qt::CaseInsensitive) != -1)
			event->acceptProposedAction();
	}

}


void DkExportTiffDialog::createLayout() {

	// progress bar
	progress = new QProgressBar(this);
	progress->hide();

	msgLabel = new QLabel(this);
	msgLabel->setObjectName("DkWarningInfo");
	msgLabel->hide();

	// open handles
	QLabel* openLabel = new QLabel(tr("Multi-Page TIFF:"), this);
	openLabel->setAlignment(Qt::AlignRight);

	QPushButton* openButton = new QPushButton(tr("&Browse"), this);
	openButton->setObjectName("openButton");

	tiffLabel = new QLabel(tr("No Multi-Page TIFF loaded"), this);

	// save handles
	QLabel* saveLabel = new QLabel(tr("Save Folder:"), this);
	saveLabel->setAlignment(Qt::AlignRight);

	QPushButton* saveButton = new QPushButton(tr("&Browse"), this);
	saveButton->setObjectName("saveButton");

	folderLabel = new QLabel(tr("Specify a Save Folder"), this);

	// file name handles
	QLabel* fileLabel = new QLabel(tr("Filename:"), this);
	fileLabel->setAlignment(Qt::AlignRight);

	fileEdit = new QLineEdit("tiff_page", this);
	fileEdit->setObjectName("fileEdit");

	suffixBox = new QComboBox(this);
	suffixBox->addItems(DkSettings::app.saveFilters);
	suffixBox->setCurrentIndex(DkSettings::app.saveFilters.indexOf(QRegExp(".*tif.*")));

	// export handles
	QLabel* exportLabel = new QLabel(tr("Export Pages"));
	exportLabel->setAlignment(Qt::AlignRight);

	fromPage = new QSpinBox(0);

	toPage = new QSpinBox(0);

	overwrite = new QCheckBox(tr("Overwrite"));

	controlWidget = new QWidget(this);
	QGridLayout* controlLayout = new QGridLayout(controlWidget);
	controlLayout->addWidget(openLabel, 0, 0);
	controlLayout->addWidget(openButton, 0, 1, 1, 2);
	controlLayout->addWidget(tiffLabel, 0, 3, 1, 2);
	//controlLayout->setColumnStretch(3, 1);

	controlLayout->addWidget(saveLabel, 1, 0);
	controlLayout->addWidget(saveButton, 1, 1, 1, 2);
	controlLayout->addWidget(folderLabel, 1, 3, 1, 2);
	//controlLayout->setColumnStretch(3, 1);

	controlLayout->addWidget(fileLabel, 2, 0);
	controlLayout->addWidget(fileEdit, 2, 1, 1, 2);
	controlLayout->addWidget(suffixBox, 2, 3, 1, 2);
	//controlLayout->setColumnStretch(3, 1);

	controlLayout->addWidget(exportLabel, 3, 0);
	controlLayout->addWidget(fromPage, 3, 1);
	controlLayout->addWidget(toPage, 3, 2);
	controlLayout->addWidget(overwrite, 3, 3);
	controlLayout->setColumnStretch(5, 1);

	// shows the image if it could be loaded
	viewport = new DkBaseViewPort(this);
	viewport->setForceFastRendering(true);
	viewport->setPanControl(QPointF(0.0f, 0.0f));

	// buttons
	buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&Export"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(viewport);
	layout->addWidget(progress);
	layout->addWidget(msgLabel);
	layout->addWidget(controlWidget);
	layout->addWidget(buttons);

	enableTIFFSave(false);
}

void DkExportTiffDialog::on_openButton_pressed() {

	// load system default open dialog
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open TIFF"),
		cFile.absolutePath(), 
		DkSettings::app.saveFilters.filter(QRegExp(".*tif.*")).join(";;"));

	setFile(fileName);
}

void DkExportTiffDialog::on_saveButton_pressed() {
	qDebug() << "save triggered...";

	// load system default open dialog
	QString dirName = QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"),
		saveDir.absolutePath());

	if (saveDir.exists()) {
		saveDir = dirName;
		folderLabel->setText(saveDir.absolutePath());
	}
}

void DkExportTiffDialog::on_fileEdit_textChanged(const QString& filename) {

	qDebug() << "new file name: " << filename;
}

void DkExportTiffDialog::reject() {

	// not sure if this is a nice way to do: but we change cancel behavior while processing
	if (processing)
		processing = false;
	else
		QDialog::reject();

}

void DkExportTiffDialog::accept() {

	progress->setMinimum(fromPage->value()-1);
	progress->setMaximum(toPage->value());
	progress->setValue(progress->minimum());
	progress->show();
	msgLabel->show();

	enableAll(false);

	QString suffix = suffixBox->currentText();

	for (int idx = 0; idx < DkSettings::app.fileFilters.size(); idx++) {
		if (suffix.contains("(" + DkSettings::app.fileFilters.at(idx))) {
			suffix = DkSettings::app.fileFilters.at(idx);
			suffix.replace("*","");
			break;
		}
	}

	QFileInfo sFile(saveDir, fileEdit->text() + "-" + suffix);
	
	QFuture<int> future = QtConcurrent::run(this, 
		&nmc::DkExportTiffDialog::exportImages,
		cFile,
		sFile, 
		fromPage->value(), 
		toPage->value(),
		overwrite->isChecked());
	watcher.setFuture(future);
}

void DkExportTiffDialog::processingFinished() {

	enableAll(true);
	progress->hide();
	msgLabel->hide();

	if (watcher.future() == QDialog::Accepted)
		QDialog::accept();
}

int DkExportTiffDialog::exportImages(QFileInfo file, QFileInfo saveFile, int from, int to, bool overwrite) {

	processing = true;

	// Do your job
	for (int idx = from; idx <= to; idx++) {

		QFileInfo sFile(saveFile.absolutePath(), saveFile.baseName() + QString::number(idx) + "." + saveFile.suffix());
		qDebug() << "trying to save: " << sFile.absoluteFilePath();

		// user wants to overwrite files
		if (sFile.exists() && overwrite) {
			QFile f(sFile.absoluteFilePath());
			f.remove();
		}

		QFileInfo saveFile = loader.save(sFile, loader.image(), 90);		//TODO: ask user for compression?
		saveFile.refresh();

		if (!saveFile.exists() || !saveFile.isFile())
			emit infoMessage(tr("Sorry, I could not save: %1").arg(sFile.fileName()));

		if (!loader.loadPage(1))	// load next
			emit infoMessage(tr("Sorry, I could not load page: %1").arg(idx));
		emit updateImage(loader.image());
		emit updateProgress(idx);

		// user canceled?
		if (!processing)
			return QDialog::Rejected;
	}

	processing = false;

	return QDialog::Accepted;
}

void DkExportTiffDialog::setFile(const QFileInfo& file) {
	
	if (!file.exists())
		return;
	
	cFile = file;
	saveDir = file.absolutePath();
	folderLabel->setText(saveDir.absolutePath());
	tiffLabel->setText(file.absoluteFilePath());
	fileEdit->setText(file.baseName());

	loader.loadGeneral(cFile, true);
	viewport->setImage(loader.image());

	enableTIFFSave(loader.getNumPages() > 1);

	fromPage->setRange(1, loader.getNumPages());
	toPage->setRange(1, loader.getNumPages());

	fromPage->setValue(1);
	toPage->setValue(loader.getNumPages());
}

void DkExportTiffDialog::enableAll(bool enable) {

	enableTIFFSave(enable);
	controlWidget->setEnabled(enable);
}

void DkExportTiffDialog::enableTIFFSave(bool enable) {

	fileEdit->setEnabled(enable);
	suffixBox->setEnabled(enable);
	fromPage->setEnabled(enable);
	toPage->setEnabled(enable);
	buttons->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

#ifdef WITH_OPENCV
// DkUnsharpDialog --------------------------------------------------------------------
DkUnsharpDialog::DkUnsharpDialog(QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QDialog(parent, f) {

	processing = false;

	setWindowTitle(tr("Sharpen Image"));
	createLayout();
	//setFixedSize(340, 400);		// due to the baseViewport we need fixed sized dialogs : (
	setAcceptDrops(true);

	connect(this, SIGNAL(updateImage(QImage)), viewport, SLOT(setImage(QImage)));
	connect(&unsharpWatcher, SIGNAL(finished()), this, SLOT(unsharpFinished()));
	connect(viewport, SIGNAL(imageUpdated()), this, SLOT(computePreview()));
	QMetaObject::connectSlotsByName(this);
}

void DkUnsharpDialog::dropEvent(QDropEvent *event) {

	if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();

		setFile(url.toString());
	}
}

void DkUnsharpDialog::dragEnterEvent(QDragEnterEvent *event) {

	if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();
		QFileInfo file = QFileInfo(url.toString());

		if (file.exists() && DkImageLoader::isValid(file))
			event->acceptProposedAction();
	}
}

void DkUnsharpDialog::createLayout() {

	// post processing sliders
	sigmaSlider = new DkSlider(tr("Sigma"), this);
	sigmaSlider->setObjectName("sigmaSlider");
	sigmaSlider->setValue(30);
	//darkenSlider->hide();

	amountSlider = new DkSlider(tr("Amount"), this);
	amountSlider->setObjectName("amountSlider");
	amountSlider->setValue(45);

	QWidget* sliderWidget = new QWidget(this);
	QVBoxLayout* sliderLayout = new QVBoxLayout(sliderWidget);
	sliderLayout->addWidget(sigmaSlider);
	sliderLayout->addWidget(amountSlider);

	// shows the image if it could be loaded
	viewport = new DkBaseViewPort(this);
	viewport->setForceFastRendering(true);
	viewport->setPanControl(QPointF(0.0f, 0.0f));

	preview = new QLabel(this);
	preview->setScaledContents(true);
	preview->setMinimumSize(QSize(200,200));
	//preview->setForceFastRendering(true);
	//preview->setPanControl(QPointF(0.0f, 0.0f));

	QWidget* viewports = new QWidget(this);
	QGridLayout* viewLayout = new QGridLayout(viewports);
	viewLayout->setColumnStretch(0,1);
	viewLayout->setColumnStretch(1,1);

	viewLayout->addWidget(viewport, 0,0);
	viewLayout->addWidget(preview, 0, 1);

	// buttons
	buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	//buttons->button(QDialogButtonBox::Save)->setText(tr("&Save"));
	//buttons->button(QDialogButtonBox::Apply)->setText(tr("&Generate"));
	//buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	//connect(buttons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
	//buttons->button(QDialogButtonBox::Save)->setEnabled(false);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(viewports);
	layout->addWidget(sliderWidget);
	layout->addWidget(buttons);
}

void DkUnsharpDialog::on_sigmaSlider_valueChanged(int) {

	computePreview();
}

void DkUnsharpDialog::on_amountSlider_valueChanged(int) {

	computePreview();
}

QImage DkUnsharpDialog::getImage() {

	return computeUnsharp(img, sigmaSlider->value(), amountSlider->value());
}

void DkUnsharpDialog::reject() {

	QDialog::reject();

}

//void DkMosaicDialog::buttonClicked(QAbstractButton* button) {
//
//	if (button == buttons->button(QDialogButtonBox::Save)) {
//
//		// render the full image
//		if (!mosaic.isNull()) {
//			sliderWidget->hide();
//			progress->setValue(progress->minimum());
//			progress->show();
//			enableAll(false);
//			button->setEnabled(false);
//
//			QFuture<bool> future = QtConcurrent::run(this, 
//				&nmc::DkMosaicDialog::postProcessMosaic,
//				darkenSlider->value()/100.0f,
//				lightenSlider->value()/100.0f, 
//				saturationSlider->value()/100.0f,
//				false);
//			postProcessWatcher.setFuture(future);
//		}
//	}
//	else if (button == buttons->button(QDialogButtonBox::Apply))
//		compute();
//}

void DkUnsharpDialog::computePreview() {
		
	if (processing)
		return;

	QFuture<QImage> future = QtConcurrent::run(this, 
		&nmc::DkUnsharpDialog::computeUnsharp,
		viewport->getCurrentImageRegion(),
		sigmaSlider->value(),
		amountSlider->value()); 
	unsharpWatcher.setFuture(future);
	processing = true;
}

void DkUnsharpDialog::unsharpFinished() {

	QImage img = unsharpWatcher.result();
	img = img.scaled(preview->size(), Qt::KeepAspectRatio, Qt::FastTransformation);
	preview->setPixmap(QPixmap::fromImage(img));

	//update();
	processing = false;
}

QImage DkUnsharpDialog::computeUnsharp(const QImage img, int sigma, int amount) {

	QImage imgC = img.copy();
	DkImage::unsharpMask(imgC, (float)sigma, 1.0f+amount/100.0f);
	return imgC;
}

void DkUnsharpDialog::setImage(const QImage& img) {
	this->img = img;
	viewport->setImage(img);
	viewport->fullView();
	viewport->zoomConstraints(viewport->get100Factor());
	computePreview();
}

void DkUnsharpDialog::setFile(const QFileInfo& file) {

	DkBasicLoader loader;
	loader.loadGeneral(file, true);
	setImage(loader.image());
}

// DkMosaicDialog --------------------------------------------------------------------
DkMosaicDialog::DkMosaicDialog(QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QDialog(parent, f) {

	processing = false;
	postProcessing = false;
	updatePostProcessing = false;

	setWindowTitle(tr("Create Mosaic Image"));
	createLayout();
	//setFixedSize(340, 400);		// due to the baseViewport we need fixed sized dialogs : (
	setAcceptDrops(true);

	connect(this, SIGNAL(updateImage(QImage)), preview, SLOT(setImage(QImage)));
	connect(&mosaicWatcher, SIGNAL(finished()), this, SLOT(mosaicFinished()));
	connect(&postProcessWatcher, SIGNAL(finished()), this, SLOT(postProcessFinished()));
	connect(&postProcessWatcher, SIGNAL(canceled()), this, SLOT(postProcessFinished()));
	connect(this, SIGNAL(infoMessage(QString)), msgLabel, SLOT(setText(QString)));
	connect(this, SIGNAL(updateProgress(int)), progress, SLOT(setValue(int)));
	QMetaObject::connectSlotsByName(this);
}

void DkMosaicDialog::dropEvent(QDropEvent *event) {

	if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();

		setFile(url.toString());
	}
}

void DkMosaicDialog::dragEnterEvent(QDragEnterEvent *event) {

	if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();
		QFileInfo file = QFileInfo(url.toString());

		if (file.exists() && DkImageLoader::isValid(file))
			event->acceptProposedAction();
	}
}

void DkMosaicDialog::createLayout() {

	// progress bar
	progress = new QProgressBar(this);
	progress->hide();

	msgLabel = new QLabel(this);
	msgLabel->setObjectName("DkWarningInfo");
	msgLabel->hide();

	// post processing sliders
	darkenSlider = new QSlider(Qt::Horizontal, this);
	darkenSlider->setObjectName("darkenSlider");
	darkenSlider->setValue(40);
	//darkenSlider->hide();

	lightenSlider = new QSlider(Qt::Horizontal, this);
	lightenSlider->setObjectName("lightenSlider");
	lightenSlider->setValue(40);
	//lightenSlider->hide();

	saturationSlider = new QSlider(Qt::Horizontal, this);
	saturationSlider->setObjectName("saturationSlider");
	saturationSlider->setValue(60);
	//saturationSlider->hide();

	sliderWidget = new QWidget(this);
	QGridLayout* sliderLayout = new QGridLayout(sliderWidget);
	sliderLayout->addWidget(new QLabel(tr("Darken")), 0, 0);
	sliderLayout->addWidget(new QLabel(tr("Lighten")), 0, 1);
	sliderLayout->addWidget(new QLabel(tr("Saturation")), 0, 2);

	sliderLayout->addWidget(darkenSlider, 1, 0);
	sliderLayout->addWidget(lightenSlider, 1, 1);
	sliderLayout->addWidget(saturationSlider, 1, 2);
	sliderWidget->hide();

	// open handles
	QLabel* openLabel = new QLabel(tr("Mosaic Image:"), this);
	openLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	QPushButton* openButton = new QPushButton(tr("&Browse"), this);
	openButton->setObjectName("openButton");
	openButton->setToolTip(tr("Specify the Root Folder of the Image Database Desired."));

	fileLabel = new QLabel(tr("No Image loaded"), this);

	// save handles
	QLabel* saveLabel = new QLabel(tr("Image Database:"), this);
	saveLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	QPushButton* dbButton = new QPushButton(tr("&Browse"), this);
	dbButton->setObjectName("dbButton");

	folderLabel = new QLabel(tr("Specify an Image Database"), this);

	// resolution handles
	QLabel* sizeLabel = new QLabel(tr("Resolution:"));
	sizeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	newWidthBox = new QSpinBox();
	newWidthBox->setObjectName("newWidthBox");
	newWidthBox->setToolTip(tr("Pixel Width"));
	newWidthBox->setMinimum(100);
	newWidthBox->setMaximum(30000);
	newHeightBox = new QSpinBox();
	newHeightBox->setObjectName("newHeightBox");
	newHeightBox->setToolTip(tr("Pixel Height"));
	newHeightBox->setMinimum(100);
	newHeightBox->setMaximum(30000);
	realResLabel = new QLabel("");
	//realResLabel->setToolTip(tr("."));

	// num patch handles
	QLabel* patchLabel = new QLabel(tr("Patches:"));
	patchLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	numPatchesH = new QSpinBox(this);
	numPatchesH->setObjectName("numPatchesH");
	numPatchesH->setToolTip(tr("Number of Horizontal Patches"));
	numPatchesH->setMinimum(1);
	numPatchesH->setMaximum(1000);
	numPatchesV = new QSpinBox(this);
	numPatchesV->setObjectName("numPatchesV");
	numPatchesV->setToolTip(tr("Number of Vertical Patches"));
	numPatchesV->setMinimum(1);
	numPatchesV->setMaximum(1000);
	patchResLabel = new QLabel("", this);
	patchResLabel->setObjectName("DkDecentInfo");
	patchResLabel->setToolTip(tr("If this label turns red, the computation might be slower."));

	// file filters
	QLabel* filterLabel = new QLabel(tr("Filters:"), this);
	filterLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	filterEdit = new QLineEdit("", this);
	filterEdit->setObjectName("fileEdit");
	filterEdit->setToolTip(tr("You can split multiple ignore words with ;"));

	QStringList filters = DkSettings::app.openFilters;
	filters.pop_front();	// replace for better readability
	filters.push_front("All Images");
	suffixBox = new QComboBox(this);
	suffixBox->addItems(filters);
	//suffixBox->setCurrentIndex(DkImageLoader::saveFilters.indexOf(QRegExp(".*tif.*")));

	controlWidget = new QWidget(this);
	QGridLayout* controlLayout = new QGridLayout(controlWidget);
	controlLayout->addWidget(openLabel, 0, 0);
	controlLayout->addWidget(openButton, 0, 1, 1, 2);
	controlLayout->addWidget(fileLabel, 0, 3, 1, 2);
	//controlLayout->setColumnStretch(3, 1);

	controlLayout->addWidget(saveLabel, 1, 0);
	controlLayout->addWidget(dbButton, 1, 1, 1, 2);
	controlLayout->addWidget(folderLabel, 1, 3, 1, 2);
	//controlLayout->setColumnStretch(3, 1);

	controlLayout->addWidget(sizeLabel, 2, 0);
	controlLayout->addWidget(newWidthBox, 2, 1);
	controlLayout->addWidget(newHeightBox, 2, 2);
	controlLayout->addWidget(realResLabel, 2, 3);

	controlLayout->addWidget(patchLabel, 4, 0);
	controlLayout->addWidget(numPatchesH, 4, 1);
	controlLayout->addWidget(numPatchesV, 4, 2);
	controlLayout->addWidget(patchResLabel, 4, 3);

	controlLayout->addWidget(filterLabel, 5, 0);
	controlLayout->addWidget(filterEdit, 5, 1, 1, 2);
	controlLayout->addWidget(suffixBox, 5, 3, 1, 2);
	controlLayout->setColumnStretch(5, 1);

	// shows the image if it could be loaded
	viewport = new DkBaseViewPort(this);
	viewport->setForceFastRendering(true);
	viewport->setPanControl(QPointF(0.0f, 0.0f));

	preview = new DkBaseViewPort(this);
	preview->setForceFastRendering(true);
	preview->setPanControl(QPointF(0.0f, 0.0f));
	preview->hide();

	QWidget* viewports = new QWidget(this);
	QHBoxLayout* viewLayout = new QHBoxLayout(viewports);
	viewLayout->addWidget(viewport);
	viewLayout->addWidget(preview);

	// buttons
	buttons = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Save)->setText(tr("&Save"));
	buttons->button(QDialogButtonBox::Apply)->setText(tr("&Generate"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	//connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
	buttons->button(QDialogButtonBox::Save)->setEnabled(false);
	
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(viewports);
	layout->addWidget(progress);
	layout->addWidget(sliderWidget);
	layout->addWidget(msgLabel);
	layout->addWidget(controlWidget);
	layout->addWidget(buttons);

	enableMosaicSave(false);
}

void DkMosaicDialog::on_openButton_pressed() {

	// load system default open dialog
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open TIFF"),
		cFile.absolutePath(), 
		DkSettings::app.openFilters.join(";;"));

	setFile(fileName);
}

void DkMosaicDialog::on_dbButton_pressed() {
	qDebug() << "save triggered...";

	// load system default open dialog
	QString dirName = QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"),
		saveDir.absolutePath());

	if (QFileInfo(dirName).exists()) {
		saveDir = dirName;
		folderLabel->setText(saveDir.absolutePath());
	}
}

void DkMosaicDialog::on_fileEdit_textChanged(const QString& filename) {

	qDebug() << "new file name: " << filename;
}

void DkMosaicDialog::on_newWidthBox_valueChanged(int) {

	if (!loader.hasImage())
		return;

	newHeightBox->blockSignals(true);
	newHeightBox->setValue(qRound((float)newWidthBox->value()/loader.image().width()*loader.image().height()));
	newHeightBox->blockSignals(false);
	realResLabel->setText(tr("%1 x %2 cm @150 dpi").arg(newWidthBox->value()/150.0*2.54, 0, 'f', 1).arg(newHeightBox->value()/150.0*2.54, 0, 'f', 1));
	updatePatchRes();
}

void DkMosaicDialog::on_newHeightBox_valueChanged(int) {

	if (!loader.hasImage())
		return;

	newWidthBox->blockSignals(true);
	newWidthBox->setValue(qRound((float)newHeightBox->value()/loader.image().height()*loader.image().width()));
	newWidthBox->blockSignals(false);
	realResLabel->setText(tr("%1 x %2 cm @150 dpi").arg(newWidthBox->value()/150.0*2.54, 0, 'f', 1).arg(newHeightBox->value()/150.0*2.54, 0, 'f', 1));
	updatePatchRes();
}

void DkMosaicDialog::on_numPatchesH_valueChanged(int) {

	if (!loader.hasImage())
		return;

	numPatchesV->blockSignals(true);
	numPatchesV->setValue(qFloor((float)loader.image().height()/((float)loader.image().width()/numPatchesH->value())));
	numPatchesV->blockSignals(false);
	updatePatchRes();
}

void DkMosaicDialog::on_numPatchesV_valueChanged(int) {
	
	if (!loader.hasImage())
		return;

	numPatchesH->blockSignals(true);
	numPatchesH->setValue(qFloor((float)loader.image().width()/((float)loader.image().height()/numPatchesV->value())));
	numPatchesH->blockSignals(false);
	updatePatchRes();
}

void DkMosaicDialog::on_darkenSlider_valueChanged(int) {

	updatePostProcess();
}

void DkMosaicDialog::on_lightenSlider_valueChanged(int) {

	updatePostProcess();
}

void DkMosaicDialog::on_saturationSlider_valueChanged(int) {

	updatePostProcess();
}

void DkMosaicDialog::updatePatchRes() {

	int patchResD = qFloor((float)newWidthBox->value()/numPatchesH->value());

	patchResLabel->setText(tr("Patch Resolution: %1 px").arg(patchResD));
	patchResLabel->show();

	// show the user if we can work with the thumbnails or not
	if (patchResD > 97)
		patchResLabel->setProperty("warning", true);
	else
		patchResLabel->setProperty("warning", false);

	patchResLabel->style()->unpolish(patchResLabel);
	patchResLabel->style()->polish(patchResLabel);
	patchResLabel->update();
}

QImage DkMosaicDialog::getImage() {

	if (mosaic.isNull() && !mosaicMat.empty())
		return DkImage::mat2QImage(mosaicMat);

	return mosaic;
}

void DkMosaicDialog::reject() {

	// not sure if this is a nice way to do: but we change cancel behavior while processing
	if (processing)
		processing = false;
	else if (!mosaic.isNull() && !buttons->button(QDialogButtonBox::Apply)->isEnabled()) {
		buttons->button(QDialogButtonBox::Apply)->setEnabled(true);
		enableAll(true);
		viewport->show();
		sliderWidget->hide();
	}
	else
		QDialog::reject();

}

void DkMosaicDialog::buttonClicked(QAbstractButton* button) {

	if (button == buttons->button(QDialogButtonBox::Save)) {

		// render the full image
		if (!mosaic.isNull()) {
			sliderWidget->hide();
			progress->setValue(progress->minimum());
			progress->show();
			enableAll(false);
			button->setEnabled(false);
			
			QFuture<bool> future = QtConcurrent::run(this, 
				&nmc::DkMosaicDialog::postProcessMosaic,
				darkenSlider->value()/100.0f,
				lightenSlider->value()/100.0f, 
				saturationSlider->value()/100.0f,
				false);
			postProcessWatcher.setFuture(future);
		}
	}
	else if (button == buttons->button(QDialogButtonBox::Apply))
		compute();
}

void DkMosaicDialog::compute() {

	if (postProcessing)
		return;

	progress->setValue(progress->minimum());
	progress->show();
	msgLabel->setText("");
	msgLabel->show();
	mosaicMatSmall.release();
	mosaicMat.release();
	origImg.release();
	mosaic = QImage();
	sliderWidget->hide();
	viewport->show();
	preview->setForceFastRendering(true);
	preview->show();

	enableAll(false);

	QString suffixTmp = suffixBox->currentText();
	QString suffix;

	for (int idx = 0; idx < DkSettings::app.fileFilters.size(); idx++) {
		if (suffixTmp.contains("(" + DkSettings::app.fileFilters.at(idx))) {
			suffix = DkSettings::app.fileFilters.at(idx);
			break;
		}
	}

	QString filter = filterEdit->text();

	QFuture<int> future = QtConcurrent::run(this, 
		&nmc::DkMosaicDialog::computeMosaic,
		cFile,
		filter,
		suffix, 
		newWidthBox->value(), 
		numPatchesH->value());
	mosaicWatcher.setFuture(future);

	//// debug
	//computeMosaic(
	//	cFile,
	//	filter,
	//	suffix,
	//	fromPage->value(), 
	//	toPage->value(),
	//	overwrite->isChecked());

}

void DkMosaicDialog::mosaicFinished() {

	progress->hide();
	//msgLabel->hide();
	
	if (!mosaicMat.empty()) {
		sliderWidget->show();
		msgLabel->hide();
		viewport->hide();
		preview->setForceFastRendering(false);

		updatePostProcess();	// add values
		buttons->button(QDialogButtonBox::Save)->setEnabled(true);
	}
	else
		enableAll(true);
}

int DkMosaicDialog::computeMosaic(QFileInfo file, QString filter, QString suffix, int newWidth, int numPatchesH) {

	DkTimer dt;
	processing = true;

	// compute new image size
	cv::Mat mImg = DkImage::qImage2Mat(loader.image());

	filesUsed.clear();
	QSize numPatches = QSize(numPatchesH, 0);

	// compute new image size
	//float aratio = (float)mImg.rows/mImg.cols;
	int patchResO = qFloor((float)mImg.cols/numPatches.width());
	numPatches.setHeight(qFloor((float)mImg.rows/patchResO));

	int patchResD = qFloor(newWidth/numPatches.width());

	float shR = (mImg.rows-patchResO*numPatches.height())/2.0f;
	float shC = (mImg.cols-patchResO*numPatches.width())/2.0f;

	mImg = mImg.rowRange(qFloor(shR), mImg.rows-qCeil(shR)).colRange(qFloor(shC), mImg.cols-qCeil(shC));
	cv::Mat mImgLab;
	cv::cvtColor(mImg, mImgLab, CV_RGB2Lab);
	std::vector<Mat> channels;
	cv::split(mImgLab, channels);
	cv::Mat imgL = channels[0];

	// keeps track of the weights
	cv::Mat cc(numPatches.height(), numPatches.width(), CV_32FC1);
	cc.setTo(0);
	cv::Mat ccD(numPatches.height(), numPatches.width(), CV_8UC1);	// tells us if we have already computed the real patch

	filesUsed.resize(numPatches.height()*numPatches.width());

	// destination image
	cv::Mat dImg(patchResD*numPatches.height(), patchResD*numPatches.width(), CV_8UC1);
	dImg = 255;

	// patch image (preview)
	cv::Mat pImg(patchResO*numPatches.height(), patchResO*numPatches.width(), CV_8UC1);
	pImg = 255;

	qDebug() << "mosaic data --------------------------------";
	qDebug() << "patchRes: " << patchResD;
	qDebug() << "new resolution: " << dImg.cols << " x " << dImg.rows;
	qDebug() << "num patches: " << numPatches.width() << " x " << numPatches.height();
	qDebug() << "mosaic data --------------------------------";

	// progress bar
	int pIdx = 0;
	int maxP = numPatches.width()*numPatches.height();

	int iDidNothing = 0;
	bool force = false;
	bool useTwice = false;

	//for (int idx = 0; idx < 10; idx++) {
	while (iDidNothing < 10000) {

		if (!processing)
			return QDialog::Rejected;

		if (iDidNothing > 100) {
			force = true;

			if (!useTwice)
				emit infoMessage(tr("Filling empty areas..."));
		}

		if (iDidNothing > 400 && !useTwice) {
			emit infoMessage(tr("I need to use some images twice - maybe the database is too small?"));
			iDidNothing = 0;
			useTwice = true;
		}
		else if (iDidNothing > 400) {
			emit infoMessage(tr("Sorry, it seems that i cannot create your mosaic with this database."));
			return QDialog::Rejected;
		}

		QString imgPath = getRandomImagePath(saveDir.absolutePath(), filter, suffix);

		if (!useTwice && filesUsed.contains(QFileInfo(imgPath))) {
			iDidNothing++;
			continue;
		}

		try {

			DkThumbNail thumb = DkThumbNail(QFileInfo(imgPath));
			thumb.setMinThumbSize(patchResO);
			thumb.setRescale(false);
			thumb.compute();

			if (!thumb.hasImage()) {
				iDidNothing++;
				continue;
			}

			cv::Mat ccTmp(cc.size(), cc.depth());
		
			if (!force)
				ccTmp.setTo(0);
			else
				ccTmp = cc.clone();

			cv::Mat thumbPatch = createPatch(thumb, patchResO);
		
			if (thumbPatch.rows != patchResO || thumbPatch.cols != patchResO) {
				iDidNothing++;
				continue;
			}
		
			matchPatch(imgL, thumbPatch, patchResO, ccTmp);

			if (force) {
				cv::Mat mask = (cc == 0);
				mask.convertTo(mask, CV_32FC1, 1.0f/255.0f);
				ccTmp = ccTmp.mul(mask);
			}
				
			double maxVal = 0;
			cv::Point maxIdx;
			cv::minMaxLoc(ccTmp, 0, &maxVal, 0, &maxIdx);
			float* ccPtr = cc.ptr<float>(maxIdx.y);

			if (maxVal > ccPtr[maxIdx.x]) {

				cv::Mat pPatch = pImg.rowRange(maxIdx.y*patchResO, maxIdx.y*patchResO+patchResO)
					.colRange(maxIdx.x*patchResO, maxIdx.x*patchResO+patchResO);
				thumbPatch.copyTo(pPatch);
			
				// visualize
				if (pIdx % 10 == 0) {
				
					channels[0] = pImg;
				
					//debug
					cv::Mat imgT3;
					cv::merge(channels, imgT3);
					cv::cvtColor(imgT3, imgT3, CV_Lab2BGR);
					emit updateImage(DkImage::mat2QImage(imgT3));
				}

				if (ccPtr[maxIdx.x] == 0) {
					pIdx++;
					emit updateProgress(qRound((float)pIdx/maxP*100));
				}

				// compute it now if we already have the full image loaded
				if (thumb.getImage().width() > patchResD && thumb.getImage().height() > patchResD) {
					thumbPatch = createPatch(thumb, patchResD);

					cv::Mat dPatch = dImg.rowRange(maxIdx.y*patchResD, maxIdx.y*patchResD+patchResD)
						.colRange(maxIdx.x*patchResD, maxIdx.x*patchResD+patchResD);
					thumbPatch.copyTo(dPatch);
					ccD.ptr<unsigned char>(maxIdx.y)[maxIdx.x] = 1;
				}
				else
					ccD.ptr<unsigned char>(maxIdx.y)[maxIdx.x] = 0;

				// update cc
				ccPtr[maxIdx.x] = (float)maxVal;

				filesUsed[maxIdx.y*numPatchesH+maxIdx.x] = thumb.getFile();	// replaces additionally the old file
				iDidNothing = 0;
			}
			else
				iDidNothing++;
		} 
		// catch cv exceptions e.g. out of memory
		catch(...) {
			iDidNothing++;
		}

		// are we done yet?
		double minVal = 0;
		cv::minMaxLoc(cc, &minVal);

		if (minVal > 0)
			break;
		
	}

	pIdx = 0;

	// compute real resolution
	for (int rIdx = 0; rIdx < ccD.rows; rIdx++) {

		const unsigned char* ccDPtr = ccD.ptr<unsigned char>(rIdx);

		for (int cIdx = 0; cIdx < ccD.cols; cIdx++) {

			// is the patch already computed?
			if (ccDPtr[cIdx])
				continue;

			QFileInfo cFile = filesUsed.at(rIdx*ccD.cols+cIdx);

			if (!cFile.exists()) {
				emit infoMessage(tr("Something is seriously wrong, I could not load: %1").arg(cFile.absoluteFilePath()));
				continue;
			}

			cv::Mat thumbPatch = createPatch(DkThumbNail(cFile), patchResD);

			cv::Mat dPatch = dImg.rowRange(rIdx*patchResD, rIdx*patchResD+patchResD)
				.colRange(cIdx*patchResD, cIdx*patchResD+patchResD);
			thumbPatch.copyTo(dPatch);
			emit updateProgress(qRound((float)pIdx/maxP*100));
			pIdx++;
		}
	}

	qDebug() << "I fully rendered: " << ccD.rows*ccD.cols-cv::sum(ccD)[0] << " images";

	// create final images
	origImg = mImgLab;
	mosaicMat = dImg;
	mosaicMatSmall = pImg;

	processing = false;

	qDebug() << "mosaic computed in: " << dt.getTotal();

	return QDialog::Accepted;
}

void DkMosaicDialog::matchPatch(const cv::Mat& img, const cv::Mat& thumb, int patchRes, cv::Mat& cc) {

	for (int rIdx = 0; rIdx < cc.rows; rIdx++) {

		float* ccPtr = cc.ptr<float>(rIdx);
		const cv::Mat imgStrip = img.rowRange(rIdx*patchRes, rIdx*patchRes+patchRes);

		for (int cIdx = 0; cIdx < cc.cols; cIdx++) {

			// already computed?
			if (ccPtr[cIdx] != 0)
				continue;

			const cv::Mat cPatch = imgStrip.colRange(cIdx*patchRes, cIdx*patchRes+patchRes);
			
			cv::Mat absDiff;
			cv::absdiff(cPatch, thumb, absDiff);
			ccPtr[cIdx] = 1.0f-((float)cv::sum(absDiff)[0]/(patchRes*patchRes*255));
		}
	}
}

cv::Mat DkMosaicDialog::createPatch(const DkThumbNail& thumb, int patchRes) {

	QImage img;

	// load full image if we have not enough resolution
	if (qMin(thumb.getImage().width(), thumb.getImage().height()) < patchRes) {
		DkBasicLoader loader;
		loader.loadGeneral(thumb.getFile(), true, true);
		img = loader.image();
	}
	else
		img = thumb.getImage();

	cv::Mat cvThumb = DkImage::qImage2Mat(img);
	cv::cvtColor(cvThumb, cvThumb, CV_RGB2Lab);
	std::vector<Mat> channels;
	cv::split(cvThumb, channels);
	cvThumb = channels[0];
	channels.clear();

	// make square
	if (cvThumb.rows != cvThumb.cols) {

		if (cvThumb.rows > cvThumb.cols) {
			float sh = (cvThumb.rows - cvThumb.cols)/2.0f;
			cvThumb = cvThumb.rowRange(qFloor(sh), cvThumb.rows-qCeil(sh));
		}
		else {
			float sh = (cvThumb.cols - cvThumb.rows)/2.0f;
			cvThumb = cvThumb.colRange(qFloor(sh), cvThumb.cols-qCeil(sh));
		}
	}

	if (cvThumb.rows < patchRes || cvThumb.cols < patchRes)
		qDebug() << "enlarging thumbs!!";

	cv::resize(cvThumb, cvThumb, cv::Size(patchRes, patchRes), 0.0, 0.0, CV_INTER_AREA);

	return cvThumb;
}

QString DkMosaicDialog::getRandomImagePath(const QString& cPath, const QString& ignore, const QString& suffix) {

	// TODO: remove hierarchy

	QStringList fileFilters = (suffix.isEmpty()) ? DkSettings::app.fileFilters : QStringList(suffix);

	// get all dirs
	QFileInfoList entries = QDir(cPath).entryInfoList(QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot);
	//qDebug() << entries;
	// get all files
	entries += QDir(cPath).entryInfoList(fileFilters);
	//qDebug() << "current entries: " << e;

	if (!ignore.isEmpty()) {

		QStringList ignoreList = ignore.split(";");
		QFileInfoList entriesTmp = entries;
		entries.clear();

		for (int idx = 0; idx < entriesTmp.size(); idx++) {
			
			bool ignore = false;
			QString p = entriesTmp.at(idx).absoluteFilePath();

			for (int iIdx = 0; iIdx < ignoreList.size(); iIdx++) {
				if (p.contains(ignoreList.at(iIdx))) {
					ignore = true;
					break;
				}
			}

			if (!ignore)
				entries.append(entriesTmp.at(idx));
		}
	}
		

	if (entries.isEmpty())
		return QString();

	int rIdx = qRound((float)qrand()/RAND_MAX*(entries.size()-1));

	//qDebug() << "rand index: " << rIdx;

	QFileInfo rPath = entries.at(rIdx);
	//qDebug() << rPath.absoluteFilePath();

	if (rPath.isDir())
		return getRandomImagePath(rPath.absoluteFilePath(), ignore, suffix);
	else
		return rPath.absoluteFilePath();
}

void DkMosaicDialog::updatePostProcess() {
	
	if (mosaicMat.empty() || processing)
		return;

	if (postProcessing) {
		updatePostProcessing = true;
		return;
	}

	buttons->button(QDialogButtonBox::Apply)->setEnabled(false);
	buttons->button(QDialogButtonBox::Save)->setEnabled(false);

	QFuture<bool> future = QtConcurrent::run(this, 
		&nmc::DkMosaicDialog::postProcessMosaic,
		darkenSlider->value()/100.0f,
		lightenSlider->value()/100.0f, 
		saturationSlider->value()/100.0f,
		true);
	postProcessWatcher.setFuture(future);

	updatePostProcessing = false;
	//postProcessMosaic(darkenSlider->value()/100.0f, lightenSlider->value()/100.0f, saturationSlider->value()/100.0f);
}

void DkMosaicDialog::postProcessFinished() {

	if (postProcessWatcher.result()) {
		QDialog::accept();
	}
	else if (updatePostProcessing)
		updatePostProcess();
	else {
		buttons->button(QDialogButtonBox::Save)->setEnabled(true);
	}
}

bool DkMosaicDialog::postProcessMosaic(float multiply /* = 0.3 */, float screen /* = 0.5 */, float saturation, bool computePreview) {

	postProcessing = true;

	qDebug() << "darken: " << multiply << " lighten: " << screen;

	cv::Mat origR;
	cv::Mat mosaicR;

	try {
		if (computePreview) {
			origR = origImg.clone();
			mosaicR = mosaicMatSmall.clone();
		}
		else {
			cv::resize(origImg, origR, mosaicMat.size(), 0, 0, CV_INTER_LANCZOS4);
			mosaicR = mosaicMat;
			origImg.release();
		}

		// multiply the two images
		for (int rIdx = 0; rIdx < origR.rows; rIdx++) {

			const unsigned char* mosaicPtr = mosaicR.ptr<unsigned char>(rIdx);
			unsigned char* origPtr = origR.ptr<unsigned char>(rIdx);

			if (!computePreview)
				emit updateProgress(qRound((float)rIdx/origR.rows*100));

			for (int cIdx = 0; cIdx < origR.cols; cIdx++) {

				// mix the luminance channel
				float mosaic = mosaicPtr[cIdx]/255.0f;
				float luminance = (*origPtr)/255.0f;

			
				float lighten = (1.0f-luminance)*screen + (1.0f-screen);
				lighten *= 1.0f-mosaic;	// multiply inverse
				lighten = 1.0f-lighten;

				float darken = luminance*multiply + (1.0f-multiply);
				darken *= lighten;	// mix with the mosaic pixel

				// now stretch to the dynamic range and save it
				*origPtr = (unsigned char)qRound(darken*255.0f);

				// now adopt the saturation
				origPtr++;
				*origPtr = (unsigned char)qRound((*origPtr-128) * saturation)+128;
				origPtr++;
				*origPtr = (unsigned char)qRound((*origPtr-128) * saturation)+128;
				origPtr++;
			}
		}

		//if (!computePreview)
		//	mosaicMat.release();
		cv::cvtColor(origR, origR, CV_Lab2BGR);
		qDebug() << "color converted";

		mosaic = DkImage::mat2QImage(origR);
		qDebug() << "mosaicing computed...";

	}
	catch(...) {
		origR.release();
		DkNoMacs::dialog("Sorry, I could not mix the image...");
		qDebug() << "exception caught...";
		mosaic = DkImage::mat2QImage(mosaicMat);
	}
	
	if (computePreview)
		preview->setImage(mosaic);

	postProcessing = false;

	return !computePreview;

}

void DkMosaicDialog::setFile(const QFileInfo& file) {

	if (!file.exists())
		return;

	cFile = file;
	saveDir = file.absolutePath();
	folderLabel->setText(saveDir.absolutePath());
	fileLabel->setText(file.absoluteFilePath());
	//filterEdit->setText(file.baseName());

	loader.loadGeneral(cFile, true);
	viewport->setImage(loader.image());

	enableMosaicSave(loader.hasImage());

	//newWidthBox->blockSignals(true);
	//newHeightBox->blockSignals(true);
	newWidthBox->setValue(loader.image().width());
	numPatchesH->setValue(qFloor((float)loader.image().width()/90));	// 130 is a pretty good patch resolution
	numPatchesH->setMaximum(qMin(1000, qFloor(loader.image().width()*0.5f)));
	numPatchesV->setMaximum(qMin(1000, qFloor(loader.image().height()*0.5f)));
	//newHeightBox->setValue(loader.image().height());
	//newWidthBox->blockSignals(false);
	//newHeightBox->blockSignals(false);

	//fromPage->setRange(1, loader.getNumPages());
	//toPage->setRange(1, loader.getNumPages());

	//fromPage->setValue(1);
	//toPage->setValue(loader.getNumPages());
}

void DkMosaicDialog::enableAll(bool enable) {

	enableMosaicSave(enable);
	controlWidget->setEnabled(enable);
}

void DkMosaicDialog::enableMosaicSave(bool enable) {

	filterEdit->setEnabled(enable);
	suffixBox->setEnabled(enable);
	newWidthBox->setEnabled(enable);
	newHeightBox->setEnabled(enable);
	numPatchesH->setEnabled(enable);
	numPatchesV->setEnabled(enable);
	buttons->button(QDialogButtonBox::Apply)->setEnabled(enable);

	if (!enable)
		buttons->button(QDialogButtonBox::Save)->setEnabled(enable);
}
#endif
// DkForceThumbDialog --------------------------------------------------------------------
DkForceThumbDialog::DkForceThumbDialog(QWidget* parent /* = 0 */, Qt::WindowFlags f /* = 0 */) : QDialog(parent, f) {

	createLayout();
}

void DkForceThumbDialog::createLayout() {

	QVBoxLayout* layout = new QVBoxLayout(this);

	infoLabel = new QLabel();
	infoLabel->setAlignment(Qt::AlignHCenter);

	cbForceSave = new QCheckBox(tr("Overwrite Existing Thumbnails"));
	cbForceSave->setToolTip("If checked, existing thumbnails will be replaced");

	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	layout->addWidget(infoLabel);
	layout->addWidget(cbForceSave);
	layout->addWidget(buttons);

}

bool DkForceThumbDialog::forceSave() const {

	return cbForceSave->isChecked();
}

void DkForceThumbDialog::setDir(const QDir& fileInfo) {

	infoLabel->setText(tr("Compute thumbnails for all images in:\n %1\n").arg(fileInfo.absolutePath()));
}

// Welcome dialog --------------------------------------------------------------------
DkWelcomeDialog::DkWelcomeDialog(QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f) {

	setWindowTitle(tr("Welcome"));
	createLayout();
	languageChanged = false;
}

void DkWelcomeDialog::createLayout() {

	QGridLayout* layout = new QGridLayout(this);

	QLabel* welcomeLabel = new QLabel(tr("Welcome to nomacs, please choose your preferred language below."), this);

	languageCombo = new QComboBox(this);
	DkUtils::addLanguages(languageCombo, languages);

	registerFilesCheckBox = new QCheckBox(tr("Register File Associations"), this);
	registerFilesCheckBox->setChecked(true);

	// buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
	
	layout->addItem(new QSpacerItem(10, 10), 0, 0, -1, -1);
	layout->addWidget(welcomeLabel, 1, 0, 1, 3);
	layout->addItem(new QSpacerItem(10, 10), 2, 0, -1, -1);
	layout->addWidget(languageCombo, 3, 1);

#ifdef WIN32
	layout->addWidget(registerFilesCheckBox, 4, 1);
#else
	registerFilesCheckBox->setChecked(false);
	registerFilesCheckBox->hide();
#endif
	
	layout->addWidget(buttons, 5, 0, 1, 3);
}

void DkWelcomeDialog::accept() {
	
	// register file associations
	if (registerFilesCheckBox->isChecked()) {
		DkFileFilterHandling fh;

		QStringList rFilters = DkSettings::app.openFilters;
		
		for (int idx = 0; idx < DkSettings::app.containerFilters.size(); idx++)
			rFilters.removeAll(DkSettings::app.containerFilters.at(idx));

		for (int idx = 1; idx < rFilters.size(); idx++) {

			// remove the icon file -> otherwise icons might be destroyed (e.g. acrobat)
			if (!rFilters.at(idx).contains("ico"))	
				fh.registerFileType(rFilters.at(idx), tr("Image"), true);
		}
	}

	// change language
	if (languageCombo->currentIndex() != languages.indexOf(DkSettings::global.language) && languageCombo->currentIndex() >= 0) {
		DkSettings::global.language = languages.at(languageCombo->currentIndex());
		languageChanged = true;
	}

	QDialog::accept();
}

bool DkWelcomeDialog::isLanguageChanged() {

	return languageChanged;
}


// archive extraction dialog --------------------------------------------------------------------
#ifdef WITH_QUAZIP
DkArchiveExtractionDialog::DkArchiveExtractionDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {
	
	fileList = QStringList();
	setWindowTitle(tr("Extract images from an archive"));
	createLayout();
	setMinimumSize(340, 400);
	setAcceptDrops(true);
}

void DkArchiveExtractionDialog::createLayout() {

	// archive file path
	QLabel* archiveLabel = new QLabel(tr("Archive (%1)").arg(DkSettings::app.containerRawFilters.replace(" *", ", *")), this);
	archivePathEdit = new QLineEdit(this);
	archivePathEdit->setObjectName("DkWarningEdit");
	archivePathEdit->setValidator(&fileValidator);
	connect(archivePathEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
	connect(archivePathEdit, SIGNAL(editingFinished()), this, SLOT(loadArchive()));

	QPushButton* openArchiveButton = new QPushButton("&Browse");
	connect(openArchiveButton, SIGNAL(pressed()), this, SLOT(openArchive()));

	// dir file path
	QLabel* dirLabel = new QLabel(tr("Extract to"));
	dirPathEdit = new QLineEdit();
	dirPathEdit->setValidator(&fileValidator);
	connect(dirPathEdit, SIGNAL(textChanged(QString)), this, SLOT(dirTextChanged(QString)));

	QPushButton* openDirButton = new QPushButton("&Browse");
	connect(openDirButton, SIGNAL(pressed()), this, SLOT(openDir()));

	feedbackLabel = new QLabel("", this);
	feedbackLabel->setObjectName("DkDecentInfo");

	fileListDisplay = new QListWidget(this);

	removeSubfolders = new QCheckBox("Remove Subfolders", this);
	removeSubfolders->setChecked(false);
	connect(removeSubfolders, SIGNAL(stateChanged(int)), this, SLOT(checkbocChecked(int)));

	// buttons
	buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	buttons->button(QDialogButtonBox::Ok)->setText(tr("&Extract"));
	buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
	buttons->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QWidget* extractWidget = new QWidget();
	QGridLayout* gdLayout = new QGridLayout(extractWidget);
	gdLayout->addWidget(archiveLabel, 0, 0);
	gdLayout->addWidget(archivePathEdit, 1, 0);
	gdLayout->addWidget(openArchiveButton, 1, 1);
	gdLayout->addWidget(dirLabel, 2, 0);
	gdLayout->addWidget(dirPathEdit, 3, 0);
	gdLayout->addWidget(openDirButton, 3, 1);
	gdLayout->addWidget(feedbackLabel, 4, 0, 1, 2);
	gdLayout->addWidget(fileListDisplay, 5, 0, 1, 2);
	gdLayout->addWidget(removeSubfolders, 6, 0, 1, 2);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(extractWidget);
	layout->addWidget(buttons);
}

void DkArchiveExtractionDialog::setCurrentFile(const QFileInfo& file, bool isZip) {

	userFeedback("", false);
	archivePathEdit->setText("");
	dirPathEdit->setText("");
	fileListDisplay->clear();
	removeSubfolders->setChecked(false);

	cFile = file;
	if (isZip) {
		archivePathEdit->setText(cFile.absoluteFilePath());
		loadArchive();
	}
};

void DkArchiveExtractionDialog::textChanged(QString text) {
	
	bool oldStyle = archivePathEdit->property("error").toBool();
	bool newStyle = false;

	if (QFileInfo(text).exists() && DkBasicLoader::isContainer(text)) {
		newStyle = false;
		archivePathEdit->setProperty("error", newStyle);
		loadArchive(text);
	}
	else {
		newStyle = true;
		archivePathEdit->setProperty("error", newStyle);
		userFeedback("", false);	
		fileListDisplay->clear();
		buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
	}

	if (oldStyle != newStyle) {
		archivePathEdit->style()->unpolish(archivePathEdit);
		archivePathEdit->style()->polish(archivePathEdit);
		archivePathEdit->update();
	}


}

void DkArchiveExtractionDialog::dirTextChanged(QString text) {
	
	if (text.isEmpty()) {
		userFeedback("", false);
		buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
	}
}

void DkArchiveExtractionDialog::checkbocChecked(int) {

	loadArchive();
}

void DkArchiveExtractionDialog::openArchive() {

	// load system default open dialog
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open Archive"),
		(archivePathEdit->text().isEmpty()) ? cFile.absolutePath() : archivePathEdit->text(), tr("Archives (%1)").arg(DkSettings::app.containerRawFilters.remove(",")));

	if (QFileInfo(filePath).exists()) {
		archivePathEdit->setText(filePath);
		loadArchive(filePath);
	}
}

void DkArchiveExtractionDialog::openDir() {

	// load system default open dialog
	QString filePath = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
		(dirPathEdit->text().isEmpty()) ? cFile.absoluteDir().absolutePath() : dirPathEdit->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if (QFileInfo(filePath).exists())
		dirPathEdit->setText(filePath);
}

void DkArchiveExtractionDialog::userFeedback(const QString& msg, bool error) {

	if (!error)
		feedbackLabel->setProperty("warning", false);
	else
		feedbackLabel->setProperty("warning", true);

	feedbackLabel->setText(msg);
	feedbackLabel->style()->unpolish(feedbackLabel);
	feedbackLabel->style()->polish(feedbackLabel);
	feedbackLabel->update();
}

void DkArchiveExtractionDialog::loadArchive(QString filePath) {

	fileList = QStringList();
	fileListDisplay->clear();

	if(filePath.isEmpty())
		filePath = archivePathEdit->text();

	QFileInfo fileInfo(filePath);
	if (!fileInfo.exists())
		return;

	if (!DkBasicLoader::isContainer(filePath)) {
		userFeedback(tr("Not a valid archive."), true);
		return;
	}

	if (dirPathEdit->text().isEmpty()) {

		dirPathEdit->setText(QFileInfo(filePath).absoluteFilePath().remove("." + QFileInfo(filePath).suffix()));
		dirPathEdit->setFocus();
	}

	QStringList fileNameList = JlCompress::getFileList(filePath);
	
	// remove the * in fileFilters
	QStringList fileFiltersClean = DkSettings::app.browseFilters;
	for (int idx = 0; idx < fileFiltersClean.size(); idx++)
		fileFiltersClean[idx].replace("*", "");

	for (int idx = 0; idx < fileNameList.size(); idx++) {
		
		for (int idxFilter = 0; idxFilter < fileFiltersClean.size(); idxFilter++) {

			if (fileNameList.at(idx).contains(fileFiltersClean[idxFilter], Qt::CaseInsensitive)) {
				fileList.append(fileNameList.at(idx));
				break;
			}
		}
	}

	if (fileList.size() > 0)
		userFeedback(tr("Number of images: ") + QString::number(fileList.size()), false);
	else {
		userFeedback(tr("The archive does not contain any images."), false);
		return;
	}

	fileListDisplay->addItems(fileList);

	if (removeSubfolders->checkState() == Qt::Checked) {
		for (int i = 0; i < fileListDisplay->count(); i++) {

			QFileInfo fi(fileListDisplay->item(i)->text());
			fileListDisplay->item(i)->setText(fi.fileName());
		}
	}
	fileListDisplay->update();

	buttons->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void DkArchiveExtractionDialog::accept() {

	QStringList extractedFiles = extractFilesWithProgress(archivePathEdit->text(), fileList, dirPathEdit->text(), removeSubfolders->isChecked());

	if ((extractedFiles.isEmpty() || extractedFiles.size() != fileList.size()) && !extractedFiles.contains("userCanceled")) {
		
		QMessageBox msgBox(this);
		msgBox.setText(tr("The images could not be extracted!"));
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.exec();
	}

	QDialog::accept();
}

void DkArchiveExtractionDialog::dropEvent(QDropEvent *event) {

	if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
		QUrl url = event->mimeData()->urls().at(0);
		qDebug() << "dropping: " << url;
		url = url.toLocalFile();

		if (QFileInfo(url.toString()).isFile()) {
			archivePathEdit->setText(url.toString());
			loadArchive(url.toString());
		}
		else
			dirPathEdit->setText(url.toString());
	}
}

void DkArchiveExtractionDialog::dragEnterEvent(QDragEnterEvent *event) {

	if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();
		QFileInfo file = QFileInfo(url.toString());

		if (file.exists())
			event->acceptProposedAction();
	}

}

QStringList DkArchiveExtractionDialog::extractFilesWithProgress(QString fileCompressed, QStringList files, QString dir, bool removeSubfolders) {

    QProgressDialog progressDialog(this);
    progressDialog.setCancelButtonText(tr("&Cancel"));
    progressDialog.setRange(0, files.size() - 1);
    progressDialog.setWindowTitle(tr("Extracting files..."));
	progressDialog.setWindowModality(Qt::WindowModal);
	progressDialog.setModal(true);
	progressDialog.hide();
	progressDialog.show();

    QStringList extracted;
    for (int i=0; i<files.count(); i++) {
		progressDialog.setValue(i);
		progressDialog.setLabelText(tr("Extracting file %1 of %2").arg(i + 1).arg(files.size()));

		QString absPath;
		if(removeSubfolders)
			absPath = QDir(dir).absoluteFilePath(QFileInfo(files.at(i)).fileName());
		else
			absPath = QDir(dir).absoluteFilePath(files.at(i));

		if (JlCompress::extractFile(fileCompressed, files.at(i), absPath).isEmpty()) {
			qDebug() << "unable to extract:" << files.at(i);
			//return QStringList();
		}
        extracted.append(absPath);
		if(progressDialog.wasCanceled()) {
			return QStringList("userCanceled");
		}
    }

	progressDialog.close();

    return extracted;
}







#endif

} // close namespace
