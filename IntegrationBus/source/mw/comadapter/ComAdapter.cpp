// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ComAdapter.hpp"
#include "ComAdapter_impl.hpp"

namespace ib {
namespace mw {
#if defined(IB_MW_HAVE_FASTRTPS)
template class ComAdapter<FastRtpsConnection>;
#endif
template class ComAdapter<VAsioConnection>;

} // namespace mw
} // namespace ib
