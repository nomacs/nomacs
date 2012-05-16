/*******************************************************************************************************
 DkUtils.cpp
 Created on:	09.03.2010
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2012 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2012 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2012 Florian Kleber <florian@nomacs.org>

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

#include "DkUtils.h"

#ifdef linux
#include <sys/sysinfo.h>
#endif

int nmc::DkUtils::debugLevel = DK_MODULE;

namespace nmc {

// code based on: http://stackoverflow.com/questions/8565430/complete-these-3-methods-with-linux-and-mac-code-memory-info-platform-independe

double DkMemory::getTotalMemory() {

	double mem = -1;

#ifdef Q_WS_WIN

	MEMORYSTATUSEX MemoryStatus;
	ZeroMemory(&MemoryStatus, sizeof(MEMORYSTATUSEX));
	MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);

	if (GlobalMemoryStatusEx(&MemoryStatus)) {
		mem = MemoryStatus.ullTotalPhys;
	}

#elif linux

	struct sysinfo info;

	if (sysinfo(&info))
		mem = info.totalram;


#elif Q_WS_MAC

#endif

	// convert to MB
	if (mem > 0)
		mem /= (1024*1024);

	return mem;
}

double DkMemory::getFreeMemory() {

	double mem = -1;
	

#ifdef Q_WS_WIN

	MEMORYSTATUSEX MemoryStatus;

	ZeroMemory(&MemoryStatus, sizeof(MEMORYSTATUSEX));
	MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);

	if (GlobalMemoryStatusEx(&MemoryStatus)) {
		mem = MemoryStatus.ullAvailPhys;
	}

#elif linux

	struct sysinfo info;
	
	if (sysinfo(&info))
		mem = info.freeram;


#elif Q_WS_MAC

#endif

	// convert to MB
	if (mem > 0)
		mem /= (1024*1024);

	return mem;
}

}