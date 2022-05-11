// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <stdexcept>

namespace ib {

class TypeConversionError : public std::runtime_error
{
public:
    using runtime_error::runtime_error;

    TypeConversionError() : runtime_error("Invalid type conversion.") { }
};

class ConfigurationError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;

    ConfigurationError() : ConfigurationError("Configuration has syntactical or semantical errors.") { }
};

class StateError : public std::logic_error
{
public:
    using std::logic_error::logic_error;

    StateError() : StateError("The requested operation is not valid in the current state.")
    {
    }
};

class ProtocolError: public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

} // namespace ib
