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

#include <mutex>
#include <functional>
#include <tuple>

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"
#include "ServiceDatatypes.hpp"

namespace SilKit {

namespace Dashboard {

//helper to cache multiple callback parameters
struct ServiceData
{
    SilKit::Core::Discovery::ServiceDiscoveryEvent::Type discoveryType;
    SilKit::Core::ServiceDescriptor serviceDescriptor;
};

struct CachedData
{
    template <typename... Ts>
    using ContainterT = std::vector<Ts...>;

    std::mutex _mutex;
    using LockT = std::unique_lock<decltype(_mutex)>;
    std::tuple<ContainterT<Services::Orchestration::ParticipantConnectionInformation>,
               ContainterT<Services::Orchestration::SystemState>,
               ContainterT<Services::Orchestration::ParticipantStatus>, ContainterT<ServiceData>>
        _dataSets;
    template <typename T>
    void Insert(const T& datum)
    {
        LockT lock{_mutex};
        using data_t = ContainterT<std::decay_t<T>>;
        auto&& dataSet = std::get<data_t>(_dataSets);
        dataSet.push_back(datum);
    }

    template <typename T>
    ContainterT<T> GetAndClear()
    {
        LockT lock{_mutex};
        using data_t = ContainterT<std::decay_t<T>>;
        auto&& dataSet = std::get<data_t>(_dataSets);
        auto dataCopy = std::move(dataSet);
        return dataCopy;
    }
};

} // namespace Dashboard
} // namespace SilKit