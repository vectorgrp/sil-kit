// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SyncDatatypeUtils.hpp"


namespace ib {
namespace mw {
namespace sync {


bool operator==(const ParticipantCommand& lhs, const ParticipantCommand& rhs)
{
    return lhs.kind == rhs.kind;
}

bool operator==(const ParticipantStatus& lhs, const ParticipantStatus& rhs)
{
    return lhs.participantName == rhs.participantName
        && lhs.state == rhs.state
        && lhs.enterReason == rhs.enterReason
        && lhs.enterTime == rhs.enterTime
        && lhs.refreshTime == rhs.refreshTime;
};

bool operator==(const SystemCommand& lhs, const SystemCommand& rhs)
{
    return lhs.kind == rhs.kind;
}


}
}
}
