#include "YamlConfig.hpp"
#include "yaml-cpp/yaml.h"

#include "ib/cfg/Config.hpp"

#include <iostream>
#include <map>
#include <iterator>
#include <set>
#include <algorithm>

namespace {
void EmitValidJson(std::ostream& out, YAML::Node& node, YAML::NodeType::value parentType = YAML::NodeType::Undefined)
{
    uint32_t seqNum = 0;
    const bool needComma = node.size() > 1;
    if (parentType == YAML::NodeType::Undefined)
    {
        //we're at the top level
        out << "{";
    }
    for (auto kv : node)
    {
        YAML::Node val;
        if (kv.IsDefined())
        {
            val = static_cast<YAML::Node>(kv);
        }
        else if (kv.second.IsDefined())
        {
            // might be key:value kind of node
            try {
                out << "\"" << kv.first.as<std::string>() << "\" : ";
            }
            catch (...)
            {

            }
            val = kv.second;
        }
        if (val.IsDefined())
        {

            if (val.IsSequence())
            {
                out << "[";
                EmitValidJson(out, val, YAML::NodeType::Sequence);
                out << "]";
            }
            else if (val.IsMap())
            {
                out << "{";
                EmitValidJson(out, val, YAML::NodeType::Map);
                out << "}";
            }
            else if (val.IsScalar())
            {
                //XXX we should be able to query the scalar's type, instead of
                //    exception bombing our way to the final value.
                try {
                        out << val.as<double>();
                }
                catch (...)
                {
                    try {
                        out << val.as<int64_t>();
                    }
                    catch (...) { 
                        try {
                            out << (val.as<bool>() ? "true" : "false");
                        }
                        catch (...)
                        {
                            out << "\"" << val.as<std::string>() << "\"";
                        }
                    }
                }
            }

        }
        if (needComma && (seqNum < (node.size()-1)))
        {
            out << ", ";
        }
        else
        {
            out << "\n";
        }
        seqNum++;
    }
    if (parentType == YAML::NodeType::Undefined)
    {
        out << "}\n";
    }
}
} //end anonymous namespace


using ib::cfg::YamlSchemaElem;

//!< Create the VIB Yaml Schema.
static auto MakeYamlSchema() -> YamlSchemaElem
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
            {"InutPath"},
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


    //The children of "Parents" are actually anonymous JSON objects, 
    // but we're interested in parent/child relations.
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
                            {"freq"},
                        }
                    },
                    {"unit"},
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
                    }
                }
            }
        }

    }
    );
    YamlSchemaElem yamlSchema{
        {"ConfigVersion"},
        {"ConfigName"},
        {"Description"},
        {"SimulationSetup", {
                participants,
                {"Switches", {
                        {"Name"},
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
    };

    return yamlSchema;
}




namespace ib {
namespace cfg {

auto YamlToJson(const std::string& yamlString) -> std::string
{
    auto doc = YAML::Load(yamlString);
    std::stringstream buffer;
    std::stringstream warnings;

    if (!Validate(yamlString, warnings))
    {
        throw Misconfiguration{ "YAML validation returned warnings: \n" + warnings.str()};
    }
    EmitValidJson(buffer, doc);

    auto jsonString = buffer.str();
    return jsonString;
}

auto JsonToYaml(const std::string& jsonString) -> std::string
{
    auto doc = YAML::Load(jsonString);
    //use default emitter
    return YAML::Dump(doc);
}

static std::ostream& operator<<(std::ostream& out, const YAML::Mark& mark)
{
    if (!mark.is_null())
    {
        out << "line " << mark.line << ", column " << mark.column;
    }
    return out;
}


const std::string YamlValidator::_elSep{"/"};
YamlValidator::YamlValidator()
{
    auto schema = MakeYamlSchema();

    //the root element in schema can be skipped
    for (auto& subelement : schema.subelements)
    {
        UpdateIndex(subelement, "");
    }
}

void YamlValidator::UpdateIndex(const YamlSchemaElem& element, const std::string& currentParent)
{
    auto uniqueName = MakeName(currentParent, element.name);
    _index[uniqueName] = element;
    for (auto& subelement : element.subelements)
    {
        UpdateIndex(subelement, uniqueName);
    }
}

auto YamlValidator::ElementName(const std::string& elementName) const -> std::string
{
    auto sep = elementName.rfind(_elSep);
    if (sep == elementName.npos || sep == elementName.size())
    {
        return {};
    }
    return elementName.substr(sep+1, elementName.size());
}

auto YamlValidator::ParentName(const std::string& elementName) const -> std::string
{
    auto sep = elementName.rfind(_elSep);
    if (sep == elementName.npos)
    {
        throw std::runtime_error("YamlValidator: elementName" 
            + elementName + " has no parent");
    }
    else if (sep == 0)
    {
        //special case for root lookups
        return _elSep;
    }
    else
    {
        return elementName.substr(0, sep);
    }
}

auto YamlValidator::MakeName(const std::string& parentEl, const std::string& elementName) const -> std::string
{
    if (parentEl == _elSep) //special case for root lookups
    {
        return _elSep + elementName;
    }
    else
    {
        return parentEl + _elSep + elementName;
    }
}

bool YamlValidator::IsSchemaElement(const std::string& elementName) const
{
    return _index.count(elementName) > 0;
}

bool YamlValidator::HasSubelements(const std::string& elementName) const
{
    if (elementName == _elSep)
    {
        //special case for root level lookups
        return true;
    }
    else
    {
        return _index.at(elementName).subelements.size() > 0;
    }
}

bool YamlValidator::IsSubelementOf(const std::string& parentName, const std::string& elementName) const
{
    return ParentName(elementName) == parentName;
}

bool YamlValidator::IsRootElement(const std::string& elementName)
{
    return IsSubelementOf(_elSep, elementName);
}

auto YamlValidator::DocumentRoot() const -> std::string
{
    return _elSep;
}
    
static bool Validate(YAML::Node& doc, const YamlValidator& v,
    std::ostream& warnings, std::string parent)
{
    bool ok = true;
    for (auto node : doc)
    {
        if (node.IsDefined())
        {
            if(node.IsScalar())
            { 
                auto nodeName = v.MakeName(parent, node.Scalar());
                if (v.IsSchemaElement(nodeName)
                    && !v.IsSubelementOf(parent, nodeName))
                {
                    warnings << "At " << node.Mark() << ": Element \""
                        << v.ElementName(nodeName)  << "\""
                        << " is not a valid sub-element of schema path \""
                        << parent << "\"\n";
                    ok &= false;
                }
                else
                {
                    // This is a user-defined value, that is, a subelement 
                    // with no corresponding schema element as parent
                }
            }
            else if (node.IsSequence() || node.IsMap())
            {
                //anonymous container, keep parent as is
                ok &= Validate(node, v, warnings, parent);
            }
        }
        else if (node.first.IsDefined() && node.second.IsDefined())
        {
            //key value pair
            auto& key = node.first;
            auto& value = node.second;
            auto keyName = v.MakeName(parent, key.Scalar());
            // a nonempty, but invalid element name
            if (!keyName.empty() && !v.IsSchemaElement(keyName))
            {
                warnings << "At " << key.Mark() << ": Element \""
                    << v.ElementName(keyName)  << "\""
                    << " is not a valid sub-element of schema path \""
                    << parent << "\"\n";
                ok &= false;
            }
            // we are not a subelement of parent
            else if (v.HasSubelements(parent)
                && !v.IsSubelementOf(parent, keyName)
                )
            {
                warnings << "At " << key.Mark() << ": Element \""
                    << v.ElementName(keyName)  << "\""
                    << " is not a valid sub-element of schema path \""
                    << parent << "\"\n";
                ok &= false;
            }
            else if(value.IsMap() || value.IsSequence())
            {
                // nested sequences and maps might have no  key name
                std::string newParent = parent; //fallback  in case keyName is not given
                if (!keyName.empty())
                {
                    newParent = keyName;
                }

                if (v.HasSubelements(newParent))
                {
                    ok &= Validate(value, v, warnings, newParent);
                }
            }
            // XXX can this happen even for IsMap and IsSequence?
            else if (value.size() > 0)
            {
                std::string newParent = parent;
                if (!keyName.empty())
                {
                    newParent = keyName;
                }
                ok &= Validate(value, v, warnings, newParent);
            }
        }
    }
    return ok;

}
bool Validate(const std::string& yamlString, std::ostream& warningMessages)
{
    auto yamlDoc = YAML::Load(yamlString);
    YamlValidator validator;
    return Validate(yamlDoc, validator,  warningMessages, validator.DocumentRoot());
}

} // namespace cfg

} // namespace ib
