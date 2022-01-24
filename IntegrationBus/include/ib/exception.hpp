// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <stdexcept>

namespace ib {

class type_conversion_error : public std::runtime_error
{
public:
    using runtime_error::runtime_error;

    type_conversion_error() : runtime_error("Invalid type conversion.") { }
};

class configuration_error : public std::runtime_error
{
    using std::runtime_error::runtime_error;

    configuration_error() : configuration_error("Configuration has syntactical or semantical errors.") { }
};

} // namespace ib
