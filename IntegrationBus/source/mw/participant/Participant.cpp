// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Participant.hpp"
#include "Participant_impl.hpp"

namespace ib {
namespace mw {
#if defined(IB_MW_HAVE_VASIO)
template class Participant<VAsioConnection>;
#endif

} // namespace mw
} // namespace ib
