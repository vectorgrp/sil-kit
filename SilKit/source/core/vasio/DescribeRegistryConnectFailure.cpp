// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/vasio/DescribeRegistryConnectFailure.hpp"

#include "util/Uri.hpp"

#include <algorithm>

#include "fmt/format.h"


namespace VSilKit {


namespace {

auto IsUnreachable(const std::error_code& errorCode) -> bool
{
    return errorCode == std::errc::host_unreachable || errorCode == std::errc::network_unreachable;
}

/// Lower values are more specific, i.e., tell the user more precisely what to fix
auto Specificity(const ConnectPeerFailure& failure) -> int
{
    switch (failure.stage)
    {
    case ConnectPeerFailure::Stage::Resolve:
        return 0;
    case ConnectPeerFailure::Stage::Connect:
        if (failure.errorCode == std::errc::connection_refused)
        {
            return 1;
        }
        if (IsUnreachable(failure.errorCode))
        {
            return 2;
        }
        if (failure.errorCode == std::errc::timed_out)
        {
            return 3;
        }
        return 4;
    default:
        return 5;
    }
}

auto WithoutTrailingPeriod(std::string text) -> std::string
{
    while (!text.empty() && (text.back() == '.' || text.back() == ' ' || text.back() == '\n' || text.back() == '\r'))
    {
        text.pop_back();
    }
    return text;
}

} // namespace


auto DescribeRegistryConnectFailure(const std::string& connectUri, const ConnectPeerFailures& failures,
                                    std::chrono::milliseconds timeout) -> RegistryConnectFailureDescription
{
    RegistryConnectFailureDescription description;

    const auto prefix{fmt::format("Failed to connect to SIL Kit Registry at '{}'", connectUri)};

    // local-domain sockets only matter if nothing else was tried
    const bool anyTcp{std::any_of(failures.begin(), failures.end(), [](const auto& f) { return !f.isLocal; })};

    const ConnectPeerFailure* primary{nullptr};
    for (const auto& failure : failures)
    {
        if (anyTcp && failure.isLocal)
        {
            continue;
        }
        if (primary == nullptr || Specificity(failure) < Specificity(*primary))
        {
            primary = &failure;
        }
    }

    if (primary == nullptr)
    {
        description.message = prefix + ".";
        return description;
    }

    std::string host;
    uint16_t port{0};
    try
    {
        SilKit::Core::Uri uri{connectUri};
        host = uri.Host();
        port = uri.Port();
    }
    catch (...)
    {
        // the connect URI has been parsed before, fall back to generic hints if it fails anyway
    }

    if (primary->stage == ConnectPeerFailure::Stage::Resolve)
    {
        description.message =
            fmt::format("{}: host name '{}' could not be resolved. Check the host name and the DNS configuration.",
                        prefix, host);
    }
    else if (primary->stage == ConnectPeerFailure::Stage::Connect
             && primary->errorCode == std::errc::connection_refused)
    {
        description.connectionRefused = true;
        description.message = fmt::format(
            "{}: connection refused. Check that the SIL Kit Registry is running and listening on port {}.", prefix,
            port);
    }
    else if (primary->stage == ConnectPeerFailure::Stage::Connect && IsUnreachable(primary->errorCode))
    {
        description.message = fmt::format("{}: {}. Check the address and the network connection (e.g., VPN).", prefix,
                                          WithoutTrailingPeriod(primary->message));
    }
    else if (primary->stage == ConnectPeerFailure::Stage::Connect && primary->errorCode == std::errc::timed_out)
    {
        description.message = fmt::format(
            "{}: no answer within {}ms. Check the address, the network connection (e.g., VPN) and firewalls.", prefix,
            timeout.count());
    }
    else
    {
        description.message = fmt::format("{}: {}.", prefix, WithoutTrailingPeriod(primary->message));
    }

    return description;
}


} // namespace VSilKit
