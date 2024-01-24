#pragma once

#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define DkEnterEvent QEvent
#else
#define DkEnterEvent QEnterEvent
#endif
