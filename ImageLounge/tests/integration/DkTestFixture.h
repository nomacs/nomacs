#pragma once

#include "DkActionManager.h"
#include "DkPluginManager.h"
#include "DkVersion.h"

#include <QApplication>
#include <QDir>
#include <QMainWindow>
#include <QMenu>

#include <gtest/gtest.h>

// Minimal fixture for tests needing QSettings or writing other temporary files
class BaseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ::testing::Test::SetUp();

        // collect all test artifacts in a common location
        QString tmpDir = QDir::tempPath() + "/nomacs-test-artifacts/";
        qputenv("TEST_ARTIFACTS", tmpDir.toUtf8());

        // give each test its own temporary space that won't conflict with other tests
        const auto *testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
        tmpDir += QString{"/"} + testInfo->test_suite_name() + "_" + testInfo->name();

        // make anything that otherwise writes to /tmp use this instead (QTemporaryFile etc)
        qputenv("TMPDIR", tmpDir.toUtf8());
        qputenv("TMP", tmpDir.toUtf8());

        // must be empty so previous run can't effect it
        QDir dir{tmpDir};
        if (dir.exists() && !dir.removeRecursively()) {
            qFatal("failed to cleanup the test's tmpdir");
        }

        // separate config file space
        tmpDir += "/config";
        qputenv("TEST_CONFIG", tmpDir.toUtf8());

#ifndef Q_OS_UNIX
#error fixme
#else
        // set the location of ~/.config to this test's own directory
        // this captures nomacs settings file or other QSettings files
        qputenv("XDG_CONFIG_HOME", tmpDir.toUtf8());
#endif

        // these attributes set the config file name
        QCoreApplication::setOrganizationName("nomacs");
        QCoreApplication::setOrganizationDomain("https://nomacs.org");
        QCoreApplication::setApplicationName("ImageLounge");
        QCoreApplication::setApplicationVersion(NOMACS_VERSION_STR);
    }

    void TearDown() override
    {
        ::testing::Test::TearDown();
    }
};

// Fixture for tests needing QApplication
class AppTest : public BaseTest
{
private:
    int mArgc{3};
    char *mArgv[3]{};
    QApplication *mApp{};

protected:
    void SetUp() override
    {
        BaseTest::SetUp();

        // stand up QApplication compatible with headless CI environment
        // for this to work, the test runner must execute each test in a separate process
        static int called = 0;
        if (called > 0) {
            qFatal("only one QApplication is possible, this test needs to be in a separate binary");
        }
        called++;

        mArgv[0] = const_cast<char *>("nomacs-test");
        mArgv[1] = const_cast<char *>("-platform");
        mArgv[2] = const_cast<char *>("offscreen");

        mApp = new QApplication{mArgc, mArgv};
    }

    void TearDown() override
    {
        delete mApp;

        BaseTest::TearDown();
    }
};

// Fixture with DkSettingsManager, DkActionManger, and DkPluginManager initialized
class ActionsTest : public AppTest
{
private:
    QMainWindow *mWindow{};
    QMenu *mMenu{};

protected:
    void SetUp() override
    {
        AppTest::SetUp();

        nmc::DkSettingsManager::instance().init();

        mWindow = new QMainWindow{};

        auto &am = nmc::DkActionManager::instance();
        am.createActions(mWindow);
        am.createMenus(mWindow->menuWidget());

#ifdef WITH_PLUGINS
        nmc::DkPluginActionManager *pm = am.pluginActionManager();
        mMenu = new QMenu{mWindow->menuWidget()};
        pm->setMenu(mMenu);
        pm->updateMenu();
#endif
    }

    void TearDown() override
    {
        delete mMenu;
        delete mWindow;

        AppTest::TearDown();
    }
};
