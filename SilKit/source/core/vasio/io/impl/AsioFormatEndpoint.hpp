// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include <string>

#include "asio.hpp"


namespace VSilKit {


auto FormatEndpoint(const asio::ip::tcp::endpoint& ep) -> std::string;


auto FormatEndpoint(const asio::local::stream_protocol::endpoint& ep) -> std::string;


auto FormatEndpoint(const asio::generic::stream_protocol::endpoint& ep) -> std::string;


} // namespace VSilKit
