// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "silkit/experimental/netsim/string_utils.hpp"
#include "silkit/services/orchestration/string_utils.hpp"

#include "NetworkModel.hpp"
#include "Terminal.hpp"

namespace Introspection {

// ================================================================================
//  Presentation of service kinds and states
// ================================================================================

struct KindStyle
{
    const char* tag;   // short badge label
    const char* badge; // SGR for the badge
    const char* color; // SGR for text in that kind's color
};

inline auto StyleOf(ServiceKind kind) -> KindStyle
{
    switch (kind)
    {
    case ServiceKind::CanController:
        return {"CAN", "1;38;5;16;48;5;214", "38;5;214"};
    case ServiceKind::EthernetController:
        return {"ETH", "1;38;5;231;48;5;33", "38;5;39"};
    case ServiceKind::FlexrayController:
        return {"FR ", "1;38;5;231;48;5;170", "38;5;176"};
    case ServiceKind::LinController:
        return {"LIN", "1;38;5;16;48;5;44", "38;5;44"};
    case ServiceKind::DataPublisher:
        return {"PUB", "1;38;5;16;48;5;41", "38;5;41"};
    case ServiceKind::DataSubscriber:
        return {"SUB", "1;38;5;16;48;5;151", "38;5;151"};
    case ServiceKind::RpcClient:
        return {"CLI", "1;38;5;16;48;5;218", "38;5;218"};
    case ServiceKind::RpcServer:
        return {"SRV", "1;38;5;231;48;5;168", "38;5;175"};
    case ServiceKind::NetworkSimulatorLink:
        return {"SIM", "1;38;5;16;48;5;141", "38;5;141"};
    default:
        return {"???", "1;38;5;16;48;5;245", "38;5;245"};
    }
}

//! The controller kind of a bus type, used to present a network like the controllers on it.
inline auto ControllerKindOf(SimulatedNetworkType networkType) -> ServiceKind
{
    switch (networkType)
    {
    case SimulatedNetworkType::CAN:
        return ServiceKind::CanController;
    case SimulatedNetworkType::Ethernet:
        return ServiceKind::EthernetController;
    case SimulatedNetworkType::FlexRay:
        return ServiceKind::FlexrayController;
    case SimulatedNetworkType::LIN:
        return ServiceKind::LinController;
    default:
        return ServiceKind::NetworkSimulatorLink;
    }
}

inline auto Badge(ServiceKind kind) -> Text
{
    const auto style = StyleOf(kind);
    return Text{std::string{" "} + style.tag + " ", style.badge};
}

inline auto StateColor(ParticipantState state) -> const char*
{
    switch (state)
    {
    case ParticipantState::Running:
        return "1;38;5;114";
    case ParticipantState::Paused:
        return "1;38;5;221";
    case ParticipantState::ServicesCreated:
    case ParticipantState::CommunicationInitializing:
    case ParticipantState::CommunicationInitialized:
    case ParticipantState::ReadyToRun:
        return "38;5;117";
    case ParticipantState::Error:
    case ParticipantState::Aborting:
        return "1;38;5;203";
    default:
        return "38;5;245";
    }
}

inline auto SystemStateColor(SystemState state) -> const char*
{
    // The system states mirror the participant states of the same name.
    switch (state)
    {
    case SystemState::Invalid:
        return "38;5;245";
    case SystemState::ServicesCreated:
        return StateColor(ParticipantState::ServicesCreated);
    case SystemState::CommunicationInitializing:
        return StateColor(ParticipantState::CommunicationInitializing);
    case SystemState::CommunicationInitialized:
        return StateColor(ParticipantState::CommunicationInitialized);
    case SystemState::ReadyToRun:
        return StateColor(ParticipantState::ReadyToRun);
    case SystemState::Running:
        return StateColor(ParticipantState::Running);
    case SystemState::Paused:
        return StateColor(ParticipantState::Paused);
    case SystemState::Error:
    case SystemState::Aborting:
        return StateColor(ParticipantState::Error);
    default:
        return "38;5;245";
    }
}

inline auto SystemStateBadge(SystemState state) -> const char*
{
    switch (state)
    {
    case SystemState::Running:
        return "1;38;5;16;48;5;114";
    case SystemState::Paused:
        return "1;38;5;16;48;5;221";
    case SystemState::ServicesCreated:
    case SystemState::CommunicationInitializing:
    case SystemState::CommunicationInitialized:
    case SystemState::ReadyToRun:
        return "1;38;5;16;48;5;117";
    case SystemState::Error:
    case SystemState::Aborting:
        return "1;38;5;231;48;5;160";
    default:
        return "38;5;250;48;5;238";
    }
}

inline auto StateText(const Participant& participant) -> Text
{
    if (participant.life.removed)
    {
        return Text{"left", Style::Red};
    }
    if (!participant.hasState)
    {
        return Text{participant.connected ? "connected" : "seen", Style::Grey};
    }
    return Text{SilKit::Services::Orchestration::to_string(participant.state), StateColor(participant.state)};
}

inline auto OperationModeText(OperationMode mode) -> std::string
{
    switch (mode)
    {
    case OperationMode::Coordinated:
        return "Coordinated";
    case OperationMode::Autonomous:
        return "Autonomous";
    default:
        return "Unknown";
    }
}

inline auto TimeSyncText(TimeSync timeSync) -> std::string
{
    switch (timeSync)
    {
    case TimeSync::Synchronized:
        return "Synchronized";
    case TimeSync::Unsynchronized:
        return "Unsynchronized";
    default:
        return "Unknown";
    }
}

inline auto OperationModeColor(OperationMode mode) -> const char*
{
    switch (mode)
    {
    case OperationMode::Coordinated:
        return "1;38;5;147";
    case OperationMode::Autonomous:
        return "1;38;5;80";
    default:
        return "38;5;245";
    }
}

inline auto TimeSyncColor(TimeSync timeSync) -> const char*
{
    return timeSync == TimeSync::Synchronized ? "1;38;5;221" : "38;5;245";
}

//! Badges for the operation mode and the time synchronization of a participant. Both are only known
//! once the participant has started its lifecycle; until then they are shown as unknown.
inline auto OrchestrationBadges(const Participant& participant) -> Text
{
    const auto unknown = "38;5;250;48;5;238";
    Text badges;
    switch (participant.operationMode)
    {
    case OperationMode::Coordinated:
        badges.Add(" COORDINATED ", "1;38;5;16;48;5;147");
        break;
    case OperationMode::Autonomous:
        badges.Add(" AUTONOMOUS ", "1;38;5;16;48;5;80");
        break;
    default:
        badges.Add(" LIFECYCLE UNKNOWN ", unknown);
        break;
    }
    badges.Spaces(1);
    switch (participant.timeSync)
    {
    case TimeSync::Synchronized:
        badges.Add(" TIME SYNC ", "1;38;5;16;48;5;221");
        break;
    case TimeSync::Unsynchronized:
        badges.Add(" NO TIME SYNC ", unknown);
        break;
    default:
        badges.Add(" TIME SYNC UNKNOWN ", unknown);
        break;
    }
    return badges;
}

inline auto FormatLabels(const std::vector<SilKit::Services::MatchingLabel>& labels) -> std::string
{
    if (labels.empty())
    {
        return {};
    }
    std::string out = " {";
    for (size_t i = 0; i < labels.size(); ++i)
    {
        out += (i == 0 ? "" : ", ") + labels[i].key + "=" + labels[i].value;
        if (labels[i].kind == SilKit::Services::MatchingLabel::Kind::Optional)
        {
            out += "?";
        }
    }
    return out + "}";
}

inline auto FormatClock(std::chrono::system_clock::time_point time, bool withMillis) -> std::string
{
    const auto t = std::chrono::system_clock::to_time_t(time);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream out;
    out << std::put_time(&tm, "%H:%M:%S");
    if (withMillis)
    {
        const auto ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count() % 1000;
        out << '.' << std::setw(3) << std::setfill('0') << ms;
    }
    return out.str();
}

//! Formats a virtual time point as [h:]mm:ss.mmm.
inline auto FormatSimulationTime(std::chrono::nanoseconds time) -> std::string
{
    const auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(time).count();
    const auto hours = totalMs / 3600000;
    const auto minutes = (totalMs / 60000) % 60;
    const auto seconds = (totalMs / 1000) % 60;
    const auto millis = totalMs % 1000;
    std::ostringstream out;
    out << std::setfill('0');
    if (hours > 0)
    {
        out << hours << ':';
    }
    out << std::setw(2) << minutes << ':' << std::setw(2) << seconds << '.' << std::setw(3) << millis;
    return out.str();
}

//! Formats one event of the change log as a single, concise line.
inline auto FormatLogEvent(const LogEvent& event) -> Text
{
    using Type = LogEvent::Type;
    Text line{FormatClock(event.time, true) + " ", Style::DarkGrey};

    const auto endpoint = [](const std::string& participant, const std::string& service, const char* style) {
        Text text{participant, Style::White};
        text.Add("/" + service, style);
        return text;
    };

    switch (event.type)
    {
    case Type::ServiceAdded:
    case Type::ServiceRemoved:
    {
        const bool added = event.type == Type::ServiceAdded;
        line.Add(added ? "+ " : "- ", added ? "1;38;5;114" : "1;38;5;203");
        line.Add(Badge(event.kind)).Spaces(1);
        line.Add(endpoint(event.participant, event.service, added ? Style::None : Style::Strike));
        if (!event.id.empty())
        {
            line.Add(" → ", Style::DarkGrey).Add(event.id, StyleOf(event.kind).color);
        }
        break;
    }
    case Type::MatchAdded:
    case Type::MatchRemoved:
    {
        const bool added = event.type == Type::MatchAdded;
        const bool rpc = event.kind == ServiceKind::RpcServer;
        const auto peerKind = rpc ? ServiceKind::RpcClient : ServiceKind::DataPublisher;
        line.Add(added ? "⇄ " : "✕ ", added ? "1;38;5;117" : "1;38;5;203");
        line.Add(endpoint(event.peerParticipant, event.peerService, StyleOf(peerKind).color));
        line.Add(added ? " ──▶ " : " ─╳─ ", added ? Style::Green : Style::Red);
        line.Add(endpoint(event.participant, event.service, StyleOf(event.kind).color));
        line.Add(std::string{"  "} + (rpc ? "ƒ " : "◇ ") + event.id, Style::Grey);
        break;
    }
    case Type::SimAdded:
        line.Add("◎ ", "1;38;5;141").Add(event.participant, Style::White);
        line.Add(" now simulates ", Style::Grey).Add(event.id, "1;38;5;141");
        line.Add(" (" + SilKit::Experimental::NetworkSimulation::to_string(event.networkType) + ")", Style::Grey);
        break;
    case Type::SimRemoved:
        line.Add("◎ ", "1;38;5;203").Add(event.participant, Style::White);
        line.Add(" stopped simulating ", Style::Grey).Add(event.id, Style::Strike);
        break;
    case Type::Joined:
        line.Add("◉ ", "1;38;5;114").Add(event.participant, Style::White).Add(" joined", Style::Green);
        break;
    case Type::Left:
        line.Add("○ ", "1;38;5;203").Add(event.participant, Style::White).Add(" left", Style::Red);
        break;
    case Type::ModeChanged:
        line.Add("▣ ", "1;38;5;147").Add(event.participant, Style::White).Spaces(1);
        if (event.operationMode != OperationMode::Invalid)
        {
            line.Add(OperationModeText(event.operationMode), OperationModeColor(event.operationMode));
        }
        if (event.operationMode != OperationMode::Invalid && event.timeSync != TimeSync::Unknown)
        {
            line.Add(" · ", Style::DarkGrey);
        }
        if (event.timeSync != TimeSync::Unknown)
        {
            line.Add(TimeSyncText(event.timeSync), TimeSyncColor(event.timeSync));
        }
        break;
    case Type::TimeSyncJoined:
        line.Add("◷ ", "1;38;5;221").Add("Observer", Style::White);
        line.Add(" joined the time synchronization at ", Style::Grey);
        line.Add(FormatSimulationTime(event.simulationTime), "1;38;5;221");
        break;
    case Type::SystemStateChanged:
        line.Add("◈ ", SystemStateColor(event.newSystemState)).Add("System", Style::White).Spaces(1);
        line.Add(SilKit::Services::Orchestration::to_string(event.oldSystemState), Style::Grey);
        line.Add(" → ", Style::DarkGrey);
        line.Add(SilKit::Services::Orchestration::to_string(event.newSystemState),
                 SystemStateColor(event.newSystemState));
        break;
    case Type::StateChanged:
        line.Add("◆ ", Style::Yellow).Add(event.participant, Style::White).Spaces(1);
        if (event.hadState)
        {
            line.Add(SilKit::Services::Orchestration::to_string(event.oldState), Style::Grey);
            line.Add(" → ", Style::DarkGrey);
        }
        line.Add(SilKit::Services::Orchestration::to_string(event.newState), StateColor(event.newState));
        break;
    }
    return line;
}

// ================================================================================
//  Dashboard
// ================================================================================

class Dashboard
{
public:
    Dashboard(std::string registryUri, std::string ownName)
        : _registryUri{std::move(registryUri)}
        , _ownName{std::move(ownName)}
    {
    }

    auto Render(const NetworkState& state, size_t width, size_t height) -> std::vector<Text>
    {
        _now = Clock::now();
        _state = &state;
        ++_frame;

        std::vector<Text> lines;
        // The last column stays empty: writing into it would make the terminal wrap the line.
        width = std::max<size_t>(width, 41) - 1;
        height = std::max<size_t>(height, 12);

        RenderHeader(lines, width);

        const size_t logHeight = std::min<size_t>(std::max<size_t>(height / 4, 5), 14);
        const size_t mainHeight = height - lines.size() - logHeight - 1;

        std::vector<Text> main;
        if (width >= 150)
        {
            const size_t leftWidth = std::min<size_t>(std::max<size_t>(width * 2 / 5, 44), 76);
            const size_t rightWidth = width - leftWidth - 3;
            auto left = Clip(RenderParticipants(leftWidth), mainHeight);
            auto right = Clip(RenderTopology(rightWidth, mainHeight), mainHeight);
            for (size_t i = 0; i < mainHeight; ++i)
            {
                Text row = i < left.size() ? left[i].Fit(leftWidth) : Text{}.Spaces(leftWidth);
                row.Add(" │ ", Style::DarkGrey);
                if (i < right.size())
                {
                    row.Add(right[i]);
                }
                main.push_back(row);
            }
        }
        else
        {
            main = RenderParticipants(width);
            main.emplace_back();
            const auto topology =
                RenderTopology(width, mainHeight > main.size() ? mainHeight - main.size() : 0);
            main.insert(main.end(), topology.begin(), topology.end());
            main = Clip(main, mainHeight);
            main.resize(mainHeight);
        }
        lines.insert(lines.end(), main.begin(), main.end());

        lines.push_back(SectionTitle("EVENTS", width, std::to_string(state.log.size()) + " recorded"));
        const size_t logLines = logHeight;
        const size_t first = state.log.size() > logLines ? state.log.size() - logLines : 0;
        for (size_t i = first; i < state.log.size(); ++i)
        {
            lines.push_back(Text{" "}.Add(FormatLogEvent(state.log[i])));
        }
        if (state.log.empty())
        {
            lines.push_back(Text{"  no events yet", Style::Grey});
        }
        return lines;
    }

private:
    static auto HighlightDuration() -> Clock::duration
    {
        return std::chrono::seconds{3};
    }

    // ----------------------------------------------------------------------------
    //  Helpers
    // ----------------------------------------------------------------------------

    //! Style of an element depending on its age: fresh elements glow, removed elements are struck through.
    auto LifeStyle(const Lifetime& life, const char* normal) const -> const char*
    {
        if (life.removed)
        {
            return Style::Strike;
        }
        if (_now - life.born < HighlightDuration())
        {
            return Style::NewItem;
        }
        return normal;
    }

    //! Color for connectors (match arrows, bus taps): new connections glow, removed ones turn red.
    auto WireStyle(const Lifetime& life, const char* normal) const -> const char*
    {
        if (life.removed)
        {
            return "1;38;5;203";
        }
        if (_now - life.born < HighlightDuration())
        {
            return Style::NewItem;
        }
        return normal;
    }

    static auto SectionTitle(const std::string& title, size_t width, const std::string& info = {}) -> Text
    {
        Text text{"━━ ", Style::DarkGrey};
        text.Add(title, Style::Section);
        if (!info.empty())
        {
            text.Add("  " + info, Style::Grey);
        }
        text.Spaces(1);
        if (text.Width() < width)
        {
            text.Add(Repeat("━", width - text.Width()), Style::DarkGrey);
        }
        return text;
    }

    static auto Clip(std::vector<Text> lines, size_t maxLines) -> std::vector<Text>
    {
        if (lines.size() > maxLines && maxLines > 0)
        {
            const auto hidden = lines.size() - maxLines + 1;
            lines.resize(maxLines - 1);
            lines.push_back(
                Text{"  ⋯ " + std::to_string(hidden) + " more lines — enlarge the terminal", Style::Grey});
        }
        return lines;
    }

    auto Endpoint(const Service& service) const -> Text
    {
        Text text{service.participant, LifeStyle(service.life, Style::White)};
        text.Add("/" + service.name, LifeStyle(service.life, StyleOf(service.kind).color));
        text.Add(FormatLabels(service.labels), Style::DarkGrey);
        return text;
    }

    auto FindService(const std::string& participant, uint64_t serviceId) const -> const Service*
    {
        const auto it = _state->services.find(NetworkState::ServiceKey{participant, serviceId});
        return it == _state->services.end() ? nullptr : &it->second;
    }

    // ----------------------------------------------------------------------------
    //  Header
    // ----------------------------------------------------------------------------

    void RenderHeader(std::vector<Text>& lines, size_t width) const
    {
        static const char* const spinner[] = {"◐", "◓", "◑", "◒"};

        size_t participants = 0;
        size_t services = 0;
        size_t matches = 0;
        size_t simulated = 0;
        std::set<std::string> simulations;
        for (const auto& p : _state->participants)
        {
            participants += p.second.life.removed ? 0 : 1;
        }
        for (const auto& s : _state->services)
        {
            services += s.second.life.removed ? 0 : 1;
        }
        for (const auto& l : _state->matches)
        {
            matches += l.second.life.removed ? 0 : 1;
        }
        for (const auto& l : _state->simLinks)
        {
            simulated += l.second.life.removed ? 0 : 1;
        }

        Text title{" " + std::string{spinner[_frame % 4]} + " SIL Kit Network Observer ", Style::Header};
        title.Add(" " + _registryUri + "  as '" + _ownName + "'", Style::HeaderDim);
        const auto clock = " " + FormatClock(std::chrono::system_clock::now(), false) + " ";
        const auto clockWidth = VisibleWidth(clock);
        Text header = title.Truncated(width > clockWidth ? width - clockWidth : 0);
        if (header.Width() + clockWidth < width)
        {
            header.Add(std::string(width - header.Width() - clockWidth, ' '), Style::HeaderDim);
        }
        header.Add(clock, Style::Header);
        lines.push_back(header);

        Text summary{" "};
        const auto systemState = _state->systemState;
        summary.Add(" SYSTEM ", Style::Header);
        summary.Add(" " + SilKit::Services::Orchestration::to_string(systemState) + " ", SystemStateBadge(systemState));
        if (systemState == SystemState::Invalid)
        {
            summary.Add(" no required participants", "3;38;5;242");
        }
        const auto& simTime = _state->simulationTime;
        if (simTime.enabled)
        {
            summary.Add("   ").Add(" SIM TIME ", Style::Header);
            if (simTime.valid)
            {
                summary.Add(" " + FormatSimulationTime(simTime.now) + " ", "1;38;5;16;48;5;221");
            }
            else
            {
                summary.Add(" --:--.--- ", "38;5;250;48;5;238");
            }
            if (simTime.waiting)
            {
                summary.Add(" waiting for synchronized participants", "3;38;5;242");
            }
            else if (simTime.realTimeFactor > 0.0)
            {
                std::ostringstream factor;
                factor << std::fixed << std::setprecision(simTime.realTimeFactor < 10.0 ? 2 : 0)
                       << simTime.realTimeFactor;
                summary.Add(" ×" + factor.str() + " real time", Style::Grey);
            }
        }
        summary.Add("   ");
        const auto count = [&summary](const char* icon, const char* iconStyle, size_t value, const char* label) {
            summary.Add(icon, iconStyle).Add(std::to_string(value), Style::White).Add(label, Style::Grey);
        };
        count("● ", Style::Green, participants, " participants   ");
        count("◆ ", Style::Yellow, services, " services   ");
        count("⇄ ", Style::Cyan, matches, " matches   ");
        count("◎ ", "38;5;141", simulated, " simulated networks");
        lines.push_back(summary);
    }

    // ----------------------------------------------------------------------------
    //  Participants panel
    // ----------------------------------------------------------------------------

    auto ParticipantBox(const Participant& participant, size_t maxWidth) const -> std::vector<Text>
    {
        std::vector<Text> content;
        content.push_back(OrchestrationBadges(participant));
        for (const auto& entry : _state->services)
        {
            const auto& service = entry.second;
            if (service.participant != participant.name)
            {
                continue;
            }
            Text line = Badge(service.kind);
            line.Spaces(1).Add(service.name, LifeStyle(service.life, Style::None));
            if (!service.id.empty())
            {
                line.Add(" → ", Style::DarkGrey).Add(service.id, LifeStyle(service.life, StyleOf(service.kind).color));
            }
            if (!service.mediaType.empty())
            {
                line.Add(" [" + service.mediaType + "]", Style::DarkGrey);
            }
            content.push_back(line);
        }
        for (const auto& entry : _state->simLinks)
        {
            const auto& sim = entry.second;
            if (sim.participant == participant.name)
            {
                Text line = Badge(ServiceKind::NetworkSimulatorLink);
                line.Add(" simulates ", Style::Grey).Add(sim.network, LifeStyle(sim.life, "1;38;5;141"));
                line.Add(" (" + SilKit::Experimental::NetworkSimulation::to_string(sim.networkType) + ")",
                         Style::Grey);
                content.push_back(line);
            }
        }
        if (content.size() == 1)
        {
            content.emplace_back(" no user-facing services", "3;38;5;242");
        }

        const auto border = participant.life.removed ? "38;5;203" : (_now - participant.life.born < HighlightDuration()
                                                                         ? "38;5;156"
                                                                         : "38;5;67");
        const auto name = Text{participant.name, participant.life.removed ? Style::Strike : Style::White};
        const auto state = StateText(participant);

        size_t inner = std::max(name.Width() + state.Width() + 5, size_t{24});
        for (const auto& line : content)
        {
            inner = std::max(inner, line.Width());
        }
        inner = std::max(std::min(inner, maxWidth - 4), state.Width() + 8);

        std::vector<Text> box;
        Text top{"╭─ ", border};
        top.Add(name.Truncated(inner - state.Width() - 4)).Spaces(1);
        const auto fill = inner + 4 - top.Width() - state.Width() - 4;
        top.Add(Repeat("─", fill), border).Spaces(1).Add(state).Add(" ─╮", border);
        box.push_back(top);
        for (const auto& line : content)
        {
            Text row{"│ ", border};
            row.Add(line.Fit(inner)).Add(" │", border);
            box.push_back(row);
        }
        box.emplace_back("╰" + Repeat("─", inner + 2) + "╯", border);
        return box;
    }

    auto RenderParticipants(size_t width) const -> std::vector<Text>
    {
        std::vector<Text> lines;
        lines.push_back(SectionTitle("PARTICIPANTS", width));
        if (_state->participants.empty())
        {
            lines.emplace_back("  waiting for participants at " + _registryUri + " …", "3;38;5;245");
            return lines;
        }

        // Flow the boxes left to right, wrapping into rows.
        std::vector<Text> row;
        size_t rowWidth = 0;
        const auto flush = [&] {
            lines.insert(lines.end(), row.begin(), row.end());
            row.clear();
            rowWidth = 0;
        };
        for (const auto& entry : _state->participants)
        {
            auto box = ParticipantBox(entry.second, width);
            const size_t boxWidth = box.front().Width();
            if (!row.empty() && rowWidth + 1 + boxWidth > width)
            {
                flush();
            }
            if (row.empty())
            {
                row = box;
                rowWidth = boxWidth;
                continue;
            }
            const size_t height = std::max(row.size(), box.size());
            row.resize(height);
            for (size_t i = 0; i < height; ++i)
            {
                row[i] = row[i].Fit(rowWidth).Spaces(1);
                if (i < box.size())
                {
                    row[i].Add(box[i]);
                }
            }
            rowWidth += 1 + boxWidth;
        }
        flush();
        return lines;
    }

    // ----------------------------------------------------------------------------
    //  Topology panel
    // ----------------------------------------------------------------------------

    //! Renders the full topology, or a compact variant (no service names on buses, no spacing) if the full
    //! one does not fit into the available height.
    auto RenderTopology(size_t width, size_t availableHeight) const -> std::vector<Text>
    {
        std::vector<Text> lines;
        for (const bool compact : {false, true})
        {
            lines.clear();
            RenderOrchestration(lines, width, compact);
            RenderNetworks(lines, width, compact);
            RenderMatches(lines, width, false, compact);
            RenderMatches(lines, width, true, compact);
            if (lines.size() <= availableHeight)
            {
                break;
            }
        }
        if (lines.empty())
        {
            lines.push_back(SectionTitle("TOPOLOGY", width));
            lines.emplace_back("  nothing to connect yet", "3;38;5;245");
        }
        return lines;
    }

    //! Who is coordinated or autonomous, and who takes part in the virtual time synchronization.
    void RenderOrchestration(std::vector<Text>& lines, size_t width, bool compact) const
    {
        struct Row
        {
            const char* label;
            const char* style;
            std::vector<const Participant*> members;
        };
        // Mode and time sync are announced by StartLifecycle; before that both are unknown.
        std::vector<Row> rows{
            {" COORDINATED       ", "1;38;5;16;48;5;147", {}}, {" AUTONOMOUS        ", "1;38;5;16;48;5;80", {}},
            {" LIFECYCLE UNKNOWN ", "38;5;250;48;5;238", {}},  {" TIME SYNC         ", "1;38;5;16;48;5;221", {}},
            {" NO TIME SYNC      ", "38;5;250;48;5;238", {}},  {" TIME SYNC UNKNOWN ", "38;5;250;48;5;238", {}},
        };
        for (const auto& entry : _state->participants)
        {
            const auto& p = entry.second;
            const auto mode = p.operationMode;
            rows[mode == OperationMode::Coordinated  ? 0
                 : mode == OperationMode::Autonomous ? 1
                                                     : 2]
                .members.push_back(&p);
            rows[p.timeSync == TimeSync::Synchronized     ? 3
                 : p.timeSync == TimeSync::Unsynchronized ? 4
                                                          : 5]
                .members.push_back(&p);
        }
        if (_state->participants.empty())
        {
            return;
        }

        std::string summary = std::to_string(rows[0].members.size()) + " coordinated · "
                              + std::to_string(rows[1].members.size()) + " autonomous · "
                              + std::to_string(rows[3].members.size()) + " time-synchronized";
        if (!rows[2].members.empty())
        {
            summary += " · " + std::to_string(rows[2].members.size()) + " unknown";
        }
        lines.push_back(SectionTitle("ORCHESTRATION", width, summary));
        for (const auto& row : rows)
        {
            if (row.members.empty())
            {
                continue;
            }
            // Participant names colored by their state, wrapped below the label.
            Text line{" "};
            line.Add(row.label, row.style).Spaces(1);
            const auto indent = line.Width();
            for (const auto* p : row.members)
            {
                const auto nameStyle = p->life.removed ? Style::Strike
                                                       : (p->hasState ? StateColor(p->state) : Style::Grey);
                Text name{p->name, nameStyle};
                if (line.Width() > indent && line.Width() + 2 + name.Width() > width)
                {
                    lines.push_back(line);
                    line = Text{}.Spaces(indent);
                }
                if (line.Width() > indent)
                {
                    line.Spaces(2);
                }
                line.Add(name);
            }
            lines.push_back(line);
        }
        if (!compact)
        {
            lines.emplace_back();
        }
    }

    //! Bus networks: one bus line per network with a tap for every attached controller.
    void RenderNetworks(std::vector<Text>& lines, size_t width, bool compact) const
    {
        // A network is identified by its bus type and name, for the controllers on it and its simulators alike.
        using NetworkKey = std::pair<SimulatedNetworkType, std::string>;
        std::map<NetworkKey, std::vector<const Service*>> networks;
        std::map<NetworkKey, std::vector<const SimLink*>> simulators;
        for (const auto& entry : _state->services)
        {
            if (IsBusController(entry.second.kind))
            {
                networks[NetworkKey{entry.second.networkType, entry.second.id}].push_back(&entry.second);
            }
        }
        for (const auto& entry : _state->simLinks)
        {
            const auto& sim = entry.second;
            const NetworkKey key{sim.networkType, sim.network};
            simulators[key].push_back(&sim);
            networks[key]; // simulated networks without any controller yet still deserve a line
        }
        if (networks.empty())
        {
            return;
        }

        lines.push_back(SectionTitle("NETWORKS", width, std::to_string(networks.size()) + " buses"));
        for (const auto& network : networks)
        {
            const auto kind = ControllerKindOf(network.first.first);
            const auto& name = network.first.second;
            const auto& controllers = network.second;
            const auto style = StyleOf(kind);

            Text title{" "};
            title.Add(Badge(kind)).Spaces(1).Add(name, std::string{"1;"} + style.color);
            title.Add("  " + std::to_string(controllers.size()) + " controller" + (controllers.size() == 1 ? "" : "s"),
                      Style::Grey);
            const auto sims = simulators.find(network.first);
            if (sims != simulators.end())
            {
                for (const auto* sim : sims->second)
                {
                    title.Add("   ◎ simulated by ", WireStyle(sim->life, "38;5;141"));
                    title.Add(sim->participant, LifeStyle(sim->life, "1;38;5;141"));
                }
            }
            else if (IsBusController(kind))
            {
                title.Add("   trivial simulation", Style::DarkGrey);
            }
            lines.push_back(title);

            if (!controllers.empty())
            {
                DrawBus(lines, controllers, style.color, width, compact);
            }
            if (!compact)
            {
                lines.emplace_back();
            }
        }
    }

    //!     ●═╤══════════════╤═════════════●
    //!       │              │
    //!       CanWriter      CanReader
    //!       CanController1 CanController1
    void DrawBus(std::vector<Text>& lines, const std::vector<const Service*>& controllers, const char* color,
                 size_t width, bool compact) const
    {
        const size_t indent = 3;
        size_t begin = 0;
        while (begin < controllers.size())
        {
            // Pack as many controllers as fit into this bus segment.
            size_t end = begin;
            size_t used = indent + 2;
            std::vector<size_t> columnWidths;
            while (end < controllers.size())
            {
                const auto* c = controllers[end];
                const size_t column =
                    std::max(VisibleWidth(c->participant), compact ? size_t{0} : VisibleWidth(c->name)) + 2;
                if (end > begin && used + column > width)
                {
                    break;
                }
                columnWidths.push_back(column);
                used += column;
                ++end;
            }

            const bool continued = begin > 0;
            const bool continues = end < controllers.size();
            Text bus{std::string(indent - 1, ' ')};
            bus.Add(continued ? "┄═" : "●═", color);
            Text taps{std::string(indent + 1, ' ')};
            Text participants{std::string(indent + 1, ' ')};
            Text names{std::string(indent + 1, ' ')};
            for (size_t i = begin; i < end; ++i)
            {
                const auto* c = controllers[i];
                const auto column = columnWidths[i - begin];
                bus.Add(c->life.removed ? "╪" : "╤", WireStyle(c->life, color));
                bus.Add(Repeat("═", column - 1), color);
                taps.Add(Text{c->life.removed ? "╳" : "│", WireStyle(c->life, color)}.Fit(column));
                participants.Add(Text{c->participant, LifeStyle(c->life, Style::White)}.Fit(column));
                names.Add(Text{c->name, LifeStyle(c->life, Style::Grey)}.Fit(column));
            }
            bus.Add(continues ? "═┄" : "═●", color);
            lines.push_back(bus);
            if (!compact)
            {
                lines.push_back(taps);
            }
            lines.push_back(participants);
            if (!compact)
            {
                lines.push_back(names);
            }
            begin = end;
        }
    }

    //! Pub/sub topics (or RPC functions): every publisher (client) fans out to its matched subscribers (servers).
    void RenderMatches(std::vector<Text>& lines, size_t width, bool rpc, bool compact) const
    {
        const auto senderKind = rpc ? ServiceKind::RpcClient : ServiceKind::DataPublisher;
        const auto receiverKind = rpc ? ServiceKind::RpcServer : ServiceKind::DataSubscriber;

        struct Group
        {
            std::vector<const Service*> senders;
            std::vector<const Service*> receivers;
        };
        std::map<std::string, Group> groups;
        for (const auto& entry : _state->services)
        {
            const auto& service = entry.second;
            if (service.kind == senderKind)
            {
                groups[service.id].senders.push_back(&service);
            }
            else if (service.kind == receiverKind)
            {
                groups[service.id].receivers.push_back(&service);
            }
        }
        if (groups.empty())
        {
            return;
        }

        size_t matchCount = 0;
        for (const auto& entry : _state->matches)
        {
            matchCount += (entry.second.rpc == rpc && !entry.second.life.removed) ? 1 : 0;
        }
        lines.push_back(SectionTitle(rpc ? "RPC" : "PUB/SUB", width,
                                     std::to_string(groups.size()) + (rpc ? " functions · " : " topics · ")
                                         + std::to_string(matchCount) + (matchCount == 1 ? " match" : " matches")));

        const auto senderColor = StyleOf(senderKind).color;
        for (const auto& group : groups)
        {
            Text title{" "};
            title.Add(rpc ? "ƒ " : "◇ ", senderColor).Add(group.first, Style::White);
            title.Add("  " + std::to_string(group.second.senders.size()) + (rpc ? " client · " : " pub · ")
                          + std::to_string(group.second.receivers.size()) + (rpc ? " server" : " sub"),
                      Style::Grey);
            std::set<std::string> mediaTypes;
            for (const auto* service : group.second.senders)
            {
                mediaTypes.insert(service->mediaType);
            }
            for (const auto* service : group.second.receivers)
            {
                mediaTypes.insert(service->mediaType);
            }
            for (const auto& mediaType : mediaTypes)
            {
                if (!mediaType.empty())
                {
                    title.Add("  [" + mediaType + "]", Style::DarkGrey);
                }
            }
            lines.push_back(title);

            std::set<const Service*> connectedReceivers;
            for (const auto* sender : group.second.senders)
            {
                std::vector<std::pair<const Match*, const Service*>> fanOut;
                for (const auto& entry : _state->matches)
                {
                    const auto& match = entry.second;
                    if (match.rpc == rpc && match.peerParticipant == sender->participant
                        && match.peerId == sender->serviceId)
                    {
                        const auto* receiver = FindService(match.parentParticipant, match.parentId);
                        fanOut.emplace_back(&match, receiver);
                        if (receiver != nullptr)
                        {
                            connectedReceivers.insert(receiver);
                        }
                    }
                }

                Text label{"   "};
                label.Add(Endpoint(*sender));
                if (fanOut.empty())
                {
                    label.Add(" ──╳ ", Style::DarkGrey);
                    label.Add(rpc ? "no server" : "no subscriber", "3;38;5;221");
                    lines.push_back(label);
                    continue;
                }
                const auto indent = label.Width();
                for (size_t i = 0; i < fanOut.size(); ++i)
                {
                    const auto& life = fanOut[i].first->life;
                    const auto wire = WireStyle(life, Style::Green);
                    Text line = i == 0 ? label : Text{}.Spaces(indent);
                    if (i == 0)
                    {
                        line.Add(fanOut.size() == 1 ? " ───" : " ──┬", wire);
                    }
                    else
                    {
                        line.Add(i + 1 == fanOut.size() ? "   └" : "   ├", wire);
                    }
                    line.Add(life.removed ? "─╳ " : "──▶ ", wire);
                    if (fanOut[i].second != nullptr)
                    {
                        line.Add(Endpoint(*fanOut[i].second));
                    }
                    else
                    {
                        line.Add(fanOut[i].first->parentParticipant + "/" + fanOut[i].first->parentService,
                                 LifeStyle(life, Style::None));
                    }
                    lines.push_back(line);
                }
            }

            for (const auto* receiver : group.second.receivers)
            {
                if (connectedReceivers.count(receiver) == 0)
                {
                    Text line{"   ◌ ", Style::DarkGrey};
                    line.Add(Endpoint(*receiver));
                    line.Add(rpc ? "  ◂ waiting for a client" : "  ◂ waiting for a publisher", "3;38;5;221");
                    lines.push_back(line);
                }
            }
            if (!compact)
            {
                lines.emplace_back();
            }
        }
    }

private:
    std::string _registryUri;
    std::string _ownName;
    Clock::time_point _now{};
    const NetworkState* _state{nullptr};
    size_t _frame{0};
};

} // namespace Introspection
