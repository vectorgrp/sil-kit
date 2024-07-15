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

#include "gmock/gmock-function-mocker.h"
#include "ISilKitEventHandler.hpp"

namespace SilKit {
namespace Dashboard {

class MockSilKitEventHandler : public ISilKitEventHandler
{
public:
    MOCK_METHOD(uint64_t, OnSimulationStart, (const std::string&, uint64_t), (override));
    MOCK_METHOD(void, OnSimulationEnd, (uint64_t, uint64_t), (override));
    MOCK_METHOD(void, OnParticipantConnected,
                (uint64_t, const Services::Orchestration::ParticipantConnectionInformation&), (override));
    MOCK_METHOD(void, OnParticipantStatusChanged, (uint64_t, const Services::Orchestration::ParticipantStatus&),
                (override));
    MOCK_METHOD(void, OnSystemStateChanged, (uint64_t, Services::Orchestration::SystemState), (override));
    MOCK_METHOD(void, OnServiceDiscoveryEvent,
                (uint64_t, Core::Discovery::ServiceDiscoveryEvent::Type, const Core::ServiceDescriptor&), (override));
    MOCK_METHOD(void, OnBulkUpdate, (uint64_t, const DashboardBulkUpdate&), (override));
};

} // namespace Dashboard
} // namespace SilKit