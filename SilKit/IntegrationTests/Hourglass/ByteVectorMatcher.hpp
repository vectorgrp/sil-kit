// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/capi/SilKit.h"

#include "silkit/util/Span.hpp"

#include <algorithm>

namespace {

MATCHER_P(ByteVectorMatcher, byteSpanParam, "")
{
    const SilKit::Util::Span<uint8_t>& cppByteSpan = byteSpanParam;
    const SilKit_ByteVector* cByteVector = arg;

    if (cByteVector == nullptr)
    {
        *result_listener << "cByteVector must not be nullptr";
        return false;
    }

    if (cByteVector->data == nullptr)
    {
        *result_listener << "cByteVector->data must not be nullptr";
        return false;
    }

    if (cByteVector->size != cppByteSpan.size())
    {
        *result_listener << "cByteVector->size does not match";
        return false;
    }

    if (!std::equal(cByteVector->data, cByteVector->data + cByteVector->size, cppByteSpan.begin(), cppByteSpan.end()))
    {
        *result_listener << "contents do not match";
        return false;
    }

    return true;
}

} // namespace
