// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "dashboard/http/HttpResponseParser.hpp"

#include <string>

#include "gtest/gtest.h"

namespace VSilKit {
namespace {

struct HeadCase
{
    const char* what;
    const char* head;
    bool valid;
    int statusCode;
    HttpBodyFraming framing;
    uint64_t contentLength;
    bool connectionClose;
    bool interim;
};

// Names for the `valid` column, so the case tables below read as prose.
constexpr auto wellFormed = true;
constexpr auto malformed = false;

class Test_HttpResponseParser_Head : public testing::TestWithParam<HeadCase>
{
};

TEST_P(Test_HttpResponseParser_Head, ParseResponseHead)
{
    const auto& c = GetParam();

    ResponseHead head{};
    const bool valid = ParseResponseHead(c.head, head);

    ASSERT_EQ(valid, c.valid) << c.what;
    if (!c.valid)
    {
        return;
    }
    EXPECT_EQ(head.statusCode, c.statusCode) << c.what;
    EXPECT_EQ(static_cast<int>(head.framing), static_cast<int>(c.framing)) << c.what;
    EXPECT_EQ(head.contentLength, c.contentLength) << c.what;
    EXPECT_EQ(head.connectionClose, c.connectionClose) << c.what;
    EXPECT_EQ(head.interim, c.interim) << c.what;
}

const HeadCase headCases[] = {
    // --- status line ---
    {"201 with content-length", "HTTP/1.1 201 Created\r\nContent-Length: 12\r\n\r\n", wellFormed, 201,
     HttpBodyFraming::ContentLength, 12, false, false},
    {"no reason phrase", "HTTP/1.1 200\r\nContent-Length: 0\r\n\r\n", wellFormed, 200, HttpBodyFraming::ContentLength,
     0, false, false},
    {"HTTP/1.0", "HTTP/1.0 200 OK\r\nContent-Length: 1\r\n\r\n", wellFormed, 200, HttpBodyFraming::ContentLength, 1,
     false, false},
    {"503", "HTTP/1.1 503 Service Unavailable\r\nContent-Length: 0\r\n\r\n", wellFormed, 503,
     HttpBodyFraming::ContentLength, 0, false, false},
    {"204 has no body", "HTTP/1.1 204 No Content\r\n\r\n", wellFormed, 204, HttpBodyFraming::None, 0, false, false},
    {"304 ignores content-length", "HTTP/1.1 304 Not Modified\r\nContent-Length: 99\r\n\r\n", wellFormed, 304,
     HttpBodyFraming::None, 0, false, false},
    {"1xx is interim", "HTTP/1.1 100 Continue\r\n\r\n", wellFormed, 100, HttpBodyFraming::None, 0, false, true},

    {"not http", "not http at all\r\n\r\n", malformed, 0, HttpBodyFraming::None, 0, false, false},
    {"no status code", "HTTP/1.1\r\n\r\n", malformed, 0, HttpBodyFraming::None, 0, false, false},
    {"two-digit code", "HTTP/1.1 20 OK\r\n\r\n", malformed, 0, HttpBodyFraming::None, 0, false, false},
    {"non-numeric code", "HTTP/1.1 2O1 Created\r\n\r\n", malformed, 0, HttpBodyFraming::None, 0, false, false},
    {"unsupported version", "HTTP/2.0 200 OK\r\n\r\n", malformed, 0, HttpBodyFraming::None, 0, false, false},
    {"empty head", "", malformed, 0, HttpBodyFraming::None, 0, false, false},

    // --- body framing ---
    {"chunked", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n", wellFormed, 200, HttpBodyFraming::Chunked, 0,
     false, false},
    {"chunked wins over content-length", "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nTransfer-Encoding: chunked\r\n\r\n",
     wellFormed, 200, HttpBodyFraming::Chunked, 0, false, false},
    {"no framing header reads until close", "HTTP/1.1 200 OK\r\n\r\n", wellFormed, 200, HttpBodyFraming::UntilClose, 0,
     false, false},
    {"agreeing duplicate content-length", "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nContent-Length: 5\r\n\r\n",
     wellFormed, 200, HttpBodyFraming::ContentLength, 5, false, false},
    {"content-length at the cap", "HTTP/1.1 200 OK\r\nContent-Length: 1048576\r\n\r\n", wellFormed, 200,
     HttpBodyFraming::ContentLength, 1048576, false, false},

    {"conflicting duplicate content-length", "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nContent-Length: 6\r\n\r\n",
     malformed, 0, HttpBodyFraming::None, 0, false, false},
    {"non-numeric content-length", "HTTP/1.1 200 OK\r\nContent-Length: abc\r\n\r\n", malformed, 0,
     HttpBodyFraming::None, 0, false, false},
    {"content-length over the cap", "HTTP/1.1 200 OK\r\nContent-Length: 1048577\r\n\r\n", malformed, 0,
     HttpBodyFraming::None, 0, false, false},
    {"undecodable transfer-encoding", "HTTP/1.1 200 OK\r\nTransfer-Encoding: gzip\r\n\r\n", malformed, 0,
     HttpBodyFraming::None, 0, false, false},

    // --- leniency about header syntax ---
    {"lowercase header name", "HTTP/1.1 200 OK\r\ncontent-length: 3\r\n\r\n", wellFormed, 200,
     HttpBodyFraming::ContentLength, 3, false, false},
    {"mixed-case header name", "HTTP/1.1 200 OK\r\nCoNtEnT-LeNgTh: 3\r\n\r\n", wellFormed, 200,
     HttpBodyFraming::ContentLength, 3, false, false},
    {"surrounding whitespace", "HTTP/1.1 200 OK\r\nContent-Length:   3   \r\n\r\n", wellFormed, 200,
     HttpBodyFraming::ContentLength, 3, false, false},
    {"bare LF line endings", "HTTP/1.1 200 OK\nContent-Length: 3\n\n", wellFormed, 200, HttpBodyFraming::ContentLength,
     3, false, false},
    {"unknown headers ignored",
     "HTTP/1.1 200 OK\r\nServer: nginx\r\nX-Whatever: 1\r\nDate: now\r\nContent-Length: 3\r\n\r\n", wellFormed, 200,
     HttpBodyFraming::ContentLength, 3, false, false},
    {"obs-fold continuation skipped", "HTTP/1.1 200 OK\r\nX-Long: a\r\n  continued\r\nContent-Length: 3\r\n\r\n",
     wellFormed, 200, HttpBodyFraming::ContentLength, 3, false, false},
    {"connection close", "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: 3\r\n\r\n", wellFormed, 200,
     HttpBodyFraming::ContentLength, 3, true, false},
    {"connection keep-alive", "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Length: 3\r\n\r\n", wellFormed, 200,
     HttpBodyFraming::ContentLength, 3, false, false},

    {"header without a colon", "HTTP/1.1 200 OK\r\nnonsense\r\n\r\n", malformed, 0, HttpBodyFraming::None, 0, false,
     false},
};

INSTANTIATE_TEST_SUITE_P(Cases, Test_HttpResponseParser_Head, testing::ValuesIn(headCases),
                         [](const testing::TestParamInfo<HeadCase>& info) {
                             std::string name{info.param.what};
                             for (auto& c : name)
                             {
                                 if (!std::isalnum(static_cast<unsigned char>(c)))
                                 {
                                     c = '_';
                                 }
                             }
                             return name;
                         });

struct ChunkCase
{
    const char* what;
    const char* line;
    bool valid;
    uint64_t size;
};

class Test_HttpResponseParser_ChunkSize : public testing::TestWithParam<ChunkCase>
{
};

TEST_P(Test_HttpResponseParser_ChunkSize, ParseChunkSize)
{
    const auto& c = GetParam();

    uint64_t size = 0;
    const bool valid = ParseChunkSize(c.line, size);

    ASSERT_EQ(valid, c.valid) << c.what;
    if (c.valid)
    {
        EXPECT_EQ(size, c.size) << c.what;
    }
}

const ChunkCase chunkCases[] = {
    {"lowercase hex", "1a3", wellFormed, 0x1a3},
    {"uppercase hex", "1A3", wellFormed, 0x1a3},
    {"terminator", "0", wellFormed, 0},
    {"chunk extension stripped", "1a3;ext=val", wellFormed, 0x1a3},
    {"surrounding whitespace", "  1a3  ", wellFormed, 0x1a3},
    {"largest representable", "FFFFFFFFFFFFFFFF", wellFormed, 0xFFFFFFFFFFFFFFFFULL},
    {"empty", "", malformed, 0},
    {"not hex", "xyz", malformed, 0},
    {"overflows uint64", "FFFFFFFFFFFFFFFFF", malformed, 0},
};

INSTANTIATE_TEST_SUITE_P(Cases, Test_HttpResponseParser_ChunkSize, testing::ValuesIn(chunkCases),
                         [](const testing::TestParamInfo<ChunkCase>& info) {
                             std::string name{info.param.what};
                             for (auto& c : name)
                             {
                                 if (!std::isalnum(static_cast<unsigned char>(c)))
                                 {
                                     c = '_';
                                 }
                             }
                             return name;
                         });

} // namespace
} // namespace VSilKit
