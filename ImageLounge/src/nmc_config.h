
#pragma once
//
// Build configuration that cannot go elsewhere
//
// To prevent issues, we have some restrictions:
// - Include only in Dk*.h header files
// - Include after all other headers
// - Omit if there is another Dk*.h header present
// - Do not use to disable compiler warnings, try cmake first, then #pragma
//
#include <qcompilerdetection.h>

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif
