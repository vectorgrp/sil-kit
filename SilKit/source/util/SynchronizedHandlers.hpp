// Copyright (c) Vector Informatik GmbH. All rights reserved.

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

public:
    template <typename... T>
    auto Add(T &&...t) -> HandlerId
    {
        const auto lock = MakeUniqueLock();

        const auto handlerId = MakeHandlerId();
        _entries.emplace(handlerId, Callable{std::forward<T>(t)...});

        return handlerId;
    }

    auto Remove(const HandlerId handlerId) -> bool
    {
        const auto lock = MakeUniqueLock();

        return _entries.erase(handlerId) != 0;
    }

    template <typename... T>
    bool InvokeAll(T &&...t)
    {
        const auto lock = MakeUniqueLock();

        for (const auto &kv : _entries)
        {
            kv.second(t...);
        }

        return !_entries.empty();
    }

    auto Size() -> size_t { 
        return _entries.size();
    }

private:
    auto MakeUniqueLock() const -> std::unique_lock<Mutex> { return std::unique_lock<Mutex>{_mutex}; }

    auto MakeHandlerId() -> HandlerId { return static_cast<HandlerId>(_nextHandlerId++); }

private:
    mutable Mutex _mutex;

    // NB: _entries must not invalidate iterators on adding or removing (i.e., node-based containers like map are fine,
    //     containers like vector are not)
    // NB: access to _entries and _nextHandlerId must be protected by locking the _mutex
    std::map<HandlerId, Callable> _entries;
    std::underlying_type_t<HandlerId> _nextHandlerId = 0;
};

} // namespace Util
} // namespace SilKit
