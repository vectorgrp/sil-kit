// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/util/HandlerId.hpp"

#include <map>
#include <mutex>

namespace SilKit {
namespace Util {

/// This container for callables is not thread-safe. Use SynchronizedHandlers, if thread-safe iteration and modification
/// is required.
template <typename Callable>
class Handlers
{
public:
    template <typename... T>
    auto Add(T&&... t) -> HandlerId
    {
        const auto handlerId = MakeHandlerId();
        _entries.emplace(handlerId, Callable{std::forward<T>(t)...});

        return handlerId;
    }

    auto Remove(const HandlerId handlerId) -> bool
    {
        return _entries.erase(handlerId) != 0;
    }

    template <typename... T>
    bool InvokeAll(T&&... t)
    {
        for (const auto& kv : _entries)
        {
            kv.second(t...);
        }

        return !_entries.empty();
    }

    void Clear()
    {
        _entries.clear();
    }

    auto Size() -> size_t
    {
        return _entries.size();
    }

    friend void swap(Handlers& a, Handlers& b) noexcept
    {
        using std::swap;
        swap(a._entries, b._entries);
        swap(a._nextHandlerId, b._nextHandlerId);
    }

private:
    auto MakeHandlerId() -> HandlerId
    {
        return static_cast<HandlerId>(_nextHandlerId++);
    }

private:
    // NB: entries must not invalidate iterators on adding or removing (i.e., node-based containers like map are
    //     fine, containers like vector are not)
    std::map<HandlerId, Callable> _entries;
    std::underlying_type_t<HandlerId> _nextHandlerId = 0;
};

template <typename Callable>
class SynchronizedHandlers
{
    using Mutex = std::recursive_mutex;

public:
    SynchronizedHandlers() = default;

    template <typename... T>
    auto Add(T&&... t) -> HandlerId
    {
        const auto lock = MakeUniqueLock();
        return _handlers.Add(std::forward<T>(t)...);
    }

    auto Remove(const HandlerId handlerId) -> bool
    {
        const auto lock = MakeUniqueLock();
        return _handlers.Remove(handlerId);
    }

    template <typename... T>
    bool InvokeAll(T&&... t)
    {
        const auto lock = MakeUniqueLock();
        return _handlers.InvokeAll(std::forward<T>(t)...);
    }

    void Clear()
    {
        const auto lock = MakeUniqueLock();
        _handlers.Clear();
    }

    auto Size() -> size_t
    {
        return _handlers.Size();
    }

public:
    friend void swap(SynchronizedHandlers& a, SynchronizedHandlers& b) noexcept
    {
        if (&a == &b)
        {
            return;
        }

        auto aLock = a.MakeDeferredLock();
        auto bLock = b.MakeDeferredLock();

        std::lock(aLock, bLock);

        using std::swap;
        swap(a._handlers, b._handlers);
    }

private:
    auto MakeUniqueLock() const -> std::unique_lock<Mutex>
    {
        return std::unique_lock<Mutex>{_mutex};
    }

    auto MakeDeferredLock() const -> std::unique_lock<Mutex>
    {
        return std::unique_lock<Mutex>{_mutex, std::defer_lock};
    }

private:
    mutable Mutex _mutex;

    // NB: access to _handlers must be protected by locking the _mutex
    Handlers<Callable> _handlers;
};

} // namespace Util
} // namespace SilKit
