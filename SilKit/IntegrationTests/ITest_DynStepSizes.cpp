// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <memory>
#include <thread>
#include <string>
#include <chrono>
#include <iostream>
#include <sstream>
#include "ITestFixture.hpp"

using namespace std::chrono_literals;

namespace testing {
namespace internal {
template <typename Rep, typename Period>
class UniversalPrinter<std::chrono::duration<Rep, Period>>
{
public:
    static void Print(const std::chrono::duration<Rep, Period>& value, ::std::ostream* os)
    {
        *os << std::chrono::duration_cast<std::chrono::nanoseconds>(value).count() << "ns";
    }
};

template <typename R1, typename P1, typename R2, typename P2>
class UniversalPrinter<std::pair<std::chrono::duration<R1, P1>, std::chrono::duration<R2, P2>>>
{
public:
    static void Print(const std::pair<std::chrono::duration<R1, P1>, std::chrono::duration<R2, P2>>& p,
                      ::std::ostream* os)
    {
        *os << "(";
        UniversalPrinter<std::chrono::nanoseconds>::Print(p.first, os);
        *os << ", ";
        UniversalPrinter<std::chrono::nanoseconds>::Print(p.second, os);
        *os << ")";
    }
};

} // namespace internal
} // namespace testing

inline std::string ToString(const std::chrono::nanoseconds& ns)
{
    std::ostringstream os;
    testing::internal::UniversalPrinter<std::chrono::nanoseconds>::Print(ns, &os);
    return os.str();
}
inline std::string ToString(const std::pair<std::chrono::nanoseconds, std::chrono::nanoseconds>& p)
{
    std::ostringstream os;
    testing::internal::UniversalPrinter<std::pair<std::chrono::nanoseconds, std::chrono::nanoseconds>>::Print(p, &os);
    return os.str();
}

namespace {

using namespace SilKit::Tests;
using namespace SilKit::Config;
using namespace SilKit::Services;
using namespace SilKit::Services::Orchestration;

template <typename Rep, typename Period>
struct Dummy
{
    std::chrono::duration<Rep, Period> value;

    explicit Dummy(const std::chrono::duration<Rep, Period>& v)
        : value{v}
    {
    }

    bool operator==(const Dummy<Rep, Period>& other) const
    {
        return value == other.value;
    }

    friend void PrintTo(const Dummy<Rep, Period>& d, std::ostream* os)
    {
        *os << std::chrono::duration_cast<std::chrono::nanoseconds>(d.value).count() << "ns";
    }
};

#define SILKIT_ASSERT_CHRONO_EQ(expected, actual) ASSERT_EQ(Dummy{(expected)}, Dummy{(actual)})
#define SILKIT_EXPECT_CHRONO_EQ(expected, actual) EXPECT_EQ(Dummy{(expected)}, Dummy{(actual)})


struct ParticipantParams
{
    std::string name{};
    std::chrono::nanoseconds initialStepSize{1ms};
    TimeAdvanceMode timeAdvanceMode{TimeAdvanceMode::ByOwnDuration};
    
    // Change step size at these time points
    std::map<std::chrono::nanoseconds /*changeTimePoint*/, std::chrono::nanoseconds /*newStepSize*/> changeStepSizeAtTimePoints{};
    
    // Result: recorded time points and durations
    std::vector<std::pair<std::chrono::nanoseconds, std::chrono::nanoseconds>> timePointsAndDurations{}; 
};

struct ITest_DynStepSizes : ITest_SimTestHarness
{
    using ITest_SimTestHarness::ITest_SimTestHarness;
    void RunTestSetup(std::vector<ParticipantParams>& participantsParams);
    void AssertAllStepsEqual(const std::vector<ParticipantParams>& participantsParams);
    void AssertAscendingStepsWithReferenceDuration(const std::vector<ParticipantParams>& participantsParams,
                                                   std::chrono::nanoseconds refDuration);
    void AssertStepsEqual(const std::vector<std::chrono::nanoseconds>& s1,
                          const std::vector<std::chrono::nanoseconds>& s2);
};

void ITest_DynStepSizes::RunTestSetup(std::vector<ParticipantParams>& participantsParams)
{
    std::vector<std::string> participantNames;
    for (const auto& participantParams : participantsParams)
    {
        participantNames.push_back(participantParams.name);
    }
    SetupFromParticipantList(participantNames);

    std::mutex mx;

    for (auto& participantParams : participantsParams)
    {
        auto&& simParticipant = _simTestHarness->GetParticipant(participantParams.name);
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto* timeSyncService = lifecycleService->CreateTimeSyncService(participantParams.timeAdvanceMode);
        timeSyncService->SetSimulationStepHandler([timeSyncService, &participantParams, &mx, lifecycleService](auto now, auto duration) {
            if (now >= 100ms)
            {
                lifecycleService->Stop("stopping the test at 100ms");
            }
            else
            {
                std::lock_guard<std::mutex> lock(mx);
                participantParams.timePointsAndDurations.emplace_back(now, duration);

                // Check if we need to change the step size at this time point
                auto it = participantParams.changeStepSizeAtTimePoints.find(now);
                if (it != participantParams.changeStepSizeAtTimePoints.end())
                {
                    auto newStepSize = it->second;
                    timeSyncService->SetStepDuration(newStepSize);
                }
            }
        }, participantParams.initialStepSize);
    }

    auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
}

void ITest_DynStepSizes::AssertAllStepsEqual(const std::vector<ParticipantParams>& participantsParams)
{
    for (size_t i = 1; i < participantsParams.size(); ++i)
    {
        const auto& ref = participantsParams[0].timePointsAndDurations;
        const auto& cmp = participantsParams[i].timePointsAndDurations;

        ASSERT_EQ(ref.size(), cmp.size())
            << "Different number of steps for " << participantsParams[0].name << " and " << participantsParams[i].name;

        for (size_t j = 0; j < ref.size(); ++j)
        {
            EXPECT_EQ(ref[j], cmp[j]) << "Differenz at index " << j << ": " << participantsParams[0].name
                                      << "(now=" << ToString(ref[j].first) << ", duration=" << ToString(ref[j].second) << ")"
                                      << " vs " << participantsParams[i].name << "(now=" << ToString(cmp[j].first)
                                      << ", duration=" << ToString(cmp[j].second) << ")";
        }
    }
}

void ITest_DynStepSizes::AssertAscendingStepsWithReferenceDuration(
    const std::vector<ParticipantParams>& participantsParams, std::chrono::nanoseconds refDuration)
{
    for (const auto& participant : participantsParams)
    {
        const auto& steps = participant.timePointsAndDurations;
        ASSERT_FALSE(steps.empty()) << "No simulation steps for participant " << participant.name;

        for (size_t i = 0; i < steps.size(); ++i)
        {
            // Check if duration matches the reference duration
            EXPECT_EQ(steps[i].second, refDuration) << "Duration mismatch for " << participant.name << " at index " << i
                                                    << ": expected " << ToString(refDuration) << ", got " << ToString(steps[i].second);

            // Check if time points are strictly increasing by refDuration
            if (i > 0)
            {
                auto diff = steps[i].first - steps[i - 1].first;
                EXPECT_EQ(diff, refDuration)
                    << "Timestep difference for " << participant.name << " at index " << i << " is " << ToString(diff)
                    << ", expected " << ToString(refDuration) << " (" << ToString(steps[i - 1].first) << " -> "
                    << ToString(steps[i].first) << ")";
            }
        }
    }
}

void ITest_DynStepSizes::AssertStepsEqual(const std::vector<std::chrono::nanoseconds>& s1,
                                          const std::vector<std::chrono::nanoseconds>& s2)
{
    ASSERT_EQ(s1.size(), s2.size()) << "Different number of steps";

    for (size_t j = 0; j < s1.size(); ++j)
    {
        SILKIT_EXPECT_CHRONO_EQ(s1[j], s2[j]) << "Differenz at index " << j << ": "
                                << "s1 now=" << ToString(s1[j]) << " vs s2 now=" << ToString(s2[j]);
    }
}

// Zero duration is invalid and should throw
TEST_F(ITest_DynStepSizes, invalid_duration)
{
    auto invalidDuration = 0ns;
    std::vector<ParticipantParams> participantsParams = {{"P1", invalidDuration, TimeAdvanceMode::ByMinimalDuration}};
    EXPECT_THROW(RunTestSetup(participantsParams), SilKit::SilKitError);
}

// Single participant with both time advance modes
TEST_F(ITest_DynStepSizes, one_participant_ByMinimalDuration)
{
    auto refDuration = 5ms;
    std::vector<ParticipantParams> participantsParams = {{"P1", refDuration, TimeAdvanceMode::ByMinimalDuration}};
    RunTestSetup(participantsParams);
    AssertAscendingStepsWithReferenceDuration(participantsParams, refDuration);
}
TEST_F(ITest_DynStepSizes, one_participant_ByOwnDuration)
{
    auto refDuration = 5ms;
    std::vector<ParticipantParams> participantsParams = {{"P1", refDuration, TimeAdvanceMode::ByOwnDuration}};
    RunTestSetup(participantsParams);
    AssertAscendingStepsWithReferenceDuration(participantsParams, refDuration);
}

// Two/Three participants with ByMinimalDuration mode; Expect steps aligned to the minimal duration
TEST_F(ITest_DynStepSizes, two_participants_ByMinimalDuration)
{
    std::vector<ParticipantParams> participantsParams = {{"P1", 1ms, TimeAdvanceMode::ByMinimalDuration},
                                                         {"P2", 5ms, TimeAdvanceMode::ByMinimalDuration}};
    RunTestSetup(participantsParams);
    AssertAscendingStepsWithReferenceDuration(participantsParams, 1ms);
}
TEST_F(ITest_DynStepSizes, three_participants_ByMinimalDuration)
{
    std::vector<ParticipantParams> participantsParams = {{"P1", 1ms, TimeAdvanceMode::ByMinimalDuration},
                                                         {"P2", 2ms, TimeAdvanceMode::ByMinimalDuration},
                                                         {"P3", 3ms, TimeAdvanceMode::ByMinimalDuration}};
    RunTestSetup(participantsParams);
    AssertAscendingStepsWithReferenceDuration(participantsParams, 1ms);
}

// Two participants with mixed modes; Expect steps aligned to the minimal/own duration
TEST_F(ITest_DynStepSizes, two_participants_MixedTimeAdvanceModes)
{
    std::vector<ParticipantParams> participantsParams = {{"P1", 5ms, TimeAdvanceMode::ByMinimalDuration},
                                                         {"P2", 1ms, TimeAdvanceMode::ByOwnDuration}};
    RunTestSetup(participantsParams);
    AssertAscendingStepsWithReferenceDuration(participantsParams, 1ms);
}

// Three participants with mixed modes; Expect steps of P3(ByMinimalDuration) are equal to the union of P1,P2(ByOwnDuration)
TEST_F(ITest_DynStepSizes, three_participants_MixedTimeAdvanceModes)
{
    std::vector<ParticipantParams> participantsParams = {{"P1", 2ms, TimeAdvanceMode::ByOwnDuration},
                                                         {"P2", 3ms, TimeAdvanceMode::ByOwnDuration},
                                                         {"P3", 4ms, TimeAdvanceMode::ByMinimalDuration}};
    RunTestSetup(participantsParams);

    AssertAscendingStepsWithReferenceDuration({participantsParams[0]}, 2ms);
    AssertAscendingStepsWithReferenceDuration({participantsParams[1]}, 3ms);

    // Collect nows for P1 and P2
    std::set<std::chrono::nanoseconds> unionNows;
    for (size_t i = 0; i < 2; ++i)
    {
        for (const auto& step : participantsParams[i].timePointsAndDurations)
        {
            unionNows.insert(step.first);
        }
    }
    // Convert unionNows to sorted vector
    std::vector<std::chrono::nanoseconds> unionNowsVec(unionNows.begin(), unionNows.end());
    // Collect nows for P3
    std::vector<std::chrono::nanoseconds> p3Nows;
    for (const auto& step : participantsParams[2].timePointsAndDurations)
    {
        p3Nows.push_back(step.first);
    }
    // Compare P3 nows with union of P1 and P2 nows
    AssertStepsEqual(p3Nows, unionNowsVec);

}

// Change to a different step size during simulation
TEST_F(ITest_DynStepSizes, one_participant_change_step_size)
{
    std::vector<ParticipantParams> participantsParams = {
        {"P1", 1ms, TimeAdvanceMode::ByOwnDuration, {{9ms, 10ms}, {80ms, 2ms}}}};
    RunTestSetup(participantsParams);

    ParticipantParams refData;
    refData.name = "Reference";
    refData.timePointsAndDurations = {
        {0ms, 1ms},   {1ms, 1ms},   {2ms, 1ms},   {3ms, 1ms},   {4ms, 1ms},   {5ms, 1ms},   {6ms, 1ms},   {7ms, 1ms},
        {8ms, 1ms},   {9ms, 1ms},   {10ms, 10ms}, {20ms, 10ms}, {30ms, 10ms}, {40ms, 10ms}, {50ms, 10ms}, {60ms, 10ms},
        {70ms, 10ms}, {80ms, 10ms}, {90ms, 2ms},  {92ms, 2ms},  {94ms, 2ms},  {96ms, 2ms},  {98ms, 2ms}};

    AssertAllStepsEqual({participantsParams[0], refData});
}

// Change to different step sizes during simulation; mixed time advance modes
TEST_F(ITest_DynStepSizes, two_participants_mixed_change_step_size)
{
    std::vector<ParticipantParams> participantsParams = {
        {"P1", 1ms, TimeAdvanceMode::ByOwnDuration, {{9ms, 10ms}, {80ms, 2ms}}},
        {"P2", 20ms, TimeAdvanceMode::ByMinimalDuration}
    };

    RunTestSetup(participantsParams);

    ParticipantParams refData;
    refData.name = "Reference";
    refData.timePointsAndDurations = {
        {0ms, 1ms},   {1ms, 1ms},   {2ms, 1ms},   {3ms, 1ms},   {4ms, 1ms},   {5ms, 1ms},   {6ms, 1ms},   {7ms, 1ms},
        {8ms, 1ms},   {9ms, 1ms},   {10ms, 10ms}, {20ms, 10ms}, {30ms, 10ms}, {40ms, 10ms}, {50ms, 10ms}, {60ms, 10ms},
        {70ms, 10ms}, {80ms, 10ms}, {90ms, 2ms},  {92ms, 2ms},  {94ms, 2ms},  {96ms, 2ms},  {98ms, 2ms}};


    AssertAllStepsEqual({participantsParams[0], refData});
    // P2 (ByMinimalDuration) follows P1 (ByOwnDuration)
    AssertAllStepsEqual({participantsParams[0], participantsParams[1]}); 
}

// Change to different step sizes during simulation; both participants with ByMinimalDuration
TEST_F(ITest_DynStepSizes, two_participants_ByMinimalDuration_change_step_size)
{
    std::vector<ParticipantParams> participantsParams = {
        {"P1", 1ms, TimeAdvanceMode::ByMinimalDuration, {{9ms, 10ms}, {80ms, 2ms}}},
        {"P2", 20ms, TimeAdvanceMode::ByMinimalDuration}};

    RunTestSetup(participantsParams);

    ParticipantParams refData;
    refData.name = "Reference";
    refData.timePointsAndDurations = {
        {0ms, 1ms},   {1ms, 1ms},   {2ms, 1ms},   {3ms, 1ms},   {4ms, 1ms},   {5ms, 1ms},   {6ms, 1ms},   {7ms, 1ms},
        {8ms, 1ms},   {9ms, 1ms},   {10ms, 10ms}, {20ms, 10ms}, {30ms, 10ms}, {40ms, 10ms}, {50ms, 10ms}, {60ms, 10ms},
        {70ms, 10ms}, {80ms, 10ms}, {90ms, 2ms},  {92ms, 2ms},  {94ms, 2ms},  {96ms, 2ms},  {98ms, 2ms}};


    AssertAllStepsEqual({participantsParams[0], refData});
    AssertAllStepsEqual({participantsParams[0], participantsParams[1]});
}


} //end namespace
