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

using SilKitHourglassTests::MockCapi;

using namespace SilKit::Services::Logging;

class Test_HourglassParticipantLogger : public SilKitHourglassTests::MockCapiTest
{
public:
    SilKit_Participant* mockParticipant{(SilKit_Participant*)784324};
    SilKit_Logger* mockLogger{(SilKit_Logger*)876453};
    SilKit_ParticipantConfiguration* mockConfiguration{(SilKit_ParticipantConfiguration*)123456};

    Test_HourglassParticipantLogger()
    {
        using testing::_;
        ON_CALL(capi, SilKit_Participant_Create(_, _, _, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockParticipant), Return(SilKit_ReturnCode_SUCCESS)));
        ON_CALL(capi, SilKit_Participant_GetLogger(_, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockLogger), Return(SilKit_ReturnCode_SUCCESS)));
        ON_CALL(capi, SilKit_ParticipantConfiguration_FromString(_, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockConfiguration), Return(SilKit_ReturnCode_SUCCESS)));
        ON_CALL(capi, SilKit_ParticipantConfiguration_FromFile(_, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockConfiguration), Return(SilKit_ReturnCode_SUCCESS)));
    }
};

TEST_F(Test_HourglassParticipantLogger, SilKit_ParticipantConfiguration_FromString)
{
    std::string configString = "";

    EXPECT_CALL(capi, SilKit_ParticipantConfiguration_FromString(testing::_, testing::StrEq(configString.c_str())))
        .Times(1);
    SilKit::Config::ParticipantConfigurationFromString(configString);
}

TEST_F(Test_HourglassParticipantLogger, SilKit_ParticipantConfiguration_FromFile)
{
    std::string configFilename = "";

    EXPECT_CALL(capi, SilKit_ParticipantConfiguration_FromFile(testing::_, testing::StrEq(configFilename.c_str())))
        .Times(1);
    SilKit::Config::ParticipantConfigurationFromFile(configFilename);
}

TEST_F(Test_HourglassParticipantLogger, SilKit_ParticipantConfiguration_Destroy)
{
    std::string configString = "";

    EXPECT_CALL(capi, SilKit_ParticipantConfiguration_Destroy(testing::_)).Times(1);
    {
        auto config = SilKit::Config::ParticipantConfigurationFromString(configString);
    }
}

TEST_F(Test_HourglassParticipantLogger, SilKit_ParticipantConfiguration_ToJson)
{
    auto&& configString = R"(schemaVersion: 1
Description: test

Middleware:
  RegistryUri: silkit://localhost:8501
  ConnectAttempts: 1
  TcpNoDelay: true
  AcceptorUris: [tcp://0.0.0.0:8502]
  RegistryAsFallbackProxy: true

Logging:
  Sinks:
    - Type: Stdout
      Level: Debug
)";

    auto&& config = SilKit::Config::ParticipantConfigurationFromString(configString);
    EXPECT_CALL(capi, SilKit_ParticipantConfiguration_ToJson(testing::_, testing::_, testing::_)).Times(1);
    {
        auto jsonString = SilKit::Config::ParticipantConfigurationToJson(config);
    }
}

TEST_F(Test_HourglassParticipantLogger, SilKit_Participant_Create1)
{
    std::string name = "Participant1";
    std::string registryUri = "silkit://localhost:1234";
    std::string configString = "";
    auto config = SilKit::Config::ParticipantConfigurationFromString(configString);

    EXPECT_CALL(capi, SilKit_Participant_Create(testing::_, testing::_, testing::StrEq(name.c_str()),
                                                testing::StrEq(registryUri.c_str())))
        .Times(1);
    SilKit::CreateParticipant(config, name, registryUri);
}

TEST_F(Test_HourglassParticipantLogger, SilKit_Participant_Create2)
{
    std::string name = "Participant1";
    std::string registryUri = "silkit://localhost:1234";
    std::string configString = "";
    auto config = SilKit::Config::ParticipantConfigurationFromString(configString);

    EXPECT_CALL(capi, SilKit_Participant_Create(testing::_, testing::_, testing::StrEq(name.c_str()), testing::_))
        .Times(1);
    SilKit::CreateParticipant(config, name);
}

TEST_F(Test_HourglassParticipantLogger, SilKit_Participant_Destroy)
{
    std::string name = "Participant1";
    std::string registryUri = "silkit://localhost:1234";
    std::string configString = "";
    auto config = SilKit::Config::ParticipantConfigurationFromString(configString);

    EXPECT_CALL(capi, SilKit_Participant_Destroy(testing::_)).Times(1);
    {
        SilKit::CreateParticipant(config, name);
    }
}

TEST_F(Test_HourglassParticipantLogger, SilKit_Participant_GetLogger)
{
    std::string name = "Participant1";
    std::string registryUri = "silkit://localhost:1234";
    std::string configString = "";
    auto config = SilKit::Config::ParticipantConfigurationFromString(configString);

    EXPECT_CALL(capi, SilKit_Participant_GetLogger(testing::_, testing::_)).Times(1);

    // NB: For optimization purposes the call to ..._GetLogger is made during participant construction and not during
    //     the call to participant->GetLogger().
    auto participant = SilKit::CreateParticipant(config, name);
    participant->GetLogger();
}

TEST_F(Test_HourglassParticipantLogger, SilKit_Logger_Log)
{
    std::string name = "Participant1";
    std::string configString = "";
    std::string logMessage = "This is a test message.";
    auto config = SilKit::Config::ParticipantConfigurationFromString(configString);

    EXPECT_CALL(capi, SilKit_Logger_Log(testing::_, SilKit_LoggingLevel_Critical, testing::StrEq(logMessage.c_str())))
        .Times(1);

    auto participant = SilKit::CreateParticipant(config, name);
    auto logger = participant->GetLogger();
    logger->Log(Level::Critical, logMessage);
}

TEST_F(Test_HourglassParticipantLogger, SilKit_Logger_Trace)
{
    std::string name = "Participant1";
    std::string configString = "";
    std::string logMessage = "This is a test message.";
    auto config = SilKit::Config::ParticipantConfigurationFromString(configString);

    EXPECT_CALL(capi, SilKit_Logger_Log(testing::_, SilKit_LoggingLevel_Trace, testing::StrEq(logMessage.c_str())))
        .Times(1);

    auto participant = SilKit::CreateParticipant(config, name);
    auto logger = participant->GetLogger();
    logger->Trace(logMessage);
}

TEST_F(Test_HourglassParticipantLogger, SilKit_Logger_Debug)
{
    std::string name = "Participant1";
    std::string configString = "";
    std::string logMessage = "This is a test message.";
    auto config = SilKit::Config::ParticipantConfigurationFromString(configString);

    EXPECT_CALL(capi, SilKit_Logger_Log(testing::_, SilKit_LoggingLevel_Debug, testing::StrEq(logMessage.c_str())))
        .Times(1);

    auto participant = SilKit::CreateParticipant(config, name);
    auto logger = participant->GetLogger();
    logger->Debug(logMessage);
}

TEST_F(Test_HourglassParticipantLogger, SilKit_Logger_Info)
{
    std::string name = "Participant1";
    std::string configString = "";
    std::string logMessage = "This is a test message.";
    auto config = SilKit::Config::ParticipantConfigurationFromString(configString);

    EXPECT_CALL(capi, SilKit_Logger_Log(testing::_, SilKit_LoggingLevel_Info, testing::StrEq(logMessage.c_str())))
        .Times(1);

    auto participant = SilKit::CreateParticipant(config, name);
    auto logger = participant->GetLogger();
    logger->Info(logMessage);
}

TEST_F(Test_HourglassParticipantLogger, SilKit_Logger_Warn)
{
    std::string name = "Participant1";
    std::string configString = "";
    std::string logMessage = "This is a test message.";
    auto config = SilKit::Config::ParticipantConfigurationFromString(configString);

    EXPECT_CALL(capi, SilKit_Logger_Log(testing::_, SilKit_LoggingLevel_Warn, testing::StrEq(logMessage.c_str())))
        .Times(1);

    auto participant = SilKit::CreateParticipant(config, name);
    auto logger = participant->GetLogger();
    logger->Warn(logMessage);
}

TEST_F(Test_HourglassParticipantLogger, SilKit_Logger_Error)
{
    std::string name = "Participant1";
    std::string configString = "";
    std::string logMessage = "This is a test message.";
    auto config = SilKit::Config::ParticipantConfigurationFromString(configString);

    EXPECT_CALL(capi, SilKit_Logger_Log(testing::_, SilKit_LoggingLevel_Error, testing::StrEq(logMessage.c_str())))
        .Times(1);

    auto participant = SilKit::CreateParticipant(config, name);
    auto logger = participant->GetLogger();
    logger->Error(logMessage);
}

TEST_F(Test_HourglassParticipantLogger, SilKit_Logger_Critical)
{
    std::string name = "Participant1";
    std::string configString = "";
    std::string logMessage = "This is a test message.";
    auto config = SilKit::Config::ParticipantConfigurationFromString(configString);

    EXPECT_CALL(capi, SilKit_Logger_Log(testing::_, SilKit_LoggingLevel_Critical, testing::StrEq(logMessage.c_str())))
        .Times(1);

    auto participant = SilKit::CreateParticipant(config, name);
    auto logger = participant->GetLogger();
    logger->Critical(logMessage);
}

TEST_F(Test_HourglassParticipantLogger, SilKit_Logger_GetLogLevel)
{
    std::string name = "Participant1";
    std::string configString = "";
    auto config = SilKit::Config::ParticipantConfigurationFromString(configString);

    EXPECT_CALL(capi, SilKit_Logger_GetLogLevel(testing::_, testing::_)).Times(1);

    auto participant = SilKit::CreateParticipant(config, name);
    auto logger = participant->GetLogger();
    (void)logger->GetLogLevel();
}

} //namespace
