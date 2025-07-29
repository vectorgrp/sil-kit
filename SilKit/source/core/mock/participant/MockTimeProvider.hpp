// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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

    MOCK_METHOD(void, SetSynchronizeVirtualTime, (bool isSynchronizingVirtualTime), (override));
    MOCK_METHOD(bool, IsSynchronizingVirtualTime, (), (const, override));

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
