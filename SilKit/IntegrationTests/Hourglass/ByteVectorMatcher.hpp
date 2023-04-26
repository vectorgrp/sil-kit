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
