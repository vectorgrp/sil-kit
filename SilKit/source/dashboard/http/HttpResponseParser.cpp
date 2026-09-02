// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "dashboard/http/HttpResponseParser.hpp"

#include <algorithm>
#include <limits>

namespace VSilKit {

namespace {

auto ToLower(char c) -> char
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

auto IEquals(std::string_view a, std::string_view b) -> bool
{
    return a.size() == b.size()
           && std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) { return ToLower(x) == ToLower(y); });
}

auto TrimOws(std::string_view s) -> std::string_view
{
    const auto isOws = [](char c) { return c == ' ' || c == '\t'; };
    while (!s.empty() && isOws(s.front()))
    {
        s.remove_prefix(1);
    }
    while (!s.empty() && isOws(s.back()))
    {
        s.remove_suffix(1);
    }
    return s;
}

//! Pop the next line, accepting both CRLF and bare LF. Returns false when nothing is left.
auto NextLine(std::string_view& rest, std::string_view& line) -> bool
{
    if (rest.empty())
    {
        return false;
    }
    const auto lf = rest.find('\n');
    if (lf == std::string_view::npos)
    {
        line = rest;
        rest = {};
        return true;
    }
    line = rest.substr(0, lf);
    rest.remove_prefix(lf + 1);
    if (!line.empty() && line.back() == '\r')
    {
        line.remove_suffix(1);
    }
    return true;
}

auto ParseDecimal(std::string_view s, uint64_t& value) -> bool
{
    if (s.empty())
    {
        return false;
    }
    uint64_t result = 0;
    for (const char c : s)
    {
        if (c < '0' || c > '9')
        {
            return false;
        }
        const auto digit = static_cast<uint64_t>(c - '0');
        if (result > (std::numeric_limits<uint64_t>::max() - digit) / 10)
        {
            return false; // overflow
        }
        result = result * 10 + digit;
    }
    value = result;
    return true;
}

//! "HTTP/1.1 201 Created" -> 201. Strict: the version and a three-digit code are both required.
auto ParseStatusLine(std::string_view line, int& statusCode) -> bool
{
    constexpr std::string_view prefix = "HTTP/1.";
    if (line.size() < prefix.size() + 1 || line.substr(0, prefix.size()) != prefix)
    {
        return false;
    }
    line.remove_prefix(prefix.size());
    if (line.empty() || line.front() < '0' || line.front() > '9')
    {
        return false; // minor version digit
    }
    line.remove_prefix(1);
    if (line.empty() || line.front() != ' ')
    {
        return false;
    }
    line.remove_prefix(1);
    if (line.size() < 3)
    {
        return false;
    }
    const auto code = line.substr(0, 3);
    if (!std::all_of(code.begin(), code.end(), [](char c) { return c >= '0' && c <= '9'; }))
    {
        return false;
    }
    // Anything after the code must be absent or a space followed by the reason phrase.
    if (line.size() > 3 && line[3] != ' ')
    {
        return false;
    }
    statusCode = (code[0] - '0') * 100 + (code[1] - '0') * 10 + (code[2] - '0');
    return true;
}

} // namespace

auto ParseResponseHead(std::string_view head, ResponseHead& out) -> bool
{
    out = ResponseHead{};

    std::string_view rest = head;
    std::string_view line;
    if (!NextLine(rest, line) || !ParseStatusLine(line, out.statusCode))
    {
        return false;
    }

    bool chunked = false;
    bool haveContentLength = false;
    uint64_t contentLength = 0;

    while (NextLine(rest, line))
    {
        if (line.empty())
        {
            break; // end of head
        }
        // An obs-fold continuation belongs to the previous field; deprecated and never emitted by
        // real servers, so skip it rather than failing.
        if (line.front() == ' ' || line.front() == '\t')
        {
            continue;
        }
        const auto colon = line.find(':');
        if (colon == std::string_view::npos)
        {
            return false; // not a header field
        }
        const auto name = TrimOws(line.substr(0, colon));
        const auto value = TrimOws(line.substr(colon + 1));

        if (IEquals(name, "content-length"))
        {
            uint64_t parsed = 0;
            if (!ParseDecimal(value, parsed))
            {
                return false;
            }
            if (haveContentLength && parsed != contentLength)
            {
                return false; // conflicting duplicates (RFC 7230 3.3.2)
            }
            haveContentLength = true;
            contentLength = parsed;
        }
        else if (IEquals(name, "transfer-encoding"))
        {
            // We only ever need to recognise chunked; any other coding we cannot decode.
            if (IEquals(value, "chunked"))
            {
                chunked = true;
            }
            else if (!value.empty() && !IEquals(value, "identity"))
            {
                return false;
            }
        }
        else if (IEquals(name, "connection"))
        {
            if (IEquals(value, "close"))
            {
                out.connectionClose = true;
            }
        }
    }

    if (out.statusCode >= 100 && out.statusCode < 200)
    {
        out.interim = true;
        out.framing = HttpBodyFraming::None;
        return true;
    }

    if (out.statusCode == 204 || out.statusCode == 304)
    {
        out.framing = HttpBodyFraming::None;
        return true;
    }

    if (chunked)
    {
        // Chunked wins; Content-Length must be ignored (RFC 7230 3.3.3).
        out.framing = HttpBodyFraming::Chunked;
        return true;
    }

    if (haveContentLength)
    {
        if (contentLength > maxHttpBodySize)
        {
            return false;
        }
        out.framing = HttpBodyFraming::ContentLength;
        out.contentLength = contentLength;
        return true;
    }

    out.framing = HttpBodyFraming::UntilClose;
    return true;
}

auto ParseChunkSize(std::string_view line, uint64_t& size) -> bool
{
    // Strip any chunk extensions.
    const auto semi = line.find(';');
    if (semi != std::string_view::npos)
    {
        line = line.substr(0, semi);
    }
    line = TrimOws(line);
    if (line.empty())
    {
        return false;
    }

    uint64_t result = 0;
    for (const char c : line)
    {
        uint64_t digit = 0;
        if (c >= '0' && c <= '9')
        {
            digit = static_cast<uint64_t>(c - '0');
        }
        else if (c >= 'a' && c <= 'f')
        {
            digit = static_cast<uint64_t>(c - 'a' + 10);
        }
        else if (c >= 'A' && c <= 'F')
        {
            digit = static_cast<uint64_t>(c - 'A' + 10);
        }
        else
        {
            return false;
        }
        if (result > (std::numeric_limits<uint64_t>::max() >> 4))
        {
            return false; // overflow
        }
        result = (result << 4) | digit;
    }
    size = result;
    return true;
}

} // namespace VSilKit
