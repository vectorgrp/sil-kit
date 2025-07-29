// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "FileHelpers.hpp"

#include <fstream>
#include <random>

namespace {

const std::string content{R"(
This is the sample content for the
FileHelpers tests.
)"};

TEST(Test_Util_FileHelpers, ReadTextFile)
{
    const auto fileName = [] {
        std::mt19937 rng{0};
        std::uniform_int_distribution<int> chr{static_cast<int>('a'), static_cast<int>('z')};
        std::string result{"Test_Util_FileHelpers_TestFile_"};
        for (int index = 0; index != 10; ++index)
        {
            result.push_back(static_cast<char>(chr(rng)));
        }
        return result;
    }();

    {
        std::ofstream output{fileName, std::ios::binary | std::ios::out | std::ios::trunc};
        output << content;
        output.close();
    }

    {
        const auto readContent = SilKit::Util::ReadTextFile(fileName);
        ASSERT_EQ(readContent, content);
    }
}

} // anonymous namespace
