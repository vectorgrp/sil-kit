// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ComAdapter.hpp"
#include "ComAdapter_impl.hpp"

namespace ib {
namespace mw {
#if defined(IB_MW_HAVE_FASTRTPS)
template class ComAdapter<FastRtpsConnection>;
#endif
#if defined(IB_MW_HAVE_VASIO)
template class ComAdapter<VAsioConnection>;
#endif

} // namespace mw
} // namespace ib
