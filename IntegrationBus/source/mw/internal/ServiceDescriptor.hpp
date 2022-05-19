#pragma once
#include <string>
#include <cassert>
#include <sstream>
#include <map>

#include "ServiceConfigKeys.hpp"
#include "Configuration.hpp"

namespace ib {
namespace mw {

inline uint64_t hash(const std::string& s)
{
    //DJB2 from dj bernstein
    // documented in http://www.cse.yorku.ca/~oz/hash.html
    uint64_t hash = 5381;
    for (auto c : s)
    {
        hash = (hash << 5) + hash + c; // hash * 33 + c 
    }
    return hash;
}

typedef std::map<std::string, std::string> SupplementalData;

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
    default: return "Unknown";
    case ServiceType::Undefined: return "Undefined";
    case ServiceType::Link: return "Link";
    case ServiceType::Controller: return "Controller";
    case ServiceType::SimulatedController: return "SimulatedController";
    case ServiceType::InternalController: return "InternalController";
    }
}

class ServiceDescriptor
{
public:
    // helpers
    inline bool operator==(const ServiceDescriptor& rhs) const;
    inline bool operator!=(const ServiceDescriptor& rhs) const;
    inline std::string to_string() const;
    inline mw::EndpointAddress to_endpointAddress() const;

    //inline void Serialize(ib::mw::MessageBuffer& buffer) const;
    //inline void Deserialize(ib::mw::MessageBuffer& buffer);

public:
    inline ParticipantId GetParticipantId() const { return _participantId; 
    }
    std::string GetParticipantName() const { return _participantName; }
    void SetParticipantName(std::string val) 
    {
        _participantId = hash(val);
        _participantName = std::move(val);
    }

    ib::mw::ServiceType GetServiceType() const { return _serviceType; }
    void SetServiceType(ib::mw::ServiceType val) {_serviceType = std::move(val); }

    std::string GetNetworkName() const { return _networkName; }
    void SetNetworkName(std::string val) {_networkName = std::move(val); }

    ib::cfg::NetworkType GetNetworkType() const { return _networkType; }
    void SetNetworkType(ib::cfg::NetworkType val) { _networkType = std::move(val); }

    std::string GetServiceName() const { return _serviceName; }
    void SetServiceName(std::string val) {_serviceName = std::move(val); }

    ib::mw::EndpointId GetServiceId() const { return _serviceId; }
    void SetServiceId(ib::mw::EndpointId val) {_serviceId = std::move(val); }

    SupplementalData GetSupplementalData() const { return _supplementalData; }
    void SetSupplementalData(SupplementalData val) { _supplementalData = std::move(val); }

    bool GetSupplementalDataItem(const std::string& key, std::string& value) const 
    {
        auto valueIter = _supplementalData.find(key);
        if (valueIter == _supplementalData.end())
        {
            return false;
        }
        value = valueIter->second;
        return true;
    }
    void SetSupplementalDataItem(std::string key, std::string val) { _supplementalData[key] = std::move(val); }

private: 

public:
    std::string _participantName; //!< name of the participant
    ParticipantId _participantId{0};
    ServiceType _serviceType{ServiceType::Undefined};
    std::string _networkName; //!< the service's link name
    ib::cfg::NetworkType _networkType{ib::cfg::NetworkType::Invalid};
    std::string _serviceName;
    EndpointId _serviceId;
    SupplementalData _supplementalData;
};

// operators
inline bool ServiceDescriptor::operator==(const ServiceDescriptor& rhs) const
{
    return 
        GetParticipantId() == rhs.GetParticipantId()
        && GetNetworkName() == rhs.GetNetworkName() 
        && GetServiceType() == rhs.GetServiceType() 
        && GetServiceId() == rhs.GetServiceId()
        ;
}

inline bool ServiceDescriptor::operator!=(const ServiceDescriptor& rhs) const
{
    return !(*this == rhs);
}

std::string ServiceDescriptor::to_string() const
{
    const std::string separator{ "/" };
    // common
    std::stringstream ss;
    ss 
        << GetParticipantName()
        << separator
        << ib::mw::to_string(GetServiceType())
        ;
    switch (GetServiceType())
    {
    case ServiceType::Link:
    {
        ss 
            << separator
            << cfg::to_string(GetNetworkType())
            << separator
            << GetNetworkName()
            ;
        break;
    }
    case ServiceType::Controller:
    case ServiceType::SimulatedController:
    {
        std::string controllerTypeName;
        if (!GetSupplementalDataItem(ib::mw::service::controllerType, controllerTypeName))
        {
            throw std::logic_error("supplementalData.size() > 0");
        }

        ss 
            << separator 
            << controllerTypeName
            << separator
            << GetNetworkName()
            << separator
            << GetServiceName()
            ;
        break;
    }
    case ServiceType::InternalController:
    {
        ss 
            << separator
            << GetServiceName()
            ;
        break;
    }
    case ServiceType::Undefined: 
    {
        ss
            << separator 
            << GetNetworkName() 
            << separator
            << GetServiceName()
            ;
        
      break;
    }
    }
    return ss.str();
}

EndpointAddress ServiceDescriptor::to_endpointAddress() const
{
    return {GetParticipantId(), GetServiceId()};
}

inline std::string to_string(const ServiceDescriptor& serviceDescriptor)
{
    return serviceDescriptor.to_string();
}
} // namespace mw
} // namespace ib
