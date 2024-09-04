// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/SilKitVersion.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "Running main() from SIlKitTestMain.cpp" << std::endl;

    testing::InitGoogleMock(&argc, argv);

#ifndef SIL_KIT_TEST_MAIN_DO_NOT_PRINT_VERSION
    std::cout << "Running with SIL Kit version '" << SilKit::Version::String() << "' and git hash '"
              << SilKit::Version::GitHash() << "'" << std::endl;
#endif

    return RUN_ALL_TESTS();
}