#include "../src/DkCore/DkUtils.h"
#include "DkVersion.h"
#include <gtest/gtest.h>
#include <string>

TEST(LinkedVersionTest, Test)
{
    // If this test fails the test executable has loaded the wrong nomacsCore shared library
    // This can be caused by LD_LIBRARY_PATH containing another installation of nomacsCore.
    EXPECT_EQ(std::string(NOMACS_REVISION_STR), std::string(nmc::revisionString));
}

TEST(DkFileNameConverterTest, Test)
{
    nmc::DkFileNameConverter base("<c:0><d:2:0>.<old>");
    EXPECT_EQ(base.convert("test.jpg", 1).toStdString(), std::string("test001.jpg"));

    nmc::DkFileNameConverter customStringUpper("image_<c:2>_num_<d:2:0>.png");
    EXPECT_EQ(customStringUpper.convert("test.jpg", 1).toStdString(), std::string("image_TEST_num_001.png"));

    nmc::DkFileNameConverter lower("<c:1>.png");
    EXPECT_EQ(lower.convert("teSt_ImaGe.jpg", 0).toStdString(), std::string("test_image.png"));
}

TEST(DkFileNameConverterTest, OldIndex)
{
    nmc::DkFileNameConverter digits("<d:2:0>");
    EXPECT_EQ(digits.convert("test.jpg", 1).toStdString(), std::string("001"));

    EXPECT_EQ(digits.convert("test.jpg", 12).toStdString(), std::string("012"));

    EXPECT_EQ(digits.convert("test.jpg", 1000).toStdString(), std::string("1000"));

    nmc::DkFileNameConverter digitsOffset("<d:2:5>");
    EXPECT_EQ(digitsOffset.convert("test.jpg", 7).toStdString(), std::string("012"));

    nmc::DkFileNameConverter digitsOmitOffset("<d:2>");
    EXPECT_EQ(digitsOmitOffset.convert("test.jpg", 1).toStdString(), std::string("001"));

    nmc::DkFileNameConverter zeroPad("<d:0:0>");
    EXPECT_EQ(zeroPad.convert("test.jpg", 1).toStdString(), std::string("1"));
    EXPECT_EQ(zeroPad.convert("test.jpg", 11).toStdString(), std::string("11"));

    nmc::DkFileNameConverter onePad("<d:1:0>");
    EXPECT_EQ(onePad.convert("test.jpg", 1).toStdString(), std::string("01"));
    EXPECT_EQ(onePad.convert("test.jpg", 11).toStdString(), std::string("11"));

    nmc::DkFileNameConverter fourPad("<d:4:0>");
    EXPECT_EQ(fourPad.convert("test.jpg", 11).toStdString(), std::string("00011"));
}
