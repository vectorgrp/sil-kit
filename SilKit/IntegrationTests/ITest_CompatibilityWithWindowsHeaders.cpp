// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// This test ensures that the public SIL Kit api is compatible with windows headers.

// 1. include any relevant windows headers, that define some evil macros, e.g.,
// max and min
#include <windows.h>

// 2. include all SIL Kit headers
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
