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


int main(int argc, char *argv[]) {

	// DEBUG --------------------------------------------------------------------
	printf("number of arguments: %i\n", argc);

	for (int idx = 0; idx < argc; idx++)
		printf("%s\n", argv[idx]);

	DkUtils::setDebug(DK_DEBUG_A);
	// DEBUG --------------------------------------------------------------------

#ifdef linux
	QApplication::setGraphicsSystem("raster");
#endif

	// init app
	QApplication a(argc, argv);
	
	// register our organization
	QCoreApplication::setOrganizationName("nomacs");
	QCoreApplication::setOrganizationDomain("http://www.nomacs.org");
	QCoreApplication::setApplicationName("Image Lounge");

	DkNoMacsIpl w;
	if (argc > 1)
		w.viewport()->loadFile(QFileInfo(argv[1]), true, true);	// update folder + be silent
	//else {
	//	// load default image...
	//	w.viewport()->loadFile(QFileInfo("C:\\VsProjects\\img\\imgs\\img0001.tif"));
	//}

	return a.exec();
}
