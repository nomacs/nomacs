/*******************************************************************************************************
 PLUGIN_CLASS_NAME.h

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2015 Markus Diem <markus@nomacs.org>

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

namespace nmc
{

class PLUGIN_CLASS_NAME : public QObject, DkPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(nmc::DkPluginInterface)
    Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.PLUGIN_CLASS_NAME/3.2" FILE "PLUGIN_CLASS_NAME.json")

public:
    PLUGIN_CLASS_NAME(QObject *parent = 0);
    ~PLUGIN_CLASS_NAME();

    QImage image() const override;

    QList<QAction *> createActions(QWidget *parent) override;
    QList<QAction *> pluginActions() const override;
    QSharedPointer<nmc::DkImageContainer> runPlugin(const QString &runID = QString(),
                                                    QSharedPointer<nmc::DkImageContainer> imgC = QSharedPointer<nmc::DkImageContainer>()) const override;

    enum {
        ID_ACTION1,
        // add actions here

        id_end
    };

protected:
    QList<QAction *> mActions;
    QStringList mRunIDs;
    QStringList mMenuNames;
    QStringList mMenuStatusTips;
};

};
