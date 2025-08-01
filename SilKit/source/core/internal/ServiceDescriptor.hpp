// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <string>
#include <sstream>
#include <map>

#include "StructuredLoggingKeys.hpp"
#include "ServiceConfigKeys.hpp"
#include "Configuration.hpp"
#include "EndpointAddress.hpp"
#include "Hash.hpp"

namespace SilKit {
namespace Core {
//forward
class MessageBuffer;

using SupplementalData = std::map<std::string, std::string>;

enum class ServiceType : uint8_t
{
    Undefined = 0,
    Link = 1,
    Controller = 2,
    SimulatedController = 3,
    InternalController = 4
};
inline std::string to_string(const ServiceType& serviceType)
{
    switch (serviceType)
    {
    default:
        return "Unknown";
    case ServiceType::Undefined:
        return "Undefined";
    case ServiceType::Link:
        return "Link";
    case ServiceType::Controller:
        return "Controller";
    case ServiceType::SimulatedController:
        return "SimulatedController";
    case ServiceType::InternalController:
        return "InternalController";
    }
}

class ServiceDescriptor
{
public:
    // helpers
    inline bool operator==(const ServiceDescriptor& rhs) const;
    inline bool operator!=(const ServiceDescriptor& rhs) const;
    inline std::string to_string() const;
    inline std::vector<std::pair<std::string, std::string>> to_keyValues() const;

    inline Core::EndpointAddress to_endpointAddress() const;

public:
    inline auto GetParticipantId() const -> ParticipantId;
    inline void SetParticipantId(ParticipantId id);
    inline auto GetParticipantName() const -> const std::string&;
    //! \brief Setting the participant name will also set the participant ID to a computed value.
    inline void SetParticipantNameAndComputeId(std::string val);

    inline auto GetServiceType() const -> SilKit::Core::ServiceType;
    inline void SetServiceType(SilKit::Core::ServiceType val);

    inline auto GetNetworkName() const -> const std::string&;
    inline void SetNetworkName(std::string val);

    inline auto GetNetworkType() const -> SilKit::Config::NetworkType;
    inline void SetNetworkType(SilKit::Config::NetworkType val);

    inline auto GetServiceName() const -> const std::string&;
    inline void SetServiceName(std::string val);

    inline auto GetServiceId() const -> SilKit::Core::EndpointId;
    inline void SetServiceId(SilKit::Core::EndpointId val);

    inline auto GetSupplementalData() const -> SupplementalData;
    inline void SetSupplementalData(SupplementalData val);

    inline bool GetSupplementalDataItem(const std::string& key, std::string& value) const;
    inline void SetSupplementalDataItem(std::string key, std::string val);

    inline auto GetSimulationName() const -> const std::string&;
    inline void SetSimulationName(const std::string& simulationName);

public: // CTor
    ServiceDescriptor() = default;
    ServiceDescriptor(ServiceDescriptor&&) noexcept = default;
    ServiceDescriptor(const ServiceDescriptor&) = default;
    ServiceDescriptor& operator=(ServiceDescriptor&&) = default;
    ServiceDescriptor& operator=(const ServiceDescriptor&) = default;
    // for unit tests
    inline ServiceDescriptor(std::string participantName, std::string networkName, std::string serviceName,
                             EndpointId serviceId);

private:
    // Ser/Des
    friend auto operator<<(SilKit::Core::MessageBuffer& buffer,
                           const SilKit::Core::ServiceDescriptor& msg) -> SilKit::Core::MessageBuffer&;
    friend auto operator>>(SilKit::Core::MessageBuffer& buffer,
                           SilKit::Core::ServiceDescriptor& updatedMsg) -> SilKit::Core::MessageBuffer&;

private:
    std::string _participantName; //!< name of the participant
    ParticipantId _participantId{0};
    ServiceType _serviceType{ServiceType::Undefined};
    std::string _networkName; //!< the service's link name
    SilKit::Config::NetworkType _networkType{SilKit::Config::NetworkType::Invalid};
    std::string _serviceName;
    EndpointId _serviceId{0};
    SupplementalData _supplementalData;
};

//////////////////////////////////////////////////////////////////////
// Inline Implementations
//////////////////////////////////////////////////////////////////////

bool ServiceDescriptor::GetSupplementalDataItem(const std::string& key, std::string& value) const
{
    auto valueIter = _supplementalData.find(key);
    if (valueIter == _supplementalData.end())
    {
        return false;
    }
    value = valueIter->second;
    return true;
}

void ServiceDescriptor::SetSupplementalDataItem(std::string key, std::string val)
{
    _supplementalData[key] = std::move(val);
}

auto ServiceDescriptor::GetParticipantId() const -> ParticipantId
{
    return _participantId;
}

void ServiceDescriptor::SetParticipantId(ParticipantId id)
{
    _participantId = id;
}

auto ServiceDescriptor::GetParticipantName() const -> const std::string&
{
    return _participantName;
}

void ServiceDescriptor::SetParticipantNameAndComputeId(std::string val)
{
    _participantId = SilKit::Util::Hash::Hash(val);
    _participantName = std::move(val);
}

auto ServiceDescriptor::GetServiceType() const -> SilKit::Core::ServiceType
{
    return _serviceType;
}

void ServiceDescriptor::SetServiceType(SilKit::Core::ServiceType val)
{
    _serviceType = std::move(val);
}

auto ServiceDescriptor::GetNetworkName() const -> const std::string&
{
    return _networkName;
}

void ServiceDescriptor::SetNetworkName(std::string val)
{
    _networkName = std::move(val);
}

auto ServiceDescriptor::GetNetworkType() const -> SilKit::Config::NetworkType
{
    return _networkType;
}

void ServiceDescriptor::SetNetworkType(SilKit::Config::NetworkType val)
{
    _networkType = std::move(val);
}

auto ServiceDescriptor::GetServiceName() const -> const std::string&
{
    return _serviceName;
}

void ServiceDescriptor::SetServiceName(std::string val)
{
    _serviceName = std::move(val);
}

auto ServiceDescriptor::GetServiceId() const -> SilKit::Core::EndpointId
{
    return _serviceId;
}

void ServiceDescriptor::SetServiceId(SilKit::Core::EndpointId val)
{
    _serviceId = std::move(val);
}

auto ServiceDescriptor::GetSupplementalData() const -> SupplementalData
{
    return _supplementalData;
}

void ServiceDescriptor::SetSupplementalData(SupplementalData val)
{
    _supplementalData = std::move(val);
}

auto ServiceDescriptor::GetSimulationName() const -> const std::string&
{
    static const std::string defaultSimulationName;
    auto it{_supplementalData.find(SilKit::Core::Discovery::simulationName)};
    if (it == _supplementalData.end())
    {
        return defaultSimulationName;
    }
    else
    {
        return it->second;
    }
}

void ServiceDescriptor::SetSimulationName(const std::string& simulationName)
{
    _supplementalData[SilKit::Core::Discovery::simulationName] = simulationName;
}

//Ctors
ServiceDescriptor::ServiceDescriptor(std::string participantName, std::string networkName, std::string serviceName,
                                     EndpointId serviceId)
{
    SetParticipantNameAndComputeId(std::move(participantName));
    SetNetworkName(std::move(networkName));
    SetServiceName(std::move(serviceName));
    SetServiceId(serviceId);
}

// operators
inline bool ServiceDescriptor::operator==(const ServiceDescriptor& rhs) const
{
    return GetParticipantId() == rhs.GetParticipantId() && GetNetworkName() == rhs.GetNetworkName()
           && GetServiceType() == rhs.GetServiceType() && GetServiceId() == rhs.GetServiceId();
}

inline bool ServiceDescriptor::operator!=(const ServiceDescriptor& rhs) const
{
    return !(*this == rhs);
}

std::string ServiceDescriptor::to_string() const
{
    const std::string separator{"/"};
    std::string controllerTypeName;
    // common
    std::stringstream ss;
    ss << GetParticipantName() << separator << SilKit::Core::to_string(GetServiceType());
    switch (GetServiceType())
    {
    case ServiceType::Link:
        ss << separator << Config::to_string(GetNetworkType()) << separator << GetNetworkName();
        break;
    case ServiceType::Controller:
    case ServiceType::SimulatedController:
        if (!GetSupplementalDataItem(SilKit::Core::Discovery::controllerType, controllerTypeName))
        {
            throw LogicError("ServiceDescriptor::to_string() failed: No controller type defined in supplemental data.");
        }

        ss << separator << controllerTypeName << separator << GetNetworkName() << separator << GetServiceName();
        break;
    case ServiceType::InternalController:
        ss << separator << GetServiceName();
        break;
    case ServiceType::Undefined:
        ss << separator << GetNetworkName() << separator << GetServiceName();
        break;
    }
    return ss.str();
}


 std::vector<std::pair<std::string, std::string>> ServiceDescriptor::to_keyValues() const
{
    namespace Keys = SilKit::Services::Logging::Keys;

    std::vector<std::pair<std::string, std::string>> kv;
    std::string controllerTypeName;
    std::stringstream ss;

    kv.push_back({Keys::participantName, GetParticipantName()});

    switch (GetServiceType())
    {
    case ServiceType::Link:
        kv.push_back({Keys::networkType, Config::to_string(GetNetworkType())});
        kv.push_back({Keys::networkName, GetNetworkName()});
        break;
    case ServiceType::Controller:
    case ServiceType::SimulatedController:
        if (!GetSupplementalDataItem(SilKit::Core::Discovery::controllerType, controllerTypeName))
        {
            throw LogicError(
                "ServiceDescriptor::to_keyValues() failed: No controller type defined in supplemental data.");
        }
        kv.push_back({Keys::controllerTypeName, controllerTypeName});
        kv.push_back({Keys::networkName, GetNetworkName()});
        kv.push_back({Keys::serviceName, GetServiceName()});
        break;
    case ServiceType::InternalController:
        kv.push_back({Keys::serviceName, GetServiceName()});
        break;
    case ServiceType::Undefined:
        kv.push_back({Keys::networkName, GetNetworkName()});
        kv.push_back({Keys::serviceName, GetServiceName()});
        break;
    }

    kv.push_back({Keys::serviceType, SilKit::Core::to_string(GetServiceType())});
    return kv;
}

EndpointAddress ServiceDescriptor::to_endpointAddress() const
{
    return {GetParticipantId(), GetServiceId()};
}

inline std::string to_string(const ServiceDescriptor& serviceDescriptor)
{
    return serviceDescriptor.to_string();
}

} // namespace Core
} // namespace SilKit
