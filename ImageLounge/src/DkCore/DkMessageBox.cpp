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
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QStyle>
#include <QTimeZone>

namespace nmc
{

// DkMessageBox --------------------------------------------------------------------
DkMessageBox::DkMessageBox(QMessageBox::Icon icon,
                           const QString &title,
                           const QString &text,
                           QMessageBox::StandardButtons buttons,
                           QWidget *parent,
                           Qt::WindowFlags f)
    : QDialog(parent, f)
{
    createLayout(icon, text, buttons);
    setWindowTitle(title);
}

DkMessageBox::~DkMessageBox() = default;

// Modified from
// https://github.com/qt/qtbase/blob/cca658d4821b6d7378df13c29d1dab53c44359ac/src/widgets/dialogs/qmessagebox.cpp#L2735C1-L2761C2
QPixmap DkMessageBox::msgBoxStandardIcon(QMessageBox::Icon icon) const
{
    QStyle *style = this->style();
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

    iconSize *= devicePixelRatio();
    QPixmap pm = tmpIcon.pixmap(QSize(iconSize, iconSize), 1.0);
    pm.setDevicePixelRatio(devicePixelRatio());
    return pm;
}

void DkMessageBox::createLayout(QMessageBox::Icon userIcon,
                                const QString &userText,
                                QMessageBox::StandardButtons buttons)
{
    auto *textLabel = new QLabel(userText);
    textLabel->setTextInteractionFlags(
        Qt::TextInteractionFlags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, nullptr, this)));
    textLabel->setObjectName("textLabel");
    textLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    textLabel->setOpenExternalLinks(true);
    textLabel->setIndent(0);

    auto *iconLabel = new QLabel;
    iconLabel->setPixmap(msgBoxStandardIcon(userIcon));
    iconLabel->setObjectName("iconLabel");
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mShowAgain = new QCheckBox(tr("Remember my choice"));
    mShowAgain->setObjectName("checkBox");
    mShowAgain->setChecked(true);

    QStringList options;
    options.resize(numOptions);
    options[opt_forever] = tr("Forever");
    options[opt_session] = tr("This Session");
    options[opt_hour] = tr("One Hour");
    options[opt_day] = tr("One Day");
    options[opt_week] = tr("One Week");

    mOptionBox = new QComboBox{};
    mOptionBox->setObjectName("comboBox");
    mOptionBox->addItems(options);
    mOptionBox->setEnabled(false);
    connect(mShowAgain, &QCheckBox::toggled, [this](bool checked) {
        mOptionBox->setEnabled(checked);
    });

    mButtonBox = new QDialogButtonBox;
    mButtonBox->setObjectName("buttonBox");
    mButtonBox->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons, nullptr, this) != 0);
    mButtonBox->setStandardButtons(QDialogButtonBox::StandardButtons(int(buttons)));
    QObject::connect(mButtonBox, &QDialogButtonBox::clicked, this, &DkMessageBox::buttonClicked);

    // less crowded with some space above button box
    auto *spacer = new QFrame;
    spacer->setFrameShape(QFrame::NoFrame);

    auto *grid = new QGridLayout;
    int leftMargin = style()->pixelMetric(QStyle::PM_LayoutLeftMargin, nullptr, this);
    grid->setSpacing(leftMargin);
    grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop);
    grid->addWidget(textLabel, 0, 1, 2, 1);
    grid->addWidget(mShowAgain, 2, 1, 1, 1);
    grid->addWidget(mOptionBox, 3, 1, 1, 1);
    grid->addWidget(spacer, 4, 0, 1, 2);
    grid->addWidget(mButtonBox, 5, 0, 1, 2);

    setLayout(grid);
    setModal(true);
}

void DkMessageBox::setVisible(bool visible)
{
    if (visible) {
        adjustSize();
    }

    QDialog::setVisible(visible);
}

int DkMessageBox::exec()
{
    const qulonglong sessionId = DkSettingsManager::param().global().sessionId;

    const QString dialogId = objectName();
    const QString answerKey = dialogId + "-answer";
    const QString optionKey = dialogId + "-option";
    const QString timeKey = dialogId + "-time";
    const QString sessionKey = "sessionId";

    DefaultSettings settings;
    settings.beginGroup("DkDialog");

    bool show = settings.value(dialogId, true).toBool();
    int answer = settings.value(answerKey, QDialog::Accepted).toInt();
    const int option = settings.value(optionKey, opt_forever).toInt();
    const QDateTime time = settings.value(timeKey).toDateTime();

    // if the dialog times out show with the last options chosen
    mShowAgain->setChecked(!show);
    mOptionBox->setEnabled(!show);
    mOptionBox->setCurrentIndex(option);

    if (!show) {
        QDateTime expiration;
        const QDateTime now = QDateTime::currentDateTime();
        const QDateTime future = now.addSecs(60 * 60);
        const QDateTime past = now.addSecs(-60 * 60);

        switch (option) {
        case opt_forever:
            expiration = future;
            break;
        case opt_session: {
            qulonglong savedId = settings.value(sessionKey).toULongLong();
            expiration = sessionId == savedId ? future : past;
            break;
        }
        case opt_hour:
            expiration = time.addSecs(60 * 60);
            break;
        case opt_day:
            expiration = time.date().endOfDay();
            break;
        case opt_week:
            expiration = time.date().addDays(6).endOfDay();
            break;
        default:
            qWarning() << "unknown dialog option:" << settings.group() << optionKey << option;
            expiration = past;
            break;
        }
        show = now > expiration;
    }

    if (!show) {
        qInfo() << this << "skipped with answer" << static_cast<QMessageBox::StandardButton>(answer) << answer;
        return answer;
    }

    if (testAttribute(Qt::WA_DeleteOnClose)) {
        qFatal("WA_DeleteOnClose deletes before exec() returns!");
        return 0;
    }

    answer = QDialog::exec();

    show = !mShowAgain->isChecked();

    if (!show && answer != QMessageBox::NoButton && answer != QMessageBox::Cancel) {
        settings.setValue(dialogId, false);
        settings.setValue(answerKey, answer);
        settings.setValue(optionKey, mOptionBox->currentIndex());
        settings.setValue(timeKey, QDateTime::currentDateTime());
        settings.setValue(sessionKey, sessionId);
    } else {
        settings.remove(dialogId);
        settings.remove(answerKey);
        settings.remove(optionKey);
        settings.remove(timeKey);
    }

    return answer;
}

void DkMessageBox::setDefaultButton(QMessageBox::StandardButton button)
{
    QPushButton *b = mButtonBox->button(QDialogButtonBox::StandardButton(button));
    if (b) {
        b->setDefault(true);
        b->setFocus();
    }
}

void DkMessageBox::setButtonText(QMessageBox::StandardButton button, const QString &text)
{
    QPushButton *b = mButtonBox->button(QDialogButtonBox::StandardButton(button));
    if (b) {
        b->setText(text);
    }
}

void DkMessageBox::setCheckBoxText(const QString &text)
{
    mShowAgain->setText(text);
}

void DkMessageBox::buttonClicked(QAbstractButton *button)
{
    int ret = mButtonBox->standardButton(button);
    done(ret); // does not trigger closeEvent
}
}
