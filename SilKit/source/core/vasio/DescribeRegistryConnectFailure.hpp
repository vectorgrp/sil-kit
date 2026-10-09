// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "core/vasio/IConnectPeer.hpp"

#include <chrono>
#include <string>


namespace VSilKit {


struct RegistryConnectFailureDescription
{
    /// One sentence stating the main cause of the failure and what to check
    std::string message;
    /// The registry host was reached, but nothing listens on the port, i.e., the registry is likely not running
    bool connectionRefused{false};
};

/// Summarizes why connecting to the registry failed.
///
/// Failures on local-domain sockets are ignored if a tcp:// URI was tried, because they are expected whenever the
/// registry runs on another machine. Of the remaining failures, the most specific one is reported: a host name that
/// cannot be resolved, a refused connection, an unreachable host or network, a timeout, and any other error.
auto DescribeRegistryConnectFailure(const std::string& connectUri, const ConnectPeerFailures& failures,
                                    std::chrono::milliseconds timeout) -> RegistryConnectFailureDescription;


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::DescribeRegistryConnectFailure;
using VSilKit::RegistryConnectFailureDescription;
} // namespace Core
} // namespace SilKit
