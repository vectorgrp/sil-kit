// SPDX-FileCopyrightText: 2022-2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT
#pragma once

#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>
#include <sstream>


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
    UriType _type{UriType::Undefined};
    std::string _scheme;
    std::string _host;
    std::optional<uint16_t> _port{};
    std::string _path;
    std::string _uriString;
};


using UriType = Uri::UriType;


auto operator<<(std::ostream& ostream, UriType uriType) -> std::ostream&;


} // namespace Core
} // namespace SilKit
