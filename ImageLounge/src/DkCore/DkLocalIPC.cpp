#include "DkLocalIPC.h"

#include "DkCentralWidget.h"

#include <QElapsedTimer>
#include <QGuiApplication>
#include <QWindow>

#if QT_CONFIG(xcb)
#include <QAbstractNativeEventFilter>
#include <xcb/xcb.h>
#endif

#ifdef WITH_KDSINGLEAPPLICATION
#include "kdsingleapplication.h"
#include <QJsonArray>
#include <QJsonObject>
#endif

#ifdef WITH_DBUS
#include <QtDBus/QtDBus>
#endif

namespace nmc
{
//
// When a file is opened from the file manager, we would like to raise the nomacs window
// to show the new image. Unfortunately, most OS implement focus-stealing prevention
// to stop this from happening.
//
// - On macOS the raise works since the open file is a special IPC event sent to the application
// - On Windows, there is no established way to bypass this, some hacks may be possible.
// - On Wayland and Xorg, we need to send a token from the new nomacs process to the first one.
//
// On Wayland, we just get/set XDG_ACTIVATION_TOKEN in the environment. Qt takes care
// of the rest.
//
// On X11, not so simple.
// 1. The parent process and child (file manager and nomacs) must support xdg startup notifications.
// 2. The child process must set the .desktop key StartupNotify=true
// 3. The parent process creates the token and puts it in DESKTOP_STARTUP_ID environment, then forks child
// 4. The parent process shows a spinning cursor or indicator that app is launching
// 5. In the child, Qt stores the token, then *clears it from the environment*
// 6. When Qt connects to X-server, it copies the token to the xproperty _NET_STARTUP_ID
// 7. When Qt shows the window, the window manager verifies the token and allows the window to be raised
// 8. After the *first window is shown*, Qt sends the startup completion message to window manager and clears the token
// 9. The parent process changes cursor back to normal pointer
//
// Due to #5 we have DkLocalIPC::initialize() to fetch that from environment before Qt
// Due to #6 we have setXcbStartupId() which makes the necessary calls
// Due to #8 we have sendXcbStartupRemoveMessage()
//
// Qt5 had QX11Info/qtx11extras.h in the public API, which would help with this.
// But that is now moved to private API in Qt6 so I elected to
// do the xcb calls directly rather than risk ABI break in the future.
//
#if QT_CONFIG(xcb)

// watches for a PropertyNotify on a specific atom/window
// so we can wait for a property change to complete without blocking the XCB connection from Qt
class XcbPropertyEventFilter : public QAbstractNativeEventFilter
{
public:
    XcbPropertyEventFilter(xcb_window_t window, xcb_atom_t atom)
        : mWindow(window)
        , mAtom(atom)
    {
    }

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override
    {
        if (eventType != "xcb_generic_event_t") {
            return false;
        }

        auto *event = static_cast<xcb_generic_event_t *>(message);
        if ((event->response_type & ~0x80) == XCB_PROPERTY_NOTIFY) {
            auto *propertyEvent = reinterpret_cast<xcb_property_notify_event_t *>(event);
            if (propertyEvent->window == mWindow && propertyEvent->atom == mAtom) {
                mTimestamp = propertyEvent->time;
                mCaptured = true;
                qInfo() << "xcb property changed" << mTimestamp;
            }
        }

        // never consume the event so Qt can update it's state
        return false;
    }

    bool captured() const
    {
        return mCaptured;
    }

    xcb_timestamp_t timestamp() const
    {
        return mTimestamp;
    }

private:
    xcb_window_t mWindow;
    xcb_atom_t mAtom;
    bool mCaptured = false;
    xcb_timestamp_t mTimestamp = XCB_CURRENT_TIME;
};

// helper to get xcb_atom_t from its name
static xcb_atom_t xcbAtomFromName(xcb_connection_t *connection, const char *name)
{
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, 0, strlen(name), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, nullptr);
    xcb_atom_t atom = reply ? reply->atom : static_cast<xcb_atom_t>(XCB_ATOM_NONE);
    free(reply);
    return atom;
}

// set _NET_STARTUP_ID on the window so wm allows the raise to occur
static void setXcbStartupId(QWindow *mainWindow, const QByteArray &startupId)
{
    auto *x11App = qApp->nativeInterface<QNativeInterface::QX11Application>();
    if (!x11App) {
        return;
    }
    xcb_connection_t *conn = x11App->connection();

    if (!mainWindow) {
        return;
    }
    auto window = static_cast<xcb_window_t>(mainWindow->winId());

    // The window manager will reject the startup id if the timestamp is too
    // far from the window's last user interaction timestamp. We can create
    // an artificial interaction by modifying a property on the window, then
    // wait for the new timestamp to arrive to prevent a race.
    // We use Qt's event loop so we don't drop any xcb events Qt might need
    xcb_atom_t timestampAtom = xcbAtomFromName(conn, "_QT_SCROLL_DONE");

    if (timestampAtom != XCB_ATOM_NONE) {
        XcbPropertyEventFilter filter(window, timestampAtom);
        qApp->installNativeEventFilter(&filter);

        xcb_change_property(conn, //
                            XCB_PROP_MODE_APPEND,
                            window,
                            timestampAtom,
                            XCB_ATOM_INTEGER,
                            32,
                            0,
                            nullptr);
        xcb_flush(conn);

        // wait for the event to arrive, but not too long so we can't hang the app
        QElapsedTimer timer;
        timer.start();
        while (!filter.captured() && timer.elapsed() < 500) {
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 50);
        }

        qApp->removeNativeEventFilter(&filter);
    }

    // set the _NET_STARTUP_ID property on the main window
    // even if we failed previous call, this might still work depending on how strict the window manager is
    xcb_atom_t startupIdAtom = xcbAtomFromName(conn, "_NET_STARTUP_ID");

    if (startupIdAtom != XCB_ATOM_NONE) {
        xcb_change_property(conn,
                            XCB_PROP_MODE_REPLACE,
                            window,
                            startupIdAtom,
                            XCB_ATOM_STRING,
                            8,
                            startupId.size(),
                            startupId.constData());
        xcb_flush(conn);
    }
}

// send the startup remove message to x-server,
// this stops any status indicator in the source (taskbar, file manager, etc)
static void sendXcbStartupRemoveMessage(const QByteArray &startupId)
{
    auto *x11App = qApp->nativeInterface<QNativeInterface::QX11Application>();
    if (!x11App) {
        return;
    }
    xcb_connection_t *conn = x11App->connection();

    xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
    xcb_window_t rootWindow = screen->root;

    xcb_atom_t beginAtom = xcbAtomFromName(conn, "_NET_STARTUP_INFO_BEGIN");
    xcb_atom_t infoAtom = xcbAtomFromName(conn, "_NET_STARTUP_INFO");
    if (beginAtom == XCB_ATOM_NONE || infoAtom == XCB_ATOM_NONE) {
        return;
    }

    QByteArray message = QByteArrayLiteral("remove: ID=") + startupId;

    // xcb has fixed-size messages so we chunk into multiple xcb_send_event
    xcb_client_message_event_t ev{};
    ev.response_type = XCB_CLIENT_MESSAGE;
    ev.format = 8;
    ev.type = beginAtom;
    ev.window = rootWindow;

    int sent = 0;
    int length = message.length() + 1; // include NUL
    const char *data = message.constData();
    do {
        if (sent == 20) {
            ev.type = infoAtom;
        }

        int numBytes = qMin(length - sent, 20);
        memset(ev.data.data8, 0, 20);
        memcpy(ev.data.data8, data + sent, numBytes);
        xcb_send_event(conn, //
                       false,
                       rootWindow,
                       XCB_EVENT_MASK_PROPERTY_CHANGE,
                       reinterpret_cast<const char *>(&ev));
        sent += numBytes;
    } while (sent < length);

    xcb_flush(conn);
}

#endif // QT_CONFIG(xcb)

// stores the initial value of getenv(DESKTOP_STARTUP_ID) when nomacs starts
static QByteArray &xcbStartupId()
{
    static QByteArray startupId;
    return startupId;
}

static QByteArray getWindowActivationToken()
{
    const QString platform = QGuiApplication::platformName();
    if (platform == "wayland") {
        return qgetenv("XDG_ACTIVATION_TOKEN");
    } else if (platform == "xcb") {
#if QT_CONFIG(xcb)
        // we can't return DESKTOP_STARTUP_ID because Qt clears it from environment!
        return xcbStartupId();
#endif
    }
    return {};
}

static void setWindowActivationToken(QWindow *window, const QByteArray &token)
{
    if (token.isEmpty()) {
        return;
    }

    const QString platform = QGuiApplication::platformName();
    if (platform == "wayland") {
        Q_UNUSED(window)
        qputenv("XDG_ACTIVATION_TOKEN", token);
    } else if (platform == "xcb") {
#if QT_CONFIG(xcb)
        setXcbStartupId(window, token);
#endif
    }
}

#ifdef WITH_KDSINGLEAPPLICATION

class DkLocalSocketIPC : public QObject, public DkLocalIPC
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DkLocalSocketIPC)

public:
    DkLocalSocketIPC()
    {
        mSocket = std::make_unique<KDSingleApplication>();
        if (mSocket->isPrimaryInstance()) {
            connect(mSocket.get(), &KDSingleApplication::messageReceived, this, &DkLocalSocketIPC::messageReceived);
        }
    }
    ~DkLocalSocketIPC() override = default;

private:
    void waitFirstInstance() override
    {
        mSocket->disconnect();

        qInfo() << "[local-socket] waiting for first instance to exit";
        QDeadlineTimer timer(5000); // plenty of time as nothing can stop exit after forking new process
        while (!mSocket->isPrimaryInstance() && !timer.hasExpired()) {
            QThread::msleep(100);
            mSocket = std::make_unique<KDSingleApplication>();
        }
        if (mSocket->isPrimaryInstance()) {
            connect(mSocket.get(), &KDSingleApplication::messageReceived, this, &DkLocalSocketIPC::messageReceived);
        } else {
            qWarning() << "[local-socket] timed out waiting for first instance";
        }
    }

    bool isFirstInstance() const override
    {
        return mSocket->isPrimaryInstance();
    }

    void messageReceived(const QByteArray &msgData)
    {
        Q_ASSERT(isFirstInstance());

        QJsonParseError error;
        QJsonObject obj = QJsonDocument::fromJson(msgData, &error).object();
        if (obj.empty()) {
            qInfo() << "[local-socket] invalid JSON:" << error.offset << error.errorString();
            return;
        }
        qInfo() << "[local-socket] message received:" << obj;

        const int version = obj.value("version").toInt();
        const QString method = obj.value("method").toString();
        QList<QVariant> args = obj.value("params").toArray().toVariantList();

        if (version != kIpcVersion) {
            qWarning() << "[local-socket] unsupported message version, expected" << kIpcVersion;
            return;
        }

        if (!mCentralWidget) {
            qWarning() << "[local-socket] no registered central widget";
            return;
        }

        if (method == "activate" && args.length() == 1) {
            const QByteArray token = args[0].toByteArray();
            qInfo() << "[local-socket] activation token received:" << token;

            QWidget *top = mCentralWidget->topLevelWidget();
            top->show();

            QWindow *window = top->windowHandle();
            if (window) {
                setWindowActivationToken(window, token);
                window->requestActivate();
            }

        } else if (method == "loadUnique" && args.length() == 2) {
            QString path = args[0].toString();
            bool newTab = args[1].toBool();
            mCentralWidget->loadUnique(path, newTab);
        } else {
            qWarning() << "[local-socket] invalid/unknown message" << args;
        }
    }

    void setCentralWidget(DkCentralWidget *centralWidget) override
    {
        Q_ASSERT(isFirstInstance());
        mCentralWidget = centralWidget;
    }

    void sendMessage(const QString &method, const QList<QVariant> &args)
    {
        Q_ASSERT(!isFirstInstance());
        QJsonObject obj;
        obj["version"] = kIpcVersion;
        obj["id"] = QCoreApplication::applicationPid();
        obj["method"] = method;
        obj["params"] = QJsonArray::fromVariantList(args);
        QByteArray encoded = QJsonDocument{obj}.toJson();
        mSocket->sendMessage(encoded);
    }

    void activate() override
    {
        QByteArray token = getWindowActivationToken();

        qInfo() << "[local-socket] forwarding activation token:" << token;
        sendMessage("activate", {token});

        // send the startup id remove message to stop cursor spinning in file managers
#if QT_CONFIG(xcb)
        if (!token.isEmpty() && QGuiApplication::platformName() == "xcb") {
            sendXcbStartupRemoveMessage(token);
        }
#endif
    }

    void loadUnique(const QString &path, bool newTab) override
    {
        sendMessage("loadUnique", {path, newTab});
    }

    std::unique_ptr<KDSingleApplication> mSocket{};
    DkCentralWidget *mCentralWidget{};
    const int kIpcVersion{1};
};

#endif // WITH_KDSINGLEAPPLICATION

#ifdef WITH_DBUS

class DkDBusAdapter : QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.nomacs.ImageLounge.main")
public:
    DkDBusAdapter(DkCentralWidget *cw, QObject *parent)
        : QDBusAbstractAdaptor(parent)
        , mCentralWidget(cw)
    {
    }

public Q_SLOTS:
    void activate(const QByteArray &token)
    {
        qInfo() << "[dbus] activation token received:" << token;

        QWidget *top = mCentralWidget->topLevelWidget();
        top->show();

        QWindow *window = top->windowHandle();
        if (window) {
            setWindowActivationToken(window, token);
            window->requestActivate();
        }
    }
    void loadUnique(const QString &path, bool loadToTab)
    {
        mCentralWidget->loadUnique(path, loadToTab);
    }

private:
    DkCentralWidget *mCentralWidget;
};

class DkDBusIPC : public DkLocalIPC
{
public:
    DkDBusIPC()
        : mDbus(QDBusConnection::sessionBus())
    {
        mConnected = mDbus.isConnected();
        if (!mConnected) {
            qWarning() << "[dbus] no connection:" << mDbus.lastError();
        } else {
            mFirstInstance = mDbus.registerService("org.nomacs.ImageLounge");
        }
    }

    void waitFirstInstance() override
    {
        qInfo() << "[dbus] waiting for first instance to exit";
        QDeadlineTimer timer(5000);
        while (!mFirstInstance && !timer.hasExpired()) {
            QThread::msleep(100);
            mFirstInstance = mDbus.registerService("org.nomacs.ImageLounge");
        }
        if (!mFirstInstance) {
            qWarning() << "[dbus] timed out waiting for first instance";
        }
    }

    bool isFirstInstance() const override
    {
        return mFirstInstance;
    }

    void setCentralWidget(DkCentralWidget *widget) override
    {
        Q_ASSERT(isFirstInstance());
        if (!mRootObject) {
            mRootObject = new QObject;
            (void)new DkDBusAdapter(widget, mRootObject);
            if (!mDbus.registerObject("/", mRootObject)) {
                qWarning() << "[dbus] failed to register object" << mDbus.lastError();
            }
        }
    }

    void sendMessage(const char *name, const QList<QVariant> &args)
    {
        Q_ASSERT(!isFirstInstance());

        if (!mConnected) {
            qWarning() << "[dbus] sendMessage: no connection";
            return;
        }

        auto msg = QDBusMessage::createMethodCall("org.nomacs.ImageLounge", "/", "org.nomacs.ImageLounge.main", name);
        msg.setArguments(args);
        msg = mDbus.call(msg);

        if (msg.type() == QDBusMessage::ErrorMessage) {
            qWarning() << "[dbus] sendMessage: error:" << msg.errorName() << msg.errorMessage();
        }
    }

    void activate() override
    {
        QByteArray token = getWindowActivationToken();
        qInfo() << "[dbus] forwarding activation token:" << token;
        sendMessage("activate", {token});

        // send the startup id remove message to stop cursor spinning in file managers
#if QT_CONFIG(xcb)
        if (!token.isEmpty() && QGuiApplication::platformName() == "xcb") {
            sendXcbStartupRemoveMessage(token);
        }
#endif
    }

    void loadUnique(const QString &path, bool loadToTab) override
    {
        sendMessage("loadUnique", {path, loadToTab});
    }

private:
    QDBusConnection mDbus;
    bool mConnected{};
    bool mFirstInstance{};
    QObject *mRootObject{};
};
#endif // WITH_DBUS

void DkLocalIPC::initialize()
{
    if (qApp) {
        qFatal("[ipc] initialize() called after QApplication()");
    }
    xcbStartupId() = qgetenv("DESKTOP_STARTUP_ID");
}

DkLocalIPC &DkLocalIPC::instance()
{
#if defined(WITH_KDSINGLEAPPLICATION)
    static DkLocalSocketIPC ipc;
#elif defined(WITH_DBUS)
    static DkDBusIPC ipc;
#else
#error No IPC implementation configured
#endif
    return ipc;
}
}

#include "DkLocalIPC.moc"
