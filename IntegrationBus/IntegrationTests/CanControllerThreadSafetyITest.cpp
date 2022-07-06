// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "GetTestPid.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"


#include "silkit/SilKit.hpp"
#include "silkit/core/sync/all.hpp"
#include "silkit/services/all.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "ConfigurationTestUtils.hpp"

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Core;
using namespace SilKit::Core::Orchestration;
using namespace SilKit::Config;
using namespace SilKit::Services::Can;

static size_t numParticipants;
std::chrono::microseconds asyncDelayCanWriter{1us};
std::chrono::microseconds asyncDelayCanReader{1us};
std::chrono::milliseconds communicationTimeout{20000ms};

const int numHandlersPerLoop = 100;
const int stopAfterReceptions = 100;

class CanControllerThreadSafetyITest : public testing::Test
{

protected:
    CanControllerThreadSafetyITest()
    {
    }

    struct TestParticipant
    {
        TestParticipant(const std::string& newName)
        {
            name = newName;
        }
        std::string                  name;
        std::unique_ptr<IParticipant> participant;
        ICanController* canController;
        uint64_t numReceptions = 0;
        bool allReceived{ false };
        std::promise<void> allReceivedPromise;

        void AwaitCommunication() 
        {
            auto futureStatus = allReceivedPromise.get_future().wait_for(communicationTimeout);
            EXPECT_EQ(futureStatus, std::future_status::ready)
                << "Test Failure: Awaiting test communication timed out";
        }
    };

    void AsyncCanWriterThread(TestParticipant& participant, const std::string& registryUri)
    {
        static uint32_t increasingCanId = 0;

        try
        {
            participant.participant =
                SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(), participant.name, registryUri);
            participant.canController = participant.participant->CreateCanController("Can");
            participant.canController->Start();

            while (runAsync)
            {
                CanFrame frame{};
                frame.canId = increasingCanId++;
                participant.canController->SendFrame(frame);
                std::this_thread::sleep_for(asyncDelayCanWriter);
            }

        }
        catch (const SilKit::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }

        // Explicitly delete the com adapter to end the async participant
        participant.participant.reset();
    }

    void AsyncCanReaderThread(TestParticipant& participant, const std::string& registryUri)
    {
        try
        {
            participant.participant =
                SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(), participant.name, registryUri);
            participant.canController = participant.participant->CreateCanController("Can");
            participant.canController->Start();

            auto frameHandler = [&participant](ICanController*, const CanFrameEvent& /*msg*/) {
                participant.numReceptions++;
                if (!participant.allReceived && participant.numReceptions >= stopAfterReceptions)
                {
                    participant.allReceived = true;
                    participant.allReceivedPromise.set_value();
                }
                std::this_thread::sleep_for(asyncDelayCanReader);
            };

            while (runAsync)
            {
                std::vector<SilKit::Services::HandlerId> handlerIds{};
                for (int i = 0; i < numHandlersPerLoop; i++)
                {
                    handlerIds.push_back(participant.canController->AddFrameHandler(frameHandler));
                }
                for (int i = 0; i < numHandlersPerLoop; i++)
                {
                    participant.canController->RemoveFrameHandler(handlerIds[i]);
                }
            }
        }
        catch (const SilKit::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }

        // Explicitly delete the com adapter to end the async participant
        participant.participant.reset();
    }

    void RunRegistry(const std::string& registryUri)
    {
        try
        {
            registry = SilKit::Vendor::CreateSilKitRegistry(SilKit::Config::MakeEmptyParticipantConfiguration());
            registry->ProvideDomain(registryUri);
        }
        catch (const SilKit::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
    }


    void RunAsyncCanWriter(TestParticipant& participant, const std::string& registryUri)
    {
        runAsync = true;

        try
        {
            asyncParticipantThreads.emplace_back([this, &participant, registryUri] {
                AsyncCanWriterThread(participant, registryUri);
            });
        }
        catch (const SilKit::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
    }

    void RunAsyncCanReader(TestParticipant& participant, const std::string& registryUri)
    {
        runAsync = true;

        try
        {
            asyncParticipantThreads.emplace_back([this, &participant, registryUri] {
                AsyncCanReaderThread(participant, registryUri);
            });
        }
        catch (const SilKit::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
    }

    void ShutdownAndFailTest(const std::string& reason) 
    {
        StopAsyncParticipants();
        FAIL() << reason; 
    }

    void StopAsyncParticipants()
    {
        runAsync = false;
        for (auto&& thread : asyncParticipantThreads)
        {
            thread.join();
        }
        asyncParticipantThreads.clear();
    }

    void ShutdownSystem()
    {
        asyncParticipantThreads.clear();
        registry.reset();
    }

protected:
    std::unique_ptr<SilKit::Vendor::ISilKitRegistry> registry;
    std::vector<std::thread> asyncParticipantThreads;
    bool runAsync{ true };
};


TEST_F(CanControllerThreadSafetyITest, add_remove_handler_during_reception)
{
    numParticipants = 0;
    auto registryUri = MakeTestRegistryUri();

    TestParticipant canWriterParticipant{ "CanWriterParticipant" };
    TestParticipant canReaderParticipant{ "CanReaderParticipant" };

    RunRegistry(registryUri);

    RunAsyncCanWriter(canWriterParticipant, registryUri);
    RunAsyncCanReader(canReaderParticipant, registryUri);

    // Await successful communication of canReader
    canReaderParticipant.AwaitCommunication();

    StopAsyncParticipants();
    ShutdownSystem();
}


} // anonymous namespace
