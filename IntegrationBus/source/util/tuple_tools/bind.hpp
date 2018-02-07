// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

namespace ib {
namespace util {
namespace tuple_tools {

/*! Helper to reduce the number of template arguments of a class template
 *
 * Bind the /first/ Arg... template arguments of class template Ctmp
 * and declare a new class template as /type/ that has sizeof...(Arg)
 * fewer template arguments.
 */
template<template<class...> class Ctmp, class... Arg>
struct bind
{
    template<class T>
    using type = Ctmp<Arg..., T>;
};

/*! Helper to reduce the number of template arguments of a class template
 *
 * Bind the /last/ Arg... template arguments of class template Ctmp
 * and declare a new class template as /type/ that has sizeof...(Arg)
 * fewer template arguments.
 */
template<template<class...> class Ctmp, class... Arg>
struct rbind
{
    template<class T>
    using type = Ctmp<T, Arg...>;
};


} // namespace tuple_tools
} // namespace util
} // namespace ib
