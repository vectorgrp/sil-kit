// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
namespace ib {
namespace filesystem {

// Cross platform and portable filesystem API modeled after C++17 std::filesystem.
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
    path(path&&) noexcept = default;
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

//!< return the current working directory of the running process.
path current_path();

//!< Get the path to the temp directory.
path temp_directory_path();

//!< Remove a file or directory.
bool remove(const path&);

} // namespace filesystem
} // namespace ib
