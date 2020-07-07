/*******************************************************************************************************
 main.cpp
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


#ifdef Q_OS_WIN
	#include "shlwapi.h"
	#pragma comment (lib, "shlwapi.lib")
#endif

#if defined(_MSC_BUILD) && !defined(DK_INSTALL) // only final release will be compiled without a CMD
	#pragma comment (linker, "/SUBSYSTEM:CONSOLE")
#else
	#pragma comment (linker, "/SUBSYSTEM:WINDOWS")
#endif

#ifdef QT_NO_DEBUG_OUTPUT
#pragma warning(disable: 4127)		// no 'conditional expression is constant' if qDebug() messages are removed
#endif

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QObject>
#include <QApplication>
#include <QFileInfo>
#include <QProcess>
#include <QTranslator>
#include <QDebug>
#include <QDir>
#include <QTextStream>
#include <QDesktopServices>
#include <QCommandLineParser>
#include <QMessageBox>
#pragma warning(pop)	// no warnings from includes - end

#include "DkNoMacs.h"
#include "DkCentralWidget.h"
#include "DkSettings.h"
#include "DkTimer.h"
#include "DkPong.h"
#include "DkUtils.h"
#include "DkProcess.h"
#include "DkPluginManager.h"

#include "DkDependencyResolver.h"
#include "DkMetaData.h"

#include "DkVersion.h"

#include <iostream>
#include <cassert>

#ifdef Q_OS_WIN
#include <shlobj.h>
#endif

#ifdef Q_OS_WIN
int main(int argc, wchar_t *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif

    QCoreApplication::setOrganizationName("nomacs");
    QCoreApplication::setOrganizationDomain("https://nomacs.org");
    QCoreApplication::setApplicationName("Image Lounge");
    QCoreApplication::setApplicationVersion(NOMACS_VERSION_STR);
	
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QApplication::setAttribute(Qt::AA_DisableHighDpiScaling, true);
#endif

	QApplication app(argc, (char**)argv);

	// init settings
	nmc::DkSettingsManager::instance().init();
	nmc::DkMetaDataHelper::initialize();	// this line makes the XmpParser thread-save - so don't delete it even if you seem to know what you do

	nmc::DefaultSettings settings;
	int mode = settings.value("AppSettings/appMode", nmc::DkSettingsManager::param().app().appMode).toInt();

	// uncomment this for the single instance feature...
	//// check for single instance
	//nmc::DkRunGuard guard;
	//
	//if (!guard.tryRunning()) {
	//	qDebug() << "nomacs is already running - quitting...";
	//	return 0;
	//}

	// CMD parser --------------------------------------------------------------------
	QCommandLineParser parser;
	
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("image", QObject::tr("An input image."));

	// fullscreen (-f)
	QCommandLineOption fullScreenOpt(QStringList() << "f" << "fullscreen", QObject::tr("Start in fullscreen."));
	parser.addOption(fullScreenOpt);

	QCommandLineOption slideshowOpt(QStringList() << "slideshow", QObject::tr("Start slideshow playback"));
	parser.addOption(slideshowOpt);

	QCommandLineOption pongOpt(QStringList() << "pong", QObject::tr("Start Pong."));
	parser.addOption(pongOpt);

	QCommandLineOption privateOpt(QStringList() << "p" << "private", QObject::tr("Start in private mode."));
	parser.addOption(privateOpt);

	QCommandLineOption modeOpt(QStringList() << "m" << "mode",
		QObject::tr("Set the viewing mode <mode>."),
		QObject::tr("default | frameless | pseudocolor"));
	parser.addOption(modeOpt);

	QCommandLineOption sourceDirOpt(QStringList() << "d" << "directory",
		QObject::tr("Load all files of a <directory>."),
		QObject::tr("directory"));
	parser.addOption(sourceDirOpt);

	QCommandLineOption tabOpt(QStringList() << "t" << "tab",
		QObject::tr("Load <images> to tabs."),
		QObject::tr("images"));
	parser.addOption(tabOpt);

	QCommandLineOption batchOpt(QStringList() << "batch",
		QObject::tr("Batch processing of <batch-settings.pnm>."),
		QObject::tr("batch-settings-path"));
	parser.addOption(batchOpt);

	QCommandLineOption batchLogOpt(QStringList() << "batch-log",
		QObject::tr("Saves batch log to <log-path.txt>."),
		QObject::tr("log-path.txt"));
	parser.addOption(batchLogOpt);

	QCommandLineOption importSettingsOpt(QStringList() << "import-settings",
		QObject::tr("Imports the settings from <settings-path.ini> and saves them."),
		QObject::tr("settings-path.ini"));
	parser.addOption(importSettingsOpt);
	
	QCommandLineOption registerFilesOpt(QStringList() << "register-files", QObject::tr("Register file associations (Windows only)."));
	parser.addOption(registerFilesOpt);

	parser.process(app);
	
	// CMD parser --------------------------------------------------------------------
	nmc::DkPluginManager::createPluginsPath();
	
	// compute batch process
	if (!parser.value(batchOpt).isEmpty()) {
		
		QString logPath;
		if (!parser.value(batchLogOpt).isEmpty())
			logPath = parser.value(batchLogOpt);

		QString batchSettingsPath = parser.value(batchOpt);
		nmc::DkBatchProcessing::computeBatch(batchSettingsPath, logPath);
		
		return 0;
	}

	bool noUI = false;

	// apply default settings
	if (!parser.value(importSettingsOpt).isEmpty()) {
		QString settingsPath = parser.value(importSettingsOpt);
		nmc::DkSettingsManager::importSettings(settingsPath);
		noUI = true;
	}

	// apply default settings
	if (parser.isSet(registerFilesOpt)) {
		
		nmc::DkFileFilterHandling::registerFileAssociations();
		noUI = true;
	}

	if (noUI)
		return 0;

	//install translations
	QString translationName = "nomacs_" + 
		settings.value("GlobalSettings/language", nmc::DkSettingsManager::param().global().language).toString() + ".qm";
	QString translationNameQt = "qt_" + 
		settings.value("GlobalSettings/language", nmc::DkSettingsManager::param().global().language).toString() + ".qm";

	QTranslator translator;
	nmc::DkSettingsManager::param().loadTranslation(translationName, translator);
	app.installTranslator(&translator);

	QTranslator translatorQt;
	nmc::DkSettingsManager::param().loadTranslation(translationNameQt, translatorQt);
	app.installTranslator(&translatorQt);

	nmc::DkNoMacs* w = 0;
	nmc::DkPong* pw = 0;	// pong

	// show pink icons if nomacs is in private mode
	if(parser.isSet(privateOpt)) {
		nmc::DkSettingsManager::param().display().iconColor = QColor(136, 0, 125);
		nmc::DkSettingsManager::param().app().privateMode = true;
	}

	if (parser.isSet(modeOpt)) {
		QString pm = parser.value(modeOpt);

		if (pm == "default")
			mode = nmc::DkSettingsManager::param().mode_default;
		else if (pm == "frameless")
			mode = nmc::DkSettingsManager::param().mode_frameless;
		else if (pm == "pseudocolor")
			mode = nmc::DkSettingsManager::param().mode_contrast;
		else
			qWarning() << "illegal mode: " << pm << "use either <default>, <frameless> or <pseudocolor>";

		nmc::DkSettingsManager::param().app().currentAppMode = mode;
	}

	nmc::DkTimer dt;

	// initialize nomacs
	if (mode == nmc::DkSettingsManager::param().mode_frameless) {
		w = new nmc::DkNoMacsFrameless();
		qDebug() << "this is the frameless nomacs...";
	}
	else if (mode == nmc::DkSettingsManager::param().mode_contrast) {
		w = new nmc::DkNoMacsContrast();
		qDebug() << "this is the contrast nomacs...";
	}
	else if (parser.isSet(pongOpt)) {
		pw = new nmc::DkPong();
		int rVal = app.exec();
		return rVal;
	}
	else
		w = new nmc::DkNoMacsIpl();

	// show what we got...
	w->show();

	// this triggers a first show
	QCoreApplication::sendPostedEvents();

	if (w)
		w->onWindowLoaded();

	qInfo() << "Initialization takes: " << dt;

	nmc::DkCentralWidget* cw = w->getTabWidget();

	bool loading = false;

	if (!parser.positionalArguments().empty()) {

		QString filePath = parser.positionalArguments()[0].trimmed();

		if (!filePath.isEmpty()) {
			w->loadFile(QFileInfo(filePath).absoluteFilePath());	// update folder + be silent
			loading = true;
		}
	}

	// load directory preview
	if (!parser.value(sourceDirOpt).trimmed().isEmpty()) {
		cw->loadDirToTab(parser.value(sourceDirOpt));
		loading = true;
	}

	// load to tabs
	if (!parser.value(tabOpt).isEmpty()) {
		QStringList tabPaths = parser.values(tabOpt);
		loading = true;

		for (const QString& filePath : tabPaths)
			cw->addTab(filePath);
	}
	
	// load recent files if there is nothing to display
	if (!loading &&
		nmc::DkSettingsManager::param().app().showRecentFiles) {
		w->showRecentFilesOnStartUp();
	}

	int fullScreenMode = settings.value("AppSettings/currentAppMode", nmc::DkSettingsManager::param().app().currentAppMode).toInt();

	if (fullScreenMode == nmc::DkSettingsManager::param().mode_default_fullscreen		||
		fullScreenMode == nmc::DkSettingsManager::param().mode_frameless_fullscreen		||
		fullScreenMode == nmc::DkSettingsManager::param().mode_contrast_fullscreen		||
		parser.isSet(fullScreenOpt)) {
		w->enterFullScreen();
		qDebug() << "trying to enter fullscreen...";
	}

	if (parser.isSet(slideshowOpt)) {
		cw->startSlideshow();
	}

#ifdef Q_WS_MAC
	nmc::DkNomacsOSXEventFilter *osxEventFilter = new nmc::DkNomacsOSXEventFilter();
	app.installEventFilter(osxEventFilter);
	QObject::connect(osxEventFilter, SIGNAL(loadFile(const QFileInfo&)),
		w, SLOT(loadFile(const QFileInfo&)));
#endif

	int rVal = -1;
	try {
		rVal = app.exec();
	}
	catch (const std::bad_alloc&) {
		
		QMessageBox::critical(0, QObject::tr("Critical Error"), 
			QObject::tr("Sorry, nomacs ran out of memory..."), QMessageBox::Ok);
	}

	if (w)
		delete w;	// we need delete so that settings are saved (from destructors)
	if (pw)
		delete pw;

	return rVal;
}
