// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "IAcceptor.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace VSilKit {


struct MockAcceptor : IAcceptor
{
    MOCK_METHOD(void, SetListener, (IAcceptorListener&), (override));
    MOCK_METHOD(std::string, GetLocalEndpoint, (), (const, override));
    MOCK_METHOD(void, AsyncAccept, (std::chrono::milliseconds), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
};


struct MockAcceptorListener : IAcceptorListener
{
    MOCK_METHOD(void, OnAsyncAcceptSuccess, (IAcceptor&, std::unique_ptr<IRawByteStream>), (override));
    MOCK_METHOD(void, OnAsyncAcceptFailure, (IAcceptor&), (override));
};


} // namespace VSilKit
