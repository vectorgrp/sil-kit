// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "RpcITest.hpp"

namespace {

#if defined(IB_MW_HAVE_VASIO)

//--------------------------------------
// Sync tests: Publish in SimulationTask
//--------------------------------------

// One client participant, one server participant
TEST_F(RpcITest, test_1client_1server_sync_vasio)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1", {}, {{"TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA"} });
    rpcs.push_back({"Server1", {{"TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}}, {}, {}});

    RunSyncTest(rpcs);
}


// Two mixed participants
TEST_F(RpcITest, test_2_mixed_participants)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Mixed1", 
        {{"TestFuncB", "A", {}, defaultMsgSize, numCallsToReceive}},  // Server
        {{"TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA","TestFuncB"}}); // Client

    rpcs.push_back({"Mixed2", 
        {{"TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}},  // Server
        {{"TestFuncB", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA","TestFuncB"}}); // Client

    RunSyncTest(rpcs);
}

// Large messages
TEST_F(RpcITest, test_1client_1server_largemsg_sync_vasio)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;
    const size_t   messageSize = 250000;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1", {}, {{"TestFuncA", "A", {}, messageSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA"} });
    rpcs.push_back({"Server1", {{"TestFuncA", "A", {}, messageSize, numCallsToReceive}}, {}, {}});

    RunSyncTest(rpcs);
}

// 100 functions and one client/server participant
TEST_F(RpcITest, test_1client_1server_100functions_sync_vasio)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;
    const int numFunctions = 100;

    std::vector<RpcParticipant> rpcs;
    std::vector<std::string> expectedFunctionNames{};
    for (int i = 0; i < numFunctions; i++)
    {
        expectedFunctionNames.push_back(std::to_string(i));
    }
    rpcs.push_back({ "Client1", {}, {}, expectedFunctionNames });
    rpcs.push_back({ "Server1", {}, {}, {} });
    for (int i = 0; i < numFunctions; i++)
    {
        std::string functionName = std::to_string(i);
        RpcClientInfo cInfo{functionName, "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn};
        RpcServerInfo sInfo{functionName, "A", {}, defaultMsgSize, numCallsToReceive };
        rpcs[0].rpcClients.push_back(std::move(cInfo));
        rpcs[1].rpcServers.push_back(std::move(sInfo));
    }

    RunSyncTest(rpcs);
}

// Two clients/servers with same functionName on one participant
TEST_F(RpcITest, test_1client_1server_samefunctionname_sync_vasio)
{
    const uint32_t numCallsToReceive = defaultNumCalls * 2;
    const uint32_t numCallsToReturn = defaultNumCalls * 2;

    std::vector<RpcParticipant> rpcs;
    std::vector<std::vector<uint8_t>> expectedReturnDataUnordered;
    for (uint8_t d = 0; d < defaultNumCalls; d++)
    {
        expectedReturnDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d + rpcFuncIncrement));
        expectedReturnDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d + rpcFuncIncrement));
    }

    rpcs.push_back({ "Client1", {},
                       {{"TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn, expectedReturnDataUnordered},
                        {"TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn, expectedReturnDataUnordered}},
                       {"TestFuncA"}});

    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < defaultNumCalls; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
    }

    rpcs.push_back({"Server1",
                       {{"TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive, expectedDataUnordered},
                        {"TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive, expectedDataUnordered}}, {}, {} });

    RunSyncTest(rpcs);
}


// One client participant, two server participants
TEST_F(RpcITest, test_1client_2server_sync_vasio)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls*2;

    std::vector<std::vector<uint8_t>> expectedReturnDataUnordered;
    for (uint8_t d = 0; d < defaultNumCalls; d++)
    {
        expectedReturnDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d + rpcFuncIncrement));
        expectedReturnDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d + rpcFuncIncrement));
    }

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1", {}, {{"TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn, expectedReturnDataUnordered}}, {"TestFuncA"} });
    rpcs.push_back({"Server1", {{"TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}}, {}, {}});
    rpcs.push_back({"Server2", {{"TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}}, {}, {}});

    RunSyncTest(rpcs);
}

// Two client participants, one server participant
TEST_F(RpcITest, test_2client_1server_sync_vasio)
{
    const uint32_t numCallsToReceive = defaultNumCalls*2;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1", {}, {{"TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA"} });
    rpcs.push_back({"Client2", {}, {{"TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA"} });

    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < defaultNumCalls; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
    }
    rpcs.push_back({ "Server1", {{"TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive, expectedDataUnordered}}, {}, {} });

    RunSyncTest(rpcs);
}

// Wrong functionName on server2
TEST_F(RpcITest, test_1client_2server_wrongFunctionName_sync_vasio)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1", {}, {{"TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA", "TestFuncB"} });
    rpcs.push_back({"Server1", {{"TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}}, {}, {}});
    rpcs.push_back({"Server2", {{"TestFuncB", "A", {}, defaultMsgSize, 0}}, {}, {} });

    RunSyncTest(rpcs);
}

// Wrong dataExchangeFormat on server2
TEST_F(RpcITest, test_1client_1server_wrongDataExchangeFormat_sync_vasio)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls; 

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1", {}, {{"TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA", "TestFuncA"} });
    rpcs.push_back({"Server1", {{"TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}}, {}, {}});
    rpcs.push_back({"Server2", {{"TestFuncA", "B", {}, defaultMsgSize, 0}}, {}, {}});

    RunSyncTest(rpcs);
}

// Wrong labels on server2
TEST_F(RpcITest, test_1client_1server_wrongLabels_sync_vasio)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back(
        { "Client1", {}, {{"TestFuncA", "A", {{"KeyA", ""},{"KeyB", "ValB"}}, defaultMsgSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA", "TestFuncA"} });
    rpcs.push_back({"Server1", {{"TestFuncA", "A", {{"KeyA", "ValA"}, {"KeyB", "ValB"}}, defaultMsgSize, numCallsToReceive}}, {}, {}});
    rpcs.push_back({"Server2", {{"TestFuncA", "A", {{"KeyC", "ValC"}}, defaultMsgSize, 0}}, {}, {}});

    RunSyncTest(rpcs);
}

// Wildcard dataExchangeFormat on server
TEST_F(RpcITest, test_1client_1server_wildcardDxf_sync_vasio)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1", {}, {{"TestFuncA", "", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA"} });
    rpcs.push_back({"Server1", {{"TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}}, {}, {}});

    RunSyncTest(rpcs);
}

//--------------------
// Self-delivery tests
//--------------------

// 1 server, 1 client on a single participant
TEST_F(RpcITest, test_1_participant_selfdelivery)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Mixed1", 
        {{"TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}}, 
        {{"TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA"}});

    RunSyncTest(rpcs);
}

// 2 servers, 2 clients on a single participant with same functionName
TEST_F(RpcITest, test_1_participant_selfdelivery_same_functionname)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < defaultNumCalls; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
    }
    std::vector<std::vector<uint8_t>> expectedReturnDataUnordered;
    for (uint8_t d = 0; d < defaultNumCalls; d++)
    {
        expectedReturnDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d + rpcFuncIncrement));
        expectedReturnDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d + rpcFuncIncrement));
    }
    rpcs.push_back({"Mixed1",
                    {{"TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive, expectedDataUnordered},
                     {"TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive, expectedDataUnordered}},
                    {{"TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn, expectedReturnDataUnordered},
                     {"TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn, expectedReturnDataUnordered}},
                    {"TestFuncA", "TestFuncA"}});

    RunSyncTest(rpcs);
}

//-----------------------------------------------------
// Async tests: No participantController/SimulationTask
//-----------------------------------------------------

// Async: Start servers first, call with delay to ensure reception
TEST_F(RpcITest, test_1client_1server_async_vasio)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({ "Client1", {}, { { "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn } }, {"TestFuncA"} });
    rpcs.push_back({ "Server1", { { "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive }}, {}, {}});

    RunAsyncTest(rpcs);
}

#endif

} // anonymous namespace
