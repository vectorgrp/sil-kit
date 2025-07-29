// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <functional>
#include <set>
#include <string>

#include "gmock/gmock.h"

#include "IServiceEndpoint.hpp"
#include "ServiceDiscovery.hpp"

namespace SilKit {
namespace Core {
namespace Tests {
using namespace SilKit::Core::Discovery;
using namespace testing;

class MockServiceEndpoint : public SilKit::Core::IServiceEndpoint
{
public:
    MockServiceEndpoint(std::string participantName, std::string networkName, std::string serviceName,
                        EndpointId serviceId = 0)
        : _serviceDescriptor{participantName, networkName, serviceName, serviceId}
    {
        ON_CALL(*this, GetServiceDescriptor()).WillByDefault(ReturnRef(_serviceDescriptor));
    }
    MOCK_METHOD(void, SetServiceDescriptor, (const ServiceDescriptor&), (override));
    MOCK_METHOD(const ServiceDescriptor&, GetServiceDescriptor, (), (const, override));

public:
    ServiceDescriptor _serviceDescriptor;
};


} // namespace Tests
} // namespace Core
} // namespace SilKit
