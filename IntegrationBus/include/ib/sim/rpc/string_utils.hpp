// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>
#include <sstream>

#include "ib/exception.hpp"
#include "ib/util/PrintableHexString.hpp"

#include "RpcDatatypes.hpp"

namespace ib {
namespace sim {
namespace rpc {

// RpcIbMessages
inline std::string   to_string(const CallUUID& msg);
inline std::ostream& operator<<(std::ostream& out, const CallUUID& msg);

inline std::string   to_string(const FunctionCall& msg);
inline std::ostream& operator<<(std::ostream& out, const FunctionCall& msg);

inline std::string   to_string(const FunctionCallResponse& msg);
inline std::ostream& operator<<(std::ostream& out, const FunctionCallResponse& msg);

inline std::string   to_string(const ClientAnnouncement& msg);
inline std::ostream& operator<<(std::ostream& out, const ClientAnnouncement& msg);

inline std::string   to_string(const ServerAcknowledge& msg);
inline std::ostream& operator<<(std::ostream& out, const ServerAcknowledge& msg);

// RpcDatatypes
inline std::string   to_string(const RpcExchangeFormat& dataExchangeFormat);
inline std::ostream& operator<<(std::ostream& out, const RpcExchangeFormat& dataExchangeFormat);

// ================================================================================
//  Inline Implementations
// ================================================================================

std::string to_string(const CallUUID& callUUID)
{
    std::stringstream out;
    out << callUUID.ab << callUUID.cd;
    return out.str();
}

std::ostream& operator<<(std::ostream& out, const CallUUID& msg)
{
    return out << to_string(msg);
}

std::string to_string(const FunctionCall& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}
std::ostream& operator<<(std::ostream& out, const FunctionCall& msg)
{
    return out << "data::FunctionCall{callUUID=" << msg.callUUID << ", data="
               << util::AsHexString(msg.data).WithSeparator(" ").WithMaxLength(16)
               << ", size=" << msg.data.size()
               << "}";
}

std::string to_string(const FunctionCallResponse& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}
std::ostream& operator<<(std::ostream& out, const FunctionCallResponse& msg)
{
    return out << "data::FunctionCallResponse{callUUID=" << msg.callUUID << ", data=" << util::AsHexString(msg.data).WithSeparator(" ").WithMaxLength(16)
               << ", size=" << msg.data.size() << "}";
}


std::string to_string(const ClientAnnouncement& clientAnnouncement)
{
    std::stringstream out;
    out << clientAnnouncement;
    return out.str();
}
std::ostream& operator<<(std::ostream& out, const ClientAnnouncement& clientAnnouncement)
{
    return out << "data::ClientAnnouncement{"
               << "functionName=" << clientAnnouncement.functionName 
               << ", exchangeFormat=" << clientAnnouncement.exchangeFormat
               << ", clientUUID=" << clientAnnouncement.clientUUID
               << "}";
}

std::string to_string(const ServerAcknowledge& serverAcknowledge)
{
    std::stringstream out;
    out << serverAcknowledge;
    return out.str();
}
std::ostream& operator<<(std::ostream& out, const ServerAcknowledge& serverAcknowledge)
{
    return out << "data::ServerAcknowledge{"
               << "clientUUID=" << serverAcknowledge.clientUUID << "}";
}


std::string to_string(const RpcExchangeFormat& rpcExchangeFormat)
{
    std::stringstream out;
    out << rpcExchangeFormat;
    return out.str();
}
std::ostream& operator<<(std::ostream& out, const RpcExchangeFormat& rpcExchangeFormat)
{
    return out << "data::RpcExchangeFormat{"
               << "mimeType=" << rpcExchangeFormat.mimeType << "}";
}

} // namespace rpc
} // namespace sim
} // namespace ib
