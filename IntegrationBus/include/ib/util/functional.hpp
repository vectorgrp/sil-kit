// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>

namespace ib {
namespace util {

template<class C, typename RT, typename... Arg>
auto bind_method(C* classPtr, RT(C::*method)(Arg...)) -> std::function<RT(Arg...)>
{
    return [classPtr, method](Arg... arg)
           {
               return (classPtr->*method)(std::forward<Arg>(arg)...);
           };
}

} // namespace util
} // namespace ib
