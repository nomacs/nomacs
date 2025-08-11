#pragma once

//
// precompiled header for nomacs
//
// speed up development builds by compiling
// and inserting this header before all other headers
//
// for fast builds must also disable plugins and use Debug target
//

// test if its working
// #error pch

// this list of headers is by no means optimal for all cases
// if too many headers go here then the build slows down

#if 1

// DkUtils.h / DkSettings.h / DkFileInfo.h
#include <QBitArray>
#include <QColor>
#include <QDate>
#include <QDebug>
#include <QSettings>
#include <QVector>

// DkBaseWidgets.h
#include <QDockWidget>
#include <QLabel>
#include <QScrollArea>
#include <QTimer>
#include <QWidget>

// DkImageContainer.h
#include <QFutureWatcher>

// Other high frequency in headers / sources
#include <QAction>
#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QImage>
#include <QSharedPointer>

#ifdef WITH_OPENCV
// #include "opencv2/core/core.hpp"
#endif

#include <cmath>
#include <functional>
#include <memory>

#endif
