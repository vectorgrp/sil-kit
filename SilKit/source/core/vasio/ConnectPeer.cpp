// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ConnectPeer.hpp"

#include "util/TracingMacros.hpp"

#include "VAsioConnection.hpp"
#include "VAsioPeerInfo.hpp"
#include "VAsioPeer.hpp"

#include "Uri.hpp"

#include <algorithm>
#include <memory>


#if SILKIT_ENABLE_TRACING_INSTRUMENTATION_ConnectPeer
#    define SILKIT_TRACE_METHOD_(logger, ...) SILKIT_TRACE_METHOD(logger, __VA_ARGS__)
#else
#    define SILKIT_TRACE_METHOD_(...)
#endif


namespace Log = SilKit::Services::Logging;


namespace VSilKit {


ConnectPeer::ConnectPeer(IIoContext* ioContext, SilKit::Services::Logging::ILogger* logger,
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

    _ioContext->Dispatch([this] {
        TryNextUri();
    });
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
        try
        {
            auto uri{Uri::Parse(str)};

            if (uri.Type() == Uri::UriType::Tcp)
            {
                // resolve the host part in tcp:// URIs

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
            Log::Warn(_logger, "Error occurred while processing acceptor URI '{}': {}", str, exception.what());
        }
        catch (...)
        {
            Log::Warn(_logger, "Error occurred while processing acceptor URI '{}'", str);
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

        _ioContext->Dispatch([this] {
            TryNextUri();
        });
        return;
    }

    const auto& uri{_uris[_uriIndex]};
    _uriIndex += 1;

    Log::Debug(_logger, "Trying to connect to {} on {}", _peerInfo.participantName, uri.EncodedString());

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
                Log::Debug(_logger, "Unable to connect via local-domain because it is disabled via configuration");
            }
            else
            {
                _connector = _ioContext->MakeLocalConnector(uri.Path());
            }
            break;

        default:
            Log::Warn(_logger, "Invalid uri type {}", static_cast<std::underlying_type_t<Uri::UriType>>(uri.Type()));
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
        Log::Warn(_logger, "Failed to start connecting to '{}': {}", uri.EncodedString(), exception.what());
    }
    catch (...)
    {
        _connector.reset();
        Log::Warn(_logger, "Failed to start connecting to '{}'", uri.EncodedString());
    }

    if (_connector == nullptr)
    {
        _ioContext->Dispatch([this] {
            TryNextUri();
        });
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
    _listener->OnConnectPeerFailure(*this, _peerInfo);
}


void ConnectPeer::OnAsyncConnectSuccess(IConnector&, std::unique_ptr<IRawByteStream> stream)
{
    SILKIT_TRACE_METHOD_(_logger, "(..., {})", static_cast<const void*>(stream.get()));

    HandleSuccess(std::move(stream));
}


void ConnectPeer::OnAsyncConnectFailure(IConnector&)
{
    SILKIT_TRACE_METHOD_(_logger, "(...)");

    _connector.reset();
    TryNextUri();
}


} // namespace VSilKit
