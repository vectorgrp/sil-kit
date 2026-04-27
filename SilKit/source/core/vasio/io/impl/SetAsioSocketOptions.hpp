// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "AsioSocketOptions.hpp"

#include "ILoggerInternal.hpp"

#include "asio.hpp"


namespace VSilKit {


void SetAsioSocketOptions(SilKit::Services::Logging::ILoggerInternal* logger, asio::ip::tcp::socket& socket,
                          const AsioSocketOptions& socketOptions, std::error_code& errorCode);


void SetAsioSocketOptions(SilKit::Services::Logging::ILoggerInternal* logger,
                          asio::local::stream_protocol::socket& socket,
                          const AsioSocketOptions& socketOptions, std::error_code& errorCode);


} // namespace VSilKit
