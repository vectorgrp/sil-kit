// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "core/vasio/mock/MockVAsioPeer.hpp"

namespace SilKit {
namespace Core {

//! A nice mock peer that remembers its info, simulation name and service descriptor.
struct FakeVAsioPeer : testing::NiceMock<MockVAsioPeer>
{
    VAsioPeerInfo _peerInfo;
    ServiceDescriptor _serviceDescriptor;
    std::string _simulationName;

    FakeVAsioPeer(const std::string& participantName, const std::string& simulationName)
        : _simulationName{simulationName}
    {
        _peerInfo.participantName = participantName;
        _peerInfo.participantId = 1234;
        _peerInfo.acceptorUris.push_back("tcp://127.0.0.1:1234");

        _serviceDescriptor.SetParticipantNameAndComputeId(participantName);
        _serviceDescriptor.SetSimulationName(simulationName);

        ON_CALL(*this, GetLocalAddress()).WillByDefault(testing::Return("tcp://127.0.0.1:1234"));
        ON_CALL(*this, GetRemoteAddress()).WillByDefault(testing::Return("tcp://127.0.0.1:1234"));
        ON_CALL(*this, GetSimulationName()).WillByDefault(testing::ReturnRef(_simulationName));
        ON_CALL(*this, GetInfo()).WillByDefault(testing::ReturnRef(_peerInfo));
        ON_CALL(*this, GetServiceDescriptor()).WillByDefault(testing::ReturnRef(_serviceDescriptor));
        ON_CALL(*this, SetServiceDescriptor(testing::_))
            .WillByDefault([this](const ServiceDescriptor& descriptor) { _serviceDescriptor = descriptor; });
        ON_CALL(*this, GetProtocolVersion()).WillByDefault(testing::Return(CurrentProtocolVersion()));
    }
};

} // namespace Core
} // namespace SilKit
