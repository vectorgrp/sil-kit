// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace VSilKit {

//! Upper bound on a response body we are willing to buffer.
constexpr uint64_t maxHttpBodySize = 1u << 20; // 1 MiB

//! How the body of a response is framed.
enum class HttpBodyFraming
{
    None,          //!< 1xx / 204 / 304: no body at all
    ContentLength, //!< exactly ResponseHead::contentLength bytes follow
    Chunked,       //!< RFC 7230 chunked transfer coding
    UntilClose,    //!< neither header present: read until the peer closes
};

struct ResponseHead
{
    int statusCode{0};
    HttpBodyFraming framing{HttpBodyFraming::UntilClose};
    uint64_t contentLength{0};
    bool connectionClose{false};
    //! True for a 1xx interim response, which the caller must skip and read another head.
    bool interim{false};
};

/*! Parse a complete response head.
 *
 *  `head` is everything up to and including the terminating blank line. Strict about the status
 *  line and about body framing; lenient about everything else (unknown headers are ignored, header
 *  names are case-insensitive, bare LF line endings are accepted, obs-fold continuation lines are
 *  skipped rather than rejected).
 *
 *  Returns false if the head is malformed, in which case `out` is unspecified and the connection
 *  must be closed rather than reused.
 */
auto ParseResponseHead(std::string_view head, ResponseHead& out) -> bool;

/*! Parse a chunk-size line, e.g. "1a3" or "1a3;ext=val" (without the trailing CRLF).
 *
 *  Returns false on a missing or non-hexadecimal size, or on overflow.
 */
auto ParseChunkSize(std::string_view line, uint64_t& size) -> bool;

} // namespace VSilKit
