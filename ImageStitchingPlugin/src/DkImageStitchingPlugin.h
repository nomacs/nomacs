/*******************************************************************************************************
 DkImageStitchingPlugin.h

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2016 Markus Diem <markus@nomacs.org>

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

#include "DkPluginInterface.h"

namespace nmc {

class DkImageStitchingPlugin : public QObject, DkPluginInterface {
    Q_OBJECT
    Q_INTERFACES(nmc::DkPluginInterface)
    Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.DkImageStitchingPlugin/3.3" FILE "DkImageStitchingPlugin.json")

public:

    DkImageStitchingPlugin(QObject* parent = 0);
    ~DkImageStitchingPlugin();

    QString id() const override;
    QImage image() const override;

    QList<QAction*> createActions(QWidget* parent) override;
    QList<QAction*> pluginActions() const override;
    QSharedPointer<nmc::DkImageContainer> runPlugin(const QString &runID = QString(), QSharedPointer<nmc::DkImageContainer> imgC = QSharedPointer<nmc::DkImageContainer>()) const override;

    enum
    {
        id_end
    };

protected:
    QList<QAction*> mActions;
    QStringList mRunIDs;
    QStringList mMenuNames;
    QStringList mMenuStatusTips;
};

}
