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

#include "TransformAcceptorUris.hpp"

#include "silkit/participant/exception.hpp"

#include "gtest/gtest.h"

#include <chrono>
#include <iostream>
#include <set>

using namespace SilKit::Core;

namespace {

inline void ExpectIp4(const UriInfo& hostInfo)
{
    EXPECT_TRUE(hostInfo.ip4);
    EXPECT_FALSE(hostInfo.ip6);
}

inline void ExpectIp6(const UriInfo& hostInfo)
{
    EXPECT_FALSE(hostInfo.ip4);
    EXPECT_TRUE(hostInfo.ip6);
}

inline void ExpectCatchall(const UriInfo& hostInfo)
{
    EXPECT_FALSE(hostInfo.loopbackIp);
    EXPECT_TRUE(hostInfo.catchallIp);
}

inline void ExpectLoopback(const UriInfo& hostInfo)
{
    EXPECT_TRUE(hostInfo.loopbackIp);
    EXPECT_FALSE(hostInfo.catchallIp);
}

inline void ExpectStandardAddress(const UriInfo& hostInfo)
{
    EXPECT_FALSE(hostInfo.loopbackIp);
    EXPECT_FALSE(hostInfo.catchallIp);
}

TEST(Test_TransformAcceptorUris, GetUriInfo)
{
    // Domain Name

    {
        const auto hostInfo = GetUriInfo(Uri::Parse("tcp://registry.example.com:45678"));
        EXPECT_FALSE(hostInfo.ip4);
        EXPECT_FALSE(hostInfo.ip6);
        EXPECT_FALSE(hostInfo.loopbackIp);
        EXPECT_FALSE(hostInfo.catchallIp);
    }

    // IPv4

    {
        const auto hostInfo = GetUriInfo(Uri::Parse("tcp://0.0.0.0:45678"));
        ExpectIp4(hostInfo);
        ExpectCatchall(hostInfo);
    }

    for (const auto& uriString : {"tcp://127.0.0.1:45678", "tcp://127.1.2.3:45678"})
    {
        const auto hostInfo = GetUriInfo(Uri::Parse(uriString));
        ExpectIp4(hostInfo);
        ExpectLoopback(hostInfo);
    }

    for (const auto& uriString : {"tcp://1.2.3.4:45678", "tcp://4.3.2.1:45678"})
    {
        const auto hostInfo = GetUriInfo(Uri::Parse(uriString));
        ExpectIp4(hostInfo);
        ExpectStandardAddress(hostInfo);
    }

    // IPv6

    for (const auto& uriString : {"tcp://[::]:45678", "tcp://[0::0]:45678", "tcp://[::0]:45678", "tcp://[0::]:45678",
                                  "tcp://[00:000:0::000]:45678"})
    {
        const auto hostInfo = GetUriInfo(Uri::Parse(uriString));
        ExpectIp6(hostInfo);
        ExpectCatchall(hostInfo);
    }

    for (const auto& uriString : {"tcp://[::1]:45678", "tcp://[0::1]:45678", "tcp://[00:000:0::00:001]:45678"})
    {
        const auto hostInfo = GetUriInfo(Uri::Parse(uriString));
        ExpectIp6(hostInfo);
        ExpectLoopback(hostInfo);
    }

    for (const auto& uriString : {"tcp://[1234:5678:90ab:cdef:1234:5678:90ab:cdef]:45678", "tcp://[1234::cdef]:45678",
                                  "tcp://[1::2]:45678", "tcp://[::2]:45678"})
    {
        const auto hostInfo = GetUriInfo(Uri::Parse(uriString));
        ExpectIp6(hostInfo);
        ExpectStandardAddress(hostInfo);
    }
}

struct MethodNotImplementedError : std::exception
{
    char const* what() const noexcept override
    {
        return "method not implemented";
    }
};

struct DummyVAsioPeerBase : IVAsioPeer
{
    void SendSilKitMsg(SerializedMessage) final
    {
        throw MethodNotImplementedError{};
    }

    void Subscribe(VAsioMsgSubscriber) final
    {
        throw MethodNotImplementedError{};
    }

    void SetInfo(VAsioPeerInfo) final
    {
        throw MethodNotImplementedError{};
    }

    void StartAsyncRead() final
    {
        throw MethodNotImplementedError{};
    }

    void Shutdown() final
    {
        throw MethodNotImplementedError{};
    }

    void EnableAggregation() final
    {
        throw MethodNotImplementedError{};
    }

    void SetProtocolVersion(ProtocolVersion) final
    {
        throw MethodNotImplementedError{};
    }

    auto GetProtocolVersion() const -> ProtocolVersion final
    {
        throw MethodNotImplementedError{};
    }

    void SetSimulationName(const std::string&) final
    {
        throw MethodNotImplementedError{};
    }

    auto GetSimulationName() const -> const std::string& final
    {
        throw MethodNotImplementedError{};
    }

    // IServiceEndpoint

    void SetServiceDescriptor(const ServiceDescriptor&) override
    {
        throw MethodNotImplementedError{};
    }

    auto GetServiceDescriptor() const -> const ServiceDescriptor& override
    {
        throw MethodNotImplementedError{};
    }
};

struct AdvertisedVAsioPeer final : DummyVAsioPeerBase
{
    VAsioPeerInfo peerInfo;
    std::string remoteAddress;

    auto GetInfo() const -> const VAsioPeerInfo& override
    {
        return peerInfo;
    }

    auto GetRemoteAddress() const -> std::string override
    {
        return remoteAddress;
    }

    auto GetLocalAddress() const -> std::string override
    {
        throw MethodNotImplementedError{};
    }
};

struct AudienceVAsioPeer final : DummyVAsioPeerBase
{
    VAsioPeerInfo peerInfo;
    std::string localAddress;

    auto GetInfo() const -> const VAsioPeerInfo& override
    {
        return peerInfo;
    }

    auto GetRemoteAddress() const -> std::string override
    {
        throw MethodNotImplementedError{};
    }

    auto GetLocalAddress() const -> std::string override
    {
        return localAddress;
    }
};

struct DummyLogger : SilKit::Services::Logging::ILogger
{
    void Log(SilKit::Services::Logging::Level level, std::string const& msg) override
    {
        const auto now = std::chrono::steady_clock::now();
        std::cout << "[DummyLogger:" << level << ":"
                  << std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count() << "] " << msg
                  << std::endl;
    }

    void Trace(std::string const& msg) override
    {
        Log(SilKit::Services::Logging::Level::Trace, msg);
    }

    void Debug(std::string const& msg) override
    {
        Log(SilKit::Services::Logging::Level::Debug, msg);
    }

    void Info(std::string const& msg) override
    {
        Log(SilKit::Services::Logging::Level::Info, msg);
    }

    void Warn(std::string const& msg) override
    {
        Log(SilKit::Services::Logging::Level::Warn, msg);
    }

    void Error(std::string const& msg) override
    {
        Log(SilKit::Services::Logging::Level::Error, msg);
    }

    void Critical(std::string const& msg) override
    {
        Log(SilKit::Services::Logging::Level::Critical, msg);
    }

    SilKit::Services::Logging::Level GetLogLevel() const override
    {
        return SilKit::Services::Logging::Level::Off;
    }
};

void RunTest(AdvertisedVAsioPeer advertised, AudienceVAsioPeer audience, std::set<std::string> expectedAcceptorUris)
{
    DummyLogger logger;

    advertised.peerInfo.participantName = "Advertised";
    audience.peerInfo.participantName = "Audience";

    const auto acceptorUris = TransformAcceptorUris(&logger, &advertised, &audience);

    // Ensure that the number of transformed URIs is the same as the number of expected URIs
    EXPECT_EQ(acceptorUris.size(), expectedAcceptorUris.size());

    for (const auto& uri : acceptorUris)
    {
        EXPECT_NE(expectedAcceptorUris.find(uri), expectedAcceptorUris.end())
            << "Unexpected Acceptor-URI '" << uri << "' returned";
    }

    for (const auto& uri : expectedAcceptorUris)
    {
        EXPECT_NE(std::find(acceptorUris.begin(), acceptorUris.end(), uri), acceptorUris.end())
            << "Expected Acceptor-URI '" << uri << "' not returned";
    }
}

TEST(Test_TransformAcceptorUris, Advertised_ViaLocal_ViaLocal_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "local://advertised.registry.silkit";

    AudienceVAsioPeer audience;
    audience.localAddress = "local://audience.silkit";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://127.0.0.1:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[::1]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp4Loopback_ViaLocal_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://127.0.0.1:4101";

    AudienceVAsioPeer audience;
    audience.localAddress = "local://audience.silkit";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://127.0.0.1:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[::1]:6002", // "tcp://127.0.0.1:6002", // !!!
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp4Standard_ViaLocal_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://192.51.100.104:4103";

    AudienceVAsioPeer audience;
    audience.localAddress = "local://audience.silkit";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://192.51.100.104:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp6Loopback_ViaLocal_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://[::1]:6101";

    AudienceVAsioPeer audience;
    audience.localAddress = "local://audience.silkit";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://127.0.0.1:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[::1]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp6Standard_ViaLocal_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://[2001:0db8:2222::4144]:6103";

    AudienceVAsioPeer audience;
    audience.localAddress = "local://audience.silkit";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[2001:0db8:2222::4144]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaLocal_ViaTcp4Loopback_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "local://";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://127.0.0.1:4101";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://127.0.0.1:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[::1]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp4Loopback_ViaTcp4Loopback_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://127.0.0.1:4101";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://127.0.0.1:4101";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://127.0.0.1:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[::1]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp4Standard_ViaTcp4Loopback_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://192.51.100.104:4103";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://127.0.0.1:4101";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://192.51.100.104:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp6Loopback_ViaTcp4Loopback_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://[::1]:6101";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://127.0.0.1:4101";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://127.0.0.1:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[::1]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp6Standard_ViaTcp4Loopback_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://[2001:0db8:2222::4144]:6103";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://127.0.0.1:4101";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[2001:0db8:2222::4144]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaLocal_ViaTcp4Standard_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "local://";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://192.51.100.125:4103";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://192.51.100.125:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp4Loopback_ViaTcp4Standard_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://127.0.0.1:4101";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://192.51.100.125:4103";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://127.0.0.1:4002",
                "tcp://192.51.100.125:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp4Standard_ViaTcp4Standard_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://192.51.100.104:4103";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://192.51.100.125:4103";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://192.51.100.104:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp6Loopback_ViaTcp4Standard_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://[::1]:6101";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://192.51.100.125:4103";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://192.51.100.125:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[::1]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp6Standard_ViaTcp4Standard_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://[2001:0db8:2222::4144]:6103";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://192.51.100.125:4103";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[2001:0db8:2222::4144]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaLocal_ViaTcp6Loopback_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "local://";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://[::1]:6103";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://127.0.0.1:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[::1]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp4Loopback_ViaTcp6Loopback_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://127.0.0.1:4101";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://[::1]:6103";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://127.0.0.1:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[::1]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp4Standard_ViaTcp6Loopback_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://192.51.100.104:4103";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://[::1]:6103";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://192.51.100.104:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp6Loopback_ViaTcp6Loopback_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://[::1]:6101";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://[::1]:6103";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://127.0.0.1:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[::1]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp6Standard_ViaTcp6Loopback_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://[2001:0db8:2222::4144]:6103";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://[::1]:6103";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[2001:0db8:2222::4144]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaLocal_ViaTcp6Standard_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "local://";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://[2001:0db8:2222::4155]:6103";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[2001:0db8:2222::4155]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp4Loopback_ViaTcp6Standard_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://127.0.0.1:4101";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://[2001:0db8:2222::4155]:6103";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://127.0.0.1:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[2001:0db8:2222::4155]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp4Standard_ViaTcp6Standard_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://192.51.100.104:4103";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://[2001:0db8:2222::4155]:6103";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://192.51.100.104:4002",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp6Loopback_ViaTcp6Standard_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://[::1]:6101";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://[2001:0db8:2222::4155]:6103";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[::1]:6002",
                "tcp://[2001:0db8:2222::4155]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, Advertised_ViaTcp6Standard_ViaTcp6Standard_Audience)
{
    AdvertisedVAsioPeer advertised;
    advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://127.0.0.1:4001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://192.0.2.104:4003");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::1]:6001");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
    advertised.peerInfo.acceptorUris.emplace_back("tcp://[2001:0db8:1111::4144]:6003");
    advertised.remoteAddress = "tcp://[2001:0db8:2222::4144]:6103";

    AudienceVAsioPeer audience;
    audience.localAddress = "tcp://[2001:0db8:2222::4155]:6103";

    RunTest(advertised, audience,
            {
                "local://advertised.silkit",
                "tcp://127.0.0.1:4001",
                "tcp://192.0.2.104:4003",
                "tcp://[::1]:6001",
                "tcp://[2001:0db8:2222::4144]:6002",
                "tcp://[2001:0db8:1111::4144]:6003",
            });
}

TEST(Test_TransformAcceptorUris, AllParticipantsAndRegistryOnSeparateHosts_DefaultCatchall)
{
    // Advertised <== TCPv4 ==> Registry <== TCPv4 ==> Audience
    {
        AdvertisedVAsioPeer advertised;
        advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
        advertised.remoteAddress = "tcp://192.0.2.104:4103";

        AudienceVAsioPeer audience;
        audience.localAddress = "tcp://192.51.100.125:4103";

        RunTest(advertised, audience,
                {
                    "local://advertised.silkit",
                    "tcp://192.0.2.104:4002",
                });
    }

    // Advertised <== TCPv6 ==> Registry <== TCPv4 ==> Audience
    {
        AdvertisedVAsioPeer advertised;
        advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
        advertised.remoteAddress = "tcp://[2001:0db8:2222::4144]:6103";

        AudienceVAsioPeer audience;
        audience.localAddress = "tcp://192.51.100.125:4103";

        RunTest(advertised, audience,
                {
                    "local://advertised.silkit",
                    "tcp://[2001:0db8:2222::4144]:6002",
                });
    }

    // Advertised <== TCPv4 ==> Registry <== TCPv6 ==> Audience
    {
        AdvertisedVAsioPeer advertised;
        advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
        advertised.remoteAddress = "tcp://192.0.2.104:4103";

        AudienceVAsioPeer audience;
        audience.localAddress = "tcp://[2001:0db8:2222::4155]:6103";

        RunTest(advertised, audience,
                {
                    "local://advertised.silkit",
                    "tcp://192.0.2.104:4002",
                });
    }

    // Advertised <== TCPv6 ==> Registry <== TCPv6 ==> Audience
    {
        AdvertisedVAsioPeer advertised;
        advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
        advertised.remoteAddress = "tcp://[2001:0db8:1111::4144]:6103";

        AudienceVAsioPeer audience;
        audience.localAddress = "tcp://[2001:0db8:2222::4155]:6103";

        RunTest(advertised, audience,
                {
                    "local://advertised.silkit",
                    "tcp://[2001:0db8:1111::4144]:6002",
                });
    }
}

TEST(Test_TransformAcceptorUris, AdvertisedAndRegistrySameHost_AudienceSeparate_DefaultCatchall)
{
    // Advertised <== TCPv4 Loopback ==> Registry <== TCPv4 ==> Audience
    {
        AdvertisedVAsioPeer advertised;
        advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
        advertised.remoteAddress = "tcp://127.0.0.1:4103";

        AudienceVAsioPeer audience;
        audience.localAddress = "tcp://192.51.100.125:4103";

        RunTest(advertised, audience,
                {
                    "local://advertised.silkit",
                    "tcp://127.0.0.1:4002",
                    "tcp://192.51.100.125:4002",
                });
    }

    // Advertised <== TCPv4 Loopback ==> Registry <== TCPv6 ==> Audience
    {
        AdvertisedVAsioPeer advertised;
        advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
        advertised.remoteAddress = "tcp://127.0.0.1:4103";

        AudienceVAsioPeer audience;
        audience.localAddress = "tcp://[2001:0db8:2222::4155]:6103";

        RunTest(advertised, audience,
                {
                    "local://advertised.silkit",
                    "tcp://127.0.0.1:4002",
                    "tcp://[2001:0db8:2222::4155]:6002",
                });
    }

    // Advertised <== TCPv6 Loopback ==> Registry <== TCPv4 ==> Audience
    {
        AdvertisedVAsioPeer advertised;
        advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
        advertised.remoteAddress = "tcp://[::1]:6103";

        AudienceVAsioPeer audience;
        audience.localAddress = "tcp://192.51.100.125:4103";

        RunTest(advertised, audience,
                {
                    "local://advertised.silkit",
                    "tcp://[::1]:6002",
                    "tcp://192.51.100.125:4002",
                });
    }

    // Advertised <== TCPv6 Loopback ==> Registry <== TCPv6 ==> Audience
    {
        AdvertisedVAsioPeer advertised;
        advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
        advertised.remoteAddress = "tcp://[::1]:6103";

        AudienceVAsioPeer audience;
        audience.localAddress = "tcp://[2001:0db8:2222::4155]:6103";

        RunTest(advertised, audience,
                {
                    "local://advertised.silkit",
                    "tcp://[::1]:6002",
                    "tcp://[2001:0db8:2222::4155]:6002",
                });
    }

    // Advertised <== Local-Domain ==> Registry <== TCPv4 ==> Audience
    {
        AdvertisedVAsioPeer advertised;
        advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
        advertised.remoteAddress = "local://advertised.registry.silkit";

        AudienceVAsioPeer audience;
        audience.localAddress = "tcp://192.51.100.125:4103";

        RunTest(advertised, audience,
                {
                    "local://advertised.silkit",
                    "tcp://192.51.100.125:4002",
                });
    }

    // Advertised <== Local-Domain ==> Registry <== TCPv6 ==> Audience
    {
        AdvertisedVAsioPeer advertised;
        advertised.peerInfo.acceptorUris.emplace_back("local://advertised.silkit");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://0.0.0.0:4002");
        advertised.peerInfo.acceptorUris.emplace_back("tcp://[::]:6002");
        advertised.remoteAddress = "local://advertised.registry.silkit";

        AudienceVAsioPeer audience;
        audience.localAddress = "tcp://[2001:0db8:2222::4155]:6103";

        RunTest(advertised, audience,
                {
                    "local://advertised.silkit",
                    "tcp://[2001:0db8:2222::4155]:6002",
                });
    }
}

TEST(Test_TransformAcceptorUris, AdvertisedFqdn)
{
    // Advertised <== TCPv4 Loopback ==> Registry <== TCPv4 ==> Audience
    {
        AdvertisedVAsioPeer advertised;
        advertised.peerInfo.acceptorUris.emplace_back("tcp://example.com:4003");
        advertised.remoteAddress = "tcp://127.0.0.1:4103";

        AudienceVAsioPeer audience;
        audience.localAddress = "tcp://192.51.100.125:4103";

        RunTest(advertised, audience,
                {
                    "tcp://example.com:4003",
                });
    }
}

TEST(Test_TransformAcceptorUris, CheckOrder)
{
    // Advertised <== TCPv4 Loopback ==> Registry <== TCPv4 ==> Audience
    {
        AdvertisedVAsioPeer advertised;
        advertised.peerInfo.acceptorUris = {
            "tcp://127.0.0.1:4001",
            "tcp://192.0.2.104:4003",
            "local://advertised.silkit",
        };
        advertised.remoteAddress = "tcp://127.0.0.1:4103";

        AudienceVAsioPeer audience;
        audience.localAddress = "tcp://192.51.100.125:4103";

        DummyLogger logger;

        advertised.peerInfo.participantName = "Advertised";
        audience.peerInfo.participantName = "Audience";

        const auto expectedAcceptorUris = std::vector<std::string>{
            // local-domain is always first
            "local://advertised.silkit",
            // non-local addresses
            "tcp://192.0.2.104:4003",
            // local addresses
            "tcp://127.0.0.1:4001",
        };

        const auto acceptorUris = TransformAcceptorUris(&logger, &advertised, &audience);

        EXPECT_EQ(expectedAcceptorUris, acceptorUris);
    }

    // Advertised <== TCPv4 ==> Registry <== TCPv4 Loopback ==> Audience
    {
        AdvertisedVAsioPeer advertised;
        advertised.peerInfo.acceptorUris = {
            "tcp://192.0.2.104:4003",
            "tcp://127.0.0.1:4001",
            "local://advertised.silkit",
        };
        advertised.remoteAddress = "tcp://192.0.2.104:4103";

        AudienceVAsioPeer audience;
        audience.localAddress = "tcp://127.0.0.1:4103";

        DummyLogger logger;

        advertised.peerInfo.participantName = "Advertised";
        audience.peerInfo.participantName = "Audience";

        const auto expectedAcceptorUris = std::vector<std::string>{
            // local-domain is always first
            "local://advertised.silkit",
            // local addresses
            "tcp://127.0.0.1:4001",
            // non-local addresses
            "tcp://192.0.2.104:4003",
        };

        const auto acceptorUris = TransformAcceptorUris(&logger, &advertised, &audience);

        EXPECT_EQ(expectedAcceptorUris, acceptorUris);
    }
}

} // namespace
