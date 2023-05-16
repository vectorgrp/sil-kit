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

#include <string>
#include <ostream>
#include <sstream>
#include <vector>
#include <functional>

#include "EndpointAddress.hpp"
#include "ServiceDescriptor.hpp"
#include "ServiceConfigKeys.hpp"

namespace SilKit {
namespace Core {
inline std::ostream& operator<<(std::ostream& out, const ServiceDescriptor& descriptor);

inline EndpointAddress to_endpointAddress(const ServiceDescriptor& descriptor);

inline bool AllowMessageProcessing(const ServiceDescriptor& lhs, const ServiceDescriptor& rhs);


class IServiceEndpoint
{
public:
    virtual ~IServiceEndpoint() = default;
    virtual void SetServiceDescriptor(const ServiceDescriptor& serviceDescriptor) = 0;
    virtual auto GetServiceDescriptor() const -> const ServiceDescriptor& = 0;
};

inline bool AllowMessageProcessing(const ServiceDescriptor& lhs, const ServiceDescriptor& rhs)
{
    return lhs.GetServiceId() == rhs.GetServiceId() && lhs.GetParticipantName() == rhs.GetParticipantName();
}

inline EndpointAddress to_endpointAddress(const ServiceDescriptor& descriptor)
{
    return descriptor.to_endpointAddress();
}

inline std::ostream& operator<<(std::ostream& out, const ServiceDescriptor& descriptor)
{
    return out << descriptor.to_string();
}

} // namespace Core
} // namespace SilKit
