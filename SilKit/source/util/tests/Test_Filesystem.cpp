/* Copyright (c) 2023 Vector Informatik GmbH

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
#include "Filesystem.hpp"

#include <iostream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace SilKit {
namespace Util {
namespace tests {

TEST(UtilsFilesystemTest, get_parent_path)
{
    const auto some_path = Filesystem::path("/This/Is/Some/Path/To/A/File/");
    auto parent_path = Filesystem::parent_path(some_path);

    const auto parent_path_ref1 = std::string("/This/Is/Some/Path/To/A/File");
    ASSERT_EQ(parent_path_ref1, parent_path.string());

    parent_path = Filesystem::parent_path(parent_path.string());
    const auto parent_path_ref2 = std::string("/This/Is/Some/Path/To/A");
    ASSERT_EQ(parent_path_ref2, parent_path.string());

    parent_path = Filesystem::parent_path(parent_path.string());
    const auto parent_path_ref3 = std::string("/This/Is/Some/Path/To");
    ASSERT_EQ(parent_path_ref3, parent_path.string());
}

TEST(UtilsFilesystemTest, test_root_parent)
{
    const Filesystem::path root_path{"RootFile"};
    const auto parent_path = Filesystem::parent_path(root_path);

    ASSERT_EQ("", parent_path.string());
}

TEST(UtilsFilesystemTest, test_concat_paths)
{
    const Filesystem::path file_name{"File"};
    const Filesystem::path root_path{"/Path/To/"};
    const auto file_path = Filesystem::concatenate_paths(root_path, file_name);

    ASSERT_EQ("/Path/To/File", file_path.string());
}

TEST(UtilsFilesystemTest, test_concat_path_strings)
{
    const auto file_path = Filesystem::concatenate_paths("/Path/To", "File");

    ASSERT_EQ("/Path/To/File", file_path.string());
}
} // namespace tests
} // namespace Util
} // namespace SilKit
