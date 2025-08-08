/*******************************************************************************************************
 DkFakeMiniaturesPlugin.h
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

#include <QImage>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <QtPlugin>

#include "DkFakeMiniaturesDialog.h"
#include "DkPluginInterface.h"

namespace nmp
{

class DkFakeMiniaturesPlugin : public QObject, nmc::DkPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(nmc::DkPluginInterface)
    Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.DkFakeMiniaturesPlugin/3.2" FILE "DkFakeMiniaturesPlugin.json")

public:
    QImage image() const override;

    QSharedPointer<nmc::DkImageContainer> runPlugin(
        const QString &runID = QString(),
        QSharedPointer<nmc::DkImageContainer> image = QSharedPointer<nmc::DkImageContainer>()) const override;
};

};
