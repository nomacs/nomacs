#include <functional>
#include <memory>
#include <optional>
#include <ostream>
#include <string_view>
#include <vector>

#include <QDebug>
#include <QPoint>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QTransform>
#include <QtContainerFwd>
#include <QtGlobal>
#include <QtTypes>
#include <QtVersionChecks>
#include <gtest/gtest.h>

#include "DkSettings.h"
#include "DkViewPortTransformViewModel.h"

// NOTE: these belongs in the global scope, otherwise they will hide the Qt provided overloads.
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
constexpr bool qFuzzyCompare(const QPointF &lhs, const QPointF &rhs) noexcept
{
    return qFuzzyCompare(lhs.x(), rhs.x()) && qFuzzyCompare(lhs.y(), rhs.y());
}

constexpr bool qFuzzyCompare(const QSizeF &lhs, const QSizeF &rhs) noexcept
{
    return qFuzzyCompare(lhs.width(), rhs.width()) && qFuzzyCompare(lhs.height(), rhs.height());
}

constexpr bool qFuzzyCompare(const QRectF &lhs, const QRectF &rhs) noexcept
{
    return qFuzzyCompare(lhs.topLeft(), rhs.topLeft()) && qFuzzyCompare(lhs.bottomRight(), rhs.bottomRight());
}
#endif

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

    vm->setZoomLevelSettingProvider([]() {
        return QVector<double>{0.5, 1.0, 1.5, 2.0, 3.0};
    });

    EXPECT_DOUBLE_EQ(vm->zoomLevel(), 1.0);

    vm->zoomIn();
    EXPECT_DOUBLE_EQ(vm->zoomLevel(), 1.5);

    vm->zoomIn();
    EXPECT_DOUBLE_EQ(vm->zoomLevel(), 2.0);

    vm->zoomOut();
    EXPECT_DOUBLE_EQ(vm->zoomLevel(), 1.5);
}

TEST_F(DkViewPortTransformViewModelTest, ZoomToFit)
{
    vm->setWidgetSize(QSize(1000, 1000));
    vm->setImgSize(QSizeF(2000, 4000));
    vm->zoomToFit();
    EXPECT_DOUBLE_EQ(vm->zoomLevel(), 0.25);
}

struct SetWidgetSizeTestParam {
    std::string_view desc;
    QSize initialWidgetSize;
    QSizeF imageSize;
    std::optional<double> zoomLevel; // If set, calls zoomTo()
    QPointF panDelta; // If non-zero, calls moveViewInWidgetCoords()
    QSize targetWidgetSize;
    QPointF expectedImageViewRectTopLeft;
    std::optional<double> expectedNewZoomLevel; // If set, zoom level will change to this.
};

void PrintTo(const SetWidgetSizeTestParam &tc, std::ostream *os)
{
    *os << tc.desc;
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

    vm->setWidgetSize(params.initialWidgetSize);
    vm->setImgSize(params.imageSize, DkSettings::zoom_always_keep); // Initial zoom level is 100%

    if (params.zoomLevel) {
        vm->zoomTo(*params.zoomLevel);
    }
    const auto zl = vm->zoomLevel();

    if (!params.panDelta.isNull()) {
        vm->moveViewInWidgetCoords(params.panDelta);
    }

    vm->setWidgetSize(params.targetWidgetSize);

    const double expectedZL = params.expectedNewZoomLevel.value_or(zl);
    EXPECT_EQ(vm->viewPortRect().size(), params.targetWidgetSize) << "unequal viewPortRect size";
    EXPECT_DOUBLE_EQ(vm->zoomLevel(), expectedZL) << "zoom level changed";
    EXPECT_TRUE(assertQFuzzy(vm->getImageViewRect().size(), params.imageSize * expectedZL))
        << "unequal getImageViewRect size";
    EXPECT_TRUE(assertQFuzzy(vm->getImageViewRect().topLeft(), params.expectedImageViewRectTopLeft))
        << "unequal getImageViewRect topLeft";
}

INSTANTIATE_TEST_SUITE_P(
    WidgetSizeCases,
    SetWidgetSizeTest,
    testing::ValuesIn(std::vector<SetWidgetSizeTestParam>{
        {
            "No zoom, viewport larger than image, increase size",
            QSize(800, 600),
            QSizeF(400, 300),
            std::nullopt,
            QPointF(0, 0),
            QSize(1000, 800),
            QPointF(300, 250),
            std::nullopt,
        },
        {
            "No zoom, viewport larger than image, decrease size but still larger",
            QSize(800, 600),
            QSizeF(400, 300),
            std::nullopt,
            QPointF(0, 0),
            QSize(600, 450),
            QPointF(100, 75),
            std::nullopt,
        },
        {
            "No zoom, viewport larger than image, decrease size to equal",
            QSize(800, 600),
            QSizeF(400, 300),
            std::nullopt,
            QPointF(0, 0),
            QSize(400, 300),
            QPointF(0, 0),
            std::nullopt,
        },
        {
            "No zoom, viewport larger than image, decrease size to smaller",
            QSize(800, 600),
            QSizeF(400, 300),
            std::nullopt,
            QPointF(0, 0),
            QSize(200, 150),
            QPointF(0, 0), // Special case, at 100% and smaller than viewport the image will shrink with it.
            0.5,
        },
        {
            "Zoom out, viewport larger than image, decrease size to smaller",
            QSize(800, 600),
            QSizeF(400, 300),
            0.8,
            QPointF(0, 0),
            QSize(200, 150),
            QPointF(0, 0), // Snap to topLeft
            std::nullopt,
        },
        {
            "No zoom, viewport smaller than image, decrease size",
            QSize(800, 600),
            QSizeF(1600, 1200),
            std::nullopt,
            QPointF(0, 0),
            QSize(600, 450),
            QPointF(-400, -300),
            std::nullopt,
        },
        {
            "No zoom, viewport smaller than image, increase size but still smaller",
            QSize(800, 600),
            QSizeF(1600, 1200),
            std::nullopt,
            QPointF(0, 0),
            QSize(1000, 750),
            QPointF(-400, -300), // Snap to topleft
            std::nullopt,
        },
        {
            "No zoom, viewport smaller than image, increase size but still smaller, snap bottomRight",
            QSize(800, 600),
            QSizeF(1600, 1200),
            std::nullopt,
            QPointF(0, 0),
            QSize(1440, 1080),
            QPointF(-160, -120), // Snap to bottomRight to fill
            std::nullopt,
        },
        {
            "No zoom, viewport smaller than image, increase size to equal",
            QSize(800, 600),
            QSizeF(1600, 1200),
            std::nullopt,
            QPointF(0, 0),
            QSize(1600, 1200),
            QPointF(0, 0),
            std::nullopt,
        },
        {
            "No zoom, viewport smaller than image, increase size to larger",
            QSize(800, 600),
            QSizeF(1600, 1200),
            std::nullopt,
            QPointF(0, 0),
            QSize(2000, 1500),
            QPointF(200, 150),
            std::nullopt,
        },
        {
            "Active zoom in",
            QSize(800, 600),
            QSizeF(400, 300),
            2.0,
            QPointF(0, 0),
            QSize(1000, 800),
            QPointF(100, 100),
            std::nullopt,
        },
        {
            "Active zoom out",
            QSize(800, 600),
            QSizeF(400, 300),
            0.5,
            QPointF(0, 0),
            QSize(1000, 800),
            QPointF(400, 325),
            std::nullopt,
        },
        {
            // Special case, when zoom out and fit to viewport, the image stretches.
            "Active zoom out, fit",
            QSize(800, 600),
            QSizeF(1600, 1200),
            0.5,
            QPointF(0, 0),
            QSize(1000, 800),
            QPointF(0, 25),
            5. / 8.,
        },
        {
            "Active zoom with pan",
            QSize(800, 600),
            QSizeF(400, 300),
            2.5,
            QPointF(100, 50),
            QSize(600, 450),
            QPointF(0, -25),
            std::nullopt,
        },
        {
            "No zoom with pan, viewport smaller than image, increase size but still smaller, snap bottomRight",
            QSize(800, 600),
            QSizeF(1600, 1200),
            std::nullopt,
            QPointF(100, 50),
            QSize(1000, 750),
            QPointF(-300, -250),
            std::nullopt,
        },
        {
            "Empty/Null image",
            QSize(800, 600),
            QSizeF(0, 0),
            std::nullopt,
            QPointF(0, 0),
            QSize(1000, 800),
            QPointF(0, 0),
            std::nullopt,
        },
    }));

struct ZoomTestParam {
    std::string_view desc;
    QSizeF imageSize;
    std::optional<double> zoomLevel; // If set, calls zoomTo()
    QPointF panDelta; // If non-zero, calls moveViewInWidgetCoords()
    double zoomFactor;
    std::optional<QPointF> zoomCenter;
    QPointF expectedImageViewRectTopLeft;
    double expectedNewZoomLevel;
};

void PrintTo(const ZoomTestParam &tc, std::ostream *os)
{
    *os << tc.desc;
}

class ZoomTest : public testing::TestWithParam<ZoomTestParam>
{
protected:
    void SetUp() override
    {
        // devicePixelRatio = 1.0, zeroPanControl = false, resetWhenZoomPastFit = false
        vm = std::make_unique<DkViewPortTransformViewModel>(1.0, false, false);
    }

    std::unique_ptr<DkViewPortTransformViewModel> vm;
};

TEST_P(ZoomTest, DkViewPortZoom)
{
    vm->setZoomCenterLimit(DkViewPortTransformViewModel::ZoomCenterLimit::CenterSmallDimension);
    const ZoomTestParam &params = GetParam();

    vm->setWidgetSize({1200, 900});
    vm->setImgSize(params.imageSize, DkSettings::zoom_always_keep); // Initial zoom level is 100%

    if (params.zoomLevel) {
        vm->zoomTo(*params.zoomLevel);
    }

    if (!params.panDelta.isNull()) {
        vm->moveViewInWidgetCoords(params.panDelta);
    }

    // Disable snap
    vm->zoom(params.zoomFactor, params.zoomCenter.value_or(QPointF(-1, -1)), true);

    EXPECT_DOUBLE_EQ(vm->zoomLevel(), params.expectedNewZoomLevel) << "unequal zoom level";
    EXPECT_TRUE(assertQFuzzy(vm->getImageViewRect().size(), params.imageSize * params.expectedNewZoomLevel))
        << "unequal getImageViewRect size";
    EXPECT_TRUE(assertQFuzzy(vm->getImageViewRect().topLeft(), params.expectedImageViewRectTopLeft))
        << "unequal getImageViewRect topLeft";
}

INSTANTIATE_TEST_SUITE_P(ZoomCases,
                         ZoomTest,
                         testing::ValuesIn(std::vector<ZoomTestParam>{
                             {
                                 "No zoom, zoom in, smaller than viewport, center",
                                 QSizeF(400, 300),
                                 std::nullopt,
                                 QPointF(0, 0),
                                 1.1,
                                 std::nullopt,
                                 QPointF(380, 285),
                                 1.1,
                             },
                             {
                                 "No zoom, zoom in, smaller than viewport, off center",
                                 QSizeF(400, 300),
                                 std::nullopt,
                                 QPointF(0, 0),
                                 1.1,
                                 QPointF(440, 340),
                                 QPointF(380, 285), // Should still be centered
                                 1.1,
                             },
                             {
                                 "No zoom, zoom in, smaller than viewport, outside",
                                 QSizeF(400, 300),
                                 std::nullopt,
                                 QPointF(0, 0),
                                 1.1,
                                 QPointF(100, 100),
                                 QPointF(380, 285), // Should still be centered
                                 1.1,
                             },
                             {
                                 "No zoom, zoom in, smaller than viewport to larger, center",
                                 QSizeF(1100, 600),
                                 std::nullopt,
                                 QPointF(0, 0),
                                 1.1,
                                 std::nullopt,
                                 QPointF(-5, 120),
                                 1.1,
                             },
                             {
                                 "No zoom, zoom in, smaller than viewport to larger, off center",
                                 QSizeF(1100, 600),
                                 std::nullopt,
                                 QPointF(0, 0),
                                 1.1,
                                 QPointF(400, 400),
                                 QPointF(15, 120), // Still center vertically
                                 1.1,
                             },
                             {
                                 "No zoom, zoom in, smaller than viewport to larger, outside",
                                 QSizeF(1100, 600),
                                 std::nullopt,
                                 QPointF(0, 0),
                                 1.1,
                                 QPointF(30, 400),
                                 QPointF(52, 120), // Still center vertically
                                 1.1,
                             },
                             {
                                 "Zoomed, zoom in, no pan, center",
                                 QSizeF(1000, 800),
                                 1.5,
                                 QPointF(0, 0),
                                 1.1,
                                 std::nullopt,
                                 QPointF(-225, -210),
                                 1.65,
                             },
                             {
                                 "Zoomed, zoom in, no pan, off center",
                                 QSizeF(1000, 800),
                                 1.5,
                                 QPointF(0, 0),
                                 1.1,
                                 QPointF(100, 100),
                                 QPointF(-175, -175),
                                 1.65,
                             },
                             {
                                 "Zoomed, zoom in, pan, center",
                                 QSizeF(1000, 800),
                                 1.5,
                                 QPointF(100, 100),
                                 1.1,
                                 std::nullopt,
                                 QPointF(-115, -100),
                                 1.65,
                             },
                             {
                                 "Zoomed, zoom in, pan, off center",
                                 QSizeF(1000, 800),
                                 1.5,
                                 QPointF(100, 100),
                                 1.1,
                                 QPointF(100, 100),
                                 QPointF(-65, -65),
                                 1.65,
                             },
                             {
                                 "Zoomed, zoom in, pan to edge, center",
                                 QSizeF(1000, 800),
                                 1.5,
                                 QPointF(750, 600),
                                 1.1,
                                 std::nullopt,
                                 QPointF(600, 450),
                                 1.65,
                             },
                             {
                                 "Zoomed, zoom in, pan to edge, off center",
                                 QSizeF(1000, 800),
                                 1.5,
                                 QPointF(750, 600),
                                 1.1,
                                 QPointF(700, 550),
                                 QPointF(590, 440),
                                 1.65,
                             },
                             {
                                 "Zoomed, zoom in, pan to edge, outside",
                                 QSizeF(1000, 800),
                                 1.5,
                                 QPointF(750, 600),
                                 1.1,
                                 QPointF(500, 350),
                                 QPointF(600, 450), // Stay at the center
                                 1.65,
                             },
                             {
                                 "Zoomed, zoom out, no pan, center",
                                 QSizeF(1000, 800),
                                 1.5,
                                 QPointF(0, 0),
                                 0.9,
                                 std::nullopt,
                                 QPointF(-75, -90),
                                 1.35,
                             },
                             {
                                 "Zoomed, zoom out, no pan, off center",
                                 QSizeF(1000, 800),
                                 1.5,
                                 QPointF(0, 0),
                                 0.9,
                                 QPointF(100, 100),
                                 QPointF(-125, -125),
                                 1.35,
                             },
                             {
                                 "Zoomed, zoom out, pan, center",
                                 QSizeF(1000, 800),
                                 1.5,
                                 QPointF(100, 100),
                                 0.9,
                                 std::nullopt,
                                 QPointF(15, 0),
                                 1.35,
                             },
                             {
                                 "Zoomed, zoom out, pan, off center",
                                 QSizeF(1000, 800),
                                 1.5,
                                 QPointF(100, 100),
                                 0.9,
                                 QPointF(100, 100),
                                 QPointF(-35, -35),
                                 1.35,
                             },
                             {
                                 "Zoomed, zoom out, pan to edge, center",
                                 QSizeF(1000, 800),
                                 1.5,
                                 QPointF(750, 600),
                                 0.9,
                                 std::nullopt,
                                 QPointF(600, 450),
                                 1.35,
                             },
                             {
                                 "Zoomed, zoom out, pan to edge, off center",
                                 QSizeF(1000, 800),
                                 1.5,
                                 QPointF(750, 600),
                                 0.9,
                                 QPointF(700, 550),
                                 QPointF(600, 450), // Stay at the center
                                 1.35,
                             },
                             {
                                 "Zoomed, zoom out, pan to edge, outside",
                                 QSizeF(1000, 800),
                                 1.5,
                                 QPointF(750, 600),
                                 0.9,
                                 QPointF(500, 350),
                                 QPointF(590, 440),
                                 1.35,
                             },
                         }));

struct SyncTestParam {
    std::string_view desc;
    QSizeF srcImageSize;
    QSizeF tgtImageSize;
    std::function<void(DkViewPortTransformViewModel *)> op;
};

void PrintTo(const SyncTestParam &tc, std::ostream *os)
{
    *os << tc.desc;
}

class SyncTest : public testing::TestWithParam<SyncTestParam>
{
protected:
    void SetUp() override
    {
        // devicePixelRatio = 1.0, zeroPanControl = false, resetWhenZoomPastFit = false
        vmSrc = std::make_unique<DkViewPortTransformViewModel>(1.0, false, false);
        vmSrc->setZoomCenterLimit(DkViewPortTransformViewModel::ZoomCenterLimit::CenterSmallDimension);
        vmSrc->setWidgetSize({1200, 900});

        vmTgt = std::make_unique<DkViewPortTransformViewModel>(1.0, false, false);
        vmTgt->setZoomCenterLimit(DkViewPortTransformViewModel::ZoomCenterLimit::CenterSmallDimension);
        vmTgt->setWidgetSize({1200, 900});
    }

    std::unique_ptr<DkViewPortTransformViewModel> vmSrc;
    std::unique_ptr<DkViewPortTransformViewModel> vmTgt;
};

TEST_P(SyncTest, SyncTransform)
{
    const SyncTestParam &params = GetParam();

    vmSrc->setImgSize(params.srcImageSize, DkSettings::zoom_always_keep);
    vmTgt->setImgSize(params.tgtImageSize, DkSettings::zoom_always_keep);

    const auto sync = [this, &params]() {
        QPointF size = vmSrc->viewPortRect().center();
        size = vmSrc->worldMatrix().inverted().map(size);
        size = vmSrc->imgMatrix().inverted().map(size);
        vmTgt->syncTransform(vmSrc->worldMatrix(),
                             vmSrc->imgMatrix(),
                             {size.x() / params.srcImageSize.width(), size.y() / params.srcImageSize.height()});
    };

    const auto getRelCenter = [](DkViewPortTransformViewModel *vm, const QSizeF &imgSize) {
        QPointF diff = (vm->getImageViewRect().center() - vm->viewPortRect().center()) / vm->zoomLevel();
        return QPointF(diff.x() / imgSize.width(), diff.y() / imgSize.height());
    };

    // Sync zoom level initially, so that the topleft is correct
    sync();
    EXPECT_DOUBLE_EQ(vmTgt->zoomLevel(), vmSrc->zoomLevel()) << "unequal zoom level initial sync";

    params.op(vmSrc.get());
    sync();

    EXPECT_DOUBLE_EQ(vmTgt->zoomLevel(), vmSrc->zoomLevel()) << "unequal zoom level after sync op";

    QPointF srcFinalRel = getRelCenter(vmSrc.get(), params.srcImageSize);
    QPointF tgtFinalRel = getRelCenter(vmTgt.get(), params.tgtImageSize);
    EXPECT_TRUE(assertQFuzzy(srcFinalRel, tgtFinalRel)) << "unequal relative center after sync op";
}

INSTANTIATE_TEST_SUITE_P(ZoomCases,
                         SyncTest,
                         testing::ValuesIn(std::vector<SyncTestParam>{
                             {
                                 "Smaller than viewport, same size, zoom out",
                                 QSizeF(800, 600),
                                 QSizeF(800, 600),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->zoom(0.5, {-1, -1}, true);
                                 },
                             },
                             {
                                 "Smaller than viewport, same size, zoom in",
                                 QSizeF(800, 600),
                                 QSizeF(800, 600),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->zoom(1.5, {-1, -1}, true);
                                 },
                             },
                             {
                                 "Smaller than viewport, diff size, zoom out",
                                 QSizeF(800, 600),
                                 QSizeF(400, 200),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->zoom(0.5, {-1, -1}, true);
                                 },
                             },
                             {
                                 "Smaller than viewport, diff size, zoom in",
                                 QSizeF(800, 600),
                                 QSizeF(400, 200),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->zoom(1.5, {-1, -1}, true);
                                 },
                             },
                             {
                                 "Larger than viewport, same size, zoom out",
                                 QSizeF(2400, 1800),
                                 QSizeF(2400, 1800),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->zoom(0.6, {-1, -1}, true);
                                 },
                             },
                             {
                                 "Larger than viewport, same size, zoom out to viewport size",
                                 QSizeF(2400, 1800),
                                 QSizeF(2400, 1800),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->zoom(0.5, {-1, -1}, true);
                                 },
                             },
                             {
                                 "Larger than viewport, same size, zoom in",
                                 QSizeF(2400, 1800),
                                 QSizeF(2400, 1800),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->zoom(1.5, {-1, -1}, true);
                                 },
                             },
                             {
                                 "Larger than viewport, same size, zoom out with center",
                                 QSizeF(2400, 1800),
                                 QSizeF(2400, 1800),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->zoom(0.6, {400, 300}, true);
                                 },
                             },
                             {
                                 "Larger than viewport, same size, zoom out to viewport size with center",
                                 QSizeF(2400, 1800),
                                 QSizeF(2400, 1800),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->zoom(0.5, {400, 300}, true);
                                 },
                             },
                             {
                                 "Larger than viewport, same size, zoom in with center",
                                 QSizeF(2400, 1800),
                                 QSizeF(2400, 1800),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->zoom(1.5, {400, 300}, true);
                                 },
                             },
                             {
                                 "Larger than viewport, same size, pan",
                                 QSizeF(2400, 1800),
                                 QSizeF(2400, 1800),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->moveViewInWidgetCoords({100, 200});
                                 },
                             },
                             {
                                 "Larger than viewport, diff size, zoom out",
                                 QSizeF(2400, 1800),
                                 QSizeF(2000, 1600),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->zoom(0.6, {-1, -1}, true);
                                 },
                             },
                             {
                                 // https://github.com/nomacs/nomacs/issues/1575
                                 "Larger than viewport, diff size, zoom out to viewport size",
                                 QSizeF(2400, 1800),
                                 QSizeF(2000, 1600),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->zoom(0.5, {-1, -1}, true);
                                 },
                             },
                             {
                                 "Larger than viewport, diff size, zoom in",
                                 QSizeF(2400, 1800),
                                 QSizeF(2000, 1600),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->zoom(1.5, {-1, -1}, true);
                                 },
                             },
                             {
                                 "Larger than viewport, diff size, zoom out with center",
                                 QSizeF(2400, 1800),
                                 QSizeF(2000, 1600),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->zoom(0.6, {400, 300}, true);
                                 },
                             },
                             {
                                 // https://github.com/nomacs/nomacs/issues/1575
                                 "Larger than viewport, diff size, zoom out to viewport size with center",
                                 QSizeF(2400, 1800),
                                 QSizeF(2000, 1600),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->zoom(0.5, {400, 300}, true);
                                 },
                             },
                             {
                                 "Larger than viewport, diff size, zoom in with center",
                                 QSizeF(2400, 1800),
                                 QSizeF(2000, 1600),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->zoom(1.5, {400, 300}, true);
                                 },
                             },
                             {
                                 "Larger than viewport, diff size, pan",
                                 QSizeF(2400, 1800),
                                 QSizeF(2000, 1600),
                                 [](DkViewPortTransformViewModel *vm) {
                                     vm->moveViewInWidgetCoords({100, 200});
                                 },
                             },
                         }));

}
