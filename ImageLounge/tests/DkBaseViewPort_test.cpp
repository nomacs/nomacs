#include "DkBaseViewPort.h"
#include <QRectF>
#include <gtest/gtest.h>

struct ScaleKeepAspectRatioAndCenterTestCase {
    QSizeF src;
    QSizeF tgt;
    qreal paddingRatio = 0;
};

void PrintTo(const ScaleKeepAspectRatioAndCenterTestCase &tc, std::ostream *os)
{
    QString s;
    QDebug(&s) << "src:" << tc.src << ", tgt:" << tc.tgt << ", paddingRatio:" << tc.paddingRatio;
    *os << s.toStdString();
}

class ScaleKeepAspectRatioAndCenterTest : public testing::TestWithParam<ScaleKeepAspectRatioAndCenterTestCase>
{
};

TEST_P(ScaleKeepAspectRatioAndCenterTest, Test)
{
    const ScaleKeepAspectRatioAndCenterTestCase params = GetParam();
    const QTransform t = nmc::scaleKeepAspectRatioAndCenter(params.src, params.tgt, params.paddingRatio);

    const QRectF srcRect = QRectF(QPointF(), params.src);
    const QRectF mappedRect = t.mapRect(srcRect);

    QRectF scaledRect = QRectF(QPointF(), params.src.scaled(params.tgt, Qt::KeepAspectRatio));
    scaledRect.moveCenter(QPointF(params.tgt.width(), params.tgt.height()) / 2);

    EXPECT_NEAR(mappedRect.x(), scaledRect.x(), 1e-5);
    EXPECT_NEAR(mappedRect.y(), scaledRect.y(), 1e-5);
    EXPECT_NEAR(mappedRect.width(), scaledRect.width(), 1e-5);
    EXPECT_NEAR(mappedRect.height(), scaledRect.height(), 1e-5);
}

INSTANTIATE_TEST_SUITE_P(Test,
                         ScaleKeepAspectRatioAndCenterTest,
                         testing::ValuesIn(std::vector<ScaleKeepAspectRatioAndCenterTestCase>{
                             {QSizeF(400, 300), QSizeF(200, 150)},
                             {QSizeF(400, 300), QSizeF(200, 100)},
                             {QSizeF(400, 300), QSizeF(200, 200)},
                             {QSizeF(400, 300), QSizeF(800, 600)},
                             {QSizeF(400, 300), QSizeF(700, 600)},
                             {QSizeF(400, 300), QSizeF(800, 700)},
                             {QSizeF(1024, 2048), QSizeF(553, 339)},
                         }));
