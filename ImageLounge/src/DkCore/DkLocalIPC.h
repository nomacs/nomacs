#pragma once

#include "nmc_config.h"

class QString;

namespace nmc
{
class DkCentralWidget;

/**
 * @brief Helper for new instance of nomacs to communicate
 * with the first started instance
 */
class DllCoreExport DkLocalIPC
{
public:
    /**
     * @brief create server or client instance; first nomacs process is the server
     */
    static DkLocalIPC &instance();
    /**
     * @brief tell if current process is the first instance or not
     */
    virtual bool isFirstInstance() const = 0;
    /**
     * @brief set the main widget for the first process to operate on
     */
    virtual void setCentralWidget(DkCentralWidget *widget) = 0;
    /**
     * @brief raise the main window of first nomacs process
     */
    virtual void activate() = 0;
    /**
     * @brief open file or dir in the first instance; replace current tab
     */
    virtual void load(const QString &path) = 0;
    /**
     * @brief open file or dir in the first instance; create a new tab
     */
    virtual void loadToTab(const QString &path) = 0;

    virtual ~DkLocalIPC() = default;

protected:
    DkLocalIPC() = default;
};
}
