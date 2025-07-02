// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "AsioCleanupEndpoint.hpp"

#include <filesystem>


namespace VSilKit {


void CleanupEndpoint(const asio::ip::tcp::endpoint&)
{
    // nothing to do
}


void CleanupEndpoint(const asio::local::stream_protocol::endpoint& ep)
{
    std::filesystem::remove(ep.path());
}


} // namespace VSilKit
