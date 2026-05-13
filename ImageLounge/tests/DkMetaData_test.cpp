#include "DkFileInfo.h"
#include "DkMetaData.h"

#include <QImage>
#include <QTemporaryDir>

#include <gtest/gtest.h>

using namespace nmc;

static QString createEmptyMetadataPng(QTemporaryDir &tempDir)
{
    QImage img(16, 16, QImage::Format_RGB32);
    img.fill(Qt::white);

    const QString filePath = tempDir.filePath("empty-metadata.png");
    EXPECT_TRUE(img.save(filePath, "PNG"));

    return filePath;
}

TEST(DkMetadata, SetExifValueOnEmptyMetadata)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    const QString filePath = createEmptyMetadataPng(tempDir);

    {
        DkMetaDataT md;
        md.readMetaData(DkFileInfo{filePath});

        // allow set metadata when none is present
        EXPECT_FALSE(md.hasMetaData());
        EXPECT_FALSE(md.isDirty());
        EXPECT_TRUE(md.setExifValue("Exif.Image.ImageDescription", "qwe123"));
        EXPECT_TRUE(md.isDirty());
        EXPECT_EQ(md.getExifValue("ImageDescription"), "qwe123");

        bool force = false;
        EXPECT_TRUE(md.saveMetaData(DkFileInfo{filePath}, force));
    }

    {
        // check it is preserved
        DkMetaDataT md;
        md.readMetaData(DkFileInfo{filePath});
        EXPECT_TRUE(md.hasMetaData());
        EXPECT_FALSE(md.isDirty());
        EXPECT_EQ(md.getExifValue("ImageDescription"), "qwe123");

        // modify existing
        EXPECT_TRUE(md.setExifValue("Exif.Image.ImageDescription", "123qwe"));
        EXPECT_TRUE(md.isDirty());
        EXPECT_EQ(md.getExifValue("ImageDescription"), "123qwe");

        bool force = false;
        EXPECT_TRUE(md.saveMetaData(DkFileInfo{filePath}, force));
    }

    {
        // check it is preserved
        DkMetaDataT md;
        md.readMetaData(DkFileInfo{filePath});
        EXPECT_TRUE(md.hasMetaData());
        EXPECT_FALSE(md.isDirty());
        EXPECT_EQ(md.getExifValue("ImageDescription"), "123qwe");
    }
}

TEST(DkMetaData, RatingInitializesAndClearsEmptyMetadata)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    const QString filePath = createEmptyMetadataPng(tempDir);

    DkMetaDataT meta;
    meta.readMetaData(DkFileInfo(filePath));

    EXPECT_FALSE(meta.hasMetaData());
    EXPECT_FALSE(meta.isDirty());
    EXPECT_EQ(meta.getRating(), -1);

    EXPECT_TRUE(meta.setRating(5));
    EXPECT_TRUE(meta.hasMetaData());
    EXPECT_TRUE(meta.isDirty());
    EXPECT_EQ(meta.getRating(), 5);

    // Re-applying the same rating is a no-op success and must not disturb state.
    EXPECT_TRUE(meta.setRating(5));
    EXPECT_TRUE(meta.isDirty());
    EXPECT_EQ(meta.getRating(), 5);

    EXPECT_FALSE(meta.setRating(7));
    EXPECT_TRUE(meta.isDirty());
    EXPECT_EQ(meta.getRating(), 5);

    EXPECT_TRUE(meta.setRating(0));
    EXPECT_TRUE(meta.isDirty());
    EXPECT_EQ(meta.getRating(), -1);

    DkMetaDataT emptyMeta;
    emptyMeta.readMetaData(DkFileInfo(filePath));

    EXPECT_FALSE(emptyMeta.hasMetaData());
    EXPECT_FALSE(emptyMeta.isDirty());
    EXPECT_TRUE(emptyMeta.setRating(0));
    EXPECT_TRUE(emptyMeta.isDirty());
    EXPECT_EQ(emptyMeta.getRating(), -1);

    emptyMeta.readMetaData(DkFileInfo(filePath));
    EXPECT_FALSE(emptyMeta.hasMetaData());
    EXPECT_FALSE(emptyMeta.isDirty());
    EXPECT_FALSE(emptyMeta.setRating(7));
    EXPECT_FALSE(emptyMeta.isDirty());
    EXPECT_EQ(emptyMeta.getRating(), -1);
}

TEST(DkMetaData, ClearExifStateResetsDirtyFlagAfterRatingChange)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    const QString filePath = createEmptyMetadataPng(tempDir);

    DkMetaDataT meta;
    meta.readMetaData(DkFileInfo(filePath));

    ASSERT_TRUE(meta.setRating(3));
    ASSERT_TRUE(meta.isDirty());

    meta.clearExifState();

    EXPECT_FALSE(meta.isDirty());
    EXPECT_EQ(meta.getRating(), 3);

    EXPECT_TRUE(meta.setRating(0));
    EXPECT_TRUE(meta.isDirty());
    EXPECT_EQ(meta.getRating(), -1);
}
