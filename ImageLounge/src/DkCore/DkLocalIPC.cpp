#include "DkLocalIPC.h"

#include "DkCentralWidget.h"

#include <QWindow>

#ifdef WITH_KDSINGLEAPPLICATION
#include "kdsingleapplication.h"
#endif

#ifdef WITH_DBUS
#include <QtDBus/QtDBus>
#endif

namespace nmc
{

#ifdef WITH_KDSINGLEAPPLICATION

class DkLocalSocketIPC : public QObject, public DkLocalIPC
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DkLocalSocketIPC)

public:
    DkLocalSocketIPC()
    {
        if (mSocket.isPrimaryInstance()) {
            connect(&mSocket, &KDSingleApplication::messageReceived, this, &DkLocalSocketIPC::messageReceived);
        }
    }
    ~DkLocalSocketIPC() override = default;

private:
    bool isFirstInstance() const override
    {
        return mSocket.isPrimaryInstance();
    }

    void messageReceived(const QByteArray &msgData)
    {
        Q_ASSERT(isFirstInstance());

        // TODO: validate the message, invalid data will segfault
        // in QDataStream::operator>>
        QByteArray encoded = msgData;
        QDataStream data(&encoded, QIODevice::ReadOnly);
        QStringList msg;
        data >> msg;
        qInfo() << "[local-socket]: msg received:" << msg;
        if (msg.empty()) {
            qWarning() << "[local-socket]: ignoring empty message";
            return;
        }
        if (!mCentralWidget) {
            qWarning() << "[local-socket]: no registered central widget";
            return;
        }
        if (msg[0] == "activate" && msg.length() == 2) {
            const QString &token = msg[1];
            qInfo() << "[local-socket] activate:" << token;

            // TODO: we need to know if we are setting wayland or X11 token
            qputenv("XDG_ACTIVATION_TOKEN", token.toLatin1());

            QWidget *top = mCentralWidget->topLevelWidget();
            top->show();

            QWindow *window = top->windowHandle();
            if (window) {
                window->requestActivate();
            }

        } else if (msg[0] == "load" && msg.length() == 2) {
            mCentralWidget->load(msg[1]);
        } else if (msg[0] == "loadToTab" && msg.length() == 2) {
            mCentralWidget->loadToTab(msg[1]);
        } else {
            qWarning() << "[local-socket]: invalid/unknown message" << msg;
        }
    }

    void setCentralWidget(DkCentralWidget *centralWidget) override
    {
        Q_ASSERT(isFirstInstance());
        mCentralWidget = centralWidget;
    }

    void sendMessage(const QStringList &msg)
    {
        Q_ASSERT(!isFirstInstance());
        QByteArray encoded;
        QDataStream ds(&encoded, QIODevice::WriteOnly);
        ds << msg;
        mSocket.sendMessage(encoded);
    }

    void activate() override
    {
        QStringList msg;
        // TODO: on X11 send DESKTOP_STARTUP_ID
        msg << "activate" << qEnvironmentVariable("XDG_APPLICATION_TOKEN");
        sendMessage(msg);
    }

    void load(const QString &path) override
    {
        QStringList msg;
        msg << "load" << path;
        sendMessage(msg);
    }

    void loadToTab(const QString &path) override
    {
        QStringList msg;
        msg << "loadToTab" << path;
        sendMessage(msg);
    }

    KDSingleApplication mSocket{};
    DkCentralWidget *mCentralWidget{};
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
    void activate(const QString &token)
    {
        qInfo() << "[dbus] xdg token received:" << token;

        // TODO: we need to know if we are setting wayland or X11 token
        qputenv("XDG_ACTIVATION_TOKEN", token.toLatin1());

        QWidget *top = mCentralWidget->topLevelWidget();
        top->show();

        QWindow *window = top->windowHandle();
        if (window) {
            window->requestActivate();
        }
    }
    void load(const QString &path)
    {
        mCentralWidget->load(path);
    }
    void loadToTab(const QString &path)
    {
        mCentralWidget->load(path);
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
        // TODO: on X11 send DESKTOP_STARTUP_ID
        sendMessage("activate", {qEnvironmentVariable("XDG_ACTIVATION_TOKEN")});
    }

    void load(const QString &path) override
    {
        sendMessage("load", {path});
    }

    void loadToTab(const QString &path) override
    {
        sendMessage("loadToTab", {path});
    }

private:
    QDBusConnection mDbus;
    bool mConnected{};
    bool mFirstInstance{};
    QObject *mRootObject{};
};
#endif // WITH_DBUS

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
