#include "../src/DkCore/DkImageStorage.h"
#include <benchmark/benchmark.h>

const int ROTATE_ANGLE = 0;
const auto IMAGE_PATH = "../ImageLounge/src/img/splash-screen.png";

static void BM_RotateImageFast(benchmark::State &state) {
  QImage img = QImage(IMAGE_PATH);
  QImage res{};
  for (auto _ : state) {
    res = nmc::rotateImageFast(img, ROTATE_ANGLE);
  }
}
BENCHMARK(BM_RotateImageFast);

static void BM_RotateImage(benchmark::State &state) {
  QImage img = QImage(IMAGE_PATH);
  QImage res{};
  for (auto _ : state) {
    res = nmc::rotateImage(img, ROTATE_ANGLE);
  }
}
BENCHMARK(BM_RotateImage);

BENCHMARK_MAIN();
