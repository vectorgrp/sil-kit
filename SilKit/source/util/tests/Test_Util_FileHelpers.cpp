// Copyright (c) 2022 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
