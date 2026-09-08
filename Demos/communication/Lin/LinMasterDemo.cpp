// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ApplicationBase.hpp"
#include "LinDemoCommon.hpp"

class LinMaster : public ApplicationBase
{
public:
    // Inherit constructors
    using ApplicationBase::ApplicationBase;

private:
    ILinController* _linController{nullptr};
    std::unique_ptr<LinDemoCommon::Schedule> _schedule;
    std::chrono::nanoseconds _now{0ns};
    std::string _networkName = "LIN1";
    std::string _invalidCase{};

    // Deliberately malformed LIN frames, to exercise a receiver's frame validation. Each case violates exactly one
    // check of CANoe's GetLinFrameInvalidReason (projects_source/CANoe/Source/RTEVENT/SilKit/SilKitLIN.cpp) and makes
    // it report write message 83-0602 with the quoted reason in its Details.
    //
    // Unlike CAN and Ethernet, the LIN frame status event a receiver sees is synthesised by the SIL Kit LIN
    // controller rather than handed over by the sender, so SIL Kit may sanitise or reject these values before they
    // reach the receiver. Whether a case arrives at all is part of what these cases test. The two null-pointer
    // reasons and "status is not a known SIL Kit LIN frame status" are not reachable from a participant - `status` is
    // chosen by SIL Kit's own state machine - and need a receiver-side unit test.
    struct InvalidCase
    {
        const char* name;
        const char* expectedReason;
        std::function<void(LinFrame&)> Build;
    };

    static auto InvalidCases() -> const std::vector<InvalidCase>&
    {
        static const std::vector<InvalidCase> cases{
            {"id", "id exceeds the maximum LIN frame id of 0x3F", [](LinFrame& frame) { frame.id = 0x40; }},
            {"checksummodel", "checksummodel is not a known SIL Kit LIN checksum model",
             [](LinFrame& frame) { frame.checksumModel = static_cast<LinChecksumModel>(3); }},
            {"datalength", "datalength exceeds the maximum LIN payload of 8 bytes",
             [](LinFrame& frame) { frame.dataLength = 9; }},
        };
        return cases;
    }

    static auto InvalidCaseNameList() -> std::string
    {
        std::string list;
        for (const auto& c : InvalidCases())
        {
            list += (list.empty() ? "" : ", ");
            list += c.name;
        }
        return list;
    }

    void AddCommandLineArgs() override
    {
        GetCommandLineParser()->Add<CommandlineParser::Option>(
            "network", "N", _networkName, "-N, --network <name>",
            std::vector<std::string>{"Name of the LIN network to use.", "Defaults to '" + _networkName + "'."});

        GetCommandLineParser()->Add<CommandlineParser::Option>(
            "invalid", "I", _invalidCase, "-I, --invalid <case>",
            std::vector<std::string>{"Send deliberately malformed LIN frames instead of the normal schedule,",
                                     "to exercise a receiver's frame validation.",
                                     "'all' sends every case in turn, then goes to sleep.",
                                     "'list' prints the cases with their expected reason and exits.",
                                     "Cases: " + InvalidCaseNameList() + "."});
    }

    void EvaluateCommandLineArgs() override
    {
        _networkName = GetCommandLineParser()->Get<CommandlineParser::Option>("network").Value();
        _invalidCase = GetCommandLineParser()->Get<CommandlineParser::Option>("invalid").Value();

        if (_invalidCase == "list")
        {
            std::cout << std::endl << "Malformed LIN frame cases:" << std::endl << std::endl;
            for (const auto& c : InvalidCases())
            {
                std::cout << "  " << std::setw(15) << std::left << c.name << "  " << c.expectedReason << std::endl;
            }
            std::cout << std::endl;
            std::exit(0);
        }

        if (!_invalidCase.empty() && _invalidCase != "all")
        {
            const auto& cases = InvalidCases();
            const bool known = std::any_of(cases.begin(), cases.end(), [this](const InvalidCase& c) {
                return _invalidCase == c.name;
            });
            if (!known)
            {
                throw std::runtime_error("Unknown --invalid case '" + _invalidCase + "'. Known cases: "
                                         + InvalidCaseNameList() + ", all, list.");
            }
        }
    }

    void CreateControllers() override
    {
        _linController = GetParticipant()->CreateLinController("LinController1", _networkName);

        _linController->AddFrameStatusHandler(
            [this](ILinController* /*linController*/, const LinFrameStatusEvent& frameStatusEvent) {
            switch (frameStatusEvent.status)
            {
            case LinFrameStatus::LIN_RX_OK:
                break; // good case, no need to warn
            case LinFrameStatus::LIN_TX_OK:
                break; // good case, no need to warn
            default:
                std::stringstream ss;
                ss << "LIN transmission failed!";
                GetLogger()->Warn(ss.str());
            }

            std::stringstream ss;
            ss << "Received " << frameStatusEvent.frame << ", status=" << frameStatusEvent.status;
            GetLogger()->Info(ss.str());
        });


        _linController->AddWakeupHandler([this](ILinController* /*linController*/, const LinWakeupEvent& wakeupEvent) {
            if (_linController->Status() != LinControllerStatus::Sleep)
            {
                std::stringstream ss;
                ss << "Received Wakeup pulse while LinControllerStatus is " << _linController->Status() << ".";
                GetLogger()->Warn(ss.str());
            }

            std::stringstream ss;
            ss << "Received Wakeup pulse, direction=" << wakeupEvent.direction;
            GetLogger()->Info(ss.str());

            _linController->WakeupInternal();
        });

        if (!_invalidCase.empty())
        {
            _schedule = MakeInvalidFrameSchedule();
            return;
        }

        _schedule = std::make_unique<LinDemoCommon::Schedule>(
            std::initializer_list<std::pair<std::chrono::nanoseconds, std::function<void(std::chrono::nanoseconds)>>>{
                {10ms, [this](std::chrono::nanoseconds /*now*/) { SendFrame_16(); }},
                {20ms, [this](std::chrono::nanoseconds /*now*/) { SendFrame_17(); }},
                {10ms, [this](std::chrono::nanoseconds /*now*/) { SendFrame_18(); }},
                {10ms, [this](std::chrono::nanoseconds /*now*/) { SendFrame_19(); }},
                {10ms, [this](std::chrono::nanoseconds /*now*/) { SendFrame_34(); }},
                {10ms, [this](std::chrono::nanoseconds /*now*/) { GoToSleep(); }}});
    }

    //! A schedule that sends the selected malformed frame(s) instead of the normal LIN schedule.
    auto MakeInvalidFrameSchedule() -> std::unique_ptr<LinDemoCommon::Schedule>
    {
        std::vector<std::pair<std::chrono::nanoseconds, std::function<void(std::chrono::nanoseconds)>>> tasks;
        for (const auto& invalidCase : InvalidCases())
        {
            if (_invalidCase != "all" && _invalidCase != invalidCase.name)
            {
                continue;
            }
            tasks.emplace_back(10ms, [this, &invalidCase](std::chrono::nanoseconds /*now*/) {
                SendInvalidFrame(invalidCase);
            });
        }
        tasks.emplace_back(10ms, [this](std::chrono::nanoseconds /*now*/) { GoToSleep(); });

        return std::make_unique<LinDemoCommon::Schedule>(std::move(tasks));
    }

    //! Send one deliberately malformed frame, see InvalidCases().
    void SendInvalidFrame(const InvalidCase& invalidCase)
    {
        // Start from a frame that passes every check, then let the case violate exactly one of them.
        LinFrame frame;
        frame.id = 16;
        frame.checksumModel = LinChecksumModel::Classic;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        invalidCase.Build(frame);

        std::stringstream ss;
        ss << "Sending malformed LIN frame '" << invalidCase.name << "': id=" << static_cast<unsigned>(frame.id)
           << ", checksummodel=" << static_cast<unsigned>(frame.checksumModel)
           << ", datalength=" << static_cast<unsigned>(frame.dataLength) << " - expecting reason '"
           << invalidCase.expectedReason << "'";
        GetLogger()->Info(ss.str());

        _linController->SendFrame(frame, LinFrameResponseType::MasterResponse);
    }

    void InitControllers() override
    {
        LinControllerConfig config;
        config.controllerMode = LinControllerMode::Master;
        config.baudRate = 20'000;
        _linController->Init(config);
    }

    void DoWorkSync(std::chrono::nanoseconds now) override
    {
        _now = now;
        _schedule->ExecuteTask(now);
    }

    void DoWorkAsync() override
    {
        _schedule->ExecuteTask(_now);
        _now += 1ms;
    }

    // LinMaster schedule

    void SendFrameIfOperational(const LinFrame& linFrame, LinFrameResponseType responseType)
    {
        const auto linId{static_cast<unsigned>(linFrame.id)};

        _linController->SendFrame(linFrame, responseType);

        std::stringstream ss;
        if (responseType == LinFrameResponseType::SlaveResponse)
        {
            ss << "LIN frame Header sent for ID=" << linId;
        }
        else
        {
            ss << "LIN frame sent with ID=" << linId;
        }
        GetLogger()->Info(ss.str());
    }

    void SendFrame_16()
    {
        LinFrame frame;
        frame.id = 16;
        frame.checksumModel = LinChecksumModel::Classic;
        frame.dataLength = 6;
        frame.data = std::array<uint8_t, 8>{1, 6, 1, 6, 1, 6, 1, 6};

        SendFrameIfOperational(frame, LinFrameResponseType::MasterResponse);
    }

    void SendFrame_17()
    {
        LinFrame frame;
        frame.id = 17;
        frame.checksumModel = LinChecksumModel::Classic;
        frame.dataLength = 6;
        frame.data = std::array<uint8_t, 8>{1, 7, 1, 7, 1, 7, 1, 7};

        SendFrameIfOperational(frame, LinFrameResponseType::MasterResponse);
    }

    void SendFrame_18()
    {
        LinFrame frame;
        frame.id = 18;
        frame.checksumModel = LinChecksumModel::Enhanced;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        SendFrameIfOperational(frame, LinFrameResponseType::MasterResponse);
    }

    void SendFrame_19()
    {
        LinFrame frame;
        frame.id = 19;
        frame.checksumModel = LinChecksumModel::Classic;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{0};

        SendFrameIfOperational(frame, LinFrameResponseType::MasterResponse);
    }

    void SendFrame_34()
    {
        LinFrame frame;
        frame.id = 34;
        frame.checksumModel = LinChecksumModel::Enhanced;
        frame.dataLength = 8;

        SendFrameIfOperational(frame, LinFrameResponseType::SlaveResponse);
    }

    void GoToSleep()
    {
        std::stringstream ss;
        ss << "Sending Go-To-Sleep command and entering sleep state";
        GetLogger()->Info(ss.str());

        _linController->GoToSleep();
    }
};

int main(int argc, char** argv)
{
    Arguments args;
    args.participantName = "LinMaster";
    LinMaster app{args};
    app.SetupCommandLineArgs(argc, argv, "SIL Kit Demo - Lin: Master Node");

    return app.Run();
}
