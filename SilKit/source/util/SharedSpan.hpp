// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/util/Span.hpp"

#include <algorithm>
#include <memory>
#include <type_traits>
#include <vector>

namespace SilKit {
namespace Util {

/*! \brief A Span<const T> together with shared ownership of the memory it views.
 *
 * Either owns a copy of a caller's payload, or views part of a larger buffer kept alive by a
 * shared_ptr, e.g. a deserialized payload aliasing the received message blob.
 *
 * Invariant: the viewed elements are immutable for as long as a SharedSpan referring to them
 * exists. Fill a buffer first, then wrap it.
 */
template <typename T>
class SharedSpan
{
    static_assert(!std::is_const<T>::value, "T must not be const");
    static_assert(!std::is_reference<T>::value, "T must not be a reference");

public:
    SharedSpan() = default;

    // NB: implicit, so that SharedSpan is a drop-in for a payload field assigned from a Span or a
    //     vector. These always allocate and copy.
    SharedSpan(std::vector<T> vector);
    SharedSpan(std::initializer_list<T> initializerList);
    SharedSpan(Span<const T> span, size_t minimumSize = 0, T padValue = T{});

    /*! \brief View elements whose storage is kept alive by owner.
     *
     * Precondition: view refers to elements inside the object owned by owner. Prefer Subspan() or
     * MakeSharedSpan(), which range-check.
     */
    template <typename U>
    SharedSpan(std::shared_ptr<U> owner, Span<const T> view);

    // NB: & -qualified so that a span cannot be taken from a temporary SharedSpan, which would
    //     drop the last reference to the owner and leave the span dangling.
    auto AsSpan() const& -> Span<const T>;

    auto size() const -> size_t;
    auto empty() const -> bool;

    //! \brief A view of count elements starting at offset, retaining the same owner.
    auto Subspan(size_t offset, size_t count) const -> SharedSpan;

private:
    std::shared_ptr<const void> _owner;
    Span<const T> _view;
};

template <typename T>
bool ItemsAreEqual(const SharedSpan<T>& lhs, const SharedSpan<T>& rhs);

//! \brief Range-checked construction of a view into a byte blob.
inline auto MakeSharedSpan(std::shared_ptr<const std::vector<uint8_t>> blob, size_t offset, size_t count)
    -> SharedSpan<uint8_t>;

// ================================================================================
//  Inline Implementations
// ================================================================================

template <typename T>
SharedSpan<T>::SharedSpan(std::vector<T> vector)
{
    auto owner = std::make_shared<std::vector<T>>(std::move(vector));
    _view = Span<const T>{owner->data(), owner->size()};
    _owner = std::move(owner);
}

template <typename T>
SharedSpan<T>::SharedSpan(std::initializer_list<T> initializerList)
    : SharedSpan(std::vector<T>{initializerList})
{
}

template <typename T>
SharedSpan<T>::SharedSpan(const Span<const T> span, const size_t minimumSize, const T padValue)
{
    auto owner = std::make_shared<std::vector<T>>(span.begin(), span.end());
    owner->resize((std::max)(owner->size(), minimumSize), padValue);
    _view = Span<const T>{owner->data(), owner->size()};
    _owner = std::move(owner);
}

template <typename T>
template <typename U>
SharedSpan<T>::SharedSpan(std::shared_ptr<U> owner, Span<const T> view)
    : _owner{std::move(owner)}
    , _view{view}
{
}

template <typename T>
auto SharedSpan<T>::AsSpan() const& -> Span<const T>
{
    return _view;
}

template <typename T>
auto SharedSpan<T>::size() const -> size_t
{
    return _view.size();
}

template <typename T>
auto SharedSpan<T>::empty() const -> bool
{
    return _view.empty();
}

template <typename T>
auto SharedSpan<T>::Subspan(size_t offset, size_t count) const -> SharedSpan
{
    if (offset > _view.size() || count > _view.size() - offset)
    {
        throw OutOfRangeError("invalid SharedSpan<T>::Subspan range");
    }

    SharedSpan result;
    result._owner = _owner;
    result._view = Span<const T>{_view.data() + offset, count};
    return result;
}

template <typename T>
bool ItemsAreEqual(const SharedSpan<T>& lhs, const SharedSpan<T>& rhs)
{
    return ItemsAreEqual(lhs.AsSpan(), rhs.AsSpan());
}

auto MakeSharedSpan(std::shared_ptr<const std::vector<uint8_t>> blob, size_t offset, size_t count)
    -> SharedSpan<uint8_t>
{
    if (blob == nullptr)
    {
        if (offset != 0 || count != 0)
        {
            throw OutOfRangeError("invalid MakeSharedSpan range for an empty blob");
        }
        return SharedSpan<uint8_t>{};
    }

    if (offset > blob->size() || count > blob->size() - offset)
    {
        throw OutOfRangeError("invalid MakeSharedSpan range");
    }

    const auto view = Span<const uint8_t>{blob->data() + offset, count};
    return SharedSpan<uint8_t>{std::move(blob), view};
}

} // namespace Util
} // namespace SilKit
