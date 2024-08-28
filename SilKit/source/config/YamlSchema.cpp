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

#include "YamlSchema.hpp"

namespace SilKit {
namespace Config {
inline namespace v1 {

//! Create the YAML schema for VAsio ParticipantConfigurations.
auto MakeYamlSchema() -> YamlSchemaElem
{
    // Note: Keep these definitions in sync with ParticipantConfiguration.schema.json,
    //       which is currently the main reference for valid configuration files.
    YamlSchemaElem replay("Replay", {{"UseTraceSource"},
                                     {"Direction"},
                                     {"MdfChannel",
                                      {
                                          {"ChannelName"},
                                          {"ChannelSource"},
                                          {"ChannelPath"},
                                          {"GroupName"},
                                          {"GroupSource"},
                                          {"GroupPath"},
                                      }}});
    YamlSchemaElem traceSinks("TraceSinks", {
                                                {"Name"},
                                                {"OutputPath"},
                                                {"Type"},
                                            });
    YamlSchemaElem traceSources("TraceSources", {
                                                    {"Name"},
                                                    {"InputPath"},
                                                    {"Type"},
                                                });

    YamlSchemaElem metricsSinks{"Sinks",
                                {
                                    {"Type"},
                                    {"Name"},
                                }};

    YamlSchemaElem logging("Logging", {
                                          {"LogFromRemotes"},
                                          {"FlushLevel"},
                                          {
                                              "Sinks",
                                              {
                                                  {"Type"},
                                                  {"Format"},
                                                  {"Level"},
                                                  {"LogName"},
                                              },
                                          },

                                      });
    YamlSchemaElem clusterParameters("ClusterParameters", {
                                                              {"gColdstartAttempts"},
                                                              {"gCycleCountMax"},
                                                              {"gdActionPointOffset"},
                                                              {"gdDynamicSlotIdlePhase"},
                                                              {"gdMiniSlot"},
                                                              {"gdMiniSlotActionPointOffset"},
                                                              {"gdStaticSlot"},
                                                              {"gdSymbolWindow"},
                                                              {"gdSymbolWindowActionPointOffset"},
                                                              {"gdTSSTransmitter"},
                                                              {"gdWakeupTxActive"},
                                                              {"gdWakeupTxIdle"},
                                                              {"gListenNoise"},
                                                              {"gMacroPerCycle"},
                                                              {"gMaxWithoutClockCorrectionFatal"},
                                                              {"gMaxWithoutClockCorrectionPassive"},
                                                              {"gNumberOfMiniSlots"},
                                                              {"gNumberOfStaticSlots"},
                                                              {"gPayloadLengthStatic"},
                                                              {"gSyncFrameIDCountMax"},
                                                          });
    YamlSchemaElem nodeParameters("NodeParameters", {
                                                        {"pAllowHaltDueToClock"},
                                                        {"pAllowPassiveToActive"},
                                                        {"pChannels"},
                                                        {"pClusterDriftDamping"},
                                                        {"pdAcceptedStartupRange"},
                                                        {"pdListenTimeout"},
                                                        {"pKeySlotId"},
                                                        {"pKeySlotOnlyEnabled"},
                                                        {"pKeySlotUsedForStartup"},
                                                        {"pKeySlotUsedForSync"},
                                                        {"pLatestTx"},
                                                        {"pMacroInitialOffsetA"},
                                                        {"pMacroInitialOffsetB"},
                                                        {"pMicroInitialOffsetA"},
                                                        {"pMicroInitialOffsetB"},
                                                        {"pMicroPerCycle"},
                                                        {"pOffsetCorrectionOut"},
                                                        {"pOffsetCorrectionStart"},
                                                        {"pRateCorrectionOut"},
                                                        {"pWakeupChannel"},
                                                        {"pWakeupPattern"},
                                                        {"pdMicrotick"},
                                                        {"pSamplesPerMicrotick"},
                                                    });
    YamlSchemaElem txBufferConfigurations("TxBufferConfigurations", {
                                                                        {"channels"},
                                                                        {"slotId"},
                                                                        {"offset"},
                                                                        {"repetition"},
                                                                        {"PPindicator"},
                                                                        {"headerCrc"},
                                                                        {"transmissionMode"},
                                                                    });
    std::initializer_list<YamlSchemaElem> flexrayControllerElements = {
        {"Name"}, {"Network"}, {"UseTraceSinks"}, clusterParameters, nodeParameters, txBufferConfigurations, replay,
    };

    YamlSchemaElem ethernetControllers("EthernetControllers", {
                                                                  {"Name"},
                                                                  {"Network"},
                                                                  {"UseTraceSinks"},
                                                                  replay,
                                                              });

    // Root element of the YAML schema
    YamlSchemaElem yamlSchema{
        // JSON schema, not interpreted by us:
        {"$schema"},
        {"SchemaVersion"},
        {"schemaVersion"}, // should be removed in the future (deprecated)
        {"Description"},
        {"ParticipantName"},
        {"Includes", {{"SearchPathHints"}, {"Files"}}},
        {"CanControllers", {{"Name"}, {"Network"}, {"UseTraceSinks"}, replay}},
        {"LinControllers", {{"Name"}, {"Network"}, {"UseTraceSinks"}, replay}},
        {"FlexrayControllers", flexrayControllerElements},
        {"FlexRayControllers", flexrayControllerElements}, // deprecated (renamed to FlexrayControllers)
        ethernetControllers,
        {"DataPublishers",
         {
             {"Name"},
             {"Topic"},
             {"UseTraceSinks"},
             replay,
         }},
        {"DataSubscribers",
         {
             {"Name"},
             {"Topic"},
             {"UseTraceSinks"},
             replay,
         }},
        {"RpcClients",
         {
             {"Name"},
             {"FunctionName"},
             {"UseTraceSinks"},
             replay,
         }},
        {"RpcServers",
         {
             {"Name"},
             {"FunctionName"},
             {"UseTraceSinks"},
             replay,
         }},
        logging,
        {"HealthCheck",
         {
             {"SoftResponseTimeout"},
             {"HardResponseTimeout"},
         }},
        {"Tracing", {traceSinks, traceSources}},
        {"Extensions", {{"SearchPathHints"}}},
        {"Middleware",
         {
             {"RegistryUri"},
             {"ConnectAttempts"},
             {"TcpNoDelay"},
             {"TcpQuickAck"},
             {"TcpReceiveBufferSize"},
             {"TcpSendBufferSize"},
             {"EnableDomainSockets"},
             {"AcceptorUris"},
             {"RegistryAsFallbackProxy"},
             {"ExperimentalRemoteParticipantConnection"},
             {"ConnectTimeoutSeconds"},
         }},
        {"Experimental",
         {
             {"TimeSynchronization", {{"AnimationFactor"}, {"EnableMessageAggregation"}}},
             {"Metrics",
              {
                  metricsSinks,
                  {"CollectFromRemote"},
              }},
         }},
    };
    return yamlSchema;
}

} // namespace v1
} // namespace Config
} // namespace SilKit
