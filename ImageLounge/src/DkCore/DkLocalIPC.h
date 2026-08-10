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
     * @brief wait until this process becomes the first instance (after restarting nomacs for example)
     *        should not be called unless we are certain the first instance is shutting down
     */
    virtual void waitFirstInstance() = 0;
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
     * @brief loadUnique open file/dir or switch to already open tab
     */
    virtual void loadUnique(const QString &path, bool newTab) = 0;

    virtual ~DkLocalIPC() = default;

protected:
    DkLocalIPC() = default;
};
}
