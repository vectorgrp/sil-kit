// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/vasio/ConnectPeer.hpp"

#include "core/vasio/io/util/TracingMacros.hpp"

#include "core/vasio/VAsioConnection.hpp"
#include "core/vasio/VAsioPeerInfo.hpp"
#include "core/vasio/VAsioPeer.hpp"
#include "core/vasio/VAsioConstants.hpp"

#include "util/Uri.hpp"

#include <algorithm>
#include <memory>


#if SILKIT_ENABLE_TRACING_INSTRUMENTATION_ConnectPeer
#define SILKIT_TRACE_METHOD_(logger, ...) SILKIT_TRACE_METHOD(logger, __VA_ARGS__)
#else
#define SILKIT_TRACE_METHOD_(...)
#endif


namespace Log = SilKit::Services::Logging;


namespace VSilKit {


ConnectPeer::ConnectPeer(IIoContext* ioContext, SilKit::Services::Logging::ILoggerInternal* logger,
                         const SilKit::Core::VAsioPeerInfo& peerInfo, bool enableDomainSockets)
    : _ioContext{ioContext}
    , _logger{logger}
    , _peerInfo{peerInfo}
    , _enableDomainSockets{enableDomainSockets}
{
    SILKIT_ASSERT(_ioContext != nullptr);
    SILKIT_ASSERT(!_peerInfo.participantName.empty());

    UpdateUris();
}


ConnectPeer::~ConnectPeer()
{
    SILKIT_TRACE_METHOD_(_logger, "()");
}


void ConnectPeer::SetListener(VSilKit::IConnectPeerListener& listener)
{
    _listener = &listener;
}


void ConnectPeer::AsyncConnect(size_t numberOfAttempts, std::chrono::milliseconds timeout)
{
    SILKIT_TRACE_METHOD_(_logger, "({}, {}ms)", numberOfAttempts, timeout.count());

    _remainingAttempts = std::max<size_t>(numberOfAttempts, 1);
    _timeout = timeout;

    _ioContext->Dispatch([this] { TryNextUri(); });
}


void ConnectPeer::Shutdown()
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    if (_connector)
    {
        _connector->Shutdown();
    }
}


static auto IsIp4(const std::string& address) -> bool
{
    return (address.find('.') != std::string::npos) && (address.find(':') == std::string::npos);
}

static auto IsIp6(const std::string& address) -> bool
{
    return (address.find('.') == std::string::npos) && (address.find(':') != std::string::npos);
}


void ConnectPeer::UpdateUris()
{
    std::vector<Uri> acceptorUris;

    for (const auto& str : _peerInfo.acceptorUris)
    {
        bool resolving{false};

        try
        {
            auto uri{Uri::Parse(str)};

            if (uri.Type() == Uri::UriType::Tcp)
            {
                // resolve the host part in tcp:// URIs

                resolving = true;
                for (std::string address : _ioContext->Resolve(uri.Host()))
                {
                    if (IsIp6(address))
                    {
                        if (address.front() != '[')
                        {
                            address.insert(0, "[");
                        }
                        if (address.back() != ']')
                        {
                            address.push_back(']');
                        }
                    }

                    auto tcpUri{Uri::MakeTcp(address, uri.Port())};
                    acceptorUris.emplace_back(std::move(tcpUri));
                }
            }
            else
            {
                acceptorUris.emplace_back(std::move(uri));
            }
        }
        catch (const std::exception& exception)
        {
            _logger->MakeMessage(Log::Level::Warn, TopicOf(*this))
                .SetMessage("Error occurred while processing acceptor URI '{}': {}", str, exception.what())
                .Dispatch();
            RecordFailure(str, resolving ? std::string{exception.what()}
                                             + "\n  Check the host name and the DNS configuration of this machine."
                                       : std::string{exception.what()});
        }
        catch (...)
        {
            _logger->MakeMessage(Log::Level::Warn, TopicOf(*this))
                .SetMessage("Error occurred while processing acceptor URI '{}'", str)
                .Dispatch();
            RecordFailure(str, "unknown error while processing the URI");
        }
    }

    // ensure local-domain URIs are tried first
    std::stable_sort(acceptorUris.begin(), acceptorUris.end(), [](const Uri& lhs, const Uri& rhs) {
        const auto ComputePenalty{[](const Uri& uri) -> int {
            switch (uri.Type())
            {
            case Uri::UriType::Local:
                return 100;
            case Uri::UriType::Tcp:
                if (IsIp4(uri.Host()))
                {
                    return 200;
                }
                if (IsIp6(uri.Host()))
                {
                    return 300;
                }
                return 400;
            default:
                return 500;
            }
        }};

        return ComputePenalty(lhs) < ComputePenalty(rhs);
    });

    _uris = std::move(acceptorUris);
}


void ConnectPeer::TryNextUri()
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    if (_remainingAttempts == 0)
    {
        HandleFailure();
        return;
    }

    if (_uris.empty())
    {
        HandleFailure();
        return;
    }

    if (_uriIndex >= _uris.size())
    {
        _remainingAttempts -= 1;
        _uriIndex = 0;

        _ioContext->Dispatch([this] { TryNextUri(); });
        return;
    }

    const auto& uri{_uris[_uriIndex]};
    _uriIndex += 1;

    _logger->MakeMessage(SilKit::Services::Logging::Level::Debug, TopicOf(*this))
        .SetMessage("Trying to connect to {} on {}", _peerInfo.participantName, uri.EncodedString())
        .Dispatch();
    try
    {
        switch (uri.Type())
        {
        case Uri::UriType::Tcp:
            _connector = _ioContext->MakeTcpConnector(uri.Host(), uri.Port());
            break;

        case Uri::UriType::Local:
            if (!_enableDomainSockets)
            {
                _logger->MakeMessage(SilKit::Services::Logging::Level::Debug, TopicOf(*this))
                    .SetMessage("Unable to connect via local-domain because it is disabled via configuration")
                    .Dispatch();
                RecordFailure(uri.EncodedString(), "local-domain sockets are disabled via configuration");
            }
            else
            {
                _connector = _ioContext->MakeLocalConnector(uri.Path());
            }
            break;

        default:
            _logger->MakeMessage(SilKit::Services::Logging::Level::Warn, TopicOf(*this))
                .SetMessage("Invalid uri type {}", static_cast<std::underlying_type_t<Uri::UriType>>(uri.Type()))
                .Dispatch();
            RecordFailure(uri.EncodedString(), "invalid URI type");
            break;
        }

        if (_connector != nullptr)
        {
            _connector->SetListener(*this);
            _connector->AsyncConnect(_timeout);
        }
    }
    catch (const std::exception& exception)
    {
        _connector.reset();
        _logger->MakeMessage(SilKit::Services::Logging::Level::Warn, TopicOf(*this))
            .SetMessage("Failed to start connecting to '{}': {}", uri.EncodedString(), exception.what())
            .Dispatch();
        RecordFailure(uri.EncodedString(), exception.what());
    }
    catch (...)
    {
        _connector.reset();
         _logger->MakeMessage(SilKit::Services::Logging::Level::Warn, TopicOf(*this))
            .SetMessage("Failed to start connecting to '{}'", uri.EncodedString())
            .Dispatch();
        RecordFailure(uri.EncodedString(), "unknown error while starting to connect");
    }

    if (_connector == nullptr)
    {
        _ioContext->Dispatch([this] { TryNextUri(); });
    }
}


void ConnectPeer::HandleSuccess(std::unique_ptr<IRawByteStream> stream)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", static_cast<const void*>(stream.get()));

    _connector.reset();
    _listener->OnConnectPeerSuccess(*this, _peerInfo, std::move(stream));
}


void ConnectPeer::HandleFailure()
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    _connector.reset();

    std::string reason;
    for (const auto& failure : _failureReasons)
    {
        if (!reason.empty())
        {
            reason += '\n';
        }
        reason += failure.first + ": " + failure.second;
    }
    if (reason.empty())
    {
        reason = "no usable acceptor URIs";
    }

    _listener->OnConnectPeerFailure(*this, _peerInfo, reason);
}


void ConnectPeer::RecordFailure(const std::string& uri, std::string reason)
{
    auto it{std::find_if(_failureReasons.begin(), _failureReasons.end(),
                         [&uri](const auto& failure) { return failure.first == uri; })};
    if (it != _failureReasons.end())
    {
        it->second = std::move(reason);
    }
    else
    {
        _failureReasons.emplace_back(uri, std::move(reason));
    }
}


auto ConnectPeer::DescribeFailure(const Uri& uri, std::error_code errorCode) const -> std::string
{
    const std::string peer{_peerInfo.participantName == SilKit::Core::REGISTRY_PARTICIPANT_NAME
                               ? std::string{"the SIL Kit Registry"}
                               : "participant '" + _peerInfo.participantName + "'"};

    // add a hint on what to check, so the user knows what to do about the error
    std::string hint;
    if (uri.Type() == Uri::UriType::Local)
    {
        if (errorCode == std::errc::connection_refused || errorCode == std::errc::no_such_file_or_directory)
        {
            hint = "Nothing listens on this local-domain socket. This is expected if " + peer
                   + " runs on another machine. Otherwise, check that it is running.";
        }
    }
    else if (errorCode == std::errc::connection_refused)
    {
        hint = "The host was reached, but nothing listens on this port. Check that " + peer
               + " is running and that the port is correct.";
    }
    else if (errorCode == std::errc::timed_out)
    {
        hint = "There was no answer within the connect timeout. Check that the address is correct and the host is up,"
               " that the network is reachable (e.g., VPN connected), and that no firewall drops the connection.";
    }
    else if (errorCode == std::errc::host_unreachable || errorCode == std::errc::network_unreachable)
    {
        hint = "Check that the address is correct and that the network is reachable (e.g., VPN connected).";
    }

    auto reason{errorCode.message()};
    if (!hint.empty())
    {
        reason += "\n  " + hint;
    }
    return reason;
}


void ConnectPeer::OnAsyncConnectSuccess(IConnector&, std::unique_ptr<IRawByteStream> stream)
{
    SILKIT_TRACE_METHOD_(_logger, "(..., {})", static_cast<const void*>(stream.get()));

    HandleSuccess(std::move(stream));
}


void ConnectPeer::OnAsyncConnectFailure(IConnector&, std::error_code errorCode)
{
    SILKIT_TRACE_METHOD_(_logger, "(..., {})", errorCode.message());

    _connector.reset();

    // _uriIndex has already been advanced past the URI that just failed
    if (_uriIndex > 0 && _uriIndex <= _uris.size())
    {
        const auto& uri{_uris[_uriIndex - 1]};
        RecordFailure(uri.EncodedString(), DescribeFailure(uri, errorCode));
    }

    TryNextUri();
}


} // namespace VSilKit
