#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/SilKitVersion.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "Running main() from SIlKitTestMain.cpp with SIL Kit version " << SilKit::Version::String() << " from "
              << SilKit::Version::GitHash() << std::endl;
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}