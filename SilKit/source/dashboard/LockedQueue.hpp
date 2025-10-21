// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>

namespace VSilKit {

template <typename T>
class LockedQueue
{
public:
    ~LockedQueue();
    void Enqueue(T&& obj);
    void Enqueue(const T& obj);
    bool DequeueAllInto(std::vector<T>& events);
    void Stop();

protected:
    std::mutex _mutex;
    std::condition_variable _cv;
    std::deque<T> _queue;
    bool _stop{false};
};


template <typename T>
LockedQueue<T>::~LockedQueue()
{
    Stop();
}

template <typename T>
void LockedQueue<T>::Enqueue(const T& obj)
{
    {
        std::lock_guard<decltype(_mutex)> lock{_mutex};
        if (!_stop)
        {
            _queue.push_back(obj);
        }
    }
    _cv.notify_one();
}

template <typename T>
void LockedQueue<T>::Enqueue(T&& obj)
{
    {
        std::lock_guard<decltype(_mutex)> lock{_mutex};
        if (!_stop)
        {
            _queue.emplace_back(std::move(obj));
        }
    }
    _cv.notify_one();
}
template <typename T>
bool LockedQueue<T>::DequeueAllInto(std::vector<T>& events)
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    _cv.wait(lock, [this] { return !_queue.empty() || _stop; });
    std::move(_queue.begin(), _queue.end(), std::back_inserter(events));
    _queue.clear();
    return !events.empty();
}

template <typename T>
void LockedQueue<T>::Stop()
{
    {
        std::lock_guard<decltype(_mutex)> lock{_mutex};
        _stop = true;
    }
    _cv.notify_one();
}

} // namespace VSilKit