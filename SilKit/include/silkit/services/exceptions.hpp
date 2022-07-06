// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <exception>

namespace SilKit {
namespace Services {

class ProtocolError : public std::exception
{
};

} // namespace Services
} // namespace SilKit
