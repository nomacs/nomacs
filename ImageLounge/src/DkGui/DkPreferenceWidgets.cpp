
/*******************************************************************************************************
DkPreferenceWidgets.cpp
Created on:	14.12.2015

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

#include "DkPreferenceWidgets.h"

#include "DkBasicWidgets.h"
#include "DkDialog.h"
#include "DkImageStorage.h"
#include "DkNoMacs.h"
#include "DkSettings.h"
#include "DkSettingsWidget.h"
#include "DkThemeManager.h"
#include "DkUtils.h"
#include "DkWidgets.h"

#pragma warning(push, 0) // no warnings from includes
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QRadioButton>
#include <QSpinBox>
#include <QStackedLayout>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QStyleOption>
#include <QTableView>
#include <QVBoxLayout>
#include <QtGlobal>
#pragma warning(pop)

namespace nmc
{

DkPreferenceWidget::DkPreferenceWidget(QWidget *parent)
    : DkFadeWidget(parent)
{
    createLayout();

    QAction *nextAction = new QAction(tr("next"), this);
    nextAction->setShortcut(Qt::Key_PageDown);
    connect(nextAction, &QAction::triggered, this, &DkPreferenceWidget::nextTab);
    addAction(nextAction);

    QAction *previousAction = new QAction(tr("previous"), this);
    previousAction->setShortcut(Qt::Key_PageUp);
    connect(previousAction, &QAction::triggered, this, &DkPreferenceWidget::previousTab);
    addAction(previousAction);
}

void DkPreferenceWidget::createLayout()
{
    // create tab entries
    QWidget *tabs = new QWidget(this);
    tabs->setObjectName("DkPreferenceTabs");

    QSize s(32, 32);
    QPixmap pm = DkImage::loadIcon(":/nomacs/img/power.svg", QColor(255, 255, 255), s);
    QPushButton *restartButton = new QPushButton(pm, "", this);
    restartButton->setFlat(true);
    restartButton->setIconSize(pm.size());
    restartButton->setObjectName("DkRestartButton");
    restartButton->setStatusTip(tr("Restart nomacs"));
    connect(restartButton, &QPushButton::clicked, this, &DkPreferenceWidget::restartSignal);

    mTabLayout = new QVBoxLayout(tabs);
    mTabLayout->setContentsMargins(0, 60, 0, 0);
    mTabLayout->setSpacing(0);
    mTabLayout->setAlignment(Qt::AlignTop);
    mTabLayout->addStretch();
    mTabLayout->addWidget(restartButton);

    // create central widget
    QWidget *centralWidget = new QWidget(this);
    mCentralLayout = new QStackedLayout(centralWidget);
    mCentralLayout->setContentsMargins(0, 0, 0, 0);

    // add a scroll area
    DkResizableScrollArea *scrollAreaTabs = new DkResizableScrollArea(this);
    scrollAreaTabs->setObjectName("DkPreferenceTabsScroller");
    scrollAreaTabs->setWidgetResizable(true);
    scrollAreaTabs->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    scrollAreaTabs->setWidget(tabs);

    QHBoxLayout *sL = new QHBoxLayout(this);
    sL->setContentsMargins(0, 0, 0, 0); // use 1 to get the border
    sL->setSpacing(0);
    sL->setAlignment(Qt::AlignLeft);
    sL->addWidget(scrollAreaTabs);
    sL->addWidget(centralWidget);
}

void DkPreferenceWidget::addTabWidget(DkPreferenceTabWidget *tabWidget)
{
    mWidgets.append(tabWidget);
    mCentralLayout->addWidget(tabWidget);

    DkTabEntryWidget *tabEntry = new DkTabEntryWidget(tabWidget->icon(), tabWidget->name(), this);
    mTabLayout->insertWidget(mTabLayout->count() - 2, tabEntry); // -2 -> insert before stretch
    connect(tabEntry, &DkTabEntryWidget::clicked, this, &DkPreferenceWidget::changeTab);
    connect(tabWidget, &DkPreferenceTabWidget::restartSignal, this, &DkPreferenceWidget::restartSignal);
    mTabEntries.append(tabEntry);

    // tick the first
    if (mTabEntries.size() == 1)
        tabEntry->click();
}

void DkPreferenceWidget::previousTab()
{
    // modulo is sign aware in cpp...
    int idx = (mCurrentIndex == 0) ? mWidgets.size() - 1 : mCurrentIndex - 1;
    setCurrentIndex(idx);
}

void DkPreferenceWidget::nextTab()
{
    setCurrentIndex((mCurrentIndex + 1) % mWidgets.size());
}

void DkPreferenceWidget::changeTab()
{
    DkTabEntryWidget *te = qobject_cast<DkTabEntryWidget *>(sender());

    for (int idx = 0; idx < mTabEntries.size(); idx++) {
        if (te == mTabEntries[idx]) {
            setCurrentIndex(idx);
        }
    }
}

void DkPreferenceWidget::setCurrentIndex(int index)
{
    // something todo here?
    if (index == mCurrentIndex)
        return;

    mCurrentIndex = index;
    mCentralLayout->setCurrentIndex(index);

    // now check the correct one
    for (int idx = 0; idx < mTabEntries.size(); idx++)
        mTabEntries[idx]->setChecked(idx == index);
}

// DkPreferenceTabWidget --------------------------------------------------------------------
DkPreferenceTabWidget::DkPreferenceTabWidget(const QIcon &icon, const QString &name, QWidget *parent)
    : DkNamedWidget(name, parent)
{
    setObjectName("DkPreferenceTab");
    mIcon = icon;

    createLayout();
}

void DkPreferenceTabWidget::createLayout()
{
    QLabel *titleLabel = new QLabel(name(), this);
    titleLabel->setObjectName("DkPreferenceTitle");

    // add a scroll area
    mCentralScroller = new DkResizableScrollArea(this);
    mCentralScroller->setObjectName("DkPreferenceScroller");
    mCentralScroller->setWidgetResizable(true);

    mInfoButton = new QPushButton(tr(""), this);
    mInfoButton->setObjectName("infoButton");
    mInfoButton->setFlat(true);
    mInfoButton->setVisible(false);
    connect(mInfoButton, &QPushButton::clicked, this, &DkPreferenceTabWidget::restartSignal);

    QGridLayout *l = new QGridLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->setAlignment(Qt::AlignTop);
    l->addWidget(titleLabel, 0, 0);
    l->addWidget(mCentralScroller, 1, 0);
    l->addWidget(mInfoButton, 2, 0, Qt::AlignBottom);
}

void DkPreferenceTabWidget::setWidget(QWidget *w)
{
    mCentralScroller->setWidget(w);
    w->setObjectName("DkPreferenceWidget");
}

void DkPreferenceTabWidget::setInfoMessage(const QString &msg)
{
    mInfoButton->setText(msg);
    mInfoButton->setVisible(!msg.isEmpty());
}

QWidget *DkPreferenceTabWidget::widget() const
{
    return mCentralScroller->widget();
}

QIcon DkPreferenceTabWidget::icon() const
{
    return mIcon;
}

// DkGroupWidget --------------------------------------------------------------------
DkGroupWidget::DkGroupWidget(const QString &title, QWidget *parent)
    : DkWidget(parent)
{
    setObjectName("DkGroupWidget");
    mTitle = title;

    createLayout();
}

void DkGroupWidget::createLayout()
{
    QLabel *titleLabel = new QLabel(mTitle, this);
    titleLabel->setObjectName("subTitle");

    // we create a content widget to have control over the margins
    QWidget *contentWidget = new QWidget(this);
    mContentLayout = new QVBoxLayout(contentWidget);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(titleLabel);
    layout->addWidget(contentWidget);
}

void DkGroupWidget::addWidget(QWidget *widget)
{
    mContentLayout->addWidget(widget);
}

void DkGroupWidget::addSpace()
{
    mContentLayout->addSpacing(10);
}

void DkGroupWidget::paintEvent(QPaintEvent *event)
{
    // fixes stylesheets which are not applied to custom widgets
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}

// DkGeneralPreference --------------------------------------------------------------------
DkGeneralPreference::DkGeneralPreference(QWidget *parent)
    : DkWidget(parent)
{
    createLayout();
}

void DkGeneralPreference::createLayout()
{
    static const QString restartText = tr("Previewing theme. Restart nomacs to apply all changes.");

    // Theme
    auto &tm = DkThemeManager::instance();
    connect(&tm, &DkThemeManager::themeApplied, this, [this] {
        emit infoSignal(restartText);
    });

    auto &display = DkSettingsManager::param().display();
    auto &slideshow = DkSettingsManager::param().slideShow();

    static bool noSetting = true; // TODO: defaultSlideShowColor
    static const QColor defaultSlideShowColor(51, 51, 51); // TODO: themeSlideShowColor

    struct {
        const QString caption;
        const bool restartRequired; // TODO: everying using icons must connect a theme change signal!
        const QColor *themeColor; // setting: system palette or theme css
        QColor *userColor; // setting: user modified color
        bool *isThemeColor; // setting: true if user modified a color
        DkColorChooser *chooser;
    } colors[] = {{tr("Icon Color"), true, &display.themeIconColor, &display.iconColor, &display.defaultIconColor},
                  {tr("Foreground Color"), false, &display.themeFgdColor, &display.fgColor, &display.defaultForegroundColor},
                  {tr("Background Color"), false, &display.themeBgdColor, &display.bgColor, &display.defaultBackgroundColor},
                  {tr("Fullscreen Color"), false, &defaultSlideShowColor, &slideshow.backgroundColor, &noSetting}};

    for (auto &c : colors) {
        c.chooser = new DkColorChooser(*c.themeColor, c.caption, this);
        c.chooser->setColor(*c.userColor);
        connect(c.chooser, &DkColorChooser::colorAccepted, this, [this, c]() {
            *c.isThemeColor = false;
        });
        connect(c.chooser, &DkColorChooser::colorReset, this, [this, c]() {
            *c.isThemeColor = true;
        });
        connect(c.chooser, &DkColorChooser::colorChanged, this, [this, c](const QColor &color) {
            *c.userColor = *c.isThemeColor ? *c.themeColor : color;
            if (c.restartRequired)
                emit infoSignal(restartText);
            else
                DkThemeManager::instance().applyTheme();
        });
        connect(&tm, &DkThemeManager::themeApplied, this, [this, c] {
            c.chooser->setDefaultColor(*c.themeColor); // reset uses new color
            if (*c.isThemeColor) // keep user color
                c.chooser->setColor(*c.themeColor);
        });
    }

    const QStringList themes = tm.getAvailableThemes();

    QComboBox *themeBox = new QComboBox(this);
    themeBox->setToolTip(tr("Sets the overall theme. The System theme uses the operating system theme except for custom widgets"));
    for (auto &themeFile : themes)
        themeBox->addItem(tm.cleanThemeName(themeFile), themeFile);
    themeBox->setCurrentText(tm.cleanThemeName(tm.getCurrentThemeName()));

    connect(themeBox, &QComboBox::currentTextChanged, this, [this, themeBox, colors](const QString &text) {
        (void)text;
        const QString themeFile = themeBox->currentData().toString();
        auto &tm = DkThemeManager::instance();

        if (tm.getCurrentThemeName() != themeFile) {
            tm.setCurrentTheme(themeFile);
            tm.applyTheme();
        }
    });

    QComboBox *stylesBox = new QComboBox(this);
    stylesBox->setToolTip(tr("Sets the appearance of buttons, checkboxes, etc. on the System theme or otherwise unstyled elements"));
    stylesBox->insertItems(0, tm.getStylePlugins());
    stylesBox->setCurrentText(display.stylePlugin);
    connect(stylesBox, &QComboBox::currentTextChanged, this, [this, stylesBox](const QString &text) {
        DkThemeManager::instance().setStylePlugin(text);
        DkSettingsManager::param().display().stylePlugin = text;
    });

    DkGroupWidget *themeGroup = new DkGroupWidget(tr("Appearance"), this);
    themeGroup->addWidget(new QLabel(tr("Nomacs  Theme")));
    themeGroup->addWidget(themeBox);
    themeGroup->addWidget(new QLabel(tr("Widget Theme")));
    themeGroup->addWidget(stylesBox);
    for (auto &c : colors)
        themeGroup->addWidget(c.chooser);

    // default pushbutton
    QPushButton *defaultSettings = new QPushButton(tr("Reset All Settings"));
    defaultSettings->setMaximumWidth(300);
    connect(defaultSettings, &QPushButton::clicked, this, &DkGeneralPreference::onDefaultSettingsClicked);

    QPushButton *importSettings = new QPushButton(tr("&Import Settings"));
    importSettings->setMaximumWidth(300);
    connect(importSettings, &QPushButton::clicked, this, &DkGeneralPreference::onImportSettingsClicked);

    QPushButton *exportSettings = new QPushButton(tr("&Export Settings"));
    exportSettings->setMaximumWidth(300);
    connect(exportSettings, &QPushButton::clicked, this, &DkGeneralPreference::onExportSettingsClicked);

    DkGroupWidget *defaultGroup = new DkGroupWidget(tr("Default Settings"), this);
    defaultGroup->addWidget(defaultSettings);
    defaultGroup->addWidget(importSettings);
    defaultGroup->addWidget(exportSettings);

    // the left column (holding all color settings)
    QWidget *leftColumn = new QWidget(this);
    leftColumn->setMinimumWidth(400);

    QVBoxLayout *leftColumnLayout = new QVBoxLayout(leftColumn);
    leftColumnLayout->setAlignment(Qt::AlignTop);
    leftColumnLayout->addWidget(themeGroup);
    leftColumnLayout->addWidget(defaultGroup);

    // checkboxes
    QCheckBox *cbRecentFiles = new QCheckBox(tr("Show Recent Files on Start-Up"), this);
    cbRecentFiles->setToolTip(tr("Show the History Panel on Start-Up"));
    cbRecentFiles->setChecked(DkSettingsManager::param().app().showRecentFiles);
    connect(cbRecentFiles, &QCheckBox::toggled, this, &DkGeneralPreference::onShowRecentFilesToggled);

    QCheckBox *cbLogRecentFiles = new QCheckBox(tr("Remember Recent Files History"), this);
    cbLogRecentFiles->setToolTip(tr("If checked, recent files will be saved."));
    cbLogRecentFiles->setChecked(DkSettingsManager::param().global().logRecentFiles);
    connect(cbLogRecentFiles, &QCheckBox::toggled, this, &DkGeneralPreference::onLogRecentFilesToggled);

    QCheckBox *cbCheckOpenDuplicates = new QCheckBox(tr("Check for Duplicates on Open"), this);
    cbCheckOpenDuplicates->setToolTip(tr("If any files are opened which are already open in a tab, don't open them again."));
    cbCheckOpenDuplicates->setChecked(DkSettingsManager::param().global().checkOpenDuplicates);
    connect(cbCheckOpenDuplicates, &QCheckBox::toggled, this, &DkGeneralPreference::onCheckOpenDuplicatesToggled);

    QCheckBox *cbExtendedTabs = new QCheckBox(tr("Show extra options related to tabs"), this);
    cbExtendedTabs->setToolTip(
        tr("Enables the \"Go to Tab\", \"First Tab\", and \"Last Tab\" options in the View menu, and the \"Open Tabs\" and \"Save Tabs\" options in the File "
           "menu."));
    cbExtendedTabs->setChecked(DkSettingsManager::param().global().extendedTabs);
    connect(cbExtendedTabs, &QCheckBox::toggled, this, &DkGeneralPreference::onExtendedTabsToggled);

    QCheckBox *cbLoopImages = new QCheckBox(tr("Loop Images"), this);
    cbLoopImages->setToolTip(tr("Start with the first image in a folder after showing the last."));
    cbLoopImages->setChecked(DkSettingsManager::param().global().loop);
    connect(cbLoopImages, &QCheckBox::toggled, this, &DkGeneralPreference::onLoopImagesToggled);

    QCheckBox *cbZoomOnWheel = new QCheckBox(tr("Mouse Wheel Zooms"), this);
    cbZoomOnWheel->setToolTip(tr("If checked, the mouse wheel zooms - otherwise it is used to switch between images."));
    cbZoomOnWheel->setChecked(DkSettingsManager::param().global().zoomOnWheel);
    connect(cbZoomOnWheel, &QCheckBox::toggled, this, &DkGeneralPreference::onZoomOnWheelToggled);

    QCheckBox *cbHorZoomSkips = new QCheckBox(tr("Next Image on Horizontal Zoom"), this);
    cbHorZoomSkips->setToolTip(tr("If checked, horizontal wheel events load the next/previous images."));
    cbHorZoomSkips->setChecked(DkSettingsManager::param().global().horZoomSkips);
    connect(cbHorZoomSkips, &QCheckBox::toggled, this, &DkGeneralPreference::onHorZoomSkipsToggled);

    QCheckBox *cbDoubleClickForFullscreen = new QCheckBox(tr("Double Click Opens Fullscreen"), this);
    cbDoubleClickForFullscreen->setToolTip(tr("If checked, a double click on the canvas opens the fullscreen mode."));
    cbDoubleClickForFullscreen->setChecked(DkSettingsManager::param().global().doubleClickForFullscreen);
    connect(cbDoubleClickForFullscreen, &QCheckBox::toggled, this, &DkGeneralPreference::onDoubleClickForFullscreenToggled);

    QCheckBox *cbShowBgImage = new QCheckBox(tr("Show Background Image"), this);
    cbShowBgImage->setToolTip(tr("If checked, the nomacs logo is shown in the bottom right corner."));
    cbShowBgImage->setChecked(DkSettingsManager::param().global().showBgImage);
    connect(cbShowBgImage, &QCheckBox::toggled, this, &DkGeneralPreference::onShowBgImageToggled);

    QCheckBox *cbSwitchModifier = new QCheckBox(tr("Switch CTRL with ALT"), this);
    cbSwitchModifier->setToolTip(tr("If checked, CTRL + Mouse is switched with ALT + Mouse."));
    cbSwitchModifier->setChecked(DkSettingsManager::param().sync().switchModifier);
    connect(cbSwitchModifier, &QCheckBox::toggled, this, &DkGeneralPreference::onSwitchModifierToggled);

    QCheckBox *cbCloseOnEsc = new QCheckBox(tr("Close on ESC"), this);
    cbCloseOnEsc->setToolTip(tr("Close nomacs if ESC is pressed."));
    cbCloseOnEsc->setChecked(DkSettingsManager::param().app().closeOnEsc);
    connect(cbCloseOnEsc, &QCheckBox::toggled, this, &DkGeneralPreference::onCloseOnEscToggled);

    QCheckBox *cbCloseOnMiddleMouse = new QCheckBox(tr("Close on Middle Mouse Button"), this);
    cbCloseOnMiddleMouse->setToolTip(tr("Close nomacs if the Middle Mouse Button is pressed over the image."));
    cbCloseOnMiddleMouse->setChecked(DkSettingsManager::param().app().closeOnMiddleMouse);
    connect(cbCloseOnMiddleMouse, &QCheckBox::toggled, this, &DkGeneralPreference::onCloseOnMiddleMouseToggled);

    QCheckBox *cbCheckForUpdates = new QCheckBox(tr("Check For Updates"), this);
    cbCheckForUpdates->setToolTip(tr("Check for updates on start-up."));
    cbCheckForUpdates->setChecked(DkSettingsManager::param().sync().checkForUpdates);
    cbCheckForUpdates->setDisabled(DkSettingsManager::param().sync().disableUpdateInteraction);
    connect(cbCheckForUpdates, &QCheckBox::toggled, this, &DkGeneralPreference::onCheckForUpdatesToggled);

    DkGroupWidget *generalGroup = new DkGroupWidget(tr("General"), this);
    generalGroup->addWidget(cbRecentFiles);
    generalGroup->addWidget(cbLogRecentFiles);
    generalGroup->addWidget(cbCheckOpenDuplicates);
    generalGroup->addWidget(cbExtendedTabs);
    generalGroup->addWidget(cbLoopImages);
    generalGroup->addWidget(cbZoomOnWheel);
    generalGroup->addWidget(cbHorZoomSkips);
    generalGroup->addWidget(cbDoubleClickForFullscreen);
    generalGroup->addWidget(cbSwitchModifier);
    generalGroup->addWidget(cbCloseOnEsc);
    generalGroup->addWidget(cbCloseOnMiddleMouse);
    generalGroup->addWidget(cbCheckForUpdates);
    generalGroup->addWidget(cbShowBgImage);

    // language
    QComboBox *languageCombo = new QComboBox(this);
    languageCombo->setView(new QListView()); // fix style
    languageCombo->setObjectName("languageCombo");
    languageCombo->setToolTip(tr("Choose your preferred language."));
    DkUtils::addLanguages(languageCombo, mLanguages);
    languageCombo->setCurrentIndex(mLanguages.indexOf(DkSettingsManager::param().global().language));
    connect(languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DkGeneralPreference::onLanguageComboCurrentIndexChanged);

    QLabel *translateLabel = new QLabel("<a href=\"https://nomacs.org/how-to-translate-nomacs/\">How-to translate nomacs</a>", this);
    translateLabel->setToolTip(tr("Info on how to translate nomacs."));
    translateLabel->setOpenExternalLinks(true);

    DkGroupWidget *languageGroup = new DkGroupWidget(tr("Language"), this);
    languageGroup->addWidget(languageCombo);
    languageGroup->addWidget(translateLabel);

    // the right column (holding all checkboxes)
    QWidget *cbWidget = new QWidget(this);
    QVBoxLayout *cbLayout = new QVBoxLayout(cbWidget);
    cbLayout->setAlignment(Qt::AlignTop);
    cbLayout->addWidget(generalGroup);

    // add language
    cbLayout->addWidget(languageGroup);

    QHBoxLayout *contentLayout = new QHBoxLayout(this);
    contentLayout->setAlignment(Qt::AlignLeft);
    contentLayout->addWidget(leftColumn);
    contentLayout->addWidget(cbWidget);
}

void DkGeneralPreference::showRestartLabel() const
{
    emit infoSignal(tr("Please Restart nomacs to apply changes"));
}

void DkGeneralPreference::onShowRecentFilesToggled(bool checked) const
{
    if (DkSettingsManager::param().app().showRecentFiles != checked)
        DkSettingsManager::param().app().showRecentFiles = checked;
}

void DkGeneralPreference::onLogRecentFilesToggled(bool checked) const
{
    if (DkSettingsManager::param().global().logRecentFiles != checked)
        DkSettingsManager::param().global().logRecentFiles = checked;
}

void DkGeneralPreference::onCheckOpenDuplicatesToggled(bool checked) const
{
    if (DkSettingsManager::param().global().checkOpenDuplicates != checked)
        DkSettingsManager::param().global().checkOpenDuplicates = checked;
}

void DkGeneralPreference::onExtendedTabsToggled(bool checked) const
{
    if (DkSettingsManager::param().global().extendedTabs != checked) {
        DkSettingsManager::param().global().extendedTabs = checked;
        showRestartLabel();
    }
}

void DkGeneralPreference::onCloseOnEscToggled(bool checked) const
{
    if (DkSettingsManager::param().app().closeOnEsc != checked)
        DkSettingsManager::param().app().closeOnEsc = checked;
}

void DkGeneralPreference::onCloseOnMiddleMouseToggled(bool checked) const
{
    if (DkSettingsManager::param().app().closeOnMiddleMouse != checked)
        DkSettingsManager::param().app().closeOnMiddleMouse = checked;
}

void DkGeneralPreference::onZoomOnWheelToggled(bool checked) const
{
    if (DkSettingsManager::param().global().zoomOnWheel != checked) {
        DkSettingsManager::param().global().zoomOnWheel = checked;
    }
}

void DkGeneralPreference::onHorZoomSkipsToggled(bool checked) const
{
    if (DkSettingsManager::param().global().horZoomSkips != checked) {
        DkSettingsManager::param().global().horZoomSkips = checked;
    }
}

void DkGeneralPreference::onDoubleClickForFullscreenToggled(bool checked) const
{
    if (DkSettingsManager::param().global().doubleClickForFullscreen != checked)
        DkSettingsManager::param().global().doubleClickForFullscreen = checked;
}

void DkGeneralPreference::onShowBgImageToggled(bool checked) const
{
    if (DkSettingsManager::param().global().showBgImage != checked) {
        DkSettingsManager::param().global().showBgImage = checked;
        showRestartLabel();
    }
}

void DkGeneralPreference::onCheckForUpdatesToggled(bool checked) const
{
    if (DkSettingsManager::param().sync().checkForUpdates != checked)
        DkSettingsManager::param().sync().checkForUpdates = checked;
}

void DkGeneralPreference::onSwitchModifierToggled(bool checked) const
{
    if (DkSettingsManager::param().sync().switchModifier != checked) {
        DkSettingsManager::param().sync().switchModifier = checked;

        if (DkSettingsManager::param().sync().switchModifier) {
            DkSettingsManager::param().global().altMod = Qt::ControlModifier;
            DkSettingsManager::param().global().ctrlMod = Qt::AltModifier;
        } else {
            DkSettingsManager::param().global().altMod = Qt::AltModifier;
            DkSettingsManager::param().global().ctrlMod = Qt::ControlModifier;
        }
    }
}

void DkGeneralPreference::onLoopImagesToggled(bool checked) const
{
    if (DkSettingsManager::param().global().loop != checked)
        DkSettingsManager::param().global().loop = checked;
}

void DkGeneralPreference::onDefaultSettingsClicked()
{
    int answer = QMessageBox::warning(this,
                                      tr("Reset All Settings"),
                                      tr("This will reset all personal settings!"),
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (answer == QMessageBox::Yes) {
        DkSettingsManager::param().setToDefaultSettings();
        showRestartLabel();
        qDebug() << "answer is: " << answer << "flushing all settings...";
    }
}

void DkGeneralPreference::onImportSettingsClicked()
{
    QString filePath = QFileDialog::getOpenFileName(DkUtils::getMainWindow(),
                                                    tr("Import Settings"),
                                                    QDir::homePath(),
                                                    "Nomacs Settings (*.ini *.conf)",
                                                    nullptr,
                                                    DkDialog::fileDialogOptions());

    // user canceled?
    if (filePath.isEmpty())
        return;

    DkSettingsManager::importSettings(filePath);

    showRestartLabel();
}

void DkGeneralPreference::onExportSettingsClicked()
{
    QString filePath = QFileDialog::getSaveFileName(DkUtils::getMainWindow(),
                                                    tr("Export Settings"),
                                                    QDir::homePath(),
                                                    "Nomacs Settings (*.ini *.conf)",
                                                    nullptr,
                                                    DkDialog::fileDialogOptions());

    // user canceled?
    if (filePath.isEmpty())
        return;

    // try copying setting
    // NOTE: unfortunately this won't copy ini files on unix
    bool copied = false;
    QFile f(DkSettingsManager::instance().param().settingsPath());
    if (f.exists())
        copied = f.copy(filePath);

    // save settings (here we lose settings such as [CustomShortcuts])
    if (!copied) {
        QSettings settings(filePath, QSettings::IniFormat);
        DkSettingsManager::instance().settings().save(settings, true);
    }

    emit infoSignal(tr("Settings exported"));
}

void DkGeneralPreference::onLanguageComboCurrentIndexChanged(int index) const
{
    if (index >= 0 && index < mLanguages.size()) {
        QString language = mLanguages[index];

        if (DkSettingsManager::param().global().language != language) {
            DkSettingsManager::param().global().language = language;
            showRestartLabel();
        }
    }
}

void DkGeneralPreference::paintEvent(QPaintEvent *event)
{
    // fixes stylesheets which are not applied to custom widgets
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}

// DkDisplaySettings --------------------------------------------------------------------
DkDisplayPreference::DkDisplayPreference(QWidget *parent)
    : DkWidget(parent)
{
    createLayout();
}

void DkDisplayPreference::createLayout()
{
    // zoom settings
    QCheckBox *invertZoom = new QCheckBox(tr("Invert mouse wheel behaviour for zooming"), this);
    invertZoom->setToolTip(tr("If checked, the mouse wheel behaviour is inverted while zooming."));
    invertZoom->setChecked(DkSettingsManager::param().display().invertZoom);
    connect(invertZoom, &QCheckBox::toggled, this, &DkDisplayPreference::onInvertZoomToggled);

    // zoom settings
    QCheckBox *hQAntiAliasing = new QCheckBox(tr("Display Images with High Quality Anti Aliasing"), this);
    hQAntiAliasing->setToolTip(tr("NOTE: if checked, nomacs might be slow while zooming."));
    hQAntiAliasing->setChecked(DkSettingsManager::param().display().highQualityAntiAliasing);
    connect(hQAntiAliasing, &QCheckBox::toggled, this, &DkDisplayPreference::onHQAntiAliasingToggled);

    // show scollbars
    QCheckBox *showScrollBars = new QCheckBox(tr("Show Scrollbars when zooming into images"), this);
    showScrollBars->setToolTip(tr("If checked, scrollbars will appear that allow panning with the mouse."));
    showScrollBars->setChecked(DkSettingsManager::param().display().showScrollBars);
    connect(showScrollBars, &QCheckBox::toggled, this, &DkDisplayPreference::onShowScrollBarsToggled);

    QLabel *interpolationLabel = new QLabel(tr("Show pixels if zoom level is above"), this);

    QSpinBox *sbInterpolation = new QSpinBox(this);
    sbInterpolation->setToolTip(tr("nomacs will not interpolate images if the zoom level is larger."));
    sbInterpolation->setSuffix("%");
    sbInterpolation->setMinimum(0);
    sbInterpolation->setMaximum(10000);
    sbInterpolation->setValue(DkSettingsManager::param().display().interpolateZoomLevel);
    connect(sbInterpolation, QOverload<int>::of(&QSpinBox::valueChanged), this, &DkDisplayPreference::onInterpolationBoxValueChanged);

    // zoom levels
    DkZoomConfig &zc = DkZoomConfig::instance();
    QCheckBox *useZoomLevels = new QCheckBox(tr("Use Fixed Zoom Levels"), this);
    useZoomLevels->setToolTip(tr("If checked, predefined zoom levels are used when zooming."));
    useZoomLevels->setChecked(zc.useLevels());
    connect(useZoomLevels, &QCheckBox::toggled, this, &DkDisplayPreference::onUseZoomLevelsToggled);

    mZoomLevelsEdit = new QLineEdit(this);
    mZoomLevelsEdit->setText(zc.levelsToString());
    connect(mZoomLevelsEdit, &QLineEdit::editingFinished, this, &DkDisplayPreference::onZoomLevelsEditingFinished);

    QPushButton *zoomLevelsDefaults = new QPushButton(tr("Load Defaults"), this);
    zoomLevelsDefaults->setObjectName("zoomLevelsDefault");
    connect(zoomLevelsDefaults, &QPushButton::clicked, this, &DkDisplayPreference::onZoomLevelsDefaultClicked);

    mZoomLevels = new QWidget(this);

    QHBoxLayout *zll = new QHBoxLayout(mZoomLevels);
    zll->addWidget(mZoomLevelsEdit);
    zll->addWidget(zoomLevelsDefaults);

    mZoomLevels->setEnabled(zc.useLevels());

    DkGroupWidget *zoomGroup = new DkGroupWidget(tr("Zoom"), this);
    zoomGroup->addWidget(invertZoom);
    zoomGroup->addWidget(hQAntiAliasing);
    zoomGroup->addWidget(showScrollBars);
    zoomGroup->addWidget(interpolationLabel);
    zoomGroup->addWidget(sbInterpolation);
    zoomGroup->addWidget(useZoomLevels);
    zoomGroup->addWidget(mZoomLevels);

    // keep zoom radio buttons
    QVector<QRadioButton *> keepZoomButtons;
    keepZoomButtons.resize(DkSettings::zoom_end);
    keepZoomButtons[DkSettings::zoom_always_keep] = new QRadioButton(tr("Always keep zoom"), this);
    keepZoomButtons[DkSettings::zoom_keep_same_size] = new QRadioButton(tr("Keep zoom if the size is the same"), this);
    keepZoomButtons[DkSettings::zoom_keep_same_size]->setToolTip(
        tr("If checked, the zoom level is only kept, if the image loaded has the same level as the previous."));
    keepZoomButtons[DkSettings::zoom_never_keep] = new QRadioButton(tr("Never keep zoom"), this);

    QCheckBox *cbZoomToFit = new QCheckBox(tr("Always zoom to fit"), this);
    cbZoomToFit->setChecked(DkSettingsManager::param().display().zoomToFit);
    connect(cbZoomToFit, &QCheckBox::toggled, this, &DkDisplayPreference::onZoomToFitToggled);

    // check wrt the current settings
    keepZoomButtons[DkSettingsManager::param().display().keepZoom]->setChecked(true);

    QButtonGroup *keepZoomButtonGroup = new QButtonGroup(this);
    keepZoomButtonGroup->addButton(keepZoomButtons[DkSettings::zoom_always_keep], DkSettings::zoom_always_keep);
    keepZoomButtonGroup->addButton(keepZoomButtons[DkSettings::zoom_keep_same_size], DkSettings::zoom_keep_same_size);
    keepZoomButtonGroup->addButton(keepZoomButtons[DkSettings::zoom_never_keep], DkSettings::zoom_never_keep);
    connect(keepZoomButtonGroup, &QButtonGroup::idClicked, this, &DkDisplayPreference::onKeepZoomButtonClicked);

    DkGroupWidget *keepZoomGroup = new DkGroupWidget(tr("When Displaying New Images"), this);
    keepZoomGroup->addWidget(keepZoomButtons[DkSettings::zoom_always_keep]);
    keepZoomGroup->addWidget(keepZoomButtons[DkSettings::zoom_keep_same_size]);
    keepZoomGroup->addWidget(keepZoomButtons[DkSettings::zoom_never_keep]);
    keepZoomGroup->addWidget(cbZoomToFit);

    // icon size
    QSpinBox *sbIconSize = new QSpinBox(this);
    sbIconSize->setToolTip(tr("Define the icon size in pixel."));
    sbIconSize->setSuffix(" px");
    sbIconSize->setMinimum(16);
    sbIconSize->setMaximum(1024);
    sbIconSize->setValue(DkSettingsManager::param().effectiveIconSize(sbIconSize));
    connect(sbIconSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &DkDisplayPreference::onIconSizeBoxValueChanged);

    DkGroupWidget *iconGroup = new DkGroupWidget(tr("Icon Size"), this);
    iconGroup->addWidget(sbIconSize);

    // show navigation
    QCheckBox *cbShowNavigation = new QCheckBox(tr("Show Navigation Arrows"), this);
    cbShowNavigation->setToolTip(tr("If checked, navigation arrows will be displayed on top of the image"));
    cbShowNavigation->setChecked(DkSettingsManager::param().display().showNavigation);
    connect(cbShowNavigation, &QCheckBox::toggled, this, &DkDisplayPreference::onShowNavigationToggled);

    DkGroupWidget *navigationGroup = new DkGroupWidget(tr("Navigation"), this);
    navigationGroup->addWidget(cbShowNavigation);

    // slideshow
    QLabel *fadeImageLabel = new QLabel(tr("Image Transition"), this);

    QComboBox *cbTransition = new QComboBox(this);
    cbTransition->setView(new QListView()); // fix style
    cbTransition->setToolTip(tr("Choose a transition when loading a new image"));
    connect(cbTransition, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DkDisplayPreference::onTransitionCurrentIndexChanged);

    for (int idx = 0; idx < DkSettings::trans_end; idx++) {
        QString str = tr("Unknown Transition");

        switch (idx) {
        case DkSettings::trans_appear:
            str = tr("Appear");
            break;
        case DkSettings::trans_swipe:
            str = tr("Swipe");
            break;
        case DkSettings::trans_fade:
            str = tr("Fade");
            break;
        }

        cbTransition->addItem(str);
    }
    cbTransition->setCurrentIndex(DkSettingsManager::param().display().transition);

    QDoubleSpinBox *fadeImageBox = new QDoubleSpinBox(this);
    fadeImageBox->setToolTip(tr("Define the image transition speed."));
    fadeImageBox->setSuffix(" sec");
    fadeImageBox->setMinimum(0.0);
    fadeImageBox->setMaximum(3);
    fadeImageBox->setSingleStep(.2);
    fadeImageBox->setValue(DkSettingsManager::param().display().animationDuration);
    connect(fadeImageBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DkDisplayPreference::onFadeImageBoxValueChanged);

    QCheckBox *cbAlwaysAnimate = new QCheckBox(tr("Always Animate Image Loading"), this);
    cbAlwaysAnimate->setToolTip(tr("If unchecked, loading is only animated if nomacs is fullscreen"));
    cbAlwaysAnimate->setChecked(DkSettingsManager::param().display().alwaysAnimate);
    connect(cbAlwaysAnimate, &QCheckBox::toggled, this, &DkDisplayPreference::onAlwaysAnimateToggled);

    QLabel *displayTimeLabel = new QLabel(tr("Display Time"), this);

    QDoubleSpinBox *displayTimeBox = new QDoubleSpinBox(this);
    displayTimeBox->setToolTip(tr("Define the time an image is displayed."));
    displayTimeBox->setSuffix(" sec");
    displayTimeBox->setMinimum(0.0);
    displayTimeBox->setMaximum(30);
    displayTimeBox->setSingleStep(.2);
    displayTimeBox->setValue(DkSettingsManager::param().slideShow().time);
    connect(displayTimeBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DkDisplayPreference::onDisplayTimeBoxValueChanged);

    QCheckBox *showPlayer = new QCheckBox(tr("Show Player"), this);
    showPlayer->setChecked(DkSettingsManager::param().slideShow().showPlayer);
    connect(showPlayer, &QCheckBox::toggled, this, &DkDisplayPreference::onShowPlayerToggled);

    DkGroupWidget *slideshowGroup = new DkGroupWidget(tr("Slideshow"), this);
    slideshowGroup->addWidget(fadeImageLabel);
    slideshowGroup->addWidget(cbTransition);
    slideshowGroup->addWidget(fadeImageBox);
    slideshowGroup->addWidget(cbAlwaysAnimate);
    slideshowGroup->addWidget(displayTimeLabel);
    slideshowGroup->addWidget(displayTimeBox);
    slideshowGroup->addWidget(showPlayer);

    // show crop from metadata
    QCheckBox *showCrop = new QCheckBox(tr("Show crop rectangle"), this);
    showCrop->setChecked(DkSettingsManager::param().display().showCrop);
    connect(showCrop, &QCheckBox::toggled, this, &DkDisplayPreference::onShowCropToggled);

    DkGroupWidget *showCropGroup = new DkGroupWidget(tr("Show Metadata Cropping"), this);
    showCropGroup->addWidget(showCrop);

    // left column
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setAlignment(Qt::AlignTop);
    l->addWidget(zoomGroup);
    l->addWidget(keepZoomGroup);
    l->addWidget(iconGroup);
    l->addWidget(navigationGroup);
    l->addWidget(slideshowGroup);
    l->addWidget(showCropGroup);
}

void DkDisplayPreference::onInterpolationBoxValueChanged(int value) const
{
    if (DkSettingsManager::param().display().interpolateZoomLevel != value)
        DkSettingsManager::param().display().interpolateZoomLevel = value;
}

void DkDisplayPreference::onFadeImageBoxValueChanged(double value) const
{
    if (DkSettingsManager::param().display().animationDuration != value)
        DkSettingsManager::param().display().animationDuration = (float)value;
}

void DkDisplayPreference::onDisplayTimeBoxValueChanged(double value) const
{
    if (DkSettingsManager::param().slideShow().time != value)
        DkSettingsManager::param().slideShow().time = (float)value;
}

void DkDisplayPreference::onShowPlayerToggled(bool checked) const
{
    if (DkSettingsManager::param().slideShow().showPlayer != checked)
        DkSettingsManager::param().slideShow().showPlayer = checked;
}

void DkDisplayPreference::onIconSizeBoxValueChanged(int value) const
{
    if (DkSettingsManager::param().display().iconSize != value) {
        DkSettingsManager::param().display().iconSize = value;
        emit infoSignal(tr("Please Restart nomacs to apply changes"));
    }
}

void DkDisplayPreference::onKeepZoomButtonClicked(int buttonId) const
{
    if (DkSettingsManager::param().display().keepZoom != buttonId)
        DkSettingsManager::param().display().keepZoom = buttonId;
}

void DkDisplayPreference::onInvertZoomToggled(bool checked) const
{
    if (DkSettingsManager::param().display().invertZoom != checked)
        DkSettingsManager::param().display().invertZoom = checked;
}

void DkDisplayPreference::onHQAntiAliasingToggled(bool checked) const
{
    if (DkSettingsManager::param().display().highQualityAntiAliasing != checked)
        DkSettingsManager::param().display().highQualityAntiAliasing = checked;
}

void DkDisplayPreference::onZoomToFitToggled(bool checked) const
{
    if (DkSettingsManager::param().display().zoomToFit != checked)
        DkSettingsManager::param().display().zoomToFit = checked;
}

void DkDisplayPreference::onTransitionCurrentIndexChanged(int index) const
{
    if (DkSettingsManager::param().display().transition != index)
        DkSettingsManager::param().display().transition = (DkSettings::TransitionMode)index;
}

void DkDisplayPreference::onAlwaysAnimateToggled(bool checked) const
{
    if (DkSettingsManager::param().display().alwaysAnimate != checked)
        DkSettingsManager::param().display().alwaysAnimate = checked;
}

void DkDisplayPreference::onShowCropToggled(bool checked) const
{
    if (DkSettingsManager::param().display().showCrop != checked)
        DkSettingsManager::param().display().showCrop = checked;
}

void DkDisplayPreference::onShowScrollBarsToggled(bool checked) const
{
    if (DkSettingsManager::param().display().showScrollBars != checked)
        DkSettingsManager::param().display().showScrollBars = checked;
}

void DkDisplayPreference::onUseZoomLevelsToggled(bool checked) const
{
    DkZoomConfig::instance().setUseLevels(checked);
    mZoomLevels->setEnabled(checked);
}

void DkDisplayPreference::onShowNavigationToggled(bool checked) const
{
    if (DkSettingsManager::param().display().showNavigation != checked)
        DkSettingsManager::param().display().showNavigation = checked;
}

void DkDisplayPreference::onZoomLevelsEditingFinished() const
{
    DkZoomConfig &zc = DkZoomConfig::instance();
    if (!zc.setLevels(mZoomLevelsEdit->text()))
        mZoomLevelsEdit->setText(zc.levelsToString());
}

void DkDisplayPreference::onZoomLevelsDefaultClicked() const
{
    DkZoomConfig::instance().setLevelsToDefault();
    mZoomLevelsEdit->setText(DkZoomConfig::instance().levelsToString());
}

void DkDisplayPreference::paintEvent(QPaintEvent *event)
{
    // fixes stylesheets which are not applied to custom widgets
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}

// DkDummySettings --------------------------------------------------------------------
DkFilePreference::DkFilePreference(QWidget *parent)
    : DkWidget(parent)
{
    createLayout();
}

void DkFilePreference::createLayout()
{
    // temp folder
    DkDirectoryChooser *dirChooser = new DkDirectoryChooser(DkSettingsManager::param().global().tmpPath, this);
    connect(dirChooser, &DkDirectoryChooser::directoryChanged, this, &DkFilePreference::onDirChooserDirectoryChanged);

    QLabel *tLabel = new QLabel(tr("Screenshots are automatically saved to this folder"), this);

    DkGroupWidget *tempFolderGroup = new DkGroupWidget(tr("Use Temporary Folder"), this);
    tempFolderGroup->addWidget(dirChooser);
    tempFolderGroup->addWidget(tLabel);

    // cache size
    int maxRam = qMax(qRound(DkMemory::getTotalMemory()), 1024);
    qInfo() << "Available RAM: " << maxRam << "MB";
    QSpinBox *cacheBox = new QSpinBox(this);
    cacheBox->setMinimum(0);
    cacheBox->setMaximum(maxRam);
    cacheBox->setSuffix(" MB");
    cacheBox->setMaximumWidth(200);
    cacheBox->setValue(qRound(DkSettingsManager::param().resources().cacheMemory));
    connect(cacheBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &DkFilePreference::onCacheBoxValueChanged);

    QLabel *cLabel =
        new QLabel(tr("We recommend to set a moderate cache value around 100 MB. [%1-%2 MB]").arg(cacheBox->minimum()).arg(cacheBox->maximum()), this);

    DkGroupWidget *cacheGroup = new DkGroupWidget(tr("Maximal Cache Size"), this);
    cacheGroup->addWidget(cacheBox);
    cacheGroup->addWidget(cLabel);

    // history size
    // cache size
    QSpinBox *historyBox = new QSpinBox(this);
    historyBox->setMinimum(0);
    historyBox->setMaximum(1024);
    historyBox->setSuffix(" MB");
    historyBox->setMaximumWidth(200);
    historyBox->setValue(qRound(DkSettingsManager::param().resources().historyMemory));
    connect(historyBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &DkFilePreference::onHistoryBoxValueChanged);

    QLabel *hLabel =
        new QLabel(tr("We recommend to set a moderate edit history value around 100 MB. [%1-%2 MB]").arg(historyBox->minimum()).arg(historyBox->maximum()),
                   this);

    DkGroupWidget *historyGroup = new DkGroupWidget(tr("History Size"), this);
    historyGroup->addWidget(historyBox);
    historyGroup->addWidget(hLabel);

    // loading policy
    QVector<QRadioButton *> loadButtons;
    loadButtons.append(new QRadioButton(tr("Skip Images"), this));
    loadButtons[0]->setToolTip(tr("Images are skipped until the Next key is released"));
    loadButtons.append(new QRadioButton(tr("Wait for Images to be Loaded"), this));
    loadButtons[1]->setToolTip(tr("The next image is loaded after the current image is shown."));

    // check wrt the current settings
    loadButtons[0]->setChecked(!DkSettingsManager::param().resources().waitForLastImg);
    loadButtons[1]->setChecked(DkSettingsManager::param().resources().waitForLastImg);

    QButtonGroup *loadButtonGroup = new QButtonGroup(this);
    loadButtonGroup->addButton(loadButtons[0], 0);
    loadButtonGroup->addButton(loadButtons[1], 1);
    connect(loadButtonGroup, &QButtonGroup::idClicked, this, &DkFilePreference::onLoadGroupButtonClicked);

    DkGroupWidget *loadGroup = new DkGroupWidget(tr("Image Loading Policy"), this);
    loadGroup->addWidget(loadButtons[0]);
    loadGroup->addWidget(loadButtons[1]);

    // save policy
    QVector<QRadioButton *> saveButtons;
    saveButtons.append(new QRadioButton(tr("Load Saved Images"), this));
    saveButtons[0]->setToolTip(tr("After saving, the saved image will be loaded in place"));
    saveButtons.append(new QRadioButton(tr("Load to Tab"), this));
    saveButtons[1]->setToolTip(tr("After saving, the saved image will be loaded to a tab."));
    saveButtons.append(new QRadioButton(tr("Do Nothing"), this));
    saveButtons[2]->setToolTip(tr("The saved image will not be loaded."));

    // check wrt the current settings
    saveButtons[DkSettingsManager::param().resources().loadSavedImage]->setChecked(true);

    QButtonGroup *saveButtonGroup = new QButtonGroup(this);
    connect(saveButtonGroup, &QButtonGroup::idClicked, this, &DkFilePreference::onSaveGroupButtonClicked);

    DkGroupWidget *saveGroup = new DkGroupWidget(tr("Image Saving Policy"), this);

    for (int idx = 0; idx < saveButtons.size(); idx++) {
        saveButtonGroup->addButton(saveButtons[idx], idx);
        saveGroup->addWidget(saveButtons[idx]);
    }

    // skip images
    QSpinBox *skipBox = new QSpinBox(this);
    skipBox->setMinimum(2);
    skipBox->setMaximum(1000);
    skipBox->setValue(DkSettingsManager::param().global().skipImgs);
    skipBox->setMaximumWidth(200);
    connect(skipBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &DkFilePreference::onSkipBoxValueChanged);

    DkGroupWidget *skipGroup = new DkGroupWidget(tr("Number of Skipped Images on PgUp/PgDown"), this);
    skipGroup->addWidget(skipBox);

    // left column
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setAlignment(Qt::AlignTop);
    l->addWidget(tempFolderGroup);
    l->addWidget(cacheGroup);
    l->addWidget(historyGroup);
    l->addWidget(loadGroup);
    l->addWidget(saveGroup);
    l->addWidget(skipGroup);
}

void DkFilePreference::onDirChooserDirectoryChanged(const QString &dirPath) const
{
    bool dirExists = QDir(dirPath).exists();

    if (DkSettingsManager::param().global().tmpPath != dirPath && dirExists)
        DkSettingsManager::param().global().tmpPath = dirPath;
    else if (!dirExists)
        DkSettingsManager::param().global().tmpPath = "";
}

void DkFilePreference::onLoadGroupButtonClicked(int buttonId) const
{
    if (DkSettingsManager::param().resources().waitForLastImg != (buttonId == 1))
        DkSettingsManager::param().resources().waitForLastImg = (buttonId == 1);
}

void DkFilePreference::onSaveGroupButtonClicked(int buttonId) const
{
    if (DkSettingsManager::param().resources().loadSavedImage != buttonId)
        DkSettingsManager::param().resources().loadSavedImage = buttonId;
}

void DkFilePreference::onSkipBoxValueChanged(int value) const
{
    if (DkSettingsManager::param().global().skipImgs != value) {
        DkSettingsManager::param().global().skipImgs = value;
    }
}

void DkFilePreference::onCacheBoxValueChanged(int value) const
{
    if (DkSettingsManager::param().resources().cacheMemory != value) {
        DkSettingsManager::param().resources().cacheMemory = (float)value;
    }
}

void DkFilePreference::onHistoryBoxValueChanged(int value) const
{
    if (DkSettingsManager::param().resources().historyMemory != value) {
        DkSettingsManager::param().resources().historyMemory = (float)value;
    }
}

void DkFilePreference::paintEvent(QPaintEvent *event)
{
    // fixes stylesheets which are not applied to custom widgets
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}

// DkFileAssocationsSettings --------------------------------------------------------------------
DkFileAssociationsPreference::DkFileAssociationsPreference(QWidget *parent)
    : DkWidget(parent)
{
    createLayout();
}

DkFileAssociationsPreference::~DkFileAssociationsPreference()
{
    // save settings
    if (mSaveSettings) {
        writeSettings();
        mSaveSettings = false;
        DkSettingsManager::param().save();
    }
}

void DkFileAssociationsPreference::createLayout()
{
    QStringList fileFilters = DkSettingsManager::param().app().openFilters;

    mModel = new QStandardItemModel(this);
    for (int rIdx = 1; rIdx < fileFilters.size(); rIdx++)
        mModel->appendRow(getItems(fileFilters.at(rIdx),
                                   checkFilter(fileFilters.at(rIdx), DkSettingsManager::param().app().browseFilters),
                                   checkFilter(fileFilters.at(rIdx), DkSettingsManager::param().app().registerFilters)));

    mModel->setHeaderData(0, Qt::Horizontal, tr("Filter"));
    mModel->setHeaderData(1, Qt::Horizontal, tr("Browse"));
    mModel->setHeaderData(2, Qt::Horizontal, tr("Register"));
    connect(mModel, &QStandardItemModel::itemChanged, this, &DkFileAssociationsPreference::onFileModelItemChanged);

    QTableView *filterTableView = new QTableView(this);
    filterTableView->setModel(mModel);
    filterTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    filterTableView->verticalHeader()->hide();
    filterTableView->setShowGrid(false);
    filterTableView->resizeColumnsToContents();
    filterTableView->resizeRowsToContents();
    filterTableView->setWordWrap(false);

    // now the final widgets
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->addWidget(filterTableView);

#ifdef Q_OS_WIN

    QPushButton *assocFiles = new QPushButton(tr("Set File Associations"), this);
    connect(assocFiles, &QPushButton::clicked, this, &DkFileAssociationsPreference::onAssociateFilesClicked);

    QPushButton *openDefault = new QPushButton(tr("Set as Default Viewer"), this);
    connect(openDefault, &QPushButton::clicked, this, &DkFileAssociationsPreference::onOpenDefaultClicked);

    QWidget *bw = new QWidget(this);
    QHBoxLayout *l = new QHBoxLayout(bw);
    l->setAlignment(Qt::AlignLeft);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(openDefault);
    l->addWidget(assocFiles);

    layout->addWidget(bw);
#endif
}

void DkFileAssociationsPreference::onFileModelItemChanged(QStandardItem *)
{
    mSaveSettings = true;
    emit infoSignal(tr("Please Restart nomacs to apply changes"));
}

void DkFileAssociationsPreference::onOpenDefaultClicked() const
{
    DkFileFilterHandling fh;
    fh.showDefaultSoftware();
}

void DkFileAssociationsPreference::onAssociateFilesClicked()
{
    mSaveSettings = true;
    emit infoSignal(tr("Please Restart nomacs to apply changes"));
}

bool DkFileAssociationsPreference::checkFilter(const QString &cFilter, const QStringList &filters) const
{
    if (filters.empty() && (DkSettingsManager::param().app().containerFilters.contains(cFilter) || cFilter.contains("ico")))
        return false;

    if (filters.empty())
        return true;

    for (int idx = 0; idx < filters.size(); idx++)
        if (cFilter.contains(filters[idx]))
            return true;

    return filters.indexOf(cFilter) != -1;
}

QList<QStandardItem *> DkFileAssociationsPreference::getItems(const QString &filter, bool browse, bool reg)
{
    QList<QStandardItem *> items;
    QStandardItem *item = new QStandardItem(filter);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    items.append(item);
    item = new QStandardItem("");
    // item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
    item->setCheckable(true);
    item->setCheckState(browse ? Qt::Checked : Qt::Unchecked);
    items.append(item);
    item = new QStandardItem("");
    // item->setFlags(Qt::Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
    item->setCheckable(true);
    item->setCheckState(reg ? Qt::Checked : Qt::Unchecked);
#ifndef Q_OS_WIN // registering is windows only
    item->setEnabled(false);
#endif
    items.append(item);

    return items;
}

void DkFileAssociationsPreference::writeSettings() const
{
    DkFileFilterHandling fh;
    DkSettingsManager::param().app().browseFilters.clear();
    DkSettingsManager::param().app().registerFilters.clear();

    for (int idx = 0; idx < mModel->rowCount(); idx++) {
        QStandardItem *item = mModel->item(idx, 0);

        if (!item)
            continue;

        QStandardItem *browseItem = mModel->item(idx, 1);
        QStandardItem *regItem = mModel->item(idx, 2);

        if (browseItem && browseItem->checkState() == Qt::Checked) {
            QString cFilter = item->text();
            cFilter = cFilter.section(QRegularExpression("(\\(|\\))"), 1);
            cFilter = cFilter.replace(")", "");

            DkSettingsManager::param().app().browseFilters += cFilter.split(" ");
        }

        fh.registerFileType(item->text(), tr("Image"), regItem->checkState() == Qt::Checked);

        if (regItem->checkState() == Qt::Checked) {
            DkSettingsManager::param().app().registerFilters.append(item->text());
            qDebug() << item->text() << " registered";
        } else
            qDebug() << item->text() << " unregistered";
    }

    fh.registerNomacs(); // register nomacs again - to be save
}

void DkFileAssociationsPreference::paintEvent(QPaintEvent *event)
{
    // fixes stylesheets which are not applied to custom widgets
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}

// DkAdvancedSettings --------------------------------------------------------------------
DkAdvancedPreference::DkAdvancedPreference(QWidget *parent)
    : DkWidget(parent)
{
    createLayout();
}

void DkAdvancedPreference::createLayout()
{
    // load RAW radio buttons
    QVector<QRadioButton *> loadRawButtons;
    loadRawButtons.resize(DkSettings::raw_thumb_end);
    loadRawButtons[DkSettings::raw_thumb_always] = new QRadioButton(tr("Always Load JPG if Embedded"), this);
    loadRawButtons[DkSettings::raw_thumb_if_large] = new QRadioButton(tr("Load JPG if it Fits the Screen Resolution"), this);
    loadRawButtons[DkSettings::raw_thumb_never] = new QRadioButton(tr("Always Load RAW Data"), this);

    // check wrt the current settings
    loadRawButtons[DkSettingsManager::param().resources().loadRawThumb]->setChecked(true);

    QButtonGroup *loadRawButtonGroup = new QButtonGroup(this);
    loadRawButtonGroup->addButton(loadRawButtons[DkSettings::raw_thumb_always], DkSettings::raw_thumb_always);
    loadRawButtonGroup->addButton(loadRawButtons[DkSettings::raw_thumb_if_large], DkSettings::raw_thumb_if_large);
    loadRawButtonGroup->addButton(loadRawButtons[DkSettings::raw_thumb_never], DkSettings::raw_thumb_never);
    connect(loadRawButtonGroup, &QButtonGroup::idClicked, this, &DkAdvancedPreference::onLoadRawButtonClicked);

    QCheckBox *cbFilterRaw = new QCheckBox(tr("Apply Noise Filtering to RAW Images"), this);
    cbFilterRaw->setToolTip(tr("If checked, a noise filter is applied which reduced color noise"));
    cbFilterRaw->setChecked(DkSettingsManager::param().resources().filterRawImages);
    connect(cbFilterRaw, &QCheckBox::toggled, this, &DkAdvancedPreference::onFilterRawToggled);

    DkGroupWidget *loadRawGroup = new DkGroupWidget(tr("RAW Loader Settings"), this);
    loadRawGroup->addWidget(loadRawButtons[DkSettings::raw_thumb_always]);
    loadRawGroup->addWidget(loadRawButtons[DkSettings::raw_thumb_if_large]);
    loadRawGroup->addWidget(loadRawButtons[DkSettings::raw_thumb_never]);
    loadRawGroup->addSpace();
    loadRawGroup->addWidget(cbFilterRaw);

    // file loading
    QCheckBox *cbSaveDeleted = new QCheckBox(tr("Ask to Save Deleted Files"), this);
    cbSaveDeleted->setToolTip(tr("If checked, nomacs asks to save files which were deleted by other applications"));
    cbSaveDeleted->setChecked(DkSettingsManager::param().global().askToSaveDeletedFiles);
    connect(cbSaveDeleted, &QCheckBox::toggled, this, &DkAdvancedPreference::onSaveDeletedToggled);

    QCheckBox *cbIgnoreExif = new QCheckBox(tr("Ignore Exif Orientation when Loading"), this);
    cbIgnoreExif->setToolTip(tr("If checked, images are NOT rotated with respect to their Exif orientation"));
    cbIgnoreExif->setChecked(DkSettingsManager::param().metaData().ignoreExifOrientation);
    connect(cbIgnoreExif, &QCheckBox::toggled, this, &DkAdvancedPreference::onIgnoreExifToggled);

    QCheckBox *cbSaveExif = new QCheckBox(tr("Save Exif Orientation"), this);
    cbSaveExif->setToolTip(tr("If checked, orientation is written to the Exif rather than rotating the image Matrix\n")
                           + tr("NOTE: this allows for rotating JPGs without losing information."));
    cbSaveExif->setChecked(DkSettingsManager::param().metaData().saveExifOrientation);
    connect(cbSaveExif, &QCheckBox::toggled, this, &DkAdvancedPreference::onSaveExifToggled);

    DkGroupWidget *loadFileGroup = new DkGroupWidget(tr("File Loading/Saving"), this);
    loadFileGroup->addWidget(cbSaveDeleted);
    loadFileGroup->addWidget(cbIgnoreExif);
    loadFileGroup->addWidget(cbSaveExif);

    // batch processing
    QSpinBox *sbNumThreads = new QSpinBox(this);
    sbNumThreads->setObjectName("numThreads");
    sbNumThreads->setToolTip(tr("Choose the number of Threads in the thread pool"));
    sbNumThreads->setMinimum(1);
    sbNumThreads->setMaximum(100);
    sbNumThreads->setValue(DkSettingsManager::param().global().numThreads);
    connect(sbNumThreads, QOverload<int>::of(&QSpinBox::valueChanged), this, &DkAdvancedPreference::onNumThreadsValueChanged);

    DkGroupWidget *threadsGroup = new DkGroupWidget(tr("Number of Threads"), this);
    threadsGroup->addWidget(sbNumThreads);

    // native dialogs
    QCheckBox *cbNative = new QCheckBox(tr("Enable Native File Dialogs"), this);
    cbNative->setToolTip(tr("If checked, native system dialogs are used for opening/saving files."));
    cbNative->setChecked(DkSettingsManager::param().resources().nativeDialog);
    connect(cbNative, &QCheckBox::toggled, this, &DkAdvancedPreference::onUseNativeToggled);

    DkGroupWidget *nativeGroup = new DkGroupWidget(tr("Native Dialogs"), this);
    nativeGroup->addWidget(cbNative);

    // log
    QCheckBox *cbUseLog = new QCheckBox(tr("Use Log File"), this);
    cbUseLog->setToolTip(tr("If checked, a log file will be created."));
    cbUseLog->setChecked(DkSettingsManager::param().app().useLogFile);
    connect(cbUseLog, &QCheckBox::toggled, this, &DkAdvancedPreference::onUseLogToggled);

    QPushButton *pbLog = new QPushButton(tr("Open Log"), this);
    pbLog->setObjectName("logFolder");
    connect(pbLog, &QPushButton::clicked, this, &DkAdvancedPreference::onLogFolderClicked);
#ifdef Q_OS_WIN
    pbLog->setVisible(DkSettingsManager::param().app().useLogFile);
#else
    pbLog->setVisible(false);
#endif

    DkGroupWidget *useLogGroup = new DkGroupWidget(tr("Logging"), this);
    useLogGroup->addWidget(cbUseLog);
    useLogGroup->addWidget(pbLog);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->addWidget(loadRawGroup);
    layout->addWidget(loadFileGroup);
    layout->addWidget(threadsGroup);
    layout->addWidget(nativeGroup);
    layout->addWidget(useLogGroup);
}

void DkAdvancedPreference::onLoadRawButtonClicked(int buttonId) const
{
    if (DkSettingsManager::param().resources().loadRawThumb != buttonId)
        DkSettingsManager::param().resources().loadRawThumb = buttonId;
}

void DkAdvancedPreference::onFilterRawToggled(bool checked) const
{
    if (DkSettingsManager::param().resources().filterRawImages != checked)
        DkSettingsManager::param().resources().filterRawImages = checked;
}

void DkAdvancedPreference::onSaveDeletedToggled(bool checked) const
{
    if (DkSettingsManager::param().global().askToSaveDeletedFiles != checked)
        DkSettingsManager::param().global().askToSaveDeletedFiles = checked;
}

void DkAdvancedPreference::onIgnoreExifToggled(bool checked) const
{
    if (DkSettingsManager::param().metaData().ignoreExifOrientation != checked)
        DkSettingsManager::param().metaData().ignoreExifOrientation = checked;
}

void DkAdvancedPreference::onSaveExifToggled(bool checked) const
{
    if (DkSettingsManager::param().metaData().saveExifOrientation != checked)
        DkSettingsManager::param().metaData().saveExifOrientation = checked;
}

void DkAdvancedPreference::onUseLogToggled(bool checked) const
{
    if (DkSettingsManager::param().app().useLogFile != checked) {
        DkSettingsManager::param().app().useLogFile = checked;
        emit infoSignal(tr("Please Restart nomacs to apply changes"));
    }
}

void DkAdvancedPreference::onUseNativeToggled(bool checked) const
{
    if (DkSettingsManager::param().resources().nativeDialog != checked) {
        DkSettingsManager::param().resources().nativeDialog = checked;
    }
}

void DkAdvancedPreference::onLogFolderClicked() const
{
    // TODO: add linux/mac os
#ifdef _WIN32
    QString logPath = QDir::toNativeSeparators(DkUtils::getLogFilePath());
    QProcess::startDetached(QString("explorer /select, \"%1\"").arg(logPath));
#endif
}

void DkAdvancedPreference::onNumThreadsValueChanged(int val) const
{
    if (DkSettingsManager::param().global().numThreads != val)
        DkSettingsManager::param().setNumThreads(val);
}

void DkAdvancedPreference::paintEvent(QPaintEvent *event)
{
    // fixes stylesheets which are not applied to custom widgets
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}

// DkEditorPreference --------------------------------------------------------------------
DkEditorPreference::DkEditorPreference(QWidget *parent)
    : DkWidget(parent)
{
    createLayout();
}

void DkEditorPreference::createLayout()
{
    mSettingsWidget = new DkSettingsWidget(this);
    mSettingsWidget->setSettingsPath(DkSettingsManager::param().settingsPath());

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignLeft);

    layout->addWidget(mSettingsWidget);

    connect(mSettingsWidget, &DkSettingsWidget::changeSettingSignal, this, &DkEditorPreference::changeSetting);
    connect(mSettingsWidget, &DkSettingsWidget::removeSettingSignal, this, &DkEditorPreference::removeSetting);
}
void DkEditorPreference::paintEvent(QPaintEvent *event)
{
    // fixes stylesheets which are not applied to custom widgets
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}

void DkEditorPreference::changeSetting(const QString &key, const QVariant &value, const QStringList &groups) const
{
    DefaultSettings settings;
    DkSettingsWidget::changeSetting(settings, key, value, groups);

    // update values
    nmc::DkSettingsManager::instance().param().load();
}

void DkEditorPreference::removeSetting(const QString &key, const QStringList &groups) const
{
    DefaultSettings settings;
    DkSettingsWidget::removeSetting(settings, key, groups);
}

//// DkDummySettings --------------------------------------------------------------------
// DkDummyPreference::DkDummyPreference(QWidget* parent) : QWidget(parent) {
//
//	createLayout();
//	QMetaObject::connectSlotsByName(this);
// }
//
// void DkDummyPreference::createLayout() {

// QHBoxLayout* layout = new QHBoxLayout(this);
// layout->setAlignment(Qt::AlignLeft);

// layout->addWidget(leftWidget);

//}
// void DkDummyPreference::paintEvent(QPaintEvent *event) {
//
//	// fixes stylesheets which are not applied to custom widgets
//	QStyleOption opt;
//	opt.init(this);
//	QPainter p(this);
//	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
//
//	QWidget::paintEvent(event);
//}

}
