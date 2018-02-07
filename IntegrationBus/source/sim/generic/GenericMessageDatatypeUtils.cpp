// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "GenericMessageDatatypeUtils.hpp"

namespace ib {
namespace sim {
namespace generic {

bool operator==(const GenericMessage& lhs, const GenericMessage& rhs)
{
    return lhs.data == rhs.data;
}

} // namespace generic
} // namespace sim
} // namespace ib
