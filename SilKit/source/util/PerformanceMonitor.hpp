// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
auto PerformanceMonitor::MaxDuration() -> std::chrono::nanoseconds
{
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
