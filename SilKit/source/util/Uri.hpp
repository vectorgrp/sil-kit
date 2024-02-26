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

#include <string>
#include <sstream>
#include <cstdint>
#include <iosfwd>

#include "Optional.hpp"

// URI encoding of asio endpoint types.
// NB: Very limited implementation for internal use only -- nothing close to standard RFC 3986


namespace SilKit {
namespace Core {


class Uri
{
public:
    // public Enums
    enum class UriType
    {
        Undefined,
        SilKit,
        Tcp,
        Local,
    };

public:
    // public CTor
    //!< Initialize Uri with host and port name, and schema of "silkit://"
    explicit Uri(const std::string& host, const uint16_t port);
    //!< Calls Parse() on uriStr
    explicit Uri(const std::string& uriStr);
    Uri(const Uri&) = default;
    Uri& operator=(const Uri&) = default;

public:
    // public static methods
    static auto Parse(std::string uriStr) -> Uri;

    static auto MakeSilKit(const std::string& host, const uint16_t port, const std::string& simulationName) -> Uri;
    static auto MakeTcp(const std::string& host, const uint16_t port) -> Uri;

    static auto UrlEncode(const std::string& name) -> std::string;

public:
    // public methods
    auto EncodedString() const -> const std::string&;
    auto Scheme() const -> const std::string&;
    auto Host() const -> const std::string&;
    auto Port() const -> uint16_t;
    //!< Path currently returns everything after the '/', including queries and fragments
    auto Path() const -> const std::string&;
    auto Type() const -> UriType;
    void SetType(UriType newType);

private:
    // private ctor
    Uri() = default;
private:
    // private members
    UriType _type{ UriType::Undefined };
    std::string _scheme;
    std::string _host;
    Util::Optional<uint16_t> _port{};
    std::string _path;
    std::string _uriString;
};


using UriType = Uri::UriType;


auto operator<<(std::ostream& ostream, UriType uriType) -> std::ostream&;


} // namespace Core
} // namespace SilKit
