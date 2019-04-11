// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>

namespace ib {
namespace util {

class PerformanceMonitor
{
public:
    PerformanceMonitor() = default;

    inline void StartMeasurement();
    inline void StopMeasurement();

    inline auto SampleCount() -> std::size_t;
    inline auto MinDuration() -> std::chrono::nanoseconds;
    inline auto MaxDuration() -> std::chrono::nanoseconds;
    template <class StdDurationT = std::chrono::duration<double, std::nano>>
    inline auto AvgDuration() -> StdDurationT;

private:
    std::chrono::high_resolution_clock::time_point _start;

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
    auto duration = std::chrono::high_resolution_clock::now() - _start;

    _minDuration = std::min(duration, _minDuration);
    _maxDuration = std::max(duration, _maxDuration);
    _durationSum += duration;
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

} // namespace util
} // namespace ib
