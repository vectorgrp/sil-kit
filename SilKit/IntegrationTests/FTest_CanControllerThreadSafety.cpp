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

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "GetTestPid.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/all.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/config/IParticipantConfiguration.hpp"

namespace {

using namespace std::chrono_literals;
using namespace SilKit;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Config;
using namespace SilKit::Services::Can;

static size_t numParticipants;
std::chrono::milliseconds communicationTimeout{20000ms};

const int numHandlersPerLoop = 100;
const int stopAfterReceptions = 100;

class FTest_CanControllerThreadSafety : public testing::Test
{

protected:
    FTest_CanControllerThreadSafety()
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
            if (futureStatus != std::future_status::ready)
            {
                FAIL() << "Test Failure: Awaiting test communication timed out";
            }
        }
    };

    void AsyncCanWriterThread(TestParticipant& participant, const std::string& registryUri)
    {
        static uint32_t increasingCanId = 0;

        try
        {
            participant.participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""),
                                                                participant.name, registryUri);
            participant.canController = participant.participant->CreateCanController("CAN", "CAN");
            participant.canController->Start();

            while (runAsync)
            {
                CanFrame frame{};
                frame.canId = increasingCanId++;
                participant.canController->SendFrame(frame);
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
            participant.participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""),
                                                                participant.name, registryUri);
            participant.canController = participant.participant->CreateCanController("CAN", "CAN");
            participant.canController->Start();

            auto frameHandler = [&participant](ICanController*, const CanFrameEvent& /*msg*/) {
                participant.numReceptions++;
                if (!participant.allReceived && participant.numReceptions >= stopAfterReceptions)
                {
                    participant.allReceived = true;
                    participant.allReceivedPromise.set_value();
                }
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
            registry =
                SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
            registry->StartListening(registryUri);
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
    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> registry;
    std::vector<std::thread> asyncParticipantThreads;
    bool runAsync{ true };
};


TEST_F(FTest_CanControllerThreadSafety, DISABLED_add_remove_handler_during_reception)
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
