// Copyright (c) 2022 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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

    MOCK_METHOD(std::unique_ptr<IAcceptor>, MakeTcpAcceptor, (std::string const&, uint16_t), (override));

    MOCK_METHOD(std::unique_ptr<IAcceptor>, MakeLocalAcceptor, (std::string const&), (override));

    MOCK_METHOD(std::unique_ptr<IConnector>, MakeTcpConnector, (std::string const&, uint16_t), (override));

    MOCK_METHOD(std::unique_ptr<IConnector>, MakeLocalConnector, (std::string const&), (override));

    MOCK_METHOD(std::unique_ptr<ITimer>, MakeTimer, (), (override));

    MOCK_METHOD(std::vector<std::string>, Resolve, (std::string const&), (override));

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

    MOCK_METHOD(std::unique_ptr<IAcceptor>, MakeTcpAcceptor, (std::string const&, uint16_t), (override));

    MOCK_METHOD(std::unique_ptr<IAcceptor>, MakeLocalAcceptor, (std::string const&), (override));

    MOCK_METHOD(std::unique_ptr<IConnector>, MakeTcpConnector, (std::string const&, uint16_t), (override));

    MOCK_METHOD(std::unique_ptr<IConnector>, MakeLocalConnector, (std::string const&), (override));

    MOCK_METHOD(std::unique_ptr<ITimer>, MakeTimer, (), (override));

    MOCK_METHOD(std::vector<std::string>, Resolve, (std::string const&), (override));

    MOCK_METHOD(void, SetLogger, (SilKit::Services::Logging::ILogger&), (override));
};


} // namespace VSilKit
