#include "DkSettings.h"
#include "DkViewPortTransformViewModel.h"
#include <gtest/gtest.h>
#include <optional>
#include <qdebug.h>
#include <qobject.h>
#include <qsize.h>
#include <qtransform.h>

namespace nmc
{

class DkViewPortTransformViewModelTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // devicePixelRatio = 1.0, zeroPanControl = false, resetWhenZoomPastFit = false
        vm = std::make_unique<DkViewPortTransformViewModel>(1.0, false, false);
    }

    std::unique_ptr<DkViewPortTransformViewModel> vm;
};

TEST_F(DkViewPortTransformViewModelTest, InitialState)
{
    EXPECT_TRUE(vm->worldMatrix().isIdentity());
    EXPECT_TRUE(vm->imgMatrix().isIdentity());
    EXPECT_DOUBLE_EQ(vm->zoomLevel(), 1.0);
}

struct SetImageSizeTestParam {
    QSizeF size;
    qreal expectedZoomLevel = 1;
    QRectF expectedImgViewRect;
    QTransform expectedWorldMatrix;
    std::optional<DkSettings::keepZoom> keepZoom;
};

void PrintTo(const SetImageSizeTestParam &tc, std::ostream *os)
{
    QString s;
    QDebug(&s) << "size:" << tc.size;
    *os << s.toStdString();
}

class SetImageSizeTest : public testing::TestWithParam<SetImageSizeTestParam>
{
protected:
    void SetUp() override
    {
        // devicePixelRatio = 1.0, zeroPanControl = false, resetWhenZoomPastFit = false
        vm = std::make_unique<DkViewPortTransformViewModel>(1.0, false, false);
    }

    std::unique_ptr<DkViewPortTransformViewModel> vm;
};

template<typename T>
testing::AssertionResult assertQFuzzy(const T &t1, const T &t2)
{
    if (!qFuzzyCompare(t1, t2)) {
        QString s;
        QDebug(&s) << t1 << "!=" << t2;
        return testing::AssertionFailure() << s.toStdString();
    }

    return testing::AssertionSuccess();
}

TEST_P(SetImageSizeTest, SetImageSize)
{
    constexpr QRect widgetRect = QRect({}, QSize(1200, 900));
    vm->setWidgetSize(widgetRect.size());
    EXPECT_EQ(vm->viewPortRect(), widgetRect);

    const SetImageSizeTestParam &params = GetParam();
    vm->setImgSize(params.size, params.keepZoom);
    EXPECT_DOUBLE_EQ(vm->zoomLevel(), params.expectedZoomLevel) << "unequal zoomLevel";
    EXPECT_TRUE(assertQFuzzy(vm->imgViewRect(), params.expectedImgViewRect)) << "unequal imgViewRect";
    EXPECT_TRUE(assertQFuzzy(vm->worldMatrix(), params.expectedWorldMatrix)) << "unequal worldMatrix";
}

INSTANTIATE_TEST_SUITE_P(Initial,
                         SetImageSizeTest,
                         testing::ValuesIn(std::vector<SetImageSizeTestParam>{
                             {
                                 QSizeF(400, 300),
                                 1,
                                 QRectF({400, 300}, QSizeF(400, 300)),
                                 QTransform(),
                                 std::nullopt,
                             },
                             {
                                 QSizeF(1200, 900),
                                 1,
                                 QRectF({}, QSizeF(1200, 900)),
                                 QTransform(),
                                 std::nullopt,
                             },
                             {
                                 QSizeF(1920, 1440),
                                 1200. / 1920.,
                                 QRectF({}, QSizeF(1200, 900)),
                                 QTransform(),
                                 std::nullopt,
                             },
                             {
                                 QSizeF(400, 200),
                                 1,
                                 QRectF({400, 350}, QSizeF(400, 200)),
                                 QTransform(),
                                 std::nullopt,
                             },
                             {
                                 QSizeF(1200, 800),
                                 1,
                                 QRectF({0, 50}, QSizeF(1200, 800)),
                                 QTransform(),
                                 std::nullopt,
                             },
                             {
                                 QSizeF(1920, 900),
                                 1200. / 1920.,
                                 QRectF({0, 900 * (1 - 1200. / 1920.) / 2}, QSizeF(1200, 900 * 1200. / 1920.)),
                                 QTransform(),
                                 std::nullopt,
                             },
                             {
                                 QSizeF(600, 800),
                                 1,
                                 QRectF({300, 50}, QSizeF(600, 800)),
                                 QTransform(),
                                 std::nullopt,
                             },
                             {
                                 QSizeF(600, 1800),
                                 0.5,
                                 QRectF({450, 0}, QSizeF(300, 900)),
                                 QTransform(),
                                 std::nullopt,
                             },
                             {
                                 QSizeF(2400, 2400),
                                 900. / 2400.,
                                 QRectF({150, 0}, QSizeF(900, 900)),
                                 QTransform(),
                                 std::nullopt,
                             },
                             // Single pixel image
                             {
                                 QSizeF(1, 1),
                                 1,
                                 QRectF({599.5, 449.5}, QSizeF(1, 1)),
                                 QTransform(),
                                 std::nullopt,
                             },
                         }));

TEST_F(DkViewPortTransformViewModelTest, Zooming)
{
    vm->setWidgetSize(QSize(1000, 1000));
    vm->setImgSize(QSizeF(500, 500));

    // Initially at 1.0
    EXPECT_DOUBLE_EQ(vm->zoomLevel(), 1.0);

    // Zoom in by 2.0x
    vm->zoom(2.0);
    EXPECT_DOUBLE_EQ(vm->zoomLevel(), 2.0);

    // Zoom out by 0.5x
    vm->zoom(0.5);
    EXPECT_DOUBLE_EQ(vm->zoomLevel(), 1.0);
}

TEST_F(DkViewPortTransformViewModelTest, PanTranslation)
{
    vm->setWidgetSize(QSize(1000, 1000));
    vm->setImgSize(QSizeF(2000, 2000)); // Make it larger to allow panning

    // Set initial zoom to 1.0 (so no scaling constraints hide the pan)
    vm->setDisablePanForSmallDimension(false); // allow free panning

    double initialDx = vm->worldMatrix().dx();

    // Panning right should move the viewport content left (negative dx)
    // Actually, panRight might move the view right, which moves the matrix left.
    // Let's just check if translateViewInWidgetCoords changes the matrix.
    vm->moveViewInWidgetCoords({-20, -30});

    EXPECT_DOUBLE_EQ(vm->worldMatrix().dx(), initialDx - 20);
    EXPECT_DOUBLE_EQ(vm->worldMatrix().dy(), -30);
}

TEST_F(DkViewPortTransformViewModelTest, ZoomLeveledWithProvider)
{
    vm->setWidgetSize(QSize(1000, 1000));
    vm->setImgSize(QSizeF(500, 500));

    // Inject our custom zoom levels
    vm->setZoomLevelSettingProvider([]() {
        return QVector<double>{0.5, 1.0, 1.5, 2.0, 3.0};
    });

    EXPECT_DOUBLE_EQ(vm->zoomLevel(), 1.0);

    // Zoom in should snap to 1.5
    vm->zoomIn();
    EXPECT_DOUBLE_EQ(vm->zoomLevel(), 1.5);

    // Zoom in again should snap to 2.0
    vm->zoomIn();
    EXPECT_DOUBLE_EQ(vm->zoomLevel(), 2.0);

    // Zoom out should snap back to 1.5
    vm->zoomOut();
    EXPECT_DOUBLE_EQ(vm->zoomLevel(), 1.5);
}

TEST_F(DkViewPortTransformViewModelTest, ZoomToFit)
{
    vm->setWidgetSize(QSize(1000, 1000));
    // Large image
    vm->setImgSize(QSizeF(2000, 4000));

    vm->zoomToFit();

    // Zoom to fit should fit the largest dimension.
    // 4000 / 1000 = 4.0 scale down => 0.25 zoom factor.
    EXPECT_DOUBLE_EQ(vm->zoomLevel(), 0.25);
}

struct SetWidgetSizeTestParam {
    QSize initialWidgetSize;
    QSizeF imageSize;
    std::optional<double> zoomLevel; // If set, calls zoomTo()
    QPointF panDelta; // If non-zero, calls moveViewInWidgetCoords()
    QSize targetWidgetSize;
    QRectF expectedImageViewRect;
};

void PrintTo(const SetWidgetSizeTestParam &tc, std::ostream *os)
{
    QString s;
    QDebug(&s) << "initialWidgetSize:" << tc.initialWidgetSize << "imageSize:" << tc.imageSize
               << "zoomLevel:" << (tc.zoomLevel.has_value() ? QString::number(*tc.zoomLevel) : "none")
               << "panDelta:" << tc.panDelta << "targetWidgetSize:" << tc.targetWidgetSize;
    *os << s.toStdString();
}

class SetWidgetSizeTest : public testing::TestWithParam<SetWidgetSizeTestParam>
{
protected:
    void SetUp() override
    {
        // devicePixelRatio = 1.0, zeroPanControl = false, resetWhenZoomPastFit = false
        vm = std::make_unique<DkViewPortTransformViewModel>(1.0, false, false);
    }

    std::unique_ptr<DkViewPortTransformViewModel> vm;
};

TEST_P(SetWidgetSizeTest, SetWidgetSize)
{
    const SetWidgetSizeTestParam &params = GetParam();

    // vm->setDisablePanForSmallDimension(false);
    // vm->setDisableControlAndCenter(true);

    vm->setWidgetSize(params.initialWidgetSize);
    vm->setImgSize(params.imageSize);

    if (params.zoomLevel) {
        vm->zoomTo(*params.zoomLevel);
    }

    if (!params.panDelta.isNull()) {
        vm->moveViewInWidgetCoords(params.panDelta);
    }

    vm->setWidgetSize(params.targetWidgetSize);

    EXPECT_EQ(vm->viewPortRect().size(), params.targetWidgetSize);
    EXPECT_TRUE(assertQFuzzy(vm->getImageViewRect(), params.expectedImageViewRect))
        << "Expected: " << params.expectedImageViewRect.x() << "," << params.expectedImageViewRect.y() << " "
        << params.expectedImageViewRect.width() << "x" << params.expectedImageViewRect.height()
        << "\nActual: " << vm->getImageViewRect().x() << "," << vm->getImageViewRect().y() << " "
        << vm->getImageViewRect().width() << "x" << vm->getImageViewRect().height();
}

INSTANTIATE_TEST_SUITE_P(
    VariousCases,
    SetWidgetSizeTest,
    testing::ValuesIn(
        std::vector<SetWidgetSizeTestParam>{// Case 1: No zoom, image smaller than viewport.
                                            {QSize(800, 600),
                                             QSizeF(400, 300),
                                             std::nullopt,
                                             QPointF(0, 0),
                                             QSize(1000, 800),
                                             QRectF(300, 250, 400, 300)},
                                            // Case 2: No zoom, image larger than viewport (initial scale to fit)
                                            {QSize(800, 600),
                                             QSizeF(1600, 1200),
                                             std::nullopt,
                                             QPointF(0, 0),
                                             QSize(1000, 800),
                                             QRectF(0, 25, 1000, 750)},
                                            // Case 3: Active zoom in (invariant)
                                            {QSize(800, 600),
                                             QSizeF(400, 300),
                                             2.0,
                                             QPointF(0, 0),
                                             QSize(1000, 800),
                                             QRectF(0, 0, 800, 600)},
                                            // Case 4: Active zoom out (invariant)
                                            {QSize(800, 600),
                                             QSizeF(400, 300),
                                             0.5,
                                             QPointF(0, 0),
                                             QSize(1000, 800),
                                             QRectF(300, 225, 200, 150)},
                                            // Case 5: Active zoom with pan translation
                                            {QSize(800, 600),
                                             QSizeF(400, 300),
                                             2.0,
                                             QPointF(100, 50),
                                             QSize(1000, 800),
                                             QRectF(100, 50, 800, 600)},
                                            // Case 6: No zoom with pan translation
                                            {QSize(800, 600),
                                             QSizeF(400, 300),
                                             std::nullopt,
                                             QPointF(100, 50),
                                             QSize(1000, 800),
                                             QRectF(400, 300, 400, 300)},
                                            // Case 7: Empty/Null image
                                            {QSize(800, 600),
                                             QSizeF(0, 0),
                                             std::nullopt,
                                             QPointF(0, 0),
                                             QSize(1000, 800),
                                             QRectF(0, 0, 0, 0)}}));

} // namespace nmc
