// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <type_traits>


namespace SilKit {
namespace Util {
// ==================================================================
//  Workaround for C++14 (Helper Type Alias)
// ==================================================================

// Implements std::void_t from C++17
template <typename...>
struct MakeVoidT
{
    using type = void;
};
template <typename... Ts>
using VoidT = typename MakeVoidT<Ts...>::type;

} // namespace Util
} // namespace SilKit
