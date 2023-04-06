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

#include <chrono>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ITimeProvider.hpp"
#include "SynchronizedHandlers.hpp"

namespace SilKit {
namespace Core {
namespace Tests {

class MockTimeProvider : public Services::Orchestration::ITimeProvider
{
public:
    MockTimeProvider()
    {
        ON_CALL(*this, TimeProviderName()).WillByDefault(testing::ReturnRef(_name));
        ON_CALL(*this, Now()).WillByDefault(testing::Return(now));
     }

    MOCK_METHOD(std::chrono::nanoseconds, Now, (), (override, const));
    MOCK_METHOD(const std::string&, TimeProviderName, (), (override, const));
    MOCK_METHOD(void, SetTime, (std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/), (override));
    MOCK_METHOD(void, ConfigureTimeProvider, (Services::Orchestration::TimeProviderKind timeProviderKind), (override));

    MOCK_METHOD(void, SetSynchronizeVirtualTime,(bool isSynchronizingVirtualTime), (override));
    MOCK_METHOD(bool, IsSynchronizingVirtualTime,(),(const, override));

    Services::HandlerId AddNextSimStepHandler(NextSimStepHandler handler) override
    {
        return _handlers.Add(std::move(handler));
    }

    void RemoveNextSimStepHandler(Services::HandlerId handlerId) override
    {
        _handlers.Remove(handlerId);
    }

    Util::SynchronizedHandlers<NextSimStepHandler> _handlers;
    const std::string _name = "MockTimeProvider";
    std::chrono::nanoseconds now{};
};

} // namespace Tests
} // namespace Core
} // namespace SilKit
