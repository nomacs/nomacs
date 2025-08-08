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

// this was built mainly by looking at header used by a lot of files
#if 1

// DkUtils.h / DkSettings.h
#include <functional>
#include <memory>

#ifdef WITH_OPENCV
#ifdef Q_OS_WIN
#pragma warning(disable : 4996)
#endif

#include "opencv2/core/core.hpp"
#endif // WITH_OPENCV

#include <QAction>
#include <QApplication>
#include <QBitArray>
#include <QColor>
#include <QDate>
#include <QDebug>
#include <QFileInfo>
#include <QImage>
#include <QMap>
#include <QObject>
#include <QRegularExpression>
#include <QSettings>
#include <QSharedMemory>
#include <QSharedPointer>
#include <QStringList>
#include <QVector>

// DkBaseWidgets.h
#include <QDockWidget>
#include <QLabel>
#include <QScrollArea>
#include <QTimer>
#include <QWidget>

#endif
