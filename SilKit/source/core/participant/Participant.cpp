// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/participant/Participant.hpp"
#include "core/participant/Participant_impl.hpp"

namespace SilKit {
namespace Core {

template class Participant<VAsioConnection>;

} // namespace Core
} // namespace SilKit
