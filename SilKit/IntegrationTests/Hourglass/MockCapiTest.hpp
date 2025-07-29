// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MockCapi.hpp"

namespace SilKitHourglassTests {

class MockCapiTest : public testing::Test
{
protected:
    MockCapi capi;

protected:
    void SetUp() override
    {
        capi.SetUpGlobalCapi();
    }
    void TearDown() override
    {
        capi.TearDownGlobalCapi();
    }
};

} // namespace SilKitHourglassTests
