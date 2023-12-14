// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "asio.hpp"


namespace VSilKit {


void CleanupEndpoint(const asio::ip::tcp::endpoint& ep);
void CleanupEndpoint(const asio::local::stream_protocol::endpoint& ep);


} // namespace VSilKit
