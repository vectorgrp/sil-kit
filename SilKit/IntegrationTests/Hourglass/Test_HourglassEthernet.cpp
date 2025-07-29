// Copyright (c) 2022 Vector Informatik GmbH
// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"

#include "silkit/SilKit.hpp"
#include "silkit/detail/impl/ThrowOnError.hpp"

#include "MockCapiTest.hpp"

namespace {

using testing::DoAll;
using testing::SetArgPointee;
using testing::Return;

using namespace SilKit::Services::Ethernet;

MATCHER_P(EthernetFrameMatcher, controlFrame, "")
{
    *result_listener << "matches can frames of the c-api to the cpp api";

    const EthernetFrame frame1 = controlFrame;
    const SilKit_EthernetFrame* frame2 = arg;

    if (frame1.raw.size() != frame2->raw.size)
    {
        return false;
    }

    for (size_t i = 0; i < frame1.raw.size(); i++)
    {
        if (frame1.raw[i] != frame2->raw.data[i])
        {
            return false;
        }
    }
    return true;
}

class Test_HourglassEthernet : public SilKitHourglassTests::MockCapiTest
{
public:
    SilKit_EthernetController* mockEthernetController{
        reinterpret_cast<SilKit_EthernetController*>(uintptr_t(0x12345678))};

    Test_HourglassEthernet()
    {
        using testing::_;
        ON_CALL(capi, SilKit_EthernetController_Create(_, _, _, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockEthernetController), Return(SilKit_ReturnCode_SUCCESS)));
    }
};

TEST_F(Test_HourglassEthernet, SilKit_EthernetController_Create)
{
    SilKit_Participant* participant{(SilKit_Participant*)123456};
    std::string name = "EthernetController1";
    std::string network = "EthernetNetwork1";

    EXPECT_CALL(capi, SilKit_EthernetController_Create(testing::_, participant, testing::StrEq(name.c_str()),
                                                       testing::StrEq(network.c_str())))
        .Times(1);
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Ethernet::EthernetController ethernetController(
        participant, name, network);
}

TEST_F(Test_HourglassEthernet, SilKit_EthernetController_Activate)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Ethernet::EthernetController ethernetController(
        nullptr, "EthernetController1", "EthernetNetwork1");

    EXPECT_CALL(capi, SilKit_EthernetController_Activate(mockEthernetController)).Times(1);
    ethernetController.Activate();
}

TEST_F(Test_HourglassEthernet, SilKit_EthernetController_Deactivate)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Ethernet::EthernetController ethernetController(
        nullptr, "EthernetController1", "EthernetNetwork1");

    EXPECT_CALL(capi, SilKit_EthernetController_Deactivate(mockEthernetController)).Times(1);
    ethernetController.Deactivate();
}

TEST_F(Test_HourglassEthernet, SilKit_EthernetController_AddFrameHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Ethernet::EthernetController ethernetController(
        nullptr, "EthernetController1", "EthernetNetwork1");

    EXPECT_CALL(capi, SilKit_EthernetController_AddFrameHandler(mockEthernetController, testing::_, testing::_,
                                                                SilKit_Direction_SendReceive, testing::_))
        .Times(1);
    ethernetController.AddFrameHandler(
        nullptr, static_cast<SilKit::Services::DirectionMask>(SilKit::Services::TransmitDirection::TXRX));
}

TEST_F(Test_HourglassEthernet, SilKit_EthernetController_RemoveFrameHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Ethernet::EthernetController ethernetController(
        nullptr, "EthernetController1", "EthernetNetwork1");

    SilKit::Util::HandlerId handlerId{(SilKit::Util::HandlerId)1234};
    EXPECT_CALL(capi, SilKit_EthernetController_RemoveFrameHandler(mockEthernetController, 1234)).Times(1);
    ethernetController.RemoveFrameHandler(handlerId);
}

TEST_F(Test_HourglassEthernet, SilKit_EthernetController_AddFrameTransmitHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Ethernet::EthernetController ethernetController(
        nullptr, "EthernetController1", "EthernetNetwork1");

    EXPECT_CALL(
        capi, SilKit_EthernetController_AddFrameTransmitHandler(mockEthernetController, testing::_, testing::_,
                                                                SilKit_EthernetTransmitStatus_Transmitted, testing::_))
        .Times(1);
    ethernetController.AddFrameTransmitHandler(nullptr,
                                               static_cast<SilKit::Services::Ethernet::EthernetTransmitStatusMask>(
                                                   SilKit::Services::Ethernet::EthernetTransmitStatus::Transmitted));
}

TEST_F(Test_HourglassEthernet, SilKit_EthernetController_RemoveFrameTransmitHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Ethernet::EthernetController ethernetController(
        nullptr, "EthernetController1", "EthernetNetwork1");

    SilKit::Util::HandlerId handlerId{(SilKit::Util::HandlerId)1234};
    EXPECT_CALL(capi, SilKit_EthernetController_RemoveFrameTransmitHandler(mockEthernetController, 1234)).Times(1);
    ethernetController.RemoveFrameTransmitHandler(handlerId);
}

TEST_F(Test_HourglassEthernet, SilKit_EthernetController_AddStateChangeHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Ethernet::EthernetController ethernetController(
        nullptr, "EthernetController1", "EthernetNetwork1");

    EXPECT_CALL(capi, SilKit_EthernetController_AddStateChangeHandler(mockEthernetController, testing::_, testing::_,
                                                                      testing::_))
        .Times(1);
    ethernetController.AddStateChangeHandler(nullptr);
}

TEST_F(Test_HourglassEthernet, SilKit_EthernetController_RemoveStateChangeHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Ethernet::EthernetController ethernetController(
        nullptr, "EthernetController1", "EthernetNetwork1");

    SilKit::Util::HandlerId handlerId{(SilKit::Util::HandlerId)1234};
    EXPECT_CALL(capi, SilKit_EthernetController_RemoveStateChangeHandler(mockEthernetController, 1234)).Times(1);
    ethernetController.RemoveStateChangeHandler(handlerId);
}

TEST_F(Test_HourglassEthernet, SilKit_EthernetController_AddBitrateChangeHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Ethernet::EthernetController ethernetController(
        nullptr, "EthernetController1", "EthernetNetwork1");

    EXPECT_CALL(capi, SilKit_EthernetController_AddBitrateChangeHandler(mockEthernetController, testing::_, testing::_,
                                                                        testing::_))
        .Times(1);
    ethernetController.AddBitrateChangeHandler(nullptr);
}

TEST_F(Test_HourglassEthernet, SilKit_EthernetController_RemoveBitrateChangeHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Ethernet::EthernetController ethernetController(
        nullptr, "EthernetController1", "EthernetNetwork1");

    SilKit::Util::HandlerId handlerId{(SilKit::Util::HandlerId)1234};
    EXPECT_CALL(capi, SilKit_EthernetController_RemoveBitrateChangeHandler(mockEthernetController, 1234)).Times(1);
    ethernetController.RemoveBitrateChangeHandler(handlerId);
}

TEST_F(Test_HourglassEthernet, SilKit_EthernetController_SendFrame)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Ethernet::EthernetController ethernetController(
        nullptr, "EthernetController1", "EthernetNetwork1");

    std::vector<uint8_t> payload{5};
    SilKit::Services::Ethernet::EthernetFrame frame{payload};
    void* userContext = &frame;
    EXPECT_CALL(capi,
                SilKit_EthernetController_SendFrame(mockEthernetController, EthernetFrameMatcher(frame), userContext))
        .Times(1);
    ethernetController.SendFrame(frame, userContext);
}

} //namespace
