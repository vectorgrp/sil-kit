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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ITest_Internals_Rpc.hpp"

namespace {

//--------------------------------------
// Sync tests: Publish in SimulationTask
//--------------------------------------

// One client participant, one server participant
TEST_F(ITest_Internals_Rpc, test_1client_1server_sync)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1", {}, {{"ClientCtrl1", "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA"} });
    rpcs.push_back({"Server1", {{"ServerCtrl1", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}}, {}, {}});

    RunSyncTest(rpcs);
}


// Two mixed participants
TEST_F(ITest_Internals_Rpc, test_2_mixed_participants)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Mixed1", 
        {{"ServerCtrl1", "TestFuncB", "A", {}, defaultMsgSize, numCallsToReceive}},  // Server
        {{"ClientCtrl1", "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA","TestFuncB"}}); // Client

    rpcs.push_back({"Mixed2", 
        {{"ServerCtrl1", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}},  // Server
        {{"ClientCtrl1", "TestFuncB", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA","TestFuncB"}}); // Client

    RunSyncTest(rpcs);
}

// Large messages
TEST_F(ITest_Internals_Rpc, test_1client_1server_sync_largemsg)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;
    const size_t   messageSize = 250000;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1", {}, {{"ClientCtrl1", "TestFuncA", "A", {}, messageSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA"} });
    rpcs.push_back({"Server1", {{"ServerCtrl1", "TestFuncA", "A", {}, messageSize, numCallsToReceive}}, {}, {}});

    RunSyncTest(rpcs);
}

// 100 functions and one client/server participant
TEST_F(ITest_Internals_Rpc, test_1client_1server_sync_100functions)
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
        std::string clientControllerName = "ClientCtrl" + std::to_string(i);
        std::string serverControllerName = "ServerCtrl" + std::to_string(i);
        rpcs[0].AddRpcClient({clientControllerName, functionName, "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn});
        rpcs[1].AddRpcServer({serverControllerName, functionName, "A", {}, defaultMsgSize, numCallsToReceive});
    }

    RunSyncTest(rpcs);
}

// Two clients/servers with same functionName on one participant
TEST_F(ITest_Internals_Rpc, test_1client_1server_sync_samefunctionname)
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
                       {{"ClientCtrl1", "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn, expectedReturnDataUnordered},
                        {"ClientCtrl2", "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn, expectedReturnDataUnordered}},
                       {"TestFuncA"}});

    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < defaultNumCalls; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
    }

    rpcs.push_back({"Server1",
                       {{"ServerCtrl1", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive, expectedDataUnordered},
                        {"ServerCtrl2", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive, expectedDataUnordered}}, {}, {} });

    RunSyncTest(rpcs);
}


// One client participant, two server participants
TEST_F(ITest_Internals_Rpc, test_1client_2server_sync)
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
    rpcs.push_back({"Client1", {}, {{"ClientCtrl1", "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn, expectedReturnDataUnordered}}, {"TestFuncA"} });
    rpcs.push_back({"Server1", {{"ServerCtrl1", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}}, {}, {}});
    rpcs.push_back({"Server2", {{"ServerCtrl1", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}}, {}, {}});

    RunSyncTest(rpcs);
}

// Two client participants, one server participant
TEST_F(ITest_Internals_Rpc, test_Nclient_1server_sync)
{
    const uint32_t numClients = 2;
    const uint32_t numCallsToReceive = defaultNumCalls * numClients;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    for (uint32_t i = 0; i < numClients; i++)
    {
        std::string participantName = "Client" + std::to_string(i+1);
        rpcs.push_back({participantName,
                        {},
                        {{"ClientCtrl1", "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}},
                        {"TestFuncA"}});
    }
    
    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < defaultNumCalls; d++)
    {
        for (uint32_t i = 0; i < numClients; i++)
        {
            expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        }
    }
    rpcs.push_back({"Server1",
                    {{"ServerCtrl1", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive, expectedDataUnordered}},
                    {},
                    {}});

    RunSyncTest(rpcs);
}

// Wrong functionName on server2
TEST_F(ITest_Internals_Rpc, test_1client_2server_sync_wrongFunctionName)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1",
                    {},
                    {{"ClientCtrl1", "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}},
                    {"TestFuncA", "TestFuncB"}});
    rpcs.push_back({"Server1", {{"ServerCtrl1", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}}, {}, {}});
    rpcs.push_back({"Server2", {{"ServerCtrl1", "TestFuncB", "A", {}, defaultMsgSize, 0}}, {}, {}});

    RunSyncTest(rpcs);
}

// Wrong mediaType on server2
TEST_F(ITest_Internals_Rpc, test_1client_1server_sync_wrongDataMediaType)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls; 

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1",
                    {},
                    {{"ClientCtrl1", "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}},
                    {"TestFuncA", "TestFuncA"}});
    rpcs.push_back({"Server1", {{"ServerCtrl1", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}}, {}, {}});
    rpcs.push_back({"Server2", {{"ServerCtrl1", "TestFuncA", "B", {}, defaultMsgSize, 0}}, {}, {}});

    RunSyncTest(rpcs);
}

// Matching mandatory and optional labels on both sides 
TEST_F(ITest_Internals_Rpc, test_1client_1server_sync_labels)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1",
                    {},
                    {{"ClientCtrl1",
                      "TestFuncA",
                      "A",
                      {{"KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Mandatory},
                       {"KeyB", "ValB", SilKit::Services::MatchingLabel::Kind::Optional},
                       {"KeyC", "ValC", SilKit::Services::MatchingLabel::Kind::Optional}},
                      defaultMsgSize,
                      defaultNumCalls,
                      numCallsToReturn}},
                    {"TestFuncA"}});
    rpcs.push_back({"Server1",
                    {{"ServerCtrl1",
                      "TestFuncA",
                      "A",
                      {{"KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Optional},
                       {"KeyB", "ValB", SilKit::Services::MatchingLabel::Kind::Mandatory},
                       {"KeyD", "ValD", SilKit::Services::MatchingLabel::Kind::Optional}},
                      defaultMsgSize,
                      numCallsToReceive}},
                    {},
                    {}});

    RunSyncTest(rpcs);
}

// No communication with server2 (missing mandatory label on client)
TEST_F(ITest_Internals_Rpc, test_1client_2server_sync_wrong_mandatory_label)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1",
                    {},
                    {{"ClientCtrl1",
                      "TestFuncA",
                      "A",
                      {{"KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Optional},
                       {"KeyB", "ValB", SilKit::Services::MatchingLabel::Kind::Optional}},
                      defaultMsgSize,
                      defaultNumCalls,
                      numCallsToReturn}},
                    {"TestFuncA", "TestFuncA"}});
    rpcs.push_back({"Server1",
                    {{"ServerCtrl1",
                      "TestFuncA",
                      "A",
                      {{"KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Optional},
                       {"KeyB", "ValB", SilKit::Services::MatchingLabel::Kind::Optional}},
                      defaultMsgSize,
                      numCallsToReceive}},
                    {},
                    {}});
    rpcs.push_back({"Server2",
                    {{"ServerCtrl2",
                      "TestFuncA",
                      "A",
                      {{"KeyC", "ValC", SilKit::Services::MatchingLabel::Kind::Mandatory}},
                      defaultMsgSize,
                      0}}, // Receives no calls
                    {},
                    {}});

    RunSyncTest(rpcs);
}

// No communication with server1 (wrong optional label value)
TEST_F(ITest_Internals_Rpc, test_1client_2server_sync_wrong_optional_label_value)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1",
                    {},
                    {{"ClientCtrl1",
                      "TestFuncA",
                      "A",
                      {{"KeyA", "ValWrong", SilKit::Services::MatchingLabel::Kind::Optional}, // Won't match Server1, but Server2
                       {"KeyB", "ValB", SilKit::Services::MatchingLabel::Kind::Optional}},
                      defaultMsgSize,
                      defaultNumCalls,
                      numCallsToReturn}},
                    {"TestFuncA", "TestFuncA"}});
    rpcs.push_back({"Server1",
                    {{"ServerCtrl1",
                      "TestFuncA",
                      "A",
                      {{"KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Optional},
                       {"KeyB", "ValB", SilKit::Services::MatchingLabel::Kind::Optional}},
                      defaultMsgSize,
                      0}}, // Receives no calls
                    {},
                    {}});
    rpcs.push_back({"Server2",
                    {{"ServerCtrl2",
                      "TestFuncA",
                      "A",
                      {{"KeyC", "ValC", SilKit::Services::MatchingLabel::Kind::Optional}},
                      defaultMsgSize,
                      numCallsToReceive}},
                    {},
                    {}});

    RunSyncTest(rpcs);
}

// Wildcard mediaType on server
TEST_F(ITest_Internals_Rpc, test_1client_1server_sync_wildcardDxf)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1", {}, {{"ClientCtrl1", "TestFuncA", "", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA"} });
    rpcs.push_back({"Server1", {{"ServerCtrl1", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}}, {}, {}});

    RunSyncTest(rpcs);
}

//--------------------
// Self-delivery tests
//--------------------

// 1 server, 1 client on a single participant
TEST_F(ITest_Internals_Rpc, test_1_participant_selfdelivery)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Mixed1", 
        {{"ServerCtrl1", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}},
        {{"ClientCtrl1", "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}}, {"TestFuncA"}});

    RunSyncTest(rpcs);
}

// 2 servers, 2 clients on a single participant with same functionName
TEST_F(ITest_Internals_Rpc, test_1_participant_selfdelivery_same_functionname)
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
                    {{"ServerCtrl1", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive, expectedDataUnordered},
                     {"ServerCtrl2", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive, expectedDataUnordered}},
                    {{"ClientCtrl1", "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn, expectedReturnDataUnordered},
                     {"ClientCtrl2", "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn, expectedReturnDataUnordered}},
                    {"TestFuncA", "TestFuncA"}});

    RunSyncTest(rpcs);
}

// One client participant, one server participant, calling with timeout, failing with timeout
TEST_F(ITest_Internals_Rpc, test_1client_1server_sync_with_timeout_timeout)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    RpcClientInfo clientInfo{"ClientCtrl1", "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, 0};

    clientInfo.timeout = 1ms;
    clientInfo.numCallsToTimeout = numCallsToReturn;

    RpcServerInfo serverInfo{"ServerCtrl1", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive};
    serverInfo.doNotReply = true;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1", {}, {clientInfo}, {"TestFuncA"}});
    rpcs.push_back({"Server1", {serverInfo}, {}, {}});

    RunSyncTest(rpcs);
}

// One client participant, one server participant, calling with timeout, no timeout
TEST_F(ITest_Internals_Rpc, test_1client_1server_sync_with_timeout_no_timeout)
{
    const uint32_t numCallsToReceive = defaultNumCalls;

    RpcClientInfo clientInfo{"ClientCtrl1", "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReceive};

    clientInfo.timeout = 1ms;
    clientInfo.numCallsToTimeout = 0;

    RpcServerInfo serverInfo{"ServerCtrl1", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive};
    serverInfo.doNotReply = false;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1", {}, {clientInfo}, {"TestFuncA"}});
    rpcs.push_back({"Server1", {serverInfo}, {}, {}});

    RunSyncTest(rpcs);
}


//-----------------------------------------------------
// Async tests: No TimeSyncService/SimulationTask
//-----------------------------------------------------

// Async: Start servers first, call with delay to ensure reception
TEST_F(ITest_Internals_Rpc, test_1client_1server_async_vasio)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1",
                    {},
                    {{"ClientCtrl1", "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReturn}},
                    {"TestFuncA"}});
    rpcs.push_back({"Server1", {{"ServerCtrl1", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive}}, {}, {}});

    RunAsyncTest(rpcs);
}

// One client participant, one server participant, calling with timeout, failing with timeout
TEST_F(ITest_Internals_Rpc, test_1client_1server_async_with_timeout_timeout)
{
    const uint32_t numCallsToReceive = defaultNumCalls;
    const uint32_t numCallsToReturn = defaultNumCalls;

    RpcClientInfo clientInfo{"ClientCtrl1", "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, 0};

    clientInfo.timeout = 1ms;
    clientInfo.numCallsToTimeout = numCallsToReturn;

    RpcServerInfo serverInfo{"ServerCtrl1", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive};
    serverInfo.doNotReply = true;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1", {}, {clientInfo}, {"TestFuncA"}});
    rpcs.push_back({"Server1", {serverInfo}, {}, {}});

    RunAsyncTest(rpcs);
}
// One client participant, one server participant, calling with timeout, no timeout
TEST_F(ITest_Internals_Rpc, test_1client_1server_async_with_timeout_no_timeout)
{
    const uint32_t numCallsToReceive = defaultNumCalls;

    RpcClientInfo clientInfo{"ClientCtrl1", "TestFuncA", "A", {}, defaultMsgSize, defaultNumCalls, numCallsToReceive};

    clientInfo.timeout = 1ms;
    clientInfo.numCallsToTimeout = 0;

    RpcServerInfo serverInfo{"ServerCtrl1", "TestFuncA", "A", {}, defaultMsgSize, numCallsToReceive};
    serverInfo.doNotReply = false;

    std::vector<RpcParticipant> rpcs;
    rpcs.push_back({"Client1", {}, {clientInfo}, {"TestFuncA"}});
    rpcs.push_back({"Server1", {serverInfo}, {}, {}});

    RunAsyncTest(rpcs);
}

} // anonymous namespace
