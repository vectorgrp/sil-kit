// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <stdexcept>
#include <string>

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

    StateError(const std::string& what = "The requested operation is not valid in the current state.")
        : std::logic_error(what.c_str())
    {
    }
};

class ProtocolError: public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

 namespace extensions {
//! \brief ExtensionError is thrown when an extension could not be loaded
class ExtensionError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}//end namespace extensions

} // namespace ib
