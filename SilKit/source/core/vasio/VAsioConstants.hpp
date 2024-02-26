// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT


#include "EndpointAddress.hpp"


namespace VSilKit {

constexpr const SilKit::Core::ParticipantId REGISTRY_PARTICIPANT_ID{0};
constexpr const char * REGISTRY_PARTICIPANT_NAME{"SilKitRegistry"};

} // namespace VSilKit


namespace SilKit {
namespace Core {

using VSilKit::REGISTRY_PARTICIPANT_ID;
using VSilKit::REGISTRY_PARTICIPANT_NAME;

} // namespace Core
} // namespace SilKit