#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ParticipantConfiguration.hpp"
#include "YamlParser.hpp"

namespace {

constexpr const char* PARTICIPANT_CONFIGURATION_A = R"(
ParticipantName: My Participant Name
Description: Some Cool Description
)";

constexpr const char* PARTICIPANT_CONFIGURATION_B = R"()";

} // namespace

TEST(Woooooooooooooot, Weeeeeeeeeeeeee)
{
    {
        auto c = VSilKit::Config::DeserializeParticipantConfiguration(PARTICIPANT_CONFIGURATION_A);
        ASSERT_EQ(c.participantName, "My Participant Name");
        ASSERT_EQ(c.description, "Some Cool Description");
    }

    {
        auto c = VSilKit::Config::DeserializeParticipantConfiguration(PARTICIPANT_CONFIGURATION_B);
        ASSERT_EQ(c.participantName, "");
        ASSERT_EQ(c.description, "");
    }
}
