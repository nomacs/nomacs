#include "DkActionManager.h"
#include "DkDialog.h"
#include "DkPluginManager.h"
#include "DkSettings.h"
#include "DkVersion.h"

#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QRectF>
#include <gtest/gtest.h>

using namespace nmc;

static void initApplication()
{
    // stand up QApplication compatible with headless CI environment
    int argc = 3;
    static char *argv[3];
    argv[0] = const_cast<char *>("nomacs-test");
    argv[1] = const_cast<char *>("-platform");
    argv[2] = const_cast<char *>("offscreen");

    // static is fine here since each test runs as separate process, and
    // that is required as there is global state destructor won't clean up
    static QApplication app(argc, argv);

    // these are needed for nomacs to find its settings file, we use
    // different values from release to isolate from development environment
    QCoreApplication::setOrganizationName("nomacs-test");
    QCoreApplication::setOrganizationDomain("https://test.nomacs.org");
    QCoreApplication::setApplicationName("ImageLounge-test");
    QCoreApplication::setApplicationVersion(NOMACS_VERSION_STR);
}

TEST(ShortcutsModel, CheckState)
{
    initApplication();

    DkSettingsManager::instance().init();
    DkSettingsManager::instance().param().setToDefaultSettings();

    QMainWindow mainWindow;

    DkActionManager &am = DkActionManager::instance();
    am.createActions(&mainWindow);
    am.createMenus(mainWindow.menuWidget());

#ifdef WITH_PLUGINS
    DkPluginActionManager *pm = am.pluginActionManager();
    QMenu pluginMenu(mainWindow.menuWidget());
    pm->setMenu(&pluginMenu);
    pm->updateMenu();

    // if this fails QT_PLUGIN_PATH needs to be <...>/nomacs-plugins
    EXPECT_TRUE(pm->pluginActions().count() > 0);
#endif

    DkShortcutsModel model;
    EXPECT_TRUE(model.checkState());
}
