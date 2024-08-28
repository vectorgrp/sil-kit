// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "MetricsManager.hpp"

#include "Assert.hpp"
#include "MetricsProcessor.hpp"

#include <string>
#include <sstream>

#include <cmath>
#include <future>
#include <thread>
#include <vector>

#include "fmt/format.h"

namespace {

// On Windows builds with optimization, std::chrono::steady_clock sometimes returns the same values,
// when called too fast. We make sure that calls to ::now are strict monotonically increasing:
#ifdef _WIN32
auto MetricClockNow() -> VSilKit::MetricTimePoint
{
    static thread_local auto sLastTimePoint = VSilKit::MetricClock::now();
    const auto now = VSilKit::MetricClock::now();
    if (now == sLastTimePoint)
    {
        sLastTimePoint = sLastTimePoint + std::chrono::nanoseconds{1};
        return sLastTimePoint;
    }
    sLastTimePoint = now;
    return now;
}
#else
// we need no overhead on other systems
auto MetricClockNow() -> VSilKit::MetricTimePoint
{
    return VSilKit::MetricClock::now();
}
#endif
} // namespace
namespace VSilKit {


class MetricsManager::CounterMetric
    : public ICounterMetric
    , public IMetric
{
public:
    CounterMetric();

public: // ICounterMetric
    void Add(uint64_t delta) override;
    void Set(uint64_t value) override;

public: // MetricsManager::IMetric
    auto GetMetricKind() const -> MetricKind override;
    auto GetUpdateTime() const -> MetricTimePoint override;
    auto FormatValue() const -> std::string override;

private:
    MetricTimePoint _timestamp;
    uint64_t _value{0};
};


class MetricsManager::StatisticMetric
    : public IStatisticMetric
    , public IMetric
{
public:
    StatisticMetric();

public: // IStatisticMetric
    void Take(double value) override;

public: // MetricsManager::IMetric
    auto GetMetricKind() const -> MetricKind override;
    auto GetUpdateTime() const -> MetricTimePoint override;
    auto FormatValue() const -> std::string override;

private:
    MetricTimePoint _timestamp;
    double _mean{0.0};
    double _variance{0.0};
    double _minimum{std::numeric_limits<double>::max()};
    double _maximum{std::numeric_limits<double>::lowest()};

    // see https://fanf2.user.srcf.net/hermes/doc/antiforgery/stats.pdf
    // Incremental calculation of weighted mean and variance
    // by Tony Finch (fanf2@cam.ac.uk) University of Cambridge Computing Service
};


class MetricsManager::StringListMetric
    : public IStringListMetric
    , public IMetric
{
public:
    StringListMetric();

public: // IStringListMetric
    void Clear() override;
    void Add(std::string const &value) override;

public: // MetricsManager::IMetric
    auto GetMetricKind() const -> MetricKind override;
    auto GetUpdateTime() const -> MetricTimePoint override;
    auto FormatValue() const -> std::string override;

private:
    MetricTimePoint _timestamp;
    std::vector<std::string> _strings;
};


MetricsManager::MetricsManager(std::string participantName, IMetricsProcessor &processor)
    : _participantName{std::move(participantName)}
    , _processor{&processor}
    , _lastSubmitUpdate{MetricClockNow()}
{
}


void MetricsManager::SetLogger(SilKit::Services::Logging::ILogger &logger)
{
    _logger = &logger;
}

void MetricsManager::SubmitUpdates()
{
    MetricsUpdate msg{};

    {
        std::lock_guard<decltype(_mutex)> lock{_mutex};

        msg.metrics.reserve(_metrics.size());

        const auto to_ns = [](const auto timepoint) {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(timepoint.time_since_epoch()).count();
        };

        for (const auto &pair : _metrics)
        {
            const auto &name = pair.first;
            const auto *metric = pair.second.get();

            const auto timepoint = metric->GetUpdateTime();
            if (timepoint <= _lastSubmitUpdate)
            {
                continue;
            }

            MetricData data{};
            data.timestamp = to_ns(timepoint);
            data.name = name;
            data.kind = metric->GetMetricKind();
            data.value = metric->FormatValue();

            msg.metrics.emplace_back(std::move(data));
        }

        _lastSubmitUpdate = MetricClockNow();
    }

    if (!msg.metrics.empty())
    {
        _processor->Process(_participantName, msg);
    }
}


// IMetricsManager

auto MetricsManager::GetCounter(const std::string &name) -> ICounterMetric *
{
    return &dynamic_cast<ICounterMetric &>(*GetOrCreateMetric(name, MetricKind::COUNTER));
}

auto MetricsManager::GetStatistic(const std::string &name) -> IStatisticMetric *
{
    return &dynamic_cast<IStatisticMetric &>(*GetOrCreateMetric(name, MetricKind::STATISTIC));
}

auto MetricsManager::GetStringList(const std::string &name) -> IStringListMetric *
{
    return &dynamic_cast<IStringListMetric &>(*GetOrCreateMetric(name, MetricKind::STRING_LIST));
}


// MetricsManager

auto MetricsManager::GetOrCreateMetric(std::string name, MetricKind kind) -> IMetric *
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};

    auto it = _metrics.find(name);

    if (it == _metrics.end())
    {
        switch (kind)
        {
        case MetricKind::COUNTER:
            it = _metrics.emplace(name, std::make_unique<CounterMetric>()).first;
            break;
        case MetricKind::STATISTIC:
            it = _metrics.emplace(name, std::make_unique<StatisticMetric>()).first;
            break;
        case MetricKind::STRING_LIST:
            it = _metrics.emplace(name, std::make_unique<StringListMetric>()).first;
            break;
        default:
            throw SilKit::SilKitError{fmt::format("Invalid MetricKind ({})", kind)};
        }
    }

    const auto existingKind = it->second->GetMetricKind();
    if (existingKind != kind)
    {
        throw SilKit::SilKitError{
            fmt::format("Existing metric {} has different kind {} than the requested {}", name, existingKind, kind)};
    }

    return it->second.get();
}


// CounterMetric

MetricsManager::CounterMetric::CounterMetric() = default;

void MetricsManager::CounterMetric::Add(uint64_t delta)
{
    _timestamp = MetricClockNow();
    _value += delta;
}

void MetricsManager::CounterMetric::Set(uint64_t value)
{
    _timestamp = MetricClockNow();
    _value = value;
}

auto MetricsManager::CounterMetric::GetMetricKind() const -> MetricKind
{
    return MetricKind::COUNTER;
}

auto MetricsManager::CounterMetric::GetUpdateTime() const -> MetricTimePoint
{
    return _timestamp;
}

auto MetricsManager::CounterMetric::FormatValue() const -> std::string
{
    return std::to_string(_value);
}


// StatisticMetric

MetricsManager::StatisticMetric::StatisticMetric() = default;

void MetricsManager::StatisticMetric::Take(double value)
{
    _timestamp = MetricClockNow();

    constexpr double alpha = 0.5;

    const auto difference = value - _mean;
    const auto increment = alpha * difference;

    _mean += increment;
    _variance = (1.0 - alpha) * (_variance + difference * increment);
    _minimum = std::min(_minimum, value);
    _maximum = std::max(_maximum, value);
}

auto MetricsManager::StatisticMetric::GetMetricKind() const -> MetricKind
{
    return MetricKind::STATISTIC;
}

auto MetricsManager::StatisticMetric::GetUpdateTime() const -> MetricTimePoint
{
    return _timestamp;
}

auto MetricsManager::StatisticMetric::FormatValue() const -> std::string
{
    return fmt::format(R"([{},{},{},{}])", _mean, std::sqrt(_variance), _minimum, _maximum);
}


// StringListMetric

MetricsManager::StringListMetric::StringListMetric() = default;

void MetricsManager::StringListMetric::Clear()
{
    _timestamp = MetricClockNow();
    _strings.clear();
}

void MetricsManager::StringListMetric::Add(std::string const &value)
{
    _timestamp = MetricClockNow();
    _strings.emplace_back(value);
}

auto MetricsManager::StringListMetric::GetMetricKind() const -> MetricKind
{
    return MetricKind::STRING_LIST;
}

auto MetricsManager::StringListMetric::GetUpdateTime() const -> MetricTimePoint
{
    return _timestamp;
}

auto MetricsManager::StringListMetric::FormatValue() const -> std::string
{
    std::string result;
    const auto formatString = [](std::string &result, const std::string &string) {
        result.push_back('"');
        for (const char ch : string)
        {
            switch (ch)
            {
            case '\\':
                result.push_back(ch);
                result.push_back(ch);
                break;
            case '"':
                result.push_back('\\');
                result.push_back(ch);
                break;
            default:
                result.push_back(ch);
                break;
            }
        }
        result.push_back('"');
    };

    result.push_back('[');
    for (size_t index = 0; index != _strings.size(); ++index)
    {
        if (index != 0)
        {
            result.push_back(',');
        }
        formatString(result, _strings[index]);
    }
    result.push_back(']');
    return result;
}


} // namespace VSilKit
