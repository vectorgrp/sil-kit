// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

namespace SilKit {
namespace Util {
namespace tuple_tools {

template <template <class> class TemplateT, class TupleT>
struct wrapped_tuple_helper;

template <template <class> class TemplateT, class... TArg>
struct wrapped_tuple_helper<TemplateT, std::tuple<TArg...>>
{
    using Type = std::tuple<TemplateT<TArg>...>;
};

template <template <class> class TemplateT, class TupleT>
using wrapped_tuple = typename wrapped_tuple_helper<TemplateT, TupleT>::Type;

} // namespace tuple_tools
} // namespace Util
} // namespace SilKit
