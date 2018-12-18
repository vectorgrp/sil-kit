// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "LinDatatypes.hpp"

#include "ib/exception.hpp"

namespace ib {
namespace sim {
namespace lin { 

inline std::string to_string(MessageStatus status);

inline std::ostream& operator<<(std::ostream& out, MessageStatus status);

// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(MessageStatus status)
{
    switch (status)
    {
    case lin::MessageStatus::TxSuccess:
        return "TxSuccess";
    case lin::MessageStatus::RxSuccess:
        return "RxSuccess";
    case lin::MessageStatus::TxResponseError:
        return "TxResponseError";
    case lin::MessageStatus::RxResponseError:
        return "RxResponseError";
    case lin::MessageStatus::RxNoResponse:
        return "RxNoResponse";
    case lin::MessageStatus::HeaderError:
        return "HeaderError";
    case lin::MessageStatus::Canceled:
        return "Canceled";
    case lin::MessageStatus::Busy:
        return "Busy";
    default:
        throw ib::type_conversion_error{};
    }
}

std::ostream& operator<<(std::ostream& out, MessageStatus status)
{
    try
    {
        return out << to_string(status);
    }
    catch (const ib::type_conversion_error&)
    {
        return out << "MessageStatus{" << static_cast<uint32_t>(status) << "}";
    }
}

    
} // namespace lin
} // namespace sim
} // namespace ib
