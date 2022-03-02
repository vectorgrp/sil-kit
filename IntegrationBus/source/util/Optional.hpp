// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace ib {
namespace util {

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

} // namespace util
} // namespace ib
