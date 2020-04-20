// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ComAdapter.hpp"
#include "ComAdapter_impl.hpp"

namespace ib {
namespace mw {

template class ComAdapter<FastRtpsConnection>;
template class ComAdapter<VAsioConnection>;

} // namespace mw
} // namespace ib
