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

#include <tuple>

namespace SilKit {
namespace Util {
namespace tuple_tools {

/*! Retrieve a tuple element identified by a boolean predicate.
 *
 *  predicative_get<Predicate>(tuple); returns a reference to the
 *  first tuple element for which Predicate<T>::value is true.
 *  If no such element exists, predicative_get<Predicate>(tuple)
 *  returns void.
 */
template<template <class> class Predicate, class Head, class... Tail>
constexpr auto& predicative_get(std::tuple<Head, Tail...>& tuple);

/*! Retrieve a tuple element identified by a boolean predicate.
 *
 *  predicative_get<Predicate>(tuple); returns a const reference to
 *  the first tuple element for which Predicate<T>::value is true.  If
 *  no such element exists, predicative_get<Predicate>(tuple) returns
 *  void.
 */
template<template <class> class Predicate, class Head, class... Tail>
constexpr const auto& predicative_get(const std::tuple<Head, Tail...>& tuple);


// ================================================================================
//  Implementation
// ================================================================================

// Primary template that also provides the terminal void get() method
// that is called in the case that no element matches the predicate
template<class Tuple, class IdxSeq, template<class> class Predicate, bool>
struct tuple_get
{
    static void get(const Tuple&) {}
};

// recurse if the current index does not fulfill the predicate
template<class Tuple, size_t I, size_t II, size_t... Tail, template<class> class Predicate>
struct tuple_get<Tuple, std::index_sequence<I, II, Tail...>, Predicate, false>
    :  tuple_get<Tuple, std::index_sequence<II, Tail...>, Predicate, Predicate<typename std::tuple_element_t<II, Tuple>>::value>
{
};

// Getters for the matching predicate
template<class Tuple, size_t I, size_t... Tail, template<class> class Predicate>
struct tuple_get<Tuple, std::index_sequence<I, Tail...>, Predicate, true>
{
    using HeadT = typename std::tuple_element_t<I, Tuple>;

    constexpr static auto get(Tuple& tuple) -> HeadT&
    {
        return std::get<I>(tuple);
    }
    constexpr static auto get(const Tuple& tuple) -> const HeadT&
    {
        return std::get<I>(tuple);
    }
};

// Implementations of the convenience helper methods
template<template <class> class Predicate, class Head, class... Tail>
constexpr auto& predicative_get(std::tuple<Head, Tail...>& tuple)
{
    using Tuple = std::tuple<Head, Tail...>;
    using tuple_indexes = std::make_index_sequence<std::tuple_size<Tuple>::value>;

    return tuple_get<Tuple, tuple_indexes, Predicate, Predicate<Head>::value>::get(tuple);
}

template<template <class> class Predicate, class Head, class... Tail>
constexpr const auto& predicative_get(const std::tuple<Head, Tail...>& tuple)
{
    using Tuple = std::tuple<Head, Tail...>;
    using tuple_indexes = std::make_index_sequence<std::tuple_size<Tuple>::value>;

    return tuple_get<Tuple, tuple_indexes, Predicate, Predicate<Head>::value>::get(tuple);
}


} // namespace tuple_tools
} // namespace Util
} // namespace SilKit
