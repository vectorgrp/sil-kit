// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include <iostream>

#include "CreateComAdapter.hpp"
#include "VAsioRegistry.hpp"

#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"

#include "gtest/gtest.h"

#include "GetTestPid.hpp"

#include "IntegrationTestUtils.hpp"

#include "Pcap.hpp"


namespace {

using namespace std::chrono_literals;
using namespace ib::mw;
using namespace ib::cfg;
using namespace ib::sim::eth;
using namespace ib::tracing;

using IntegrationTestUtils::Barrier;

class TracingITest: public testing::Test
{
protected:

    TracingITest()
    {
        domainId = static_cast<uint32_t>(GetTestPid());
    }

    virtual void Sender()
    {
        auto comAdapter = CreateComAdapterImpl(ibConfig, "EthWriter");
        comAdapter->joinIbDomain(domainId);

        std::atomic_uint64_t receiveCount{0};

        auto* controller = comAdapter->CreateEthController("ETH1");
        controller->RegisterMessageAckHandler(
            [this, &receiveCount](IEthController*, const EthTransmitAcknowledge& ) {
            receiveCount++;
        });

        for (auto i = 0u; i < message.size(); i++)
        {
            message[i] = 'A' + (i % 26);
        }

        EthMessage msg{};
        msg.ethFrame.SetDestinationMac(EthMac{ 0x12,0x23,0x45,0x67,0x89,0x9a });
        msg.ethFrame.SetSourceMac(EthMac{ 0x9a, 0x89, 0x67, 0x45, 0x23, 0x12});
        msg.ethFrame.SetEtherType(0x8100);
        msg.ethFrame.SetPayload(message.data(), message.size());
        
        frameSize = msg.ethFrame.GetFrameSize();

        barrier->Enter();

        for (auto i = 0u; i < numMessages; i++)
        {

            //let controller use its Time Provider to add timestamps
            controller->SendFrame(msg.ethFrame);
            std::this_thread::sleep_for(50ms);
        }

        ASSERT_EQ(isDone.wait_for(waitTime), std::future_status::ready)
            << "Sender receiveACKCount:" << receiveCount  
            << " numMessages: " << numMessages;
    }

    virtual void Receiver(const std::string& participantName)
    {
        auto comAdapter = CreateComAdapterImpl(ibConfig, participantName);
        comAdapter->joinIbDomain(domainId);

        auto* controller = comAdapter->CreateEthController("ETH1");
        controller->RegisterReceiveMessageHandler(
            [this, &participantName](IEthController* , const EthMessage& msg) {
                receiveCount++;
                if ((participantName == "EthReader1") && (receiveCount == numMessages))
                    donePromise.set_value();
                
        });
        //wait until sender is ready:
        barrier->Enter();

        //block until all messages transmitted:
        ASSERT_EQ(isDone.wait_for(waitTime), std::future_status::ready)
            << "Receiver " << participantName 
            <<" receiveCount: " << receiveCount 
            << " numMessages:" << numMessages;
    }

    void ExecuteFileTest(const std::string& fileName)
    {
        isDone = donePromise.get_future();
        barrier = std::make_unique<Barrier>(2, waitTime);

        std::thread receiver1Thread{&TracingITest::Receiver, this,"EthReader1"};
        std::thread senderThread{&TracingITest::Sender, this};

        senderThread.join();
        receiver1Thread.join();

        ValidateFilesize(fileName);
        ValidatePcapMagic(fileName);
        IntegrationTestUtils::removeTempFile(fileName);
    }

    void ExecutePipeTest(const std::string& fileName)
    {
        isDone = donePromise.get_future();
        barrier = std::make_unique<Barrier>(3, waitTime);

        std::thread receiver1Thread{&TracingITest::Receiver, this,"EthReader1"};
        std::thread senderThread{&TracingITest::Sender, this};
        std::thread pipeReader{&TracingITest::PipeReader, this, fileName};

        senderThread.join();
        receiver1Thread.join();
        pipeReader.join();

    }

    void ValidatePcapMagic(const std::string& fileName)
    {
        std::ifstream ifs(fileName, std::ios::binary);
        uint32_t actual{};
        ifs.read(reinterpret_cast<char*>(&actual), sizeof(actual));
        ASSERT_EQ(actual, Pcap::NativeMagic)
            << "PcapFile " 
            << fileName 
            << " does not contain PCAP magic in native byte order!";
    }

    void ValidateFilesize(const std::string& fileName)
    {
        //Validate the pcap file
        auto actualPcapFileSize = IntegrationTestUtils::getFileSize(fileName);
        ASSERT_EQ(
            actualPcapFileSize, 
            Pcap::GlobalHeaderSize + (numMessages * (Pcap::PacketHeaderSize + frameSize))
        ) << "Pcap file " << fileName  << ": size does not match expected size!";
    }

    void PipeReader(const std::string& pipeName)
    {
        size_t numBytes = 0;
        //wait until Sender and Receiver are set up
        barrier->Enter();

        IntegrationTestUtils::Pipe pipe{pipeName};
        auto header = pipe.Read(Pcap::GlobalHeaderSize);
        ASSERT_EQ(header.size(), Pcap::GlobalHeaderSize)
            << "Short pipe read when reading global PCAP header";

        auto magic = *reinterpret_cast<uint32_t*>(header.data());
        ASSERT_EQ(magic, Pcap::NativeMagic)
            << "Data read from pipe does not begin with the native PCAP magic number!";

        auto major = *reinterpret_cast<uint16_t*>(&header.at(2 * sizeof(uint16_t)));
        ASSERT_EQ(major, Pcap::MajorVersion)
            << "PCAP header has wrong major version";

        auto minor = *reinterpret_cast<uint16_t*>(&header.at(3 * sizeof(uint16_t)));
        ASSERT_EQ(minor, Pcap::MinorVersion)
            << "PCAP header has wrong minor version";

        numBytes += header.size();
        auto done = false;
        do {
            auto buf = pipe.Read(100);
            if (buf.empty())
            {
                break;
            }
            numBytes += buf.size();
        } while (true);

        ASSERT_EQ(
            numBytes, 
            Pcap::GlobalHeaderSize + (numMessages * (Pcap::PacketHeaderSize + frameSize))
        ) << "Pcap PIPE " << pipeName  << ": size does not match expected size!";

    }

    std::string MakeFilename(const std::string& prefix)
    {
        std::stringstream ss;
        ss << prefix << "_"
            << std::to_string(domainId)
            << "_" << IntegrationTestUtils::randomString(8)
            << ".pcap";
        return ss.str();
    }

protected:
    uint32_t domainId;
    ib::cfg::Config ibConfig;

    std::array<uint8_t, 1123> message = {};
    std::atomic_uint64_t receiveCount{0};
    std::size_t frameSize{};

    std::unique_ptr<Barrier> barrier;
    std::promise<void> allReceived;
    std::shared_future<void> isAllReceived;

    std::promise<void> donePromise;
    std::shared_future<void> isDone;

    const uint32_t numMessages{10};
    const std::chrono::seconds waitTime{15};
};


auto makeLoggingConfig(ib::cfg::TraceSink::Type type,const std::string& outputPath) -> ib::cfg::Config
{
    ib::cfg::ConfigBuilder builder{"TraceSinkTest"};
    builder.WithActiveMiddleware(ib::cfg::Middleware::VAsio);
    auto &&setup = builder.SimulationSetup();
    auto& writer = setup.AddParticipant("EthWriter");
    writer->AddEthernet("ETH1")
        .WithLink("LINK1")
        .WithTraceSink("EthWriterSink");

    writer->AddTraceSink("EthWriterSink")
        .WithOutputPath(outputPath)
        .WithType(type);

    auto& reader1 = setup.AddParticipant("EthReader1");
    reader1->AddEthernet("ETH1").WithLink("LINK1");


    return builder.Build();
}

TEST_F(TracingITest, test_vasio_trace_sink_pcap_file)
{
    auto fileName = MakeFilename("PcapWriter_Vasio");
    ibConfig = makeLoggingConfig(
        ib::cfg::TraceSink::Type::PcapFile,
        fileName
    );

    auto registry = std::make_unique<VAsioRegistry>(ibConfig);
    registry->ProvideDomain(domainId);

    ExecuteFileTest(fileName);
}

TEST_F(TracingITest, test_fasttps_trace_sink_pcap_file)
{
    auto fileName = MakeFilename("PcapWriter_Fastrtps");
    ibConfig = makeLoggingConfig(
        ib::cfg::TraceSink::Type::PcapFile,
        fileName
    );
    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::FastRTPS;

    ExecuteFileTest(fileName);
}


TEST_F(TracingITest, test_vasio_trace_sink_pcap_pipe)
{
    //TODO we should use a temporary but unique file name
    auto fileName = MakeFilename("PcapWriter_Vasio_Pipe");

    ibConfig = makeLoggingConfig(
        ib::cfg::TraceSink::Type::PcapPipe,
        fileName
    );

    auto registry = std::make_unique<VAsioRegistry>(ibConfig);
    registry->ProvideDomain(domainId);

    ExecutePipeTest(fileName);
}

TEST_F(TracingITest, test_fastrtps_trace_sink_pcap_pipe)
{
    auto fileName = MakeFilename("PcapWriter_Fastrtps_Pipe");

    ibConfig = makeLoggingConfig(
        ib::cfg::TraceSink::Type::PcapPipe,
        fileName
    );
    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::FastRTPS;

    ExecutePipeTest(fileName);
}

} // anonymous namespace
