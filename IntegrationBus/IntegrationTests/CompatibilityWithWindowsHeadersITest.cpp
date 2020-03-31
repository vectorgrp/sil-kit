// Copyright (c) Vector Informatik GmbH. All rights reserved.

// This test ensures that the public VIB api is compatible with windows headers.

// 1. include any relevant windows headers, that define some evil macros, e.g.,
// max and min
#include <windows.h>

// 2. include all VIB headers
#include "ib/cfg/all.hpp"
#include "ib/exception.hpp"
#include "ib/IbMacros.hpp"
#include "ib/IntegrationBus.hpp"

#include "ib/mw/all.hpp"
#include "ib/mw/string_utils.hpp"
#include "ib/mw/sync/string_utils.hpp"

#include "ib/sim/all.hpp"
#include "ib/sim/exceptions.hpp"
#include "ib/sim/can/string_utils.hpp"
#include "ib/sim/eth/string_utils.hpp"
#include "ib/sim/fr/string_utils.hpp"
#include "ib/sim/generic/string_utils.hpp"
#include "ib/sim/io/string_utils.hpp"
#include "ib/sim/lin/string_utils.hpp"

#include "ib/util/functional.hpp"
#include "ib/util/PrintableHexString.hpp"
#include "ib/util/vector_view.hpp"
#include "ib/version.hpp"

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
