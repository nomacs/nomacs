/*******************************************************************************************************
 main.cpp
 Created on:	21.04.2011

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

#ifdef Q_OS_WIN
#include "shlwapi.h"
#pragma comment(lib, "shlwapi.lib")

#if defined(_MSC_BUILD) && !defined(DK_INSTALL) // only final release will be compiled without a CMD
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#else
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#endif
#endif

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QImageReader>
#include <QMessageBox>
#include <QObject>
#include <QTextStream>
#include <QThread>
#include <QTranslator>

#include "DkCentralWidget.h"
#include "DkNoMacs.h"
#include "DkPluginManager.h"
#include "DkPong.h"
#include "DkProcess.h"
#include "DkSettings.h"
#include "DkShortcuts.h"
#include "DkThemeManager.h"
#include "DkTimer.h"
#include "DkUtils.h"
#include "DkVersion.h"
#include "DkViewPort.h"

#ifdef _MSC_BUILD
#include <shlobj.h>
#endif

#ifdef _MSC_BUILD
int main(int argc, wchar_t *argv[])
{
#else
int main(int argc, char *argv[])
{
#endif

    QCoreApplication::setOrganizationName("nomacs");
    QCoreApplication::setOrganizationDomain("https://nomacs.org");
    QCoreApplication::setApplicationName("Image Lounge");
    QCoreApplication::setApplicationVersion(NOMACS_VERSION_STR);
#ifdef Q_OS_MAC
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus, true);
#endif

    QApplication app(argc, (char **)argv);

#ifdef Q_OS_LINUX
    app.setDesktopFileName("org.nomacs.ImageLounge");
#endif

    QImageReader::setAllocationLimit(2048);

    // init settings
    nmc::DkSettingsManager::instance().init();
    nmc::DkMetaDataHelper::initialize(); // this line makes the XmpParser thread-save - so don't delete it even if you
                                         // seem to know what you do

    // uncomment this for the single instance feature...
    //// check for single instance
    // nmc::DkRunGuard guard;
    //
    // if (!guard.tryRunning()) {
    //	qDebug() << "nomacs is already running - quitting...";
    //	return 0;
    // }

    // CMD parser --------------------------------------------------------------------
    QCommandLineParser parser;

    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("[File|Directory...]", QObject::tr("List of files and/or directories to open"));

    // fullscreen (-f)
    QCommandLineOption fullScreenOpt(QStringList() << "f" << "fullscreen", QObject::tr("Start in fullscreen."));
    parser.addOption(fullScreenOpt);

    QCommandLineOption slideshowOpt(QStringList() << "slideshow", QObject::tr("Start slideshow playback"));
    parser.addOption(slideshowOpt);

    QCommandLineOption pongOpt(QStringList() << "pong", QObject::tr("Start Pong."));
    parser.addOption(pongOpt);

    QCommandLineOption privateOpt(QStringList() << "p" << "private", QObject::tr("Start in private mode."));
    parser.addOption(privateOpt);

    QCommandLineOption modeOpt(QStringList() << "m" << "mode",
                               QObject::tr("Set the viewing mode <mode>."),
                               QObject::tr("default | frameless | pseudocolor"));
    parser.addOption(modeOpt);

    QCommandLineOption batchOpt(QStringList() << "batch",
                                QObject::tr("Batch processing of <batch-settings.pnm>."),
                                QObject::tr("batch-settings-path"));
    parser.addOption(batchOpt);

    QCommandLineOption batchLogOpt(QStringList() << "batch-log",
                                   QObject::tr("Saves batch log to <log-path.txt>."),
                                   QObject::tr("log-path.txt"));
    parser.addOption(batchLogOpt);

    QCommandLineOption importSettingsOpt(QStringList() << "import-settings",
                                         QObject::tr("Imports the settings from <settings-path.ini> and saves them."),
                                         QObject::tr("settings-path.ini"));
    parser.addOption(importSettingsOpt);

    QCommandLineOption registerFilesOpt(QStringList() << "register-files",
                                        QObject::tr("Register file associations (Windows only)."));
    parser.addOption(registerFilesOpt);

    QCommandLineOption listFormatsOpt(QStringList("list-formats"),
                                      QObject::tr("List available image formats"),
                                      "csv | plist | xdg");
    listFormatsOpt.setFlags(QCommandLineOption::ShortOptionStyle);
    parser.addOption(listFormatsOpt);

    QCommandLineOption aboutOpt(QStringList("about"), QObject::tr("Print build information"));
    parser.addOption(aboutOpt);

    parser.process(app);

    // CMD parser --------------------------------------------------------------------
    nmc::DkPluginManager::createPluginsPath();

    // compute batch process
    if (!parser.value(batchOpt).isEmpty()) {
        QString logPath;
        if (!parser.value(batchLogOpt).isEmpty())
            logPath = parser.value(batchLogOpt);

        QString batchSettingsPath = parser.value(batchOpt);
        nmc::DkBatchProcessing::computeBatch(batchSettingsPath, logPath);

        return 0;
    }

    bool noUI = false;

    // apply default settings
    if (!parser.value(importSettingsOpt).isEmpty()) {
        QString settingsPath = parser.value(importSettingsOpt);
        nmc::DkSettingsManager::importSettings(settingsPath);
        noUI = true;
    }

    // apply default settings
    if (parser.isSet(registerFilesOpt)) {
        nmc::DkFileFilterHandling::registerFileAssociations();
        noUI = true;
    }

    // extract file type information for builds/documentation/etc
    if (parser.isSet(listFormatsOpt)) {
        noUI = true;
        const QString outputFormat = parser.value(listFormatsOpt);
        QFile file;
        if (parser.positionalArguments().count()) {
            file.setFileName(parser.positionalArguments().first());
            file.open(QFile::WriteOnly | QFile::Truncate);
            qInfo() << "[list-formats] writing metadata to" << file.fileName();
        } else
            file.open(stdout, QFile::WriteOnly);

        if (!file.isOpen()) {
            qCritical() << "open failed:" << file.errorString();
            return 1;
        }
        QTextStream out(&file);
        nmc::DkFileFilterHandling::printFormats(out, outputFormat);
    }

    if (parser.isSet(aboutOpt)) {
        noUI = true;
        // attempts to load all runtime dependencies without showing the UI,
        // this is used on macOS and cross-compiled windows version
        // to discover what dylibs/dlls are being used
        QTextStream out(stdout);
        out << nmc::DkUtils::getBuildInfo();
        out << "qt-imageformats:" << QImageReader::supportedImageFormats().join(",") << "\n";

        nmc::DkPluginManager::instance().loadPlugins();
        for (auto &plugin : nmc::DkPluginManager::instance().getPlugins())
            out << "plugin:" << plugin->pluginPath() << "\n";
    }

    if (noUI)
        return 0;

    // install translations
    const QString translationName = "nomacs_" + nmc::DkSettingsManager::param().global().language + ".qm";
    const QString translationNameQt = "qt_" + nmc::DkSettingsManager::param().global().language + ".qm";

    QTranslator translator;
    nmc::DkSettingsManager::param().loadTranslation(translationName, translator);
    app.installTranslator(&translator);

    QTranslator translatorQt;
    nmc::DkSettingsManager::param().loadTranslation(translationNameQt, translatorQt);
    app.installTranslator(&translatorQt);

    nmc::DkNoMacs *w = nullptr;

    if (parser.isSet(privateOpt))
        nmc::DkSettingsManager::param().app().privateMode = true;

    int mode = nmc::DkSettingsManager::param().app().appMode;
    qInfo() << "loaded app mode:" << mode;

    if (parser.isSet(modeOpt)) {
        QString pm = parser.value(modeOpt);

        if (pm == "default")
            mode = nmc::DkSettings::mode_default;
        else if (pm == "frameless")
            mode = nmc::DkSettings::mode_frameless;
        else if (pm == "pseudocolor")
            mode = nmc::DkSettings::mode_contrast;
        else
            qWarning() << "illegal mode: " << pm << "use either <default>, <frameless> or <pseudocolor>";
    }

    if (parser.isSet(fullScreenOpt))
        mode = nmc::DkSettings::fullscreenMode(mode);

    if (!nmc::DkSettings::modeIsValid(mode)) {
        qWarning() << "invalid mode:" << mode;
        mode = nmc::DkSettings::mode_default;
    }

    // TODO: currentAppMode is obsolete now; use appMode instead
    // currentAppMode is not saved/restored by settings but must be set early
    nmc::DkSettingsManager::param().app().currentAppMode = mode;

    nmc::DkTimer dt;

    // bring up theme before any widgets
    nmc::DkThemeManager::instance().applyTheme();

    // initialize nomacs
    const int modeType = nmc::DkSettings::normalMode(mode);
    if (modeType == nmc::DkSettings::mode_frameless) {
        w = new nmc::DkNoMacsFrameless();
        qDebug() << "this is the frameless nomacs...";
    } else if (modeType == nmc::DkSettings::mode_contrast) {
        w = new nmc::DkNoMacsContrast();
        qDebug() << "this is the contrast nomacs...";
    } else if (parser.isSet(pongOpt)) {
        auto pong = std::make_unique<nmc::DkPong>();
        return app.exec();
    } else {
        w = new nmc::DkNoMacsIpl();
    }

    qInfo() << "init window: appMode:" << nmc::DkSettingsManager::param().app().currentAppMode
            << "maximized:" << w->isMaximized() << "fullscreen:" << w->isFullScreen() << "geometry:" << w->geometry()
            << "windowState:" << w->windowState();

    if (nmc::DkSettings::modeIsFullscreen(mode))
        w->showFullScreen();
    else if (w->windowState() & Qt::WindowMaximized)
        w->showMaximized();
    else
        w->showNormal();

    const bool maximized = w->isMaximized(); // check if show* actually maximized us

    qInfo() << "show window: appMode:" << nmc::DkSettingsManager::param().app().currentAppMode
            << "maximized:" << w->isMaximized() << "fullscreen:" << w->isFullScreen() << "geometry:" << w->geometry()
            << "windowState:" << w->windowState();

    while (!w->isActiveWindow() && dt.elapsed() < 5000) {
        qDebug() << "waiting for active window";
        QThread::msleep(10);
        qApp->processEvents();
    }

    qInfo() << "active window: appMode:" << nmc::DkSettingsManager::param().app().currentAppMode
            << "maximized:" << w->isMaximized() << "fullscreen:" << w->isFullScreen() << "geometry:" << w->geometry()
            << "windowState:" << w->windowState();

    qInfo() << "window font:" << w->font();
    qInfo() << "system font:" << qApp->font();

    // Qt emulates showMaximized() on some platforms (X11), so it might not work.
    // If we try again with a visible window, it *could* work correctly (GNOME)
    if (maximized && !w->isMaximized())
        w->showMaximized();

    if (w)
        w->onWindowLoaded();

    qInfo() << "Initialization takes: " << dt;

    nmc::DkCentralWidget *cw = w->getTabWidget();

    // Handle slideshow with file list support
    if (parser.isSet(slideshowOpt)) {
        QStringList slideshowFiles = parser.positionalArguments();

        if (!slideshowFiles.isEmpty()) {
            // Expand directories into file lists and collect all image files
            QStringList allImageFiles;
            for (const QString &path : slideshowFiles) {
                nmc::DkFileInfo fileInfo(path);
                if (fileInfo.isDir()) {
                    // Directory: expand to image files
                    QDir dir(path);
                    QStringList filters = nmc::DkSettingsManager::param().app().fileFilters;
                    QStringList imageFiles = dir.entryList(filters, QDir::Files, QDir::Name);
                    for (const QString &imageFile : imageFiles) {
                        allImageFiles.append(dir.filePath(imageFile));
                    }
                } else {
                    // Single file: add directly
                    allImageFiles.append(path);
                }
            }

            if (!allImageFiles.isEmpty()) {
                // First ensure we have a proper viewport and tab setup by loading the first file normally
                cw->load(allImageFiles[0]);
                // Then start slideshow with the expanded file list
                cw->startSlideshowWithFiles(allImageFiles);
            } else {
                // No image files found
                qWarning() << "No image files found in the provided paths";
            }
        } else {
            // Directory mode: use current directory for slideshow
            cw->startSlideshow();
        }
    } else {
        // Normal mode: load files into tabs
        bool loading = false;

        for (auto &filePath : parser.positionalArguments()) {
            if (filePath.isEmpty())
                continue;

            if (loading)
                cw->loadToTab(filePath);
            else
                cw->load(filePath);

            loading = true;
        }

        // load recent files if there is nothing to display
        if (!loading && nmc::DkSettingsManager::param().app().showRecentFiles)
            w->showRecentFilesOnStartUp();
    }

    if (w->isFullScreen())
        w->enterFullScreen();

    if (cw->hasViewPort())
        cw->getViewPort()->setFocus(Qt::TabFocusReason);

// since Qt5 only Q_OS_MACOS is defined, see https://doc.qt.io/qt-5/macos-issues.html#compile-time-flags
#ifdef Q_OS_MACOS
    nmc::DkNomacsOSXEventFilter *osxEventFilter = new nmc::DkNomacsOSXEventFilter();
    app.installEventFilter(osxEventFilter);
    QObject::connect(osxEventFilter, &nmc::DkNomacsOSXEventFilter::loadFile, w, &nmc::DkNoMacs::loadFile);
#endif

    app.installEventFilter(nmc::DkShortcutEventFilter::instance());

    int rVal = -1;
    try {
        rVal = app.exec();
    } catch (const std::bad_alloc &) {
        QMessageBox::critical(nullptr,
                              QObject::tr("Critical Error"),
                              QObject::tr("Sorry, nomacs ran out of memory..."),
                              QMessageBox::Ok);
    }

    // restore message handler, workaround for: https://github.com/nomacs/nomacs/issues/874
    qInstallMessageHandler(nullptr);

    if (w)
        delete w; // we need delete so that settings are saved (from destructors)

    return rVal;
}
