// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

namespace SilKit {
namespace Util {
namespace tuple_tools {

template <class T>
using unconditional = std::true_type;

template <bool predicate, class Func, class... Arg>
constexpr bool invoke_if(Func&& func, Arg&&... element);

// ================================================================================
//  Implementation
// ================================================================================
template <bool = false>
struct conditional
{
    template <class Func, class... Arg>
    constexpr static bool invoke(Func&&, Arg&&...)
    {
        return false;
    }
};

template <>
struct conditional<true>
{
    template <class Func, class... Arg>
    constexpr static bool invoke(Func&& func, Arg&&... arg)
    {
        func(std::forward<Arg>(arg)...);
        return true;
    }
};

template <bool predicate, class Func, class... Arg>
constexpr bool invoke_if(Func&& func, Arg&&... element)
{
    return conditional<predicate>::invoke(std::forward<Func>(func), std::forward<Arg>(element)...);
}

} // namespace tuple_tools
} // namespace Util
} // namespace SilKit
