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

#include "SerializedMessage.hpp"

#include <cstdint>
#include <array>
#include <string>

#include "gtest/gtest.h"

#pragma pack(push, 1)
struct PackedHandshake
{
    //Implicit message size
    uint32_t messageSize;
    // implicit message kind
    uint8_t messageKind;
    uint8_t registryMessageKind;
    // VasioMsgHeader
    std::array<uint8_t, 4> preambel;
    uint16_t versionHigh;
    uint16_t versionLow;
    //VAsioPeerInfo
    uint32_t participantNameSize;
    std::array<uint8_t, sizeof("SerdesTest") - 1> participantName; //without trailing '\0' byte
    uint64_t participantId;
    //VAsioPeerInfo::acceptorUris
    uint32_t acceptorUrisSize;
    //[0]
    uint32_t acceptorUri0Size;
    std::array<uint8_t, sizeof("https://example.com:1234") - 1> acceptorUri0;
    uint32_t capabilitiesSize;
    //capabilities empty
    uint32_t simulationNameSize;
    std::array<uint8_t, sizeof("test/sim") - 1> simulationName;
};
#pragma pack(pop)

using namespace SilKit::Core;

TEST(Test_SerializedMessage, packed_handshake_message)
{
    //test if network layout of connection handshake changed
    ParticipantAnnouncement announcement;
    announcement.peerInfo.participantId = 1234;
    announcement.peerInfo.participantName = "SerdesTest";
    announcement.peerInfo.acceptorUris = {"https://example.com:1234"};
    announcement.simulationName = "test/sim";

    // check that the default-constructed announcement contains the correct preamble
    ASSERT_EQ(announcement.messageHeader.preamble, REGISTRY_MESSAGE_HEADER_PREAMBLE_VALUE);

    SerializedMessage msg{announcement};
    auto blob = msg.ReleaseStorage();
    const auto* ptr = reinterpret_cast<const PackedHandshake*>(blob.data());

    ASSERT_EQ(blob.size(), sizeof(PackedHandshake));

    ASSERT_EQ(blob.size(), ptr->messageSize);
    ASSERT_EQ(ptr->messageKind, (uint8_t)VAsioMsgKind::SilKitRegistryMessage);
    ASSERT_EQ(ptr->registryMessageKind, (uint8_t)RegistryMessageKind::ParticipantAnnouncement);

    // check that the serialized preamble contains the exact bytes
    ASSERT_EQ(ptr->preambel, REGISTRY_MESSAGE_HEADER_PREAMBLE_BYTES);
    ASSERT_EQ(ptr->versionHigh, announcement.messageHeader.versionHigh);
    ASSERT_EQ(ptr->versionLow, announcement.messageHeader.versionLow);

    ASSERT_EQ(ptr->participantNameSize, announcement.peerInfo.participantName.size());

    auto to_string = [](const auto& data, auto size) {
        return std::string{(const char*)data.data(), size};
    };

    ASSERT_EQ(to_string(ptr->participantName, ptr->participantNameSize), announcement.peerInfo.participantName);

    ASSERT_EQ(ptr->participantId, announcement.peerInfo.participantId);

    ASSERT_EQ(ptr->acceptorUrisSize, announcement.peerInfo.acceptorUris.size());

    ASSERT_EQ(to_string(ptr->acceptorUri0, ptr->acceptorUri0Size), announcement.peerInfo.acceptorUris.at(0));

    ASSERT_EQ(ptr->simulationNameSize, announcement.simulationName.size());
    ASSERT_EQ(to_string(ptr->simulationName, ptr->simulationNameSize), announcement.simulationName);
}
