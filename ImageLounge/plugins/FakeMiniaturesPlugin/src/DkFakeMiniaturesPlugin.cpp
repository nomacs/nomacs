/*******************************************************************************************************
 DkFakeMiniaturesPlugin.cpp
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

#include "DkFakeMiniaturesPlugin.h"

namespace nmp
{

/**
 * Returns descriptive iamge for every ID
 * @param plug-in ID
 **/
QImage DkFakeMiniaturesPlugin::image() const
{
    return QImage(":/nomacsPluginFakeMin/img/fakeMinDesc.png");
};

/**
 * Main function: runs plug-in based on its ID
 * @param plug-in ID
 * @param current imgC in the Nomacs viewport
 **/
QSharedPointer<nmc::DkImageContainer> DkFakeMiniaturesPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC) const
{
    qDebug() << "run id" << runID;
    if (imgC) {
        QMainWindow *mainWindow = getMainWindow();
        DkFakeMiniaturesDialog *fakeMiniaturesDialog;
        if (mainWindow)
            fakeMiniaturesDialog = new DkFakeMiniaturesDialog(mainWindow);
        else
            fakeMiniaturesDialog = new DkFakeMiniaturesDialog();

        QImage img = imgC->image();
        fakeMiniaturesDialog->setImage(&img);

        fakeMiniaturesDialog->exec();

        QImage returnImg(imgC->image());
        if (fakeMiniaturesDialog->wasOkPressed())
            returnImg = fakeMiniaturesDialog->getImage();

        fakeMiniaturesDialog->deleteLater();

        imgC->setImage(returnImg, tr("Fake Miniature"));
    }

    return imgC;
};

};
