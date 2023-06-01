/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include <stdexcept>
#include <string>

namespace SilKit {

//! \brief Base class of all SilKit exceptions
class SilKitError: public std::exception
{
protected:
    std::string _what;
public:

    SilKitError(std::string message)
        :_what{std::move(message)}
    {
    }

    SilKitError(const char* message)
        :_what{message}
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

    TypeConversionError() : SilKitError("SilKit: Invalid type conversion.") { }
};

class ConfigurationError : public SilKitError
{
public:
    using SilKitError::SilKitError;

    ConfigurationError() : ConfigurationError("SilKit: Configuration has syntactical or semantical errors.") { }
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
class AssertionError: public SilKitError
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
