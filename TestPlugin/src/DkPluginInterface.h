/*******************************************************************************************************
 DkPluginInterface.h
 Created on:	14.04.2013

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

#include <QStringList>
#include <QString>
#include <QImage>

namespace nmc {

class DkPluginInterface {
public:
	virtual ~DkPluginInterface() {}

    virtual QString pluginID() const = 0;
    virtual QString pluginName() const = 0;
    virtual QString pluginDescription() const = 0;
    virtual QImage pluginDescriptionImage() const = 0;
    virtual QString pluginVersion() const = 0;

    virtual QStringList runID() const = 0;
    virtual QString pluginMenuName(const QString &runID = "") const = 0;
    virtual QString pluginStatusTip(const QString &runID) const = 0;
	virtual QList<QAction*> pluginActions() { return QList<QAction*>();};
    virtual QImage runPlugin(const QString &runID, const QImage &image) const = 0;
};

};

Q_DECLARE_INTERFACE(nmc::DkPluginInterface, "com.TUWien.nomacs.DkPluginInterface/0.1")
