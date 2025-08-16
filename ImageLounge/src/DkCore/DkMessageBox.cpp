/*******************************************************************************************************
 DkMessageBox.cpp
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

#include "DkMessageBox.h"

#include "DkSettings.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QStyle>

namespace nmc
{

// DkMessageBox --------------------------------------------------------------------
DkMessageBox::DkMessageBox(QMessageBox::Icon icon,
                           const QString &title,
                           const QString &text,
                           QMessageBox::StandardButtons buttons /* = QMessageBox::NoButton */,
                           QWidget *parent /* = 0 */,
                           Qt::WindowFlags f /* = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint */)
    : QDialog(parent, f)
{
    createLayout(icon, text, buttons);
    setWindowTitle(title);
}

DkMessageBox::~DkMessageBox()
{
    // save settings
    DefaultSettings settings;
    settings.beginGroup("DkDialog");
    settings.setValue(objectName(), !showAgain->isChecked());
    settings.endGroup();
}

// Modified from
// https://github.com/qt/qtbase/blob/cca658d4821b6d7378df13c29d1dab53c44359ac/src/widgets/dialogs/qmessagebox.cpp#L2735C1-L2761C2
QPixmap msgBoxStandardIcon(QMessageBox::Icon icon)
{
    QStyle *style = QApplication::style();
    int iconSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize);
    QIcon tmpIcon;
    switch (icon) {
    case QMessageBox::Information:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxInformation);
        break;
    case QMessageBox::Warning:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning);
        break;
    case QMessageBox::Critical:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxCritical);
        break;
    case QMessageBox::Question:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxQuestion);
        break;
    default:
        break;
    }
    if (tmpIcon.isNull()) {
        return {};
    }
    return tmpIcon.pixmap(QSize(iconSize, iconSize), qApp->devicePixelRatio());
}

void DkMessageBox::createLayout(QMessageBox::Icon userIcon,
                                const QString &userText,
                                QMessageBox::StandardButtons buttons)
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    // schamlos von qmessagebox.cpp geklaut
    textLabel = new QLabel(userText);
    textLabel->setTextInteractionFlags(
        Qt::TextInteractionFlags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, nullptr, this)));

    textLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    textLabel->setOpenExternalLinks(true);
    textLabel->setContentsMargins(2, 0, 0, 0);
    textLabel->setIndent(9);

    iconLabel = new QLabel;
    iconLabel->setPixmap(msgBoxStandardIcon(userIcon));
    iconLabel->setObjectName(QLatin1String("iconLabel"));
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    showAgain = new QCheckBox(tr("Remember my choice"));
    showAgain->setChecked(true);

    buttonBox = new QDialogButtonBox;
    buttonBox->setObjectName(QLatin1String("buttonBox"));
    buttonBox->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons, nullptr, this) != 0);
    QObject::connect(buttonBox, &QDialogButtonBox::clicked, this, &DkMessageBox::buttonClicked);

    buttonBox->setStandardButtons(QDialogButtonBox::StandardButtons(int(buttons)));

    auto *grid = new QGridLayout;
#if 1
    grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop);
    grid->addWidget(textLabel, 0, 1, 1, 1);
    grid->addWidget(showAgain, 2, 1, 1, 2);
    grid->addWidget(buttonBox, 3, 0, 1, 2);
#else
    grid->setVerticalSpacing(8);
    grid->setHorizontalSpacing(0);
    setContentsMargins(24, 15, 24, 20);
    grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop | Qt::AlignLeft);
    grid->addWidget(textLabel, 0, 1, 1, 1);
    // -- leave space for information label --
    grid->setRowStretch(1, 100);
    grid->setRowMinimumHeight(2, 6);
    grid->addWidget(buttonBox, 3, 1, 1, 1);
#endif

    // grid->setSizeConstraint(QLayout::SetNoConstraint);
    setLayout(grid);

    setModal(true);
}

void DkMessageBox::setVisible(bool visible)
{
    if (visible)
        adjustSize();

    QDialog::setVisible(visible);
}

int DkMessageBox::exec()
{
    QString objName = objectName();

    DefaultSettings settings;
    settings.beginGroup("DkDialog");
    bool show = settings.value(objName, true).toBool();
    int answer = settings.value(objName + "-answer", QDialog::Accepted).toInt();
    settings.endGroup();
    showAgain->setChecked(!show);

    if (!show)
        return answer;

    answer = QDialog::exec(); // destroys dialog - be careful with what you do afterwards

    settings.beginGroup("DkDialog");
    if (answer != QMessageBox::NoButton && answer != QMessageBox::Cancel) {
        // save show again
        settings.setValue(objName + "-answer", answer);
    } else
        settings.setValue(objName, true);
    settings.endGroup();

    return answer;
}

void DkMessageBox::setDefaultButton(QMessageBox::StandardButton button)
{
    QPushButton *b = buttonBox->button(QDialogButtonBox::StandardButton(button));

    if (!b)
        return;

    b->setDefault(true);
    b->setFocus();
}

void DkMessageBox::setButtonText(QMessageBox::StandardButton button, const QString &text)
{
    if (QAbstractButton *abstractButton = buttonBox->button(QDialogButtonBox::StandardButton(button)))
        abstractButton->setText(text);
}

void DkMessageBox::buttonClicked(QAbstractButton *button)
{
    int ret = buttonBox->standardButton(button);
    done(ret); // does not trigger closeEvent
}

void DkMessageBox::updateSize()
{
    if (!isVisible())
        return;

    QFontMetrics fm(QApplication::font("QMdiSubWindowTitleBar"));

    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
    QSize screenSize = screen ? screen->size() : QSize(1024, 768); // diem: be safe
    int textWidth = fm.horizontalAdvance(windowTitle()) + 50;

#if defined(Q_OS_WINCE)
    // the width of the screen, less the window border.
    int hardLimit = screenSize.width() - (frameGeometry().width() - geometry().width());
#else
    int hardLimit = qMin(screenSize.width() - 480, 1000); // can never get bigger than this
    // on small screens allows the messagebox be the same size as the screen
    if (screenSize.width() <= 1024)
        hardLimit = screenSize.width();
#endif
#ifdef Q_OS_MAC
    int softLimit = qMin(screenSize.width() / 2, 420);
#else
        // note: ideally on windows, hard and soft limits but it breaks compat
#ifndef Q_OS_WINCE
    int softLimit = qMin(screenSize.width() / 2, 500);
#else
    int softLimit = qMin(screenSize.width() * 3 / 4, 500);
#endif // Q_OS_WINCE
#endif

    textLabel->setWordWrap(false); // makes the label return min size
    int width = minimumWidth();

    if (width > softLimit) {
        textLabel->setWordWrap(true);
        width = qMax(softLimit, minimumWidth());

        if (width > hardLimit)
            width = hardLimit;
    }

    int windowTitleWidth = qMin(textWidth, hardLimit);
    if (windowTitleWidth > width)
        width = windowTitleWidth;

    this->setFixedSize(width, minimumHeight());
    QCoreApplication::removePostedEvents(this, QEvent::LayoutRequest);
}

}
