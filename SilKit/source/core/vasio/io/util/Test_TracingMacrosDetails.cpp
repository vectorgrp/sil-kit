// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "TracingMacros.hpp"


#include <string>
#include <vector>


namespace {


TEST(Test_TracingMacrosDetails, ExtractFileNameFromFileMacro)
{
    auto Extract{[](fmt::string_view filePath) {
        const auto fileName{VSilKit::Details::ExtractFileNameFromFileMacro(filePath)};
        return std::string{fileName.begin(), fileName.end()};
    }};

    // absolute

    EXPECT_EQ("absolute.cpp", Extract("C:\\Some\\Windows\\Path\\absolute.cpp"));
    EXPECT_EQ("absolute.cpp", Extract("C:\\absolute.cpp"));
    EXPECT_EQ("absolute.cpp", Extract("\\absolute.cpp"));
    EXPECT_EQ("absolute.cpp", Extract("/some/posix/path/absolute.cpp"));
    EXPECT_EQ("absolute.cpp", Extract("/absolute.cpp"));

    EXPECT_EQ("a", Extract("C:\\Some\\Windows\\Path\\a"));
    EXPECT_EQ("a", Extract("C:\\a"));
    EXPECT_EQ("a", Extract("\\a"));
    EXPECT_EQ("a", Extract("/some/posix/path/a"));
    EXPECT_EQ("a", Extract("/a"));

    // relative

    EXPECT_EQ("relative.cpp", Extract(".\\relative.cpp"));
    EXPECT_EQ("relative.cpp", Extract("./relative.cpp"));

    EXPECT_EQ("r", Extract(".\\r"));
    EXPECT_EQ("r", Extract("./r"));

    EXPECT_EQ("relative.cpp", Extract(".\\Subdirectory\\relative.cpp"));
    EXPECT_EQ("relative.cpp", Extract("./subdirectory/relative.cpp"));

    EXPECT_EQ("r", Extract(".\\Subdirectory\\r"));
    EXPECT_EQ("r", Extract("./subdirectory/r"));

    // lone file name

    EXPECT_EQ("lone.cpp", Extract("lone.cpp"));
    EXPECT_EQ("l", Extract("l"));
}


} // namespace
