// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Config.hpp"

#include <memory>

#include "ib/IbMacros.hpp"

namespace ib {
namespace cfg {

template<class ParentT>
class ParentBuilder
{
public:
    ParentBuilder() = default;
    ParentBuilder(ParentT* parent);
    virtual ~ParentBuilder() = default;

    auto Parent() -> ParentT*;

private:
    ParentT* _parent{nullptr};
};

// ================================================================================
//  Inline Implementations
// ================================================================================
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

} // namespace cfg
} // namespace ib
