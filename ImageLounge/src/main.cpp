/*******************************************************************************************************
 main.cpp
 Created on:	21.04.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011 Florian Kleber <florian@nomacs.org>

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




#include "DkNoMacs.h"
#include "DkImage.h"

#include <QtGui/QApplication>
#include <QFileInfo>
#include <QProcess>

#include <exiv2/exiv2.hpp>
#include <libraw/libraw.h>
#include <iostream>
#include <cassert>

//#include <QFile>

#ifdef WIN32
int main(int argc, wchar_t *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif

#ifdef linux
	QApplication::setGraphicsSystem("raster");
#endif
	//! \warning those QSettings setup *must* go before QApplication object
    //           to prevent random crashes (well, crashes are regular on mac
    //           opening from Finder)
	// register our organization
	QCoreApplication::setOrganizationName("nomacs");
	QCoreApplication::setOrganizationDomain("http://www.nomacs.org");
	QCoreApplication::setApplicationName("Image Lounge");


#ifdef  Q_WS_MAC
	//! \warning	this is somehow embarrassing but
	//				even though the DkNomacsApp does not do anything
	//				but calling the QApplication on windows/linux
	//				it causes a segmentation fault in QMenuBar.
	//				Thus, we call QApplication for these systems
	DkNoMacsApp a(argc, argv);
#else
	QApplication a(argc, (char**)argv);
#endif

	QStringList args = a.arguments();
	DkNoMacs* w;

	// DEBUG --------------------------------------------------------------------
	qDebug() << "input arguments:";
	for (int idx = 0; idx < args.size(); idx++)
		qDebug() << args[idx];
	qDebug() << "\n";
	// DEBUG --------------------------------------------------------------------

	if (args.size() > 2 && !args[2].compare("1")) {
		w = static_cast<DkNoMacs*> (new DkNoMacsFrameless());
		qDebug() << "this is the frameless nomacs...";
	}
	else
		w = static_cast<DkNoMacs*> (new DkNoMacsIpl());	// slice it

	if (args.size() > 1)
		w->viewport()->loadFile(QFileInfo(args[1]), true, true);	// update folder + be silent
#ifdef Q_WS_MAC
	QObject::connect(&a, SIGNAL(loadFile(const QFileInfo&)),
	                 w->viewport(), SLOT(loadFile(const QFileInfo&)));
#endif

	int rVal = a.exec();
	delete w;
	return rVal;
}

