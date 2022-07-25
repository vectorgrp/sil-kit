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

namespace SilKit {
namespace Util {

//! \brief Super simple drop in replacement for std::optional
//!
//!  NB: Replace with std::optional once we moved to C++17
template<class T>
class Optional
{
public:
    // ----------------------------------------
    // Public Data Types
    using value_type = T;

public:
    // ----------------------------------------
    // Constructors and Destructor
    Optional() = default;
    Optional(const Optional&) = default;
    Optional(Optional&&) = default;
    Optional(const T& value) : _value{value}, _has_value{true} {}
    Optional(T&& value) : _value{std::move(value)}, _has_value{true} {}
                
public:
    // ----------------------------------------
    // Operator Implementations
    Optional& operator=(const Optional& other) = default;
    Optional& operator=(Optional&& other) = default;

public:
    // ----------------------------------------
    // Public Methods
    //
    auto operator=(const T& value) { _value = value; _has_value = true; }
    auto operator=(T&& value) { _value = std::move(value); _has_value = true; }
    
    auto operator->() const -> const T* { return &_value; };
    auto operator->() -> T* { return &_value; };
    auto operator*() const -> const T& { return _value; };
    auto operator*() -> T& { return _value; };

    operator bool() const noexcept { return _has_value; }
    auto has_value() const noexcept -> bool { return _has_value; }

    auto value() const -> const T& { return _value; };
    auto value() -> T& { return _value; };

    /// <summary>
    /// Be aware that this does NOT actually delete the value!
    /// </summary>
    void reset() { _has_value = false; }

private:
    // ----------------------------------------
    // private members
    T _value;
    bool _has_value={false};
};

template<class T>
bool operator==(const Optional<T>& lhs, const Optional<T>& rhs)
{
    if (!lhs.has_value() || !rhs.has_value())
        return lhs.has_value() == rhs.has_value();

    return lhs.has_value() == rhs.has_value()
        && lhs.value() == rhs.value();
}

} // namespace Util
} // namespace SilKit
