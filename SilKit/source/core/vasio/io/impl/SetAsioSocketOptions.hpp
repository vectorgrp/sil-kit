// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "AsioSocketOptions.hpp"

#include "silkit/services/logging/ILogger.hpp"

#include "asio.hpp"


namespace VSilKit {


void SetAsioSocketOptions(SilKit::Services::Logging::ILogger* logger, asio::ip::tcp::socket& socket,
                          AsioSocketOptions const& socketOptions, std::error_code& errorCode);


void SetAsioSocketOptions(SilKit::Services::Logging::ILogger* logger, asio::local::stream_protocol::socket& socket,
                          const AsioSocketOptions& socketOptions, std::error_code& errorCode);


} // namespace VSilKit
