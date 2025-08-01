// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "IConnectionMethods.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace SilKit {
namespace Core {


struct MockConnectionMethods : IConnectionMethods
{
    MOCK_METHOD(std::unique_ptr<IConnectPeer>, MakeConnectPeer, (VAsioPeerInfo const &), (override));
    MOCK_METHOD(std::unique_ptr<SilKit::Core::IVAsioPeer>, MakeVAsioPeer, (std::unique_ptr<IRawByteStream>),
                (override));

    MOCK_METHOD(void, HandleConnectedPeer, (SilKit::Core::IVAsioPeer *), (override));
    MOCK_METHOD(void, AddPeer, (std::unique_ptr<SilKit::Core::IVAsioPeer>), (override));

    MOCK_METHOD(bool, TryRemoteConnectRequest, (VAsioPeerInfo const &), (override));
    MOCK_METHOD(bool, TryProxyConnect, (VAsioPeerInfo const &), (override));
};


} // namespace Core
} // namespace SilKit
