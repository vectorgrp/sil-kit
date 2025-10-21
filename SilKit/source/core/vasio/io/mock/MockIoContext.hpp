// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "IIoContext.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <deque>
#include <functional>


namespace VSilKit {


struct MockIoContext : IIoContext
{
    MOCK_METHOD(void, Run, (), (override));

    MOCK_METHOD(void, Post, (std::function<void()>), (override));

    MOCK_METHOD(void, Dispatch, (std::function<void()>), (override));

    MOCK_METHOD(std::unique_ptr<IAcceptor>, MakeTcpAcceptor, (const std::string&, uint16_t), (override));

    MOCK_METHOD(std::unique_ptr<IAcceptor>, MakeLocalAcceptor, (const std::string&), (override));

    MOCK_METHOD(std::unique_ptr<IConnector>, MakeTcpConnector, (const std::string&, uint16_t), (override));

    MOCK_METHOD(std::unique_ptr<IConnector>, MakeLocalConnector, (const std::string&), (override));

    MOCK_METHOD(std::unique_ptr<ITimer>, MakeTimer, (), (override));

    MOCK_METHOD(std::vector<std::string>, Resolve, (const std::string&), (override));

    MOCK_METHOD(void, SetLogger, (SilKit::Services::Logging::ILogger&), (override));
};


/// IIoContext mock that provides actual implementations for the Run, Post, and Dispatch methods. The implementation is
/// not thread-safe, do not use it in multi-threaded tests.
struct MockIoContextWithExecutionQueue : IIoContext
{
    std::deque<std::function<void()>> handlerQueue;
    bool executingHandler{false};

    void Run() override
    {
        while (!handlerQueue.empty())
        {
            auto function{std::move(handlerQueue.front())};
            handlerQueue.pop_front();

            executingHandler = true;
            function();
            executingHandler = false;
        }
    }

    void Post(std::function<void()> function) override
    {
        handlerQueue.emplace_back(std::move(function));
    }

    void Dispatch(std::function<void()> function) override
    {
        if (executingHandler)
        {
            function();
        }
        else
        {
            Post(std::move(function));
        }
    }

    MOCK_METHOD(std::unique_ptr<IAcceptor>, MakeTcpAcceptor, (const std::string&, uint16_t), (override));

    MOCK_METHOD(std::unique_ptr<IAcceptor>, MakeLocalAcceptor, (const std::string&), (override));

    MOCK_METHOD(std::unique_ptr<IConnector>, MakeTcpConnector, (const std::string&, uint16_t), (override));

    MOCK_METHOD(std::unique_ptr<IConnector>, MakeLocalConnector, (const std::string&), (override));

    MOCK_METHOD(std::unique_ptr<ITimer>, MakeTimer, (), (override));

    MOCK_METHOD(std::vector<std::string>, Resolve, (const std::string&), (override));

    MOCK_METHOD(void, SetLogger, (SilKit::Services::Logging::ILogger&), (override));
};


} // namespace VSilKit
