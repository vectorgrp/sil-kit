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

//!< AssertionError is a replacement for cassert's assert.
class AssertionError: public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};
 
//! \brief ExtensionError is thrown when an extension could not be loaded
class ExtensionError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};


} // namespace SilKit
