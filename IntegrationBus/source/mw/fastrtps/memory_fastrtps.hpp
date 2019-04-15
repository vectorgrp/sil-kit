// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "fastrtps/Domain.h"

namespace ib {
namespace mw {
namespace FastRtps {

template<class FastRtpsClass>
struct Deleter
{
    inline void operator()(FastRtpsClass* obj);
};

template <class FastRtpsClass>
using unique_ptr = std::unique_ptr<FastRtpsClass, Deleter<FastRtpsClass>>;

template<typename T, typename... Args>
inline auto make_unique(Args&&... args) -> unique_ptr<T>;

// ================================================================================
//  Inline Implementations
// ================================================================================
template<>
inline void Deleter<eprosima::fastrtps::Participant>::operator()(eprosima::fastrtps::Participant* participant)
{
    eprosima::fastrtps::Domain::removeParticipant(participant);
}

template<>
inline void Deleter<eprosima::fastrtps::Publisher>::operator()(eprosima::fastrtps::Publisher* publisher)
{
    eprosima::fastrtps::Domain::removePublisher(publisher);
}

template<>
inline void Deleter<eprosima::fastrtps::Subscriber>::operator()(eprosima::fastrtps::Subscriber* subscriber)
{
    eprosima::fastrtps::Domain::removeSubscriber(subscriber);
}

template<typename T, typename... Args>
auto make_unique(Args&&... args) -> unique_ptr<T>
{
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

    
} // namespace FastRtps
} // namespace mw
} // namespace ib
