#include "DkFileInfo.h"
#include "DkMetaData.h"

#include <QImage>
#include <QTemporaryDir>

#include <gtest/gtest.h>

using namespace nmc;

namespace
{
QString createEmptyMetadataPng(QTemporaryDir &tempDir)
{
    QImage img(16, 16, QImage::Format_RGB32);
    img.fill(Qt::white);

    const QString filePath = tempDir.filePath("empty-metadata.png");
    EXPECT_TRUE(img.save(filePath, "PNG"));

    return filePath;
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
    EXPECT_FALSE(emptyMeta.setRating(0));
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
