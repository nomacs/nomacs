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



#ifdef WIN32
	#include "shlwapi.h"
	#pragma comment (lib, "shlwapi.lib")
#endif

#if defined(_MSC_BUILD) && !defined(QT_NO_DEBUG_OUTPUT) // fixes cmake bug - really release uses subsystem windows, debug and release subsystem console
	#pragma comment (linker, "/SUBSYSTEM:CONSOLE")
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
#pragma warning(pop)	// no warnings from includes - end

#include "DkNoMacs.h"
#include "DkCentralWidget.h"
#include "DkSettings.h"
#include "DkTimer.h"
#include "DkPong.h"
#include "DkUtils.h"

#include <iostream>
#include <cassert>

#ifdef WIN32
#include <shlobj.h>
#endif

void createPluginsPath();

#ifdef WIN32
int main(int argc, wchar_t *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif

	qDebug() << "nomacs - Image Lounge\n";

	//! \warning those QSettings setup *must* go before QApplication object
    //           to prevent random crashes (well, crashes are regular on mac
    //           opening from Finder)
	// register our organization
	QCoreApplication::setOrganizationName("nomacs");
	QCoreApplication::setOrganizationDomain("http://www.nomacs.org");
	QCoreApplication::setApplicationName("Image Lounge");
	nmc::DkUtils::registerFileVersion();

	// NOTE: raster option destroys the frameless view on mac
	// but raster is so much faster when zooming
#if !defined(Q_WS_MAC) && !defined(QT5)
	QApplication::setGraphicsSystem("raster");
//#elif !defined(QT5)
//	if (mode != nmc::Settings::param().mode_frameless)
//		QApplication::setGraphicsSystem("raster");
#endif
	
	QApplication a(argc, (char**)argv);
	qDebug() << "argument count: " << argc;

	// CMD parser --------------------------------------------------------------------
	QCommandLineParser parser;

	//parser.setApplicationDescription("Test helper");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("image", QObject::tr("An input image."));

	// fullscreen (-f)
	QCommandLineOption fullScreenOpt(QStringList() << "f" << "fullscreen", QObject::tr("Start in fullscreen."));
	parser.addOption(fullScreenOpt);

	QCommandLineOption pongOpt(QStringList() << "x" << "pong", QObject::tr("Start Pong."));
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

	parser.process(a);
	// CMD parser --------------------------------------------------------------------

	nmc::Settings::param().initFileFilters();
	QSettings& settings = nmc::Settings::instance().getSettings();
	
	nmc::Settings::param().load();	// load in constructor??

	int mode = settings.value("AppSettings/appMode", nmc::Settings::param().app().appMode).toInt();
	nmc::Settings::param().app().currentAppMode = mode;

	createPluginsPath();

	nmc::DkNoMacs* w = 0;
	nmc::DkPong* pw = 0;	// pong

	QString translationName = "nomacs_"+ settings.value("GlobalSettings/language", nmc::Settings::param().global().language).toString() + ".qm";
	QString translationNameQt = "qt_"+ settings.value("GlobalSettings/language", nmc::Settings::param().global().language).toString() + ".qm";
	
	QTranslator translator;
	nmc::Settings::param().loadTranslation(translationName, translator);
	a.installTranslator(&translator);
	
	QTranslator translatorQt;
	nmc::Settings::param().loadTranslation(translationNameQt, translatorQt);
	a.installTranslator(&translatorQt);

	// show pink icons if nomacs is in private mode
	if(parser.isSet(privateOpt)) {
		nmc::Settings::param().display().iconColor = QColor(136, 0, 125);
		nmc::Settings::param().app().privateMode = true;
	}

	qInfo() << "breaking nomacs";

	if (parser.isSet(modeOpt)) {
		QString pm = parser.value(modeOpt);// .trimmed();

		if (pm == "default")
			mode = nmc::Settings::param().mode_default;
		else if (pm == "frameless")
			mode = nmc::Settings::param().mode_frameless;
		else if (pm == "pseudocolor")
			mode = nmc::Settings::param().mode_contrast;
		else
			qWarning() << "illegal mode: " << pm << "use either <default>, <frameless> or <pseudocolor>";
	}

	qDebug() << "mode: " << mode;

	nmc::DkTimer dt;

	// initialize nomacs
	if (mode == nmc::Settings::param().mode_frameless) {
		w = static_cast<nmc::DkNoMacs*> (new nmc::DkNoMacsFrameless());
		qDebug() << "this is the frameless nomacs...";
	}
	else if (mode == nmc::Settings::param().mode_contrast) {
		w = static_cast<nmc::DkNoMacs*> (new nmc::DkNoMacsContrast());
		qDebug() << "this is the contrast nomacs...";
	}
	else if (parser.isSet(pongOpt)) {
		pw = new nmc::DkPong();
		int rVal = a.exec();
		return rVal;
	}
	else
		w = static_cast<nmc::DkNoMacs*> (new nmc::DkNoMacsIpl());	// slice it

	if (w)
		w->onWindowLoaded();

	qDebug() << "Initialization takes: " << dt.getTotal();

	if (!parser.positionalArguments().empty()) {
		w->loadFile(QFileInfo(parser.positionalArguments()[0]).absoluteFilePath());	// update folder + be silent
		qDebug() << "loading: " << parser.positionalArguments()[0];
	}
	else if (nmc::Settings::param().app().showRecentFiles)
		w->showRecentFiles();

	// load directory preview
	if (!parser.value(sourceDirOpt).isEmpty()) {
		nmc::DkCentralWidget* cw = w->getTabWidget();
		cw->loadDirToTab(parser.value(sourceDirOpt));
	}

	// load to tabs
	if (!parser.value(tabOpt).isEmpty()) {
		nmc::DkCentralWidget* cw = w->getTabWidget();
		
		QStringList tabPaths = parser.values(tabOpt);
		
		for (const QString& filePath : tabPaths)
			cw->addTab(filePath);
	}

	int fullScreenMode = settings.value("AppSettings/currentAppMode", nmc::Settings::param().app().currentAppMode).toInt();

	if (fullScreenMode == nmc::Settings::param().mode_default_fullscreen		||
		fullScreenMode == nmc::Settings::param().mode_frameless_fullscreen	||
		fullScreenMode == nmc::Settings::param().mode_contrast_fullscreen		||
		parser.isSet(fullScreenOpt)) {
		w->enterFullScreen();
		qDebug() << "trying to enter fullscreen...";
	}

#ifdef Q_WS_MAC
	nmc::DkNomacsOSXEventFilter *osxEventFilter = new nmc::DkNomacsOSXEventFilter();
	a.installEventFilter(osxEventFilter);
	QObject::connect(osxEventFilter, SIGNAL(loadFile(const QFileInfo&)),
		w, SLOT(loadFile(const QFileInfo&)));
#endif

	int rVal = a.exec();

	if (w)
		delete w;	// we need delete so that settings are saved (from destructors)
	if (pw)
		delete pw;

	return rVal;
}

void createPluginsPath() {

#ifdef WITH_PLUGINS
	// initialize plugin paths -----------------------------------------
#ifdef WIN32
	QDir pluginsDir;
	if (!nmc::Settings::param().isPortable())
		pluginsDir = QDir::home().absolutePath() + "/AppData/Roaming/nomacs/plugins";
	else
		pluginsDir = QCoreApplication::applicationDirPath() + "/plugins";
#else
	QDir pluginsDir = QDir("/usr/lib/nomacs-plugins/");
#endif // WIN32


	if (!pluginsDir.exists())
		pluginsDir.mkpath(pluginsDir.absolutePath());

	nmc::Settings::param().global().pluginsDir = pluginsDir.absolutePath();
	qDebug() << "plugins dir set to: " << nmc::Settings::param().global().pluginsDir;

	QCoreApplication::addLibraryPath(nmc::Settings::param().global().pluginsDir);

	QCoreApplication::addLibraryPath("./imageformats");

#endif // WITH_PLUGINS

}