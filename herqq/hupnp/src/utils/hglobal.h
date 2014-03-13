/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HGLOBAL_H_
#define HGLOBAL_H_

#include <QtCore/QtGlobal>

#define H_DISABLE_ASSIGN(Class) \
    Class& operator=(const Class& clazz);

#define H_DISABLE_COPY(Class) \
    Class(const Class& clazz); \
    Class& operator=(const Class& clazz);

#define H_FORCE_SINGLETON(Class) \
private: \
    Class(const Class& clazz); \
    Class& operator=(const Class& clazz); \
    Class(); \
    ~Class();

#define H_DECLARE_PRIVATE(Class) \
    inline Class##Private* h_func() { return reinterpret_cast<Class##Private *>(h_ptr); } \
    inline const Class##Private* h_func() const { return reinterpret_cast<const Class##Private *>(h_ptr); } \
    friend class Class##Private;

#define H_DECLARE_PUBLIC(Class) \
    inline Class* q_func() { return static_cast<Class *>(q_ptr); } \
    inline const Class* q_func() const { return static_cast<const Class *>(q_ptr); } \
    friend class Class;

#define H_DECLARE_PARENT(Class) \
    inline Class* p_func() { return static_cast<Class *>(p_ptr); } \
    inline const Class* p_func() const { return static_cast<const Class *>(p_ptr); }

#define H_D(Class) Class##Private * const h = h_func()
#define H_Q(Class) Class * const q = q_func()
#define H_P(Class) Class * const p = p_func()
#define H_D_P(Class) Class * const p = h_func()->p_func()

#ifdef H_BUILD_STATIC
#define H_DECL_EXPORT
#define H_DECL_IMPORT
#endif

#ifndef H_DECL_EXPORT
#  ifdef Q_OS_WIN
#    define H_DECL_EXPORT __declspec(dllexport)
#  elif defined(QT_VISIBILITY_AVAILABLE)
#    define H_DECL_EXPORT __attribute__((visibility("default")))
#  endif
#  ifndef H_DECL_EXPORT
#    define H_DECL_EXPORT
#  endif
#endif
#ifndef H_DECL_IMPORT
#  if defined(Q_OS_WIN)
#    define H_DECL_IMPORT __declspec(dllimport)
#  else
#    define H_DECL_IMPORT
#  endif
#endif

#if defined(H_BUILD_UPNP_CORE_LIB)
#    define H_UPNP_CORE_EXPORT H_DECL_EXPORT
#else
#    define H_UPNP_CORE_EXPORT H_DECL_IMPORT
#endif
#if defined(H_BUILD_UPNP_LIGHTING_LIB)
#    define H_UPNP_LIGHTING_EXPORT H_DECL_EXPORT
#else
#    define H_UPNP_LIGHTING_EXPORT H_DECL_IMPORT
#endif
#if defined(H_BUILD_UPNP_AV_LIB)
#    define H_UPNP_AV_EXPORT H_DECL_EXPORT
#else
#    define H_UPNP_AV_EXPORT H_DECL_IMPORT
#endif

// ********************** IMPORTANT *****************************
// the following asserts are never undefined by the Herqq libraries ==>
// the purpose of them is just that; they are supposed to be always enabled,
// unlike Q_ASSERT, which is a no-op in release builds
#ifndef H_ASSERT
    #define H_ASSERT(cond) ((!(cond)) ? qt_assert(#cond,__FILE__,__LINE__) : qt_noop())
#endif

#ifndef H_ASSERT_X
    #define H_ASSERT_X(cond, where, what) ((!(cond)) ? qt_assert_x(where, what,__FILE__,__LINE__) : qt_noop())
#endif

#define STR(X) #X
#define STRX(X) STR(X)

#ifndef H_FUN
#define H_FUN __FUNCTION__
#endif

#define H_AT __FILE__ ":" STRX(__LINE__)

#endif /* HGLOBAL_H_ */
