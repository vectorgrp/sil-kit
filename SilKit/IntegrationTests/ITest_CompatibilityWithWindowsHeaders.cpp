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

// This test ensures that the public SILKIT api is compatible with windows headers.

// 1. include any relevant windows headers, that define some evil macros, e.g.,
// max and min
#include <windows.h>

// 2. include all SILKIT headers
#include "silkit/config/all.hpp"
#include "silkit/participant/exception.hpp"
#include "silkit/SilKitMacros.hpp"
#include "silkit/SilKit.hpp"

#include "silkit/services/orchestration/string_utils.hpp"

#include "silkit/services/all.hpp"
#include "silkit/services/can/string_utils.hpp"
#include "silkit/services/ethernet/string_utils.hpp"
#include "silkit/services/flexray/string_utils.hpp"
#include "silkit/services/lin/string_utils.hpp"

#include "silkit/util/PrintableHexString.hpp"
#include "silkit/util/Span.hpp"

// 3. Provide a dummy test for proper integration with our test runner.
#include "gtest/gtest.h"
TEST(ITest_CompatibilityWithWindowsHeaders, min_max_macros_still_work)
{
#ifndef min
    ASSERT_FALSE();
#endif //min
#ifndef max
    ASSERT_FALSE();
#endif //max
    int min_ = min(3, 5);
    int max_ = max(3, 5);
    ASSERT_EQ(min_, 3);
    ASSERT_EQ(max_, 5);
}
