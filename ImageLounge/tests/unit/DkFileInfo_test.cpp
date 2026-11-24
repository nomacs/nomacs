#include "DkFileInfo.h"

#include <QDir>
#include <QFile>
#include <QImage>
#include <QTemporaryDir>

#include <gtest/gtest.h>

using namespace nmc;

#ifdef WITH_QUAZIP
TEST(DkFileInfo, OpensOrdinaryFilesInDirectoriesEndingInHash)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QStringList directories = {"#middle", "#ending#", "#ending#/nested#"};
    QFile sibling(tempDir.filePath("#ending"));
    ASSERT_TRUE(sibling.open(QIODevice::WriteOnly));
    sibling.close();

    for (const QString &directory : directories) {
        ASSERT_TRUE(QDir(tempDir.path()).mkpath(directory));
        const QString path = tempDir.filePath(directory + "/cover.png");
        QImage image(1, 1, QImage::Format_RGB32);
        image.fill(Qt::red);
        ASSERT_TRUE(image.save(path));

        DkFileInfo info(path);
        EXPECT_FALSE(info.isFromZip()) << path.toStdString();
        std::unique_ptr<QIODevice> io = info.getIODevice();
        ASSERT_NE(io, nullptr) << path.toStdString();
        QImage loaded;
        ASSERT_TRUE(loaded.load(io.get(), "PNG"));
        EXPECT_EQ(loaded.pixelColor(0, 0), QColor(Qt::red));
    }
}

TEST(DkFileInfo, OpensZipMembersInDirectoriesEndingInHash)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    ASSERT_TRUE(QDir(tempDir.path()).mkpath("archive#"));
    QFile sibling(tempDir.filePath("archive"));
    ASSERT_TRUE(sibling.open(QIODevice::WriteOnly));
    sibling.close();
    const QString archivePath = tempDir.filePath("archive#/images.zip");
    // 1x1 red PNG inside the member "inside#/cover.png".
    const QByteArray archive = QByteArray::fromBase64(
        "UEsDBBQAAAAAABddeF1RD9oCRQAAAEUAAAARAAAAaW5zaWRlIy9jb3Zlci5wbmeJUE5HDQoaCgAAAA1JSERSAAAAAQAAAAEIAgAAAJB3U94AAA"
        "AMSURBVHicY/jPwAAAAwEBAMn+ku8AAAAASUVORK5CYIJQSwECFAMUAAAAAAAXXXhdUQ/"
        "aAkUAAABFAAAAEQAAAAAAAAAAAAAAgAEAAAAAaW5zaWRlIy9jb3Zlci5wbmdQSwUGAAAAAAEAAQA/AAAAdAAAAAAA");
    QFile file(archivePath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    ASSERT_EQ(file.write(archive), archive.size());
    file.close();

    const QString ordinaryArchivePath = tempDir.filePath("images.zip");
    ASSERT_TRUE(QFile::copy(archivePath, ordinaryArchivePath));
    for (const QString &path : {ordinaryArchivePath, archivePath}) {
        DkFileInfo info(path + "#/inside#/cover.png");
        EXPECT_TRUE(info.isFromZip());
        EXPECT_EQ(info.pathInZip(), "inside#/cover.png");
        std::unique_ptr<QIODevice> io = info.getIODevice();
        ASSERT_NE(io, nullptr);
        QImage loaded;
        ASSERT_TRUE(loaded.load(io.get(), "PNG"));
        EXPECT_EQ(loaded.pixelColor(0, 0), QColor(Qt::red));
    }
}
#endif
