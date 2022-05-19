// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "YamlSchema.hpp"

namespace ib {
namespace cfg {
inline namespace v4 {

//!< Create the YAML schema for VAsio ParticipantConfigurations.
auto MakeYamlSchema() -> YamlSchemaElem
{
    // Note: Keep these definitions in sync with ParticipantConfiguration.schema.json,
    //       which is currently the main reference for valid configuration files.
    YamlSchemaElem replay("Replay",
        {
            {"UseTraceSource"},
            {"Direction"},
            {"MdfChannel", {
                {"ChannelName"}, {"ChannelSource"}, {"ChannelPath"},
                {"GroupName"}, {"GroupSource"}, {"GroupPath"},
                }
            }
        }
    );
    YamlSchemaElem traceSinks("TraceSinks",
        {
            {"Name"},
            {"OutputPath"},
            {"Type"},
        }
    );
    YamlSchemaElem traceSources("TraceSources",
        {
            {"Name"},
            {"InputPath"},
            {"Type"},
        }
    );
    YamlSchemaElem logging("Logging",
        {
            {"LogFromRemotes"},
            {"FlushLevel"},
            {"Sinks", {
                    {"Type"},
                    {"Level"},
                    {"LogName"},
                },
            },

        }
    );
    YamlSchemaElem clusterParameters("ClusterParameters",
        {
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
        }
    );
    YamlSchemaElem nodeParameters("NodeParameters",
        {
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
        }
    );
    YamlSchemaElem txBufferConfigurations("TxBufferConfigurations",
        {
            {"channels"}, 
            {"slotId"}, 
            {"offset"}, 
            {"repetition"}, 
            {"PPindicator"}, 
            {"headerCrc"}, 
            {"transmissionMode"}, 
        }
    );
    std::initializer_list<YamlSchemaElem> flexrayControllerElements = {
        {"Name"},
        {"Network"},
        {"UseTraceSinks"},
        clusterParameters,
        nodeParameters,
        txBufferConfigurations,
        replay,
    };

    YamlSchemaElem ethernetControllers("EthernetControllers",
        {
            {"Name"},
            {"Network"},
            {"UseTraceSinks"},
            {"MacAddress"},
            replay,
        }
    );

    // Root element of the YAML schema
    YamlSchemaElem yamlSchema{
        // JSON schema, not interpreted by us:
        {"$schema"},
        {"SchemaVersion"},
        {"Description"},
        {"ParticipantName"},
        {"CanControllers", {
                {"Name"},
                {"Network"},
                {"UseTraceSinks"},
                replay
            }
        },
        {"LinControllers", {
                {"Name"},
                {"Network"},
                {"UseTraceSinks"},
                replay
            }
        },
        {"FlexrayControllers", flexrayControllerElements},
        {"FlexRayControllers", flexrayControllerElements}, // deprecated (renamed to FlexrayControllers)
        ethernetControllers,
        {"DataPublishers", {
                {"Name"},
                {"Topic"},
                {"UseTraceSinks"},
                replay,
            }
        },
        {"DataSubscribers", {
                {"Name"},
                {"Topic"},
                {"UseTraceSinks"},
                replay,
            }
        },
        {"RpcClients", {
                {"Name"},
                {"FunctionName"},
                {"UseTraceSinks"},
                replay,
            }
        },
        {"RpcServers", {
                {"Name"},
                {"FunctionName"},
                {"UseTraceSinks"},
                replay,
            }
        },
        logging,
        {"HealthCheck", {
                {"SoftResponseTimeout"},
                {"HardResponseTimeout"},
            }
        },
        {"Tracing", {
                traceSinks,
                traceSources
            }
        },
        {"Extensions", {
                {"SearchPathHints"}
            }
        },
        {"Middleware", {
                {"Registry", {
                        {"Hostname"},
                        {"Port"},
                        {"Logging"},
                        {"ConnectAttempts"},
                    },
                },
                {"TcpNoDelay"},
                {"TcpQuickAck"},
                {"TcpReceiveBufferSize"},
                {"TcpSendBufferSize"},
                {"EnableDomainSockets"}
            }
        }
    };
    return yamlSchema;
}

} // inline namespace v4
} // namespace cfg
} // namespace ib
