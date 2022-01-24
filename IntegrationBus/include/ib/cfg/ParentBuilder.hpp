// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Config.hpp"

#include <memory>

#include "ib/IbMacros.hpp"

namespace ib {
namespace cfg {
inline namespace deprecated {

template<class ParentT>
class ParentBuilder
{
public:
    ParentBuilder() = delete;
    ParentBuilder(const ParentBuilder<ParentT>&) = delete;
    ParentBuilder operator=(const ParentBuilder<ParentT>&) = delete;
    ParentBuilder(ParentBuilder<ParentT>&&) = default;
    ParentBuilder(ParentT* parent);
    virtual ~ParentBuilder();

    auto Parent() -> ParentT*;

private:
    ParentT* _parent{nullptr};
};

// ================================================================================
//  Inline Implementations
// ================================================================================


template<class ParentT>
ParentBuilder<ParentT>::~ParentBuilder()
{
    _parent = nullptr;
}

template<class ParentT>
ParentBuilder<ParentT>::ParentBuilder(ParentT* parent)
    : _parent{parent}
{
}

template<class ParentT>
auto ParentBuilder<ParentT>::Parent() -> ParentT*
{
    return _parent;
}

} // namespace deprecated
} // namespace cfg
} // namespace ib
