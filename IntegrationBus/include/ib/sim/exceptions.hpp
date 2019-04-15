// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <exception>

namespace ib {
namespace sim {

class ProtocolError : public std::exception
{
};

} // namespace sim
} // namespace ib
