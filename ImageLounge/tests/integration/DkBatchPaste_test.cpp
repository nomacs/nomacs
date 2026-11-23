#include "DkBatch.h"
#include "DkSettings.h"
#include "DkTestFixture.h"
#include "DkThumbs.h"

#include <QClipboard>
#include <QFile>
#include <QImage>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMimeData>
#include <QSettings>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QUrl>

class BatchPasteTest : public AppTest
{
protected:
    void SetUp() override
    {
        AppTest::SetUp();
        ASSERT_TRUE(QDir().mkpath(qEnvironmentVariable("TMPDIR")));
        nmc::DkSettingsManager::param().initFileFilters();
    }
};

TEST_F(BatchPasteTest, BatchPasteAcceptsOnlyExistingLocalUrlsOnce)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath(QString::fromUtf8("synthetic 图 #1.png"));
    QImage image(2, 2, QImage::Format_RGB32);
    image.fill(Qt::red);
    ASSERT_TRUE(image.save(path));

    nmc::DkInputTextEdit edit;
    auto *mime = new QMimeData;
    mime->setUrls({QUrl::fromLocalFile(path),
                   QUrl::fromLocalFile(path),
                   QUrl::fromLocalFile(dir.filePath("missing.png")),
                   QUrl("https://example.invalid/remote.png")});
    QApplication::clipboard()->setMimeData(mime);
    edit.paste();
    EXPECT_EQ(edit.getFileList(), QStringList({path}));
    edit.paste();
    EXPECT_EQ(edit.getFileList(), QStringList({path}));
}

TEST_F(BatchPasteTest, BatchPasteParsesMixedPlainTextPaths)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString first = dir.filePath(QString::fromUtf8("synthetic 图 #1.png"));
    const QString second = dir.filePath("second image.png");
    const QString otherFile = dir.filePath("notes.txt");
    QFile other(otherFile);
    ASSERT_TRUE(other.open(QIODevice::WriteOnly));
    other.write("synthetic");
    other.close();
    QImage image(2, 2, QImage::Format_RGB32);
    image.fill(Qt::red);
    ASSERT_TRUE(image.save(first));
    ASSERT_TRUE(image.save(second));

    nmc::DkInputTextEdit edit;
    auto *mime = new QMimeData;
    mime->setText(QString("\"%1\"\r\n%2\n%3\nhttps://example.invalid/remote.png\n%1\n%4")
                      .arg(first, QUrl::fromLocalFile(second).toString(), dir.filePath("missing.png"), otherFile));
    QApplication::clipboard()->setMimeData(mime);
    edit.paste();
    EXPECT_EQ(edit.getFileList(), QStringList({first, second, otherFile}));
}

TEST_F(BatchPasteTest, BatchPastePrefersUrlsOverTheirTextRepresentation)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString first = dir.filePath("first.png");
    const QString second = dir.filePath("second.png");
    QImage image(2, 2, QImage::Format_RGB32);
    image.fill(Qt::red);
    ASSERT_TRUE(image.save(first));
    ASSERT_TRUE(image.save(second));

    nmc::DkInputTextEdit edit;
    auto *mime = new QMimeData;
    mime->setUrls({QUrl::fromLocalFile(first)});
    mime->setText(second);
    QApplication::clipboard()->setMimeData(mime);
    edit.paste();
    EXPECT_EQ(edit.getFileList(), QStringList({first}));
}

TEST_F(BatchPasteTest, BatchInputAcceptsClipboardThroughPasteShortcut)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QCoreApplication::setOrganizationName("nomacs-batch-paste-test");
    QCoreApplication::setApplicationName("nomacs-batch-paste-test");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, dir.path());
    const QString path = dir.filePath(QString::fromUtf8("synthetic 图 #1.png"));
    QImage image(2, 2, QImage::Format_RGB32);
    image.fill(Qt::red);
    ASSERT_TRUE(image.save(path));

    nmc::DkSettingsManager::instance().init();
    QMainWindow window;
    window.show();
    nmc::DkActionManager &actions = nmc::DkActionManager::instance();
    actions.createActions(&window);
    actions.createMenus(window.menuWidget());
    nmc::DkThumbLoader thumbLoader;
    nmc::DkBatchInput input(&thumbLoader, &window);
    nmc::DkInputTextEdit *edit = input.getInputEdit();
    ASSERT_NE(edit, nullptr);
    QTabWidget *tabs = input.findChild<QTabWidget *>();
    ASSERT_NE(tabs, nullptr);
    tabs->setCurrentWidget(edit);
    input.show();
    edit->setFocus();
    auto *mime = new QMimeData;
    mime->setUrls({QUrl::fromLocalFile(path),
                   QUrl::fromLocalFile(dir.filePath("missing.png")),
                   QUrl("https://example.invalid/remote.png")});
    QApplication::clipboard()->setMimeData(mime);
    const QList<QKeySequence> bindings = QKeySequence::keyBindings(QKeySequence::Paste);
    ASSERT_FALSE(bindings.isEmpty());
    const QKeyCombination shortcut = bindings.first()[0];
    QKeyEvent press(QEvent::KeyPress, shortcut.key(), shortcut.keyboardModifiers());
    QApplication::sendEvent(edit, &press);
    EXPECT_EQ(input.getSelectedFiles(), QStringList({path}));
}

class DropTestEdit : public nmc::DkInputTextEdit
{
public:
    using nmc::DkInputTextEdit::dropEvent;
};

TEST_F(BatchPasteTest, DropStillFiltersUnsupportedFiles)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString imagePath = dir.filePath("image.png");
    const QString textPath = dir.filePath("notes.txt");
    QImage image(2, 2, QImage::Format_RGB32);
    image.fill(Qt::red);
    ASSERT_TRUE(image.save(imagePath));
    QFile text(textPath);
    ASSERT_TRUE(text.open(QIODevice::WriteOnly));
    text.write("synthetic");
    text.close();

    QMimeData mime;
    mime.setUrls({QUrl::fromLocalFile(imagePath), QUrl::fromLocalFile(textPath)});
    QDropEvent event(QPointF(0, 0), Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier);
    DropTestEdit edit;
    edit.dropEvent(&event);
    EXPECT_EQ(edit.getFileList(), QStringList({imagePath}));
}
