/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

namespace SilKit {
namespace Util {
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
} // namespace Util
} // namespace SilKit
