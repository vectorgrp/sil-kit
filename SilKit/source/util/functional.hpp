// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>

namespace SilKit {
namespace Util {

template <class C, typename RT, typename... Arg>
auto bind_method(C* classPtr, RT (C::*method)(Arg...)) -> std::function<RT(Arg...)>
{
    return [classPtr, method](Arg... arg) { return (classPtr->*method)(std::forward<Arg>(arg)...); };
}

} // namespace Util
} // namespace SilKit
