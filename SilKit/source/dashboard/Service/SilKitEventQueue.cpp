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

#include "SilKitEventQueue.hpp"

namespace SilKit {

namespace Dashboard {

SilKitEventQueue::SilKitEventQueue() {}

SilKitEventQueue::~SilKitEventQueue() {}

void SilKitEventQueue::Enqueue(const SilKitEvent& obj)
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

bool SilKitEventQueue::DequeueAllInto(std::vector<SilKitEvent>& events)
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    _cv.wait(lock, [this] { return !_queue.empty() || _stop; });
    std::move(_queue.begin(), _queue.end(), std::back_inserter(events));
    _queue.clear();
    return !events.empty();
}

void SilKitEventQueue::Stop()
{
    {
        std::lock_guard<decltype(_mutex)> lock{_mutex};
        _stop = true;
    }
    _cv.notify_one();
}

} // namespace Dashboard
} // namespace SilKit