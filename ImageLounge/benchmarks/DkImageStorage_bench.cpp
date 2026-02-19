#include "../src/DkCore/DkImageStorage.h"
#include <benchmark/benchmark.h>

static void BM_RotateImage(benchmark::State &state)
{
    int cacheLevel = state.range(0);
    int bytesPerPixel = 4;
    auto &cpu = benchmark::CPUInfo::Get();
    int cacheSize = 0;
    if (cacheLevel < 4) {
        for (auto cache : cpu.caches) {
            if (cache.level == cacheLevel) {
                cacheSize = cache.size;
                break;
            }
        }
    }
    if (cacheLevel == 4) {
        cacheSize = 128 * 1024 * 1024;
    }
    if (cacheSize == 0) {
        state.SkipWithMessage("No CPU cache at this level?"); // no L3 cache, probably
    }

    int dim = sqrt(cacheSize / bytesPerPixel);
    QImage img{dim, dim, QImage::Format_ARGB32};

    state.SetLabel(QString().asprintf("%dKB %dx%d", cacheSize / 1024, dim, dim).toStdString());

    double angle = state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(nmc::DkImage::rotateImage(img, angle));
    }
}
BENCHMARK(BM_RotateImage)
    // L1 Cache
    ->Args({1, 0})
    ->Args({1, 90})
    ->Args({1, 180})
    ->Args({1, 270})
    ->Args({1, 45})
    ->Args({1, 135})
    ->Args({1, 225})
    ->Args({1, 315})
    // L2 Cache
    ->Args({2, 0})
    ->Args({2, 90})
    ->Args({2, 180})
    ->Args({2, 270})
    ->Args({2, 45})
    ->Args({2, 135})
    ->Args({2, 225})
    ->Args({2, 315})
    // L3 cache
    ->Args({3, 0})
    ->Args({3, 90})
    ->Args({3, 180})
    ->Args({3, 270})
    ->Args({3, 45})
    ->Args({3, 135})
    ->Args({3, 225})
    ->Args({3, 315})
    // Main Memory
    ->Args({4, 0})
    ->Args({4, 90})
    ->Args({4, 180})
    ->Args({4, 270})
    ->Args({4, 45})
    ->Args({4, 135})
    ->Args({4, 225})
    ->Args({4, 315});

BENCHMARK_MAIN();
