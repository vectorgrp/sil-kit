// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <sstream>
#include <thread>

#include "ib/IntegrationBus.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/all.hpp"
#include "ib/util/serdes/Deserializer.hpp"
#include "ib/util/serdes/Serializer.hpp"

using namespace ib::mw;
using namespace ib::sim::generic;
using namespace std::chrono_literals;

// std::chrono convenience
using namespace std::chrono_literals;
std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

// Simulation time
std::chrono::nanoseconds simTime = 0ns;

// (De-)Serialization header data
const ib::util::serdes::DataMemberUpdateHeader dataHeader;

// Simple struct used in communication
struct SimpleStruct
{
    int32_t fieldInt32;
    bool    fieldBool;
    float   fieldFloat;
};
// Output convenience
std::ostream& operator<<(std::ostream& out, const SimpleStruct& data)
{
    out << "[" << data.fieldInt32 << ", " << (data.fieldBool ? "true" : "false") << ", " << data.fieldFloat << "]";
    return out;
}

// Array size used in communication
const size_t array_size = 4;
// Output convenience
std::ostream& operator<<(std::ostream& out, const std::array<int8_t, array_size>& data)
{
    out << "[" << +data[0] << ", " << +data[1] << ", " << +data[2] << ", " << +data[3] << "]";
    return out;
}

void Publish_Int32(IGenericPublisher* publisher)
{
    static int32_t data_int32 = 0;

    // Alter the test data on every call
    data_int32++;

    std::cout << "T = " << simTime << " >> Publish int32: " << data_int32 << std::endl;

    // The serializer to buffer to data and create the data
    ib::util::serdes::Serializer serializer;
    // Resulting byte-vector needs some header data; encode the constant header
    serializer.Serialize(dataHeader);
    // Serialize the data
    serializer.Serialize(data_int32, 32);
    // Finally fill a byte-vector with the buffer
    auto genericMessageData = serializer.ReleaseBuffer();

    // Publish the data
    publisher->Publish(std::move(genericMessageData));
}

void Publish_SimpleStruct(IGenericPublisher* publisher)
{
    static SimpleStruct data_SimpleStruct{0, false, 0.0f};

    // Alter the test data on every call
    data_SimpleStruct.fieldInt32++;
    data_SimpleStruct.fieldBool = !data_SimpleStruct.fieldBool;
    data_SimpleStruct.fieldFloat += 0.1f;

    std::cout << "T = " << simTime << " >> Publish SimpleStruct: " << data_SimpleStruct << std::endl;

    // The serializer to buffer to data and create the data
    ib::util::serdes::Serializer serializer;
    // Resulting byte-vector needs some header data; encode the constant header
    serializer.Serialize(dataHeader);
    // Serialize the data
    serializer.BeginStruct();
    serializer.Serialize(data_SimpleStruct.fieldInt32, 32);
    serializer.Serialize(data_SimpleStruct.fieldBool);
    serializer.Serialize(data_SimpleStruct.fieldFloat);
    serializer.EndStruct();
    // Finally fill a byte-vector with the buffer
    auto genericMessageData = serializer.ReleaseBuffer();

    // Publish the data
    publisher->Publish(std::move(genericMessageData));
}

void Publish_Array(IGenericPublisher* publisher)
{
    static std::array<int8_t, array_size> data_array{0, 1, 2, 3};

    // Alter the test data on every call
    for (auto& d : data_array)
    {
        d++;
    }

    std::cout << "T = " << simTime << " >> Publish Array: " << data_array << std::endl;

    // The serializer to buffer to data and create the data
    ib::util::serdes::Serializer serializer;
    // Resulting byte-vector needs some header data; encode the constant header
    serializer.Serialize(dataHeader);
    // Serialize the data
    serializer.BeginArray(array_size);
    for (auto& d : data_array)
    {
        serializer.Serialize(d, 8);
    }
    serializer.EndStruct();
    // Finally fill a byte-vector with the buffer
    auto genericMessageData = serializer.ReleaseBuffer();

    // Publish the data
    publisher->Publish(std::move(genericMessageData));
}

void Receive_int32(IGenericSubscriber* subscriber, const std::vector<uint8_t>& genericMessageData)
{
    // Init deserializer with received data
    ib::util::serdes::Deserializer deserializer(genericMessageData);
    // Deserialize and verify header; throws on version error
    const auto outHeader = deserializer.Deserialize<ib::util::serdes::DataMemberUpdateHeader>();

    // Deserialize int32_t data
    int32_t data_int32 = deserializer.Deserialize<int32_t>(32);

    std::cout << "T = " << simTime << " << Receive int32: " << data_int32 << std::endl;
}

void Receive_SimpleStruct(IGenericSubscriber* subscriber, const std::vector<uint8_t>& genericMessageData)
{
    // Init deserializer with received data
    ib::util::serdes::Deserializer deserializer(genericMessageData);
    // Deserialize and verify header; throws on version error
    const auto outHeader = deserializer.Deserialize<ib::util::serdes::DataMemberUpdateHeader>();

    // Deserialize struct data
    SimpleStruct data_SimpleStruct;
    deserializer.BeginStruct();
    data_SimpleStruct.fieldInt32 = deserializer.Deserialize<int32_t>(32);
    data_SimpleStruct.fieldBool = deserializer.Deserialize<bool>();
    data_SimpleStruct.fieldFloat = deserializer.Deserialize<float>();
    deserializer.EndStruct();

    std::cout << "T = " << simTime << " << Receive SimpleStruct: " << data_SimpleStruct << std::endl;
}

void Receive_Array(IGenericSubscriber* subscriber, const std::vector<uint8_t>& genericMessageData)
{
    // Init deserializer with received data
    ib::util::serdes::Deserializer deserializer(genericMessageData);
    // Deserialize and verify header; throws on version error
    const auto outHeader = deserializer.Deserialize<ib::util::serdes::DataMemberUpdateHeader>();

    // Deserialize array data
    std::array<int8_t, array_size> data_array;
    deserializer.BeginArray();
    for (auto& d : data_array)
    {
        d = deserializer.Deserialize<int8_t>(8);
    }
    deserializer.EndArray();

    std::cout << "T = " << simTime << " << Receive Array: " << data_array << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0]
                  << " <IbConfig.json> <ParticipantName> [domainId]" << std::endl;
        return -1;
    }

    try
    {
        std::string configFilename(argv[1]);
        std::string participantName(argv[2]);

        uint32_t domainId = 42;
        if (argc >= 4)
        {
            domainId = static_cast<uint32_t>(std::stoul(argv[3]));
        }

        auto ibConfig = ib::cfg::Config::FromJsonFile(configFilename);

        std::cout << "Creating GenericAdapter for participant=" << participantName << " in domain " << domainId
                  << std::endl;
        auto comAdapter = ib::CreateComAdapter(ibConfig, participantName, domainId);

        auto&& participantController = comAdapter->GetParticipantController();

        participantController->SetInitHandler([](auto initCmd) {
            std::cout << "Initializing..." << std::endl;
            simTime = 0ns;
        });
        participantController->SetStopHandler([]() { std::cout << "Stopping..." << std::endl; });
        participantController->SetShutdownHandler([]() { std::cout << "Shutting down..." << std::endl; });

        if (participantName == "Publisher")
        {
            auto pub_Int32 = comAdapter->CreateGenericPublisher("Int32");
            auto pub_SimpleStruct = comAdapter->CreateGenericPublisher("SimpleStruct");
            auto pub_Array = comAdapter->CreateGenericPublisher("Array");

            participantController->SetPeriod(1000ms);
            participantController->SetSimulationTask(
                [pub_Int32, pub_SimpleStruct, pub_Array](std::chrono::nanoseconds now,
                                                         std::chrono::nanoseconds duration) {
                    simTime = now;

                    Publish_Int32(pub_Int32);
                    Publish_SimpleStruct(pub_SimpleStruct);
                    Publish_Array(pub_Array);
                });
        }
        else
        {
            auto sub_Int32 = comAdapter->CreateGenericSubscriber("Int32");
            sub_Int32->SetReceiveMessageHandler(Receive_int32);

            auto sub_SimpleStruct = comAdapter->CreateGenericSubscriber("SimpleStruct");
            sub_SimpleStruct->SetReceiveMessageHandler(Receive_SimpleStruct);

            auto sub_Array = comAdapter->CreateGenericSubscriber("Array");
            sub_Array->SetReceiveMessageHandler(Receive_Array);

            participantController->SetPeriod(1000ms);
            participantController->SetSimulationTask(
                [](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
                    std::this_thread::sleep_for(1s);
                    simTime = now; 
                });
        }

        participantController->Run();

        return 0;
    }
    catch (const ib::cfg::Misconfiguration& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -2;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -3;
    }
}
