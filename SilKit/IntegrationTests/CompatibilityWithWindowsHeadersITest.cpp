// Copyright (c) Vector Informatik GmbH. All rights reserved.

// This test ensures that the public SILKIT api is compatible with windows headers.

// 1. include any relevant windows headers, that define some evil macros, e.g.,
// max and min
#include <windows.h>

// 2. include all SILKIT headers
#include "silkit/cfg/all.hpp"
#include "silkit/exception.hpp"
#include "silkit/SilKitMacros.hpp"
#include "silkit/SilKit.hpp"

#include "silkit/services/orchestration/string_utils.hpp"

#include "silkit/services/all.hpp"
#include "silkit/services/exceptions.hpp"
#include "silkit/services/can/string_utils.hpp"
#include "silkit/services/eth/string_utils.hpp"
#include "silkit/services/fr/string_utils.hpp"
#include "silkit/services/lin/string_utils.hpp"

#include "silkit/util/functional.hpp"
#include "silkit/util/PrintableHexString.hpp"
#include "silkit/util/vector_view.hpp"
#include "silkit/version.hpp"

// 3. Provide a dummy test for proper integration with our test runner.
#include "gtest/gtest.h"
TEST(WindowsHeaderCompatibility, min_max_macros_still_work)
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
