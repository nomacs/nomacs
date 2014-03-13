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

/* -----------------------------------------------------------------------------
 * -----------------------------PLEASE NOTE-------------------------------------
 * -----------------------------------------------------------------------------
 *
 * The generalized functor presented in this file is largely based on
 * the work of Mr. Andrei Alexandrescu. Below is the copyright notice and
 * permission to use found in the Loki library.
 */

////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2001 by Andrei Alexandrescu
// This code accompanies the book:
// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design
//     Patterns Applied". Copyright (c) 2001. Addison-Wesley.
// Permission to use, copy, modify, distribute and sell this software for any
//     purpose is hereby granted without fee, provided that the above copyright
//     notice appear in all copies and that both that copyright notice and this
//     permission notice appear in supporting documentation.
// The author or Addison-Wesley Longman make no representations about the
//     suitability of this software for any purpose. It is provided "as is"
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

/*
 * The functor template presented in this file is simpler (but not faster), less general and
 * less useful than the one Andrei has implemented in his Loki library, but it
 * suffices well for the purposes of Herqq libraries. In fact, it is _made_ for
 * Herqq libraries.
 *
 * For more information, check out the web page of the Loki library @
 * http://loki-lib.sourceforge.net/ and Andrei's book Modern C++ Design.
 */

#ifndef HFUNCTOR_H_
#define HFUNCTOR_H_

#include <QtCore/QtGlobal>

namespace Herqq
{

//
// \internal
//
struct EmptyType
{
};

//
// \internal
//
struct NullType
{
};

//
// \internal
//
template <class T, class U>
struct Typelist
{
   typedef T Head;
   typedef U Tail;
};

#define H_TYPELIST_1(T1) ::Herqq::Typelist<T1, NullType>

#define H_TYPELIST_2(T1, T2) ::Herqq::Typelist<T1, H_TYPELIST_1(T2) >

#define H_TYPELIST_3(T1, T2, T3) ::Herqq::Typelist<T1, H_TYPELIST_2(T2, T3) >

#define H_TYPELIST_4(T1, T2, T3, T4) ::Herqq::Typelist<T1, H_TYPELIST_3(T2, T3, T4) >

#define H_TYPELIST_5(T1, T2, T3, T4, T5) ::Herqq::Typelist<T1, H_TYPELIST_4(T2, T3, T4, T5) >

//
// \internal
//
template<typename R>
class FunctorImplBase
{
template<typename W, class TList>
friend class Functor;

private:

    virtual FunctorImplBase* doClone() const = 0;

    template <class T>
    static T* clone(T* pObj)
    {
        if (!pObj) return 0;
        T* pClone = static_cast<T*>(pObj->doClone());
        return pClone;
    }

public:

    typedef R ReturnValue;

    typedef EmptyType Parameter1;
    typedef EmptyType Parameter2;
    typedef EmptyType Parameter3;
    typedef EmptyType Parameter4;
    typedef EmptyType Parameter5;

    FunctorImplBase(){}
    virtual ~FunctorImplBase() {}
};

//
// \internal
//
template<typename R, class TList>
class FunctorImpl;

//
// \internal
//
template<typename R>
class FunctorImpl<R, NullType> :
    public FunctorImplBase<R>
{
public:

    typedef R ReturnValue;
    virtual R operator()() = 0;
};

//
// \internal
//
template<typename R, typename P1>
class FunctorImpl<R, H_TYPELIST_1(P1)> :
    public FunctorImplBase<R>
{
public:

    typedef R ReturnValue;
    typedef P1 Parameter1;
    virtual R operator()(Parameter1) = 0;
};

//
// \internal
//
template<typename R, typename P1, typename P2>
class FunctorImpl<R, H_TYPELIST_2(P1, P2)> :
    public FunctorImplBase<R>
{
public:

    typedef R ReturnValue;
    typedef P1 Parameter1;
    typedef P2 Parameter2;
    virtual R operator()(Parameter1, Parameter2) = 0;
};

//
// \internal
//
template<typename R, typename P1, typename P2, typename P3>
class FunctorImpl<R, H_TYPELIST_3(P1, P2, P3)> :
    public FunctorImplBase<R>
{
public:

    typedef R ReturnValue;
    typedef P1 Parameter1;
    typedef P2 Parameter2;
    typedef P3 Parameter3;
    virtual R operator()(Parameter1, Parameter2, Parameter3) = 0;
};

//
// \internal
//
template<typename R, typename P1, typename P2, typename P3, typename P4>
class FunctorImpl<R, H_TYPELIST_4(P1, P2, P3, P4)> :
    public FunctorImplBase<R>
{
public:

    typedef R ReturnValue;
    typedef P1 Parameter1;
    typedef P2 Parameter2;
    typedef P3 Parameter3;
    typedef P4 Parameter4;
    virtual R operator()(Parameter1, Parameter2, Parameter3, Parameter4) = 0;
};

//
// \internal
//
template<typename R, typename P1, typename P2, typename P3, typename P4,
         typename P5>
class FunctorImpl<R, H_TYPELIST_5(P1, P2, P3, P4, P5)> :
    public FunctorImplBase<R>
{
public:

    typedef R ReturnValue;
    typedef P1 Parameter1;
    typedef P2 Parameter2;
    typedef P3 Parameter3;
    typedef P4 Parameter4;
    typedef P5 Parameter5;
    virtual R operator()(Parameter1, Parameter2, Parameter3, Parameter4, Parameter5) = 0;
};

//
// \internal
//
template<class ParentFunctor, typename Fun>
class FunctorHandler :
    public ParentFunctor::Impl
{
private:

    typedef typename ParentFunctor::Impl Base;

    Fun m_fun;

    virtual FunctorHandler* doClone() const
    {
        return new FunctorHandler(m_fun);
    }

public:

    typedef typename Base::ReturnValue ReturnValue;
    typedef typename Base::Parameter1 Parameter1;
    typedef typename Base::Parameter2 Parameter2;
    typedef typename Base::Parameter3 Parameter3;
    typedef typename Base::Parameter4 Parameter4;
    typedef typename Base::Parameter5 Parameter5;

    FunctorHandler(const Fun& fun) :
        m_fun(fun)
    {
    }

    ReturnValue operator()()
    {
        return m_fun();
    }

    ReturnValue operator()(Parameter1 p1)
    {
        return m_fun(p1);
    }

    ReturnValue operator()(Parameter1 p1, Parameter2 p2)
    {
        return m_fun(p1, p2);
    }

    ReturnValue operator()(Parameter1 p1, Parameter2 p2, Parameter3 p3)
    {
        return m_fun(p1, p2, p3);
    }

    ReturnValue operator()(
        Parameter1 p1, Parameter2 p2, Parameter3 p3, Parameter4 p4)
    {
        return m_fun(p1, p2, p3, p4);
    }

    ReturnValue operator()(
        Parameter1 p1, Parameter2 p2, Parameter3 p3, Parameter4 p4, Parameter5 p5)
    {
        return m_fun(p1, p2, p3, p4, p5);
    }
};

//
// \internal
//
template<
    class ParentFunctor,
    typename PointerToObject, typename PointerToMemFun>
class FunctorMemFunHandler :
    public ParentFunctor::Impl
{
private:

    PointerToObject m_pobj;
    PointerToMemFun m_pmemf;

    typedef typename ParentFunctor::Impl Base;

    virtual FunctorMemFunHandler* doClone() const
    {
        return new FunctorMemFunHandler(m_pobj, m_pmemf);
    }

public:

    typedef typename Base::ReturnValue ReturnValue;
    typedef typename Base::Parameter1 Parameter1;
    typedef typename Base::Parameter2 Parameter2;
    typedef typename Base::Parameter3 Parameter3;
    typedef typename Base::Parameter4 Parameter4;
    typedef typename Base::Parameter5 Parameter5;

    FunctorMemFunHandler(const PointerToObject& pobj, PointerToMemFun pmemf) :
        m_pobj(pobj), m_pmemf(pmemf)
    {
    }

    ReturnValue operator()()
    {
        return ((*m_pobj).*m_pmemf)();
    }

    ReturnValue operator()(Parameter1 p1)
    {
        return ((*m_pobj).*m_pmemf)(p1);
    }

    ReturnValue operator()(Parameter1 p1, Parameter2 p2)
    {
        return ((*m_pobj).*m_pmemf)(p1, p2);
    }

    ReturnValue operator()(Parameter1 p1, Parameter2 p2, Parameter3 p3)
    {
        return ((*m_pobj).*m_pmemf)(p1, p2, p3);
    }

    ReturnValue operator()(
        Parameter1 p1, Parameter2 p2, Parameter3 p3, Parameter4 p4)
    {
        return ((*m_pobj).*m_pmemf)(p1, p2, p3, p4);
    }

    ReturnValue operator()(
        Parameter1 p1, Parameter2 p2, Parameter3 p3, Parameter4 p4, Parameter5 p5)
    {
        return ((*m_pobj).*m_pmemf)(p1, p2, p3, p4, p5);
    }
};

/*!
 * A template class for generalizing the <em>callable entity</em> concept.
 *
 * You can test if the object can be invoked simply by issuing
 * <tt>if (FunctorObject) { ... } </tt>
 */
template<typename ReturnValue = void, class TypeList = NullType>
class Functor
{
public:

    typedef FunctorImpl<ReturnValue, TypeList> Impl;
    typedef typename Impl::Parameter1 Parameter1;
    typedef typename Impl::Parameter2 Parameter2;
    typedef typename Impl::Parameter3 Parameter3;
    typedef typename Impl::Parameter4 Parameter4;
    typedef typename Impl::Parameter5 Parameter5;

    typedef ReturnValue (*FunType)();

    typedef ReturnValue (*FunType1)(typename Impl::Parameter1);

    typedef ReturnValue (*FunType2)(
        typename Impl::Parameter1, typename Impl::Parameter2);

    typedef ReturnValue (*FunType3)(
        typename Impl::Parameter1, typename Impl::Parameter2,
        typename Impl::Parameter3);

    typedef ReturnValue (*FunType4)(
        typename Impl::Parameter1, typename Impl::Parameter2,
        typename Impl::Parameter3, typename Impl::Parameter4);

    typedef ReturnValue (*FunType5)(
        typename Impl::Parameter1, typename Impl::Parameter2,
        typename Impl::Parameter3, typename Impl::Parameter4,
        typename Impl::Parameter5);

private:

    Impl* m_impl;

public:

    Functor() : m_impl(0)
    {
    }

    template<typename Fun>
    Functor(const Fun& fun) :
        m_impl(new FunctorHandler<Functor, Fun>(fun))
    {
    }

    Functor(const FunType& fun) :
        m_impl(new FunctorHandler<Functor, FunType>(fun))
    {
    }

    Functor(const FunType1& fun) :
        m_impl(new FunctorHandler<Functor, FunType1>(fun))
    {
    }

    Functor(const FunType2& fun) :
        m_impl(new FunctorHandler<Functor, FunType2>(fun))
    {
    }

    Functor(const FunType3& fun) :
        m_impl(new FunctorHandler<Functor, FunType3>(fun))
    {
    }

    Functor(const FunType4& fun) :
        m_impl(new FunctorHandler<Functor, FunType4>(fun))
    {
    }

    Functor(const FunType5& fun) :
        m_impl(new FunctorHandler<Functor, FunType5>(fun))
    {
    }

    template<class PointerToObject, typename PointerToMemFun>
    Functor(const PointerToObject& pobj, PointerToMemFun pmemfun) :
        m_impl(new FunctorMemFunHandler<Functor, PointerToObject, PointerToMemFun>(pobj, pmemfun))
    {
    }

    ~Functor() { delete m_impl; }

    Functor& operator=(const Functor& other)
    {
        Q_ASSERT(&other != this);

        Impl* newImpl = other.m_impl ? Impl::clone(other.m_impl) : 0;
        delete m_impl;
        m_impl = newImpl;

        return *this;
    }

    Functor(const Functor& other) :
        m_impl(0)
    {
        Q_ASSERT(&other != this);
        m_impl = other.m_impl ? Impl::clone(other.m_impl) : 0;
    }

    ReturnValue operator()()
    {
        Q_ASSERT(m_impl);
        return (*m_impl)();
    }

    ReturnValue operator()(Parameter1 p1)
    {
        Q_ASSERT(m_impl);
        return (*m_impl)(p1);
    }

    ReturnValue operator()(Parameter1 p1, Parameter2 p2)
    {
        Q_ASSERT(m_impl);
        return (*m_impl)(p1, p2);
    }

    ReturnValue operator()(Parameter1 p1, Parameter2 p2, Parameter3 p3)
    {
        Q_ASSERT(m_impl);
        return (*m_impl)(p1, p2, p3);
    }

    ReturnValue operator()(
        Parameter1 p1, Parameter2 p2, Parameter3 p3, Parameter4 p4)
    {
        Q_ASSERT(m_impl);
        return (*m_impl)(p1, p2, p3, p4);
    }

    ReturnValue operator()(
        Parameter1 p1, Parameter2 p2, Parameter3 p3, Parameter4 p4, Parameter5 p5)
    {
        Q_ASSERT(m_impl);
        return (*m_impl)(p1, p2, p3, p4, p5);
    }

    bool operator!() const
    {
        return !m_impl;
    }

private:

    // Helper for enabling 'if (sp)'
    struct Tester
    {
        Tester(int) {}
        void dummy() {}
    };

    typedef void (Tester::*unspecified_boolean_type_)();

public:

    // enable 'if (sp)'
    operator unspecified_boolean_type_() const
    {
        return !m_impl ? 0 : &Tester::dummy;
    }
};

}

#endif /* HFUNCTOR_H_ */
