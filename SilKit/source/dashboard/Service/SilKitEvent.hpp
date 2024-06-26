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

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"
#include "ServiceDatatypes.hpp"

namespace SilKit {
namespace Dashboard {

struct SimulationStart
{
    std::string connectUri;
    uint64_t time;
};

struct ServiceData
{
    Core::Discovery::ServiceDiscoveryEvent::Type discoveryType;
    Core::ServiceDescriptor serviceDescriptor;
};

struct SimulationEnd
{
    uint64_t time;
};

enum class SilKitEventType
{
    OnSimulationStart,
    OnParticipantConnected,
    OnSystemStateChanged,
    OnParticipantStatusChanged,
    OnServiceDiscoveryEvent,
    OnSimulationEnd
};

template <SilKitEventType id>
struct TypeIdTrait
{
    static const SilKitEventType typeId = id;
};

template <class T>
struct SilKitEventTrait;

#define SILKIT_EVENT(TYPENAME, SILKIT_EVENT_TYPE_ENUMERATOR) \
    template <> \
    struct SilKitEventTrait<TYPENAME> : TypeIdTrait<SilKitEventType::SILKIT_EVENT_TYPE_ENUMERATOR> \
    { \
    };

SILKIT_EVENT(SimulationStart, OnSimulationStart)
SILKIT_EVENT(Services::Orchestration::ParticipantConnectionInformation, OnParticipantConnected)
SILKIT_EVENT(Services::Orchestration::SystemState, OnSystemStateChanged)
SILKIT_EVENT(Services::Orchestration::ParticipantStatus, OnParticipantStatusChanged)
SILKIT_EVENT(ServiceData, OnServiceDiscoveryEvent)
SILKIT_EVENT(SimulationEnd, OnSimulationEnd)

#undef SILKIT_EVENT

class SilKitEvent
{
public:
    SilKitEvent() = delete;

    SilKitEvent(const SilKitEvent& other)
        : _type(other._type)
        , _simulationName{other._simulationName}
        , _data(other._clone(other._data))
        , _clone(other._clone)
        , _destroy(other._destroy)
    {
    }

    SilKitEvent(SilKitEvent&& other) noexcept
    {
        swap(other);
    }

    ~SilKitEvent()
    {
        if (_destroy != nullptr)
        {
            _destroy(_data);
        }
    };

    template <typename T>
    explicit SilKitEvent(std::string simulationName, const T& value)
        : _type{getTypeId<T>()}
        , _simulationName{std::move(simulationName)}
        , _data{new T{value}}
        , _clone([](void* otherData) -> void* { return new T(*static_cast<T*>(otherData)); })
        , _destroy([](void* data) { delete static_cast<T*>(data); })
    {
    }

    SilKitEvent& operator=(const SilKitEvent& other)
    {
        if (this == &other)
        {
            return *this;
        }
        if (_destroy != nullptr)
        {
            _destroy(_data);
        }
        _type = other._type;
        _simulationName = other._simulationName;
        _data = other._clone(other._data);
        _clone = other._clone;
        _destroy = other._destroy;
        return *this;
    }

    SilKitEvent& operator=(SilKitEvent&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }
        swap(other);
        return *this;
    }

    auto Type() const -> SilKitEventType
    {
        return _type;
    }

    auto GetSimulationName() const -> const std::string&
    {
        return _simulationName;
    }

    auto GetSimulationStart() const -> const SimulationStart&
    {
        return Get<SimulationStart>();
    }

    auto GetParticipantConnectionInformation() const -> const Services::Orchestration::ParticipantConnectionInformation&
    {
        return Get<Services::Orchestration::ParticipantConnectionInformation>();
    }

    auto GetParticipantStatus() const -> const Services::Orchestration::ParticipantStatus&
    {
        return Get<Services::Orchestration::ParticipantStatus>();
    }

    auto GetSystemState() const -> const Services::Orchestration::SystemState&
    {
        return Get<Services::Orchestration::SystemState>();
    }

    auto GetServiceData() const -> const ServiceData&
    {
        return Get<ServiceData>();
    }

    auto GetSimulationEnd() const -> const SimulationEnd&
    {
        return Get<SimulationEnd>();
    }

private:
    template <typename T>
    constexpr SilKitEventType getTypeId() const
    {
        return SilKitEventTrait<T>::typeId;
    }

    template <typename T>
    inline const T& Get() const;

    inline void swap(SilKitEvent& other) noexcept;

private:
    SilKitEventType _type{};
    std::string _simulationName;
    void* _data{nullptr};
    void* (*_clone)(void* otherData){nullptr};
    void (*_destroy)(void* data){nullptr};
};

template <typename T>
const T& SilKitEvent::Get() const
{
    const auto tag = getTypeId<T>();
    if (tag != _type)
    {
        throw SilKitError("SilKitEvent::Get() Requested type does not match stored type.");
    }

    return *(reinterpret_cast<const T*>(_data));
}

void SilKitEvent::swap(SilKitEvent& other) noexcept
{
    using std::swap;
    swap(_type, other._type);
    swap(_simulationName, other._simulationName);
    swap(_data, other._data);
    swap(_clone, other._clone);
    swap(_destroy, other._destroy);
}

} // namespace Dashboard

} // namespace SilKit
