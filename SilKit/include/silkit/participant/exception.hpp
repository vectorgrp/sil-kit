// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <stdexcept>
#include <string>

namespace SilKit {

//! \brief Base class of all SilKit exceptions
class SilKitError : public std::exception
{
protected:
    std::string _what;

public:
    SilKitError(std::string message)
        : _what{std::move(message)}
    {
    }

    SilKitError(const char* message)
        : _what{message}
    {
    }

    const char* what() const noexcept override
    {
        return _what.c_str();
    }
};

class TypeConversionError : public SilKitError
{
public:
    using SilKitError::SilKitError;

    TypeConversionError()
        : SilKitError("SilKit: Invalid type conversion.")
    {
    }
};

class ConfigurationError : public SilKitError
{
public:
    using SilKitError::SilKitError;

    ConfigurationError()
        : ConfigurationError("Configuration has syntactical or semantical errors.")
    {
    }
};

class StateError : public SilKitError
{
public:
    using SilKitError::SilKitError;
};

class ProtocolError : public SilKitError
{
public:
    using SilKitError::SilKitError;
};

//! \brief Error thrown as a replacement for cassert's assert.
class AssertionError : public SilKitError
{
public:
    using SilKitError::SilKitError;
};

//! \brief Error thrown when an extension could not be loaded.
class ExtensionError : public SilKitError
{
    using SilKitError::SilKitError;
};

//! \brief Error thrown as a consequence of faulty logic within the program such as violating logical
//! preconditions or class invariants and may be preventable.
class LogicError : public SilKitError
{
    using SilKitError::SilKitError;
};

//! \brief Error thrown on attempts to exceed implementation defined length limits for some object.
class LengthError : public LogicError
{
    using LogicError::LogicError;
};

//! \brief Error thrown when trying to access elements outside the defined range.
class OutOfRangeError : public LogicError
{
    using LogicError::LogicError;
};

} // namespace SilKit
