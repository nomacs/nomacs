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

//#include "DkImage.h"

#include <QObject>

#include "DkNoMacs.h"
#include "DkSettings.h"
#include "DkViewPort.h"
//#include "DkPong.h"
//#include "DkUtils.h"
//#include "DkTimer.h"

#include <QApplication>
#include <QFileInfo>
#include <QProcess>
#include <QTranslator>
#include <QDebug>

#include <iostream>
#include <cassert>

#ifdef WIN32
int main(int argc, wchar_t *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif

	qDebug() << "nomacs - Image Lounge\n";

	//qDebug() << "total memory: " << nmc::DkMemory::getTotalMemory() << " MB";
	//qDebug() << "free memory: " << nmc::DkMemory::getFreeMemory() << " MB";

	//! \warning those QSettings setup *must* go before QApplication object
    //           to prevent random crashes (well, crashes are regular on mac
    //           opening from Finder)
	// register our organization
	QCoreApplication::setOrganizationName("nomacs");
	QCoreApplication::setOrganizationDomain("http://www.nomacs.org");
	QCoreApplication::setApplicationName("Image Lounge - Fotobox");

	QSettings settings;
	int mode = settings.value("AppSettings/appMode", nmc::DkSettings::app.appMode).toInt();
	nmc::DkSettings::app.currentAppMode = mode;

	// NOTE: raster option destroys the frameless view on mac
	// but raster is so much faster when zooming
#if !defined(Q_WS_MAC) && !defined(QT5)
	QApplication::setGraphicsSystem("raster");
#elif !defined(QT5)
	if (mode != nmc::DkSettings::mode_frameless)
		QApplication::setGraphicsSystem("raster");
#endif

	QApplication a(argc, (char**)argv);
	QStringList args = a.arguments();

	//// pong --------------------------------------------------------------------
	//nmc::DkPong *p = new nmc::DkPong();
	//int pVal = a.exec();
	//return pVal;
	//// pong --------------------------------------------------------------------

	nmc::DkNoMacs* w;

	// DEBUG --------------------------------------------------------------------
	qDebug() << "input arguments:";
	for (int idx = 0; idx < args.size(); idx++)
		qDebug() << args[idx];
	qDebug() << "\n";
	// DEBUG --------------------------------------------------------------------


	//QSettings settings;
	QString translationName = "nomacs_"+ settings.value("GlobalSettings/language", nmc::DkSettings::global.language).toString() + ".qm";

	QTranslator translator;
	if (!translator.load(translationName, qApp->applicationDirPath())) {
		QDir appDir = QDir(qApp->applicationDirPath());
		if (!translator.load(translationName, appDir.filePath("../share/nomacs/translations/")) && !translationName.contains("_en"))
			qDebug() << "unable to load translation: " << translationName;
	}
	a.installTranslator(&translator);
	
	if (mode == nmc::DkSettings::mode_frameless) {
		w = static_cast<nmc::DkNoMacs*> (new nmc::DkNoMacsFrameless());
		qDebug() << "this is the frameless nomacs...";
	}
	else if (mode == nmc::DkSettings::mode_contrast) {
		w = static_cast<nmc::DkNoMacs*> (new nmc::DkNoMacsContrast());
		qDebug() << "this is the contrast nomacs...";
	}
	else
		w = static_cast<nmc::DkNoMacs*> (new nmc::DkNoMacsIpl());	// slice it

	QString imgDir = settings.value("Fotobox/defaultImgPath", nmc::DkSettings::foto.defaultImgPath).toString();

	if (args.size() > 1)
		w->loadFile(QFileInfo(args[1]), true);	// update folder + be silent
	else
		w->viewport()->getImageLoader()->setDir(QDir(imgDir));

	int fullScreenMode = settings.value("AppSettings/currentAppMode", nmc::DkSettings::app.currentAppMode).toInt();

	if (fullScreenMode == nmc::DkSettings::mode_default_fullscreen		||
		fullScreenMode == nmc::DkSettings::mode_frameless_fullscreen	||
		fullScreenMode == nmc::DkSettings::mode_contrast_fullscreen		) {
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
	delete w;	// we need delete so that settings are saved (from destructors)

	return rVal;
}
