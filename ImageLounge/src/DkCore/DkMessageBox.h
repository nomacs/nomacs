/*******************************************************************************************************
 DkMessageBox.h
 Created on:	28.03.2014

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2014 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2014 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2014 Florian Kleber <florian@nomacs.org>

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

#pragma warning(push, 0) // no warnings from includes - begin
#include <QDialog>
#include <QMessageBox>
#pragma warning(pop) // no warnings from includes - end

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

// Qt defines
class QDialogButtonBox;
class QCheckBox;

namespace nmc
{

class DllCoreExport DkMessageBox : public QDialog
{
    Q_OBJECT

public:
    DkMessageBox(QMessageBox::Icon icon,
                 const QString &title,
                 const QString &text,
                 QMessageBox::StandardButtons buttons = QMessageBox::NoButton,
                 QWidget *parent = nullptr,
                 Qt::WindowFlags f = Qt::Dialog);

    ~DkMessageBox();

    virtual void setVisible(bool visible) override;
    void setDefaultButton(QMessageBox::StandardButton button);
    void setButtonText(QMessageBox::StandardButton button, const QString &text);

public slots:
    void buttonClicked(QAbstractButton *button);
    int exec() override;

protected:
    QLabel *iconLabel;
    QLabel *textLabel;
    QDialogButtonBox *buttonBox;
    QCheckBox *showAgain;

    void createLayout(QMessageBox::Icon userIcon, const QString &userText, QMessageBox::StandardButtons buttons);
    void updateSize();
};

}
