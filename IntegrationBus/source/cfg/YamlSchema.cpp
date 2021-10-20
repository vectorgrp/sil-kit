// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "YamlSchema.hpp"

namespace ib {
namespace cfg {


//!< Create the VIB YAML Schema.
namespace v1 {
auto MakeYamlSchema() -> YamlSchemaElem
{
    //NB: Keep these YamlSchemaElem in sync with IbConfig.schema.json.
    //    Currently the main reference for valid configuration files.
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
    YamlSchemaElem networkSimulators("NetworkSimulators", //XXX parent of network simulator is ambiguous?
        {
            {"Name"},
            {"SimulatedLinks"},
            {"SimulatedSwitches"},
            {"UseTraceSinks"},
            replay,
        }
    );

    YamlSchemaElem logger("Logger",
        {
            {"LogFromRemotes"},
            {"FlushLevel"},
            {"Sinks", {
                    {"Type"},
                    {"Level"},
                    {"Logname"},
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
    YamlSchemaElem txBufferConfigs("TxBufferConfigs",
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
    YamlSchemaElem ethernetControllers("EthernetControllers",
        {
            {"Name"},
            {"UseTraceSinks"},
            {"MacAddr"},
            {"PcapFile"},
            {"PcapPipe"},
            replay,
        }
    );


    //The children of "Participants" are actually anonymous JSON objects, 
    // but currently we're only interested in parent/child relations,
    // not whether the children sequence/map types do match.
    YamlSchemaElem participants("Participants",
        {
            {"Name"},
            {"Description"},
            {"IsSyncMaster"},
            networkSimulators, //new style NetworkSimulator declaration, was in SimulationSetup previously 
            traceSinks,
            traceSources,
            logger,
            {"ParticipantController", {
                    {"SyncType"},
                    {"ExecTimeLimitSoftMs"},
                    {"ExecTimeLimitHardMs"},
                }
            },
            {"CanControllers", {
                    {"Name"},
                    {"UseTraceSinks"},
                    replay
                }
            },
            {"LinControllers", {
                    {"Name"},
                    {"UseTraceSinks"},
                    replay
                }
            },
            {"FlexRayControllers", {
                    {"Name"},
                    {"UseTraceSinks"},
                    clusterParameters,
                    nodeParameters,
                    txBufferConfigs,
                    replay
                }
            },
            ethernetControllers,
            // O ports
            {"Digital-Out", {
                    {"Name"},
                    {"UseTraceSinks"},
                    {"value"},
                    replay,
                }
            },
            {"Analog-Out", {
                    {"Name"},
                    {"UseTraceSinks"},
                    {"value"},
                    {"unit"},
                    replay,
                }
            },
            {"Pwm-Out", {
                    {"Name"},
                    {"UseTraceSinks"},
                    {"freq", {
                            {"value"},
                            {"unit"},
                        }
                    },
                    {"duty"},
                    replay,
                }
            },
            {"Pattern-Out", {
                    {"Name"},
                    {"UseTraceSinks"},
                    {"value"},
                    replay,
                }
            },
            // I ports
            {"Digital-In", {
                    {"Name"},
                    {"UseTraceSinks"},
                    replay,
                }
            },
            {"Analog-In", {
                    {"Name"},
                    {"UseTraceSinks"},
                    replay,
                }
            },
            {"Pattern-In", {
                    {"Name"},
                    {"UseTraceSinks"},
                    replay,
                }
            },
            {"Pwm-In", {
                    {"Name"},
                    {"UseTraceSinks"},
                    replay,
                }
            },
            {"GenericPublishers", {
                    {"Name"},
                    {"UseTraceSinks"},
                    {"Protocol"},
                    {"DefinitionUri"},
                    replay,
                }
            },
            {"GenericSubscribers", {
                    {"Name"},
                    {"UseTraceSinks"},
                    replay,
                }
            },
        }
    );
    YamlSchemaElem middlewareConfig("MiddlewareConfig",
    {
        {"ActiveMiddleware"},
        {"FastRTPS", {
                {"DiscoveryType"},
                {"UnicastLocators"},
                {"ConfigFileName"},
                {"SendSocketBufferSize"},
                {"ListenSocketBufferSize"},
                {"HistoryDepth"},
          }
        },
        {"VAsio", {
                {"Registry", {
                        {"Hostname"},
                        {"Port"},
                        {"Logger"},
                        {"ConnectAttempts"},
                    },
                },
                {"TcpNoDelay"},
                {"TcpQuickAck"},
                {"TcpReceiveBufferSize"},
                {"TcpSendBufferSize"}
                
            }
        }

    }
    );
    // Root element of the YAML schema
    YamlSchemaElem yamlSchema{
        // JSON schema, not interpreted by us:
        {"$schema"},
        {"ConfigVersion"},
        {"SchemaVersion"},
        {"ConfigName"},
        {"Description"},
        {"SimulationSetup", {
                participants,
                {"Switches", {
                        {"Name"},
                        {"Description"},
                        {"Ports", {
                                {"Name"},
                                {"VlanIds"},
                            }
                        }
                    }
                },
                {"Links", {
                        {"Name"},
                        {"Endpoints", {
                                {"Name"}
                            },
                        },
                    }
                },
                networkSimulators, // deprecated
                {"TimeSync", {
                        {"SyncPolicy"},
                        {"TickPeriodNs"},
                    }
                },

            }
        },
        middlewareConfig,
        {"LaunchConfigurations", {
                {"Name"},
                {"Description"},
                {"NetworkNodes", {
                        {"Name"},
                        {"Platform"},
                        {"Ipv4Address"},
                        {"SshPort"},
                    },
                },
                {"ParticipantEnvironments", {
                        {"Participant"},
                        {"Environment"},
                        {"NetworkNode"},
                        {"WorkingFolder"},
                        //XXX the following is in a "oneOf" statement: we should make a choice here
                        {"CANoeProject"},
                        {"Executable"},
                        {"Arguments"},
                    }
                },
            }
        },
        {"ExtensionConfig", {
                {"SearchPathHints"}
            }
        },
    };

    return yamlSchema;
}
} // namespace v1

} // namespace cfg
} // namespace ib
