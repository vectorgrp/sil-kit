// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <tuple>

#include "conditional.hpp"

namespace SilKit {
namespace Util {
namespace tuple_tools {

/*! Apply a generic function to all members of a tuple
 *
 * If template parameter /Predicate/ is not unconditional, /func/ will only
 * be applied to tuple elements that have a type T fulfilling
 * Predicate<T>::value == true.
 *
 * Example:
 *  - Print all elements to std::cout assuming streaming overloads exist:
 *     for_each(tuple, [](auto&& elem) { std::cout << elem; });
 */
template <template <class...> class Predicate = unconditional, class Tuple, class Func>
inline void for_each(Tuple&& tuple, Func&& func);

// ================================================================================
//  Implementation
// ================================================================================
template <template <class...> class Predicate, class Tuple, class Func, std::size_t... I>
inline void for_each_impl(Tuple&& tuple, Func&& func, std::index_sequence<I...>)
{
    auto results = {invoke_if<Predicate<std::tuple_element_t<I, std::decay_t<Tuple>>>::value>(
        func, std::get<I>(std::forward<Tuple>(tuple)))...};
    (void)results;
}

template <template <class...> class Predicate, class Func, std::size_t... I>
inline void for_each_impl(std::tuple<>&, Func&&, std::index_sequence<I...>)
{
}

template <template <class...> class Predicate, class Func, std::size_t... I>
inline void for_each_impl(const std::tuple<>&, Func&&, std::index_sequence<I...>)
{
}

template <template <class...> class Predicate, class Tuple, class Func>
void for_each(Tuple&& tuple, Func&& func)
{
    using tuple_indexes = std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>;
    for_each_impl<Predicate>(std::forward<Tuple>(tuple), func, tuple_indexes{});
}

} // namespace tuple_tools
} // namespace Util
} // namespace SilKit
