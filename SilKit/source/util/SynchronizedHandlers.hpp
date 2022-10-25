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

#include "silkit/util/HandlerId.hpp"

#include <map>
#include <mutex>

namespace SilKit {
namespace Util {

template <typename Callable>
class SynchronizedHandlers
{
    using Mutex = std::recursive_mutex;

    struct State
    {
        // NB: entries must not invalidate iterators on adding or removing (i.e., node-based containers like map are
        //     fine, containers like vector are not)
        std::map<HandlerId, Callable> entries;
        std::underlying_type_t<HandlerId> nextHandlerId = 0;

        friend void swap(State &a, State &b) noexcept
        {
            using std::swap;
            swap(a.entries, b.entries);
            swap(a.nextHandlerId, b.nextHandlerId);
        }
    };

public:
    SynchronizedHandlers() = default;

    template <typename... T>
    auto Add(T &&...t) -> HandlerId
    {
        const auto lock = MakeUniqueLock();

        const auto handlerId = MakeHandlerId();
        _state.entries.emplace(handlerId, Callable{std::forward<T>(t)...});

        return handlerId;
    }

    auto Remove(const HandlerId handlerId) -> bool
    {
        const auto lock = MakeUniqueLock();

        return _state.entries.erase(handlerId) != 0;
    }

    template <typename... T>
    bool InvokeAll(T &&...t)
    {
        const auto lock = MakeUniqueLock();

        for (const auto &kv : _state.entries)
        {
            kv.second(t...);
        }

        return !_state.entries.empty();
    }

    auto Size() -> size_t { return _state.entries.size(); }

public:
    friend void swap(SynchronizedHandlers &a, SynchronizedHandlers &b) noexcept
    {
        if (&a == &b)
        {
            return;
        }

        auto aLock = a.MakeDeferredLock();
        auto bLock = b.MakeDeferredLock();

        std::lock(aLock, bLock);

        using std::swap;
        swap(a._state, b._state);
    }

private:
    auto MakeUniqueLock() const -> std::unique_lock<Mutex> { return std::unique_lock<Mutex>{_mutex}; }

    auto MakeDeferredLock() const -> std::unique_lock<Mutex>
    {
        return std::unique_lock<Mutex>{_mutex, std::defer_lock};
    }

    auto MakeHandlerId() -> HandlerId { return static_cast<HandlerId>(_state.nextHandlerId++); }

private:
    mutable Mutex _mutex;

    // NB: access to _state must be protected by locking the _mutex
    State _state;
};

} // namespace Util
} // namespace SilKit
