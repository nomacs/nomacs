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

#include "DkDependencyResolver.h"

#include <iostream>
#include <cassert>

#ifdef Q_OS_WIN
#include <shlobj.h>
#endif

void createPluginsPath();
void computeBatch(const QString& settingsPath, const QString& logPath = QString());

#ifndef Q_OS_WIN
void silence_info_message_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
  {
      QByteArray localMsg = msg.toLocal8Bit();
      switch (type) {
      case QtWarningMsg:
          fprintf(stderr, "[WARN] %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
          break;
      case QtCriticalMsg:
          fprintf(stderr, "[CRIT] %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
          break;
      case QtFatalMsg:
          fprintf(stderr, "[FATAL] %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
          abort();
      }
  }
#endif

#ifdef Q_OS_WIN
int main(int argc, wchar_t *argv[]) {
#else
int main(int argc, char *argv[]) {
    if (getenv("NOMACS_SILENT")) qInstallMessageHandler(silence_info_message_handler);
#endif

#ifdef READ_TUWIEN
	QCoreApplication::setOrganizationName("TU Wien");
	QCoreApplication::setOrganizationDomain("http://www.nomacs.org");
	QCoreApplication::setApplicationName("nomacs - Image Lounge [READ]");
#else
	QCoreApplication::setOrganizationName("nomacs");
	QCoreApplication::setOrganizationDomain("http://www.nomacs.org");
	QCoreApplication::setApplicationName("Image Lounge");
#endif
	
	nmc::DkUtils::registerFileVersion();

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QApplication::setAttribute(Qt::AA_DisableHighDpiScaling, true);
#endif
	QApplication a(argc, (char**)argv);

	// init settings
	nmc::Settings::instance().init();

	QSettings& settings = nmc::Settings::instance().getSettings();
	int mode = settings.value("AppSettings/appMode", nmc::Settings::param().app().appMode).toInt();

	//if (!nmc::Settings::param().app().openFilters.empty())
	//	qInfoClean() << "supported image extensions: " << nmc::Settings::param().app().openFilters[0];

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

	QCommandLineOption batchOpt(QStringList() << "batch",
		QObject::tr("Batch processing of <batch-settings.pnm>."),
		QObject::tr("batch-settings-path"));
	parser.addOption(batchOpt);

	QCommandLineOption batchLogOpt(QStringList() << "batch-log",
		QObject::tr("Saves batch log to <log-path.txt>."),
		QObject::tr("log-path.txt"));
	parser.addOption(batchLogOpt);

	parser.process(a);
	// CMD parser --------------------------------------------------------------------

	createPluginsPath();

	// load to tabs
	if (!parser.value(batchOpt).isEmpty()) {
		
		QString logPath;
		if (!parser.value(batchLogOpt).isEmpty())
			logPath = parser.value(batchLogOpt);

		QString batchSettingsPath = parser.value(batchOpt);
		computeBatch(batchSettingsPath, logPath);
		return 0;
	}

	//// DEBUG --------------------------------------------------------------------
	//nmc::DkDependencyWalker dw("C:/VSProjects/READ/nomacs/build2015-x64/Debug/plugins/writerIdentificationPlugin.dll");
	//if (!dw.findDependencies())
	//	qWarning() << "could not find dependencies for" << dw.filePath();

	//qDebug() << "all dependencies:" << dw.dependencies();
	//qDebug() << "filtered dependencies:" << dw.filteredDependencies();

	//return 0;
	//// DEBUG --------------------------------------------------------------------

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

		nmc::Settings::param().app().currentAppMode = mode;
	}

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

	qInfo() << "Initialization takes: " << dt;

	if (!parser.positionalArguments().empty()) {
		w->loadFile(QFileInfo(parser.positionalArguments()[0]).absoluteFilePath());	// update folder + be silent
		qDebug() << "loading: " << parser.positionalArguments()[0];
	}
	else {
		bool showRecent = nmc::Settings::param().app().showRecentFiles;
		showRecent &= nmc::Settings::param().app().currentAppMode != nmc::DkSettings::mode_frameless;
		w->showRecentFiles(showRecent);
	}

	// load directory preview
	if (!parser.value(sourceDirOpt).trimmed().isEmpty()) {
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
		fullScreenMode == nmc::Settings::param().mode_frameless_fullscreen		||
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

	int rVal = -1;
	try {
		rVal = a.exec();
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

void computeBatch(const QString& settingsPath, const QString& logPath) {
	
	nmc::DkBatchConfig bc = nmc::DkBatchProfile::loadProfile(settingsPath);

	// guarantee that the output path exists
	if (!QDir().mkpath(bc.getOutputDirPath())) {
		qCritical() << "Could not create:" << bc.getOutputDirPath();
		return;
	}

	QSharedPointer<nmc::DkBatchProcessing> process(new nmc::DkBatchProcessing());
	process->setBatchConfig(bc);
	process->compute();

	process->waitForFinished();	// block

	if (!logPath.isEmpty()) {
		
		QFileInfo fi(logPath);
		
		QDir().mkpath(fi.absolutePath());

		QFile file(logPath);
		if (!file.open(QIODevice::WriteOnly))
			qWarning() << "Sorry, I could not write to" << logPath;
		else {
			QStringList log = process->getLog();
			QTextStream s(&file);
			for (const QString& line : log)
				s << line << '\n';
			qInfo() << "log written to: " << logPath;
		}
	}
}

void createPluginsPath() {

#ifdef WITH_PLUGINS
	// initialize plugin paths -----------------------------------------
#ifdef Q_OS_WIN
	QDir pluginsDir = QCoreApplication::applicationDirPath() + "/plugins";
#else
	QDir pluginsDir = QCoreApplication::applicationDirPath() +  "/../lib/nomacs-plugins/";
#endif // Q_OS_WIN


	if (!pluginsDir.exists())
		pluginsDir.mkpath(pluginsDir.absolutePath());

	nmc::Settings::param().global().pluginsDir = pluginsDir.absolutePath();
	qDebug() << "plugins dir set to: " << nmc::Settings::param().global().pluginsDir;

	QCoreApplication::addLibraryPath(nmc::Settings::param().global().pluginsDir);

	QCoreApplication::addLibraryPath("./imageformats");

#endif // WITH_PLUGINS

}
