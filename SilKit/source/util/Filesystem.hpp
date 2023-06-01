/* Copyright (c) 2022 Vector Informatik GmbH

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

#pragma once

#include <string>

namespace SilKit {
namespace Filesystem {

// Cross platform and portable filesystem API modeled after C++17 std::Filesystem.
// Only our required functionality is implemented.
// Will be removed when we upgrade to C++17.

class path
{
public:
    //public types
    using value_type = char; //only posix
    using string_type = std::basic_string<value_type>;

public:
    //static method
#if defined(_WIN32)
    static constexpr value_type preferred_separator = '\\';
#else
    static constexpr value_type preferred_separator = '/';
#endif
public:
    //CTor and assignment
    path() noexcept = default;
    path(const path&) = default;
    path& operator=(const path&) = default;
    path(path&&) noexcept = default;
    path& operator=(path&&) noexcept = default;
    path(const string_type& source);

public:
    // methods
    auto string() const -> std::string;
    auto c_str() const noexcept -> const value_type*;
    auto native() const noexcept -> const string_type&;
private:
    string_type _path;
};

// Functions

//! Return the current working directory of the running process.
path current_path();
//! Set the current working directory of the running process.
void current_path(const path& p);
//! Create a directory like in POSIX mkdir
bool create_directory(const path& where);

//! Get the path to the temp directory.
path temp_directory_path();

//! Remove a file or directory.
bool remove(const path&);

//! Rename a file.
void rename(const path& old_p, const path& new_p);

} // namespace Filesystem
} // namespace SilKit
