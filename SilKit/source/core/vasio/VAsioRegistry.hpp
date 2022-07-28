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

#include <list>

#include "VAsioConnection.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/vendor/ISilKitRegistry.hpp"
#include "ParticipantConfiguration.hpp"
#include "ProtocolVersion.hpp"
#include "TimeProvider.hpp"

namespace SilKit {
namespace Core {

class VAsioRegistry
    : public SilKit::Vendor::Vector::ISilKitRegistry
{
public: // CTor
    VAsioRegistry() = delete;
    VAsioRegistry(const VAsioRegistry&) = delete;
    VAsioRegistry(VAsioRegistry&&) = delete;
    VAsioRegistry(std::shared_ptr<SilKit::Config::IParticipantConfiguration> cfg,
                  ProtocolVersion version = CurrentProtocolVersion());

public: // methods
    auto StartListening(const std::string& listenUri) -> std::string override;

    void SetAllConnectedHandler(std::function<void()> handler) override;
    void SetAllDisconnectedHandler(std::function<void()> handler) override;
    auto GetLogger() -> Services::Logging::ILogger* override;

private:
    // ----------------------------------------
    // private data types
    struct ConnectedParticipantInfo {
        IVAsioPeer* peer;
        SilKit::Core::VAsioPeerInfo peerInfo;
    };

private:
    // ----------------------------------------
    // private methods
    void OnParticipantAnnouncement(IVAsioPeer* from, const ParticipantAnnouncement& announcement);
    auto FindConnectedPeer(const std::string& name) const->std::vector<ConnectedParticipantInfo>::const_iterator;
    void SendKnownParticipants(IVAsioPeer* peer);
    void OnPeerShutdown(IVAsioPeer* peer);

    bool AllParticipantsAreConnected() const;

private:
    // ----------------------------------------
    // private members
    std::unique_ptr<Services::Logging::ILogger> _logger;
    std::vector<ConnectedParticipantInfo> _connectedParticipants;
    std::function<void()> _onAllParticipantsConnected;
    std::function<void()> _onAllParticipantsDisconnected;
    std::shared_ptr<SilKit::Config::ParticipantConfiguration> _vasioConfig;
    Services::Orchestration::TimeProvider _timeProvider;
    VAsioConnection _connection;
};

} // namespace Core
} // namespace SilKit
