/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
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
