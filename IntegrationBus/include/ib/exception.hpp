// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <exception>

namespace ib {

class type_conversion_error : public std::runtime_error
{
public:
    using runtime_error::runtime_error;

    type_conversion_error() : runtime_error("Invalid type conversion.") { }
};

} // namespace ib
