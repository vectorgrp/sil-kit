// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>

#include "ib/cfg/Config.hpp"

#include "tuple_tools/for_each.hpp"

namespace ib {
namespace mw {

class VAsioConnection
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    VAsioConnection() = default;
    VAsioConnection(const VAsioConnection&) = default;
    VAsioConnection(VAsioConnection&&) = default;
    VAsioConnection(cfg::Config config, std::string participantName);

public:
    // ----------------------------------------
    // Operator Implementations
    VAsioConnection& operator=(VAsioConnection& other) = default;
    VAsioConnection& operator=(VAsioConnection&& other) = default;

public:
    // ----------------------------------------
    // Public methods
    //
    void joinDomain(uint32_t domainId);

    template<class IbServiceT>
    inline void RegisterIbService(const std::string& topicName, EndpointId endpointId, IbServiceT* receiver);

    template<typename IbMessageT>
    void SendIbMessageImpl(EndpointAddress from, IbMessageT&& msg);

    void WaitForMessageDelivery() {};
    void FlushSendBuffers() {};

    void Run() {};
    void Stop() {};

private:
    // ----------------------------------------
    // private datatypes

private:
    // ----------------------------------------
    // private methods

private:
    // ----------------------------------------
    // private members
    cfg::Config _config;
    std::string _participantName;
    ParticipantId _participantId{0};
   
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template<class IbServiceT>
void VAsioConnection::RegisterIbService(const std::string& topicName, EndpointId endpointId, IbServiceT* receiver)
{
}

template<typename IbMessageT>
void VAsioConnection::SendIbMessageImpl(EndpointAddress from, IbMessageT&& msg)
{
}



} // mw
} // namespace ib
