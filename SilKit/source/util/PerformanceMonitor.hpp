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

#include <chrono>

namespace SilKit {
namespace Util {

class PerformanceMonitor
{
public:
    PerformanceMonitor() = default;

    inline void StartMeasurement();
    inline void StopMeasurement();

    inline auto CurrentDuration() -> std::chrono::nanoseconds;
    inline auto SampleCount() -> std::size_t;
    inline auto MinDuration() -> std::chrono::nanoseconds;
    inline auto MaxDuration() -> std::chrono::nanoseconds;
    template <class StdDurationT = std::chrono::duration<double, std::nano>>
    inline auto AvgDuration() -> StdDurationT;

private:
    std::chrono::high_resolution_clock::time_point _start;
    std::chrono::nanoseconds _currentDuration{0};

    std::chrono::nanoseconds _maxDuration{0};
    std::chrono::nanoseconds _minDuration{std::chrono::nanoseconds::max()};

    std::chrono::nanoseconds _durationSum{0};
    size_t _sampleCount{0u};
};


void PerformanceMonitor::StartMeasurement()
{
    _start = std::chrono::high_resolution_clock::now();
}
void PerformanceMonitor::StopMeasurement()
{
    _currentDuration = std::chrono::high_resolution_clock::now() - _start;

    _minDuration = std::min(_currentDuration, _minDuration);
    _maxDuration = std::max(_currentDuration, _maxDuration);
    _durationSum += _currentDuration;
    _sampleCount++;
}

auto PerformanceMonitor::SampleCount() -> std::size_t
{
    return _sampleCount;
}
auto PerformanceMonitor::MinDuration() -> std::chrono::nanoseconds
{
    return _minDuration;
}
auto PerformanceMonitor::CurrentDuration() -> std::chrono::nanoseconds
{
    return _currentDuration;
}
auto PerformanceMonitor::MaxDuration() -> std::chrono::nanoseconds {
    return _maxDuration;
}
template <class StdDurationT>
auto PerformanceMonitor::AvgDuration() -> StdDurationT
{
    if (_sampleCount == 0)
        return StdDurationT{static_cast<typename StdDurationT::rep>(0)};
    else
        return std::chrono::duration_cast<StdDurationT>(_durationSum) / _sampleCount;
}

} // namespace Util
} // namespace SilKit
