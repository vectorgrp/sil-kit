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

#include <iostream>

#include "silkit/services/all.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/pubsub/PubSubSpec.hpp"

#include "SimTestHarness.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;

auto Now()
{
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now);
}

class FTest_PubSubPerf : public testing::Test
{
protected:
    FTest_PubSubPerf() {}

    enum class TopicMode
    {
        IndividualTopics,
        CommonTopic
    };
    enum class LabelMode
    {
        NoLabels,
        IndividualLabelsMandatory,
        IndividualLabelsOptional,
        CommonLabels
    };
    enum class StartOrderMode
    {
        SubFirst,
        PubFirst
    };

    void ExecuteTest(std::vector<int> numberOfTopicsList, TopicMode topicModePub, LabelMode labelModePub,
                     TopicMode topicModeSub, LabelMode labelModeSub, StartOrderMode startOrder)
    {
        for (auto numberOfTopics : numberOfTopicsList)
        {
            const std::chrono::seconds timeout = 100s;
            auto start = Now();

            std::vector<std::string> syncParticipantNames = {"Publisher", "Subscriber"};
            SilKit::Tests::SimTestHarness testHarness(syncParticipantNames, "silkit://localhost:0", true);

            auto definePubSpec = [topicModePub, labelModePub](int i) {
                std::string topic = "Topic";
                if (topicModePub == TopicMode::IndividualTopics)
                {
                    topic += std::to_string(i);
                }
                SilKit::Services::PubSub::PubSubSpec dataSpec{topic, ""};
                if (labelModePub == LabelMode::IndividualLabelsMandatory)
                {
                    dataSpec.AddLabel("Key1", "Val1", SilKit::Services::MatchingLabel::Kind::Mandatory);
                    dataSpec.AddLabel("Key2", "Val2", SilKit::Services::MatchingLabel::Kind::Mandatory);
                    dataSpec.AddLabel("Key3", std::to_string(i), SilKit::Services::MatchingLabel::Kind::Mandatory);
                }
                else if (labelModePub == LabelMode::IndividualLabelsOptional)
                {
                    dataSpec.AddLabel("Key1", "Val1", SilKit::Services::MatchingLabel::Kind::Optional);
                    dataSpec.AddLabel("Key2", "Val2", SilKit::Services::MatchingLabel::Kind::Optional);
                    dataSpec.AddLabel("Key3", std::to_string(i), SilKit::Services::MatchingLabel::Kind::Optional);
                }
                else if (labelModePub == LabelMode::CommonLabels)
                {
                    dataSpec.AddLabel("Key1", "Val1", SilKit::Services::MatchingLabel::Kind::Mandatory);
                    dataSpec.AddLabel("Key2", "Val2", SilKit::Services::MatchingLabel::Kind::Optional);
                    dataSpec.AddLabel("Key3", "Val3", SilKit::Services::MatchingLabel::Kind::Mandatory);
                }
                return dataSpec;
            };

            auto defineSubSpec = [topicModeSub, labelModeSub](int i) {
                std::string topic = "Topic";
                if (topicModeSub == TopicMode::IndividualTopics)
                {
                    topic += std::to_string(i);
                }
                SilKit::Services::PubSub::PubSubSpec dataSpec{topic, ""};
                if (labelModeSub == LabelMode::IndividualLabelsMandatory)
                {
                    dataSpec.AddLabel("Key3", std::to_string(i), SilKit::Services::MatchingLabel::Kind::Mandatory);
                    dataSpec.AddLabel("Key2", "Val2", SilKit::Services::MatchingLabel::Kind::Mandatory);
                    dataSpec.AddLabel("Key1", "Val1", SilKit::Services::MatchingLabel::Kind::Mandatory);
                }
                else if (labelModeSub == LabelMode::IndividualLabelsOptional)
                {
                    dataSpec.AddLabel("Key3", std::to_string(i), SilKit::Services::MatchingLabel::Kind::Optional);
                    dataSpec.AddLabel("Key2", "Val2", SilKit::Services::MatchingLabel::Kind::Optional);
                    dataSpec.AddLabel("Key1", "Val1", SilKit::Services::MatchingLabel::Kind::Optional);
                }
                else if (labelModeSub == LabelMode::CommonLabels)
                {
                    dataSpec.AddLabel("Key3", "Val3", SilKit::Services::MatchingLabel::Kind::Mandatory);
                    dataSpec.AddLabel("Key2", "Val2", SilKit::Services::MatchingLabel::Kind::Optional);
                    dataSpec.AddLabel("Key1", "Val1", SilKit::Services::MatchingLabel::Kind::Mandatory);
                }
                return dataSpec;
            };

            // Publisher
            auto&& publisher = testHarness.GetParticipant("Publisher");
            std::vector<SilKit::Services::PubSub::IDataPublisher*> pubController;
            pubController.reserve(numberOfTopics);
            std::vector<uint8_t> testData = {1, 1, 1};
            bool allPublished = false;
            auto* timeSyncService = publisher->GetOrCreateTimeSyncService();

            auto CreatePub = [timeSyncService, &allPublished, testData, &publisher, &pubController, definePubSpec,
                              numberOfTopics]() {
                for (auto i = 0; i < numberOfTopics; i++)
                {
                    const auto controllerName = "Pub-" + std::to_string(i);
                    SilKit::Services::PubSub::PubSubSpec dataSpec = definePubSpec(i);
                    pubController.push_back(publisher->Participant()->CreateDataPublisher(controllerName, dataSpec, 0));
                }
                timeSyncService->SetSimulationStepHandler([&allPublished, testData, pubController](auto, auto) {
                    if (!allPublished)
                    {
                        for (auto p : pubController)
                        {
                            p->Publish(testData);
                        }
                        allPublished = true;
                    }
                }, 1ms);
            };

            // Subscriber
            auto&& subscriber = testHarness.GetParticipant("Subscriber");
            int receptionCount = 0;
            auto* subLifecycleService = subscriber->GetOrCreateLifecycleService();

            auto CreateSub = [subLifecycleService, &receptionCount, &subscriber, defineSubSpec, numberOfTopics]() {
                //auto subLogger = subscriber->GetLogger();
                //subLogger->Info(">>> Created Subscriber participant");

                for (auto i = 0; i < numberOfTopics; i++)
                {
                    const auto controllerName = "Sub-" + std::to_string(i);
                    SilKit::Services::PubSub::PubSubSpec dataSpec = defineSubSpec(i);
                    (void)subscriber->Participant()->CreateDataSubscriber(
                        controllerName, dataSpec,
                        [&receptionCount, numberOfTopics, &subLifecycleService](
                            SilKit::Services::PubSub::IDataSubscriber* /*subscriber*/,
                            const SilKit::Services::PubSub::DataMessageEvent& /*data*/) {
                        receptionCount++;
                        if (receptionCount == numberOfTopics)
                        {
                            subLifecycleService->Stop("Reception complete");
                        }
                    });
                }
            };

            if (startOrder == StartOrderMode::PubFirst)
            {
                CreatePub();
                CreateSub();
            }
            else
            {
                CreateSub();
                CreatePub();
            }

            testHarness.Run(timeout);

            std::chrono::duration<double> duration = Now() - start;
            std::cout << std::left << std::setw(16) << numberOfTopics << " " << duration.count() << std::endl;
        }
    }
};


TEST_F(FTest_PubSubPerf, test_pubsub_performance)
{
    TopicMode topicModePub;
    LabelMode labelModePub;
    TopicMode topicModeSub;
    LabelMode labelModeSub;
    StartOrderMode startOrder;

    // Larger set for production
    //std::vector<int> testSetBadScaling{10, 100, 500};
    //std::vector<int> testSetGoodScaling{10, 100, 500, 1000, 5000, 10000};

    // For testing
    std::vector<int> testSetBadScaling{10, 100};
    std::vector<int> testSetGoodScaling{10, 100, 1000};

    std::cout << std::endl;
    std::cout << "# IndividualTopics + NoLabels + Pub first" << std::endl;
    std::cout << "# NumberOfTopics Runtime(s)" << std::endl;
    topicModePub = TopicMode::IndividualTopics;
    labelModePub = LabelMode::NoLabels;
    topicModeSub = TopicMode::IndividualTopics;
    labelModeSub = LabelMode::NoLabels;
    startOrder = StartOrderMode::PubFirst;
    ExecuteTest(testSetGoodScaling, topicModePub, labelModePub, topicModeSub, labelModeSub, startOrder);

    std::cout << std::endl;
    std::cout << "# IndividualTopics + NoLabels + Sub first" << std::endl;
    std::cout << "# NumberOfTopics Runtime(s)" << std::endl;
    topicModePub = TopicMode::IndividualTopics;
    labelModePub = LabelMode::NoLabels;
    topicModeSub = TopicMode::IndividualTopics;
    labelModeSub = LabelMode::NoLabels;
    startOrder = StartOrderMode::SubFirst;
    ExecuteTest(testSetGoodScaling, topicModePub, labelModePub, topicModeSub, labelModeSub, startOrder);

    std::cout << std::endl;
    std::cout << "# IndividualTopics + IndividualLabels + Pub Mandatory + Sub Mandatory + Pub first" << std::endl;
    std::cout << "# NumberOfTopics Runtime(s)" << std::endl;
    topicModePub = TopicMode::IndividualTopics;
    labelModePub = LabelMode::IndividualLabelsMandatory;
    topicModeSub = TopicMode::IndividualTopics;
    labelModeSub = LabelMode::IndividualLabelsMandatory;
    startOrder = StartOrderMode::PubFirst;
    ExecuteTest(testSetGoodScaling, topicModePub, labelModePub, topicModeSub, labelModeSub, startOrder);

    std::cout << std::endl;
    std::cout << "# IndividualTopics + IndividualLabels + Pub Mandatory + Sub Mandatory + Sub first" << std::endl;
    std::cout << "# NumberOfTopics Runtime(s)" << std::endl;
    topicModePub = TopicMode::IndividualTopics;
    labelModePub = LabelMode::IndividualLabelsMandatory;
    topicModeSub = TopicMode::IndividualTopics;
    labelModeSub = LabelMode::IndividualLabelsMandatory;
    startOrder = StartOrderMode::SubFirst;
    ExecuteTest(testSetGoodScaling, topicModePub, labelModePub, topicModeSub, labelModeSub, startOrder);

    std::cout << std::endl;
    std::cout << "# CommonTopic + NoLabels + Pub first" << std::endl;
    std::cout << "# NumberOfTopics Runtime(s)" << std::endl;
    topicModePub = TopicMode::CommonTopic;
    labelModePub = LabelMode::NoLabels;
    topicModeSub = TopicMode::CommonTopic;
    labelModeSub = LabelMode::NoLabels;
    startOrder = StartOrderMode::PubFirst;
    ExecuteTest(testSetBadScaling, topicModePub, labelModePub, topicModeSub, labelModeSub, startOrder);

    std::cout << std::endl;
    std::cout << "# CommonTopic + NoLabels + Sub first" << std::endl;
    std::cout << "# NumberOfTopics Runtime(s)" << std::endl;
    topicModePub = TopicMode::CommonTopic;
    labelModePub = LabelMode::NoLabels;
    topicModeSub = TopicMode::CommonTopic;
    labelModeSub = LabelMode::NoLabels;
    startOrder = StartOrderMode::SubFirst;
    ExecuteTest(testSetBadScaling, topicModePub, labelModePub, topicModeSub, labelModeSub, startOrder);

    std::cout << std::endl;
    std::cout << "# CommonTopic + IndividualLabels + Pub Optional + Sub Optional + Pub first" << std::endl;
    std::cout << "# NumberOfTopics Runtime(s)" << std::endl;
    topicModePub = TopicMode::CommonTopic;
    labelModePub = LabelMode::IndividualLabelsOptional;
    topicModeSub = TopicMode::CommonTopic;
    labelModeSub = LabelMode::IndividualLabelsOptional;
    startOrder = StartOrderMode::PubFirst;
    ExecuteTest(testSetGoodScaling, topicModePub, labelModePub, topicModeSub, labelModeSub, startOrder);

    std::cout << std::endl;
    std::cout << "# CommonTopic + IndividualLabels + Pub Optional + Sub Optional + Sub first" << std::endl;
    std::cout << "# NumberOfTopics Runtime(s)" << std::endl;
    topicModePub = TopicMode::CommonTopic;
    labelModePub = LabelMode::IndividualLabelsOptional;
    topicModeSub = TopicMode::CommonTopic;
    labelModeSub = LabelMode::IndividualLabelsOptional;
    startOrder = StartOrderMode::SubFirst;
    ExecuteTest(testSetGoodScaling, topicModePub, labelModePub, topicModeSub, labelModeSub, startOrder);

    std::cout << std::endl;
    std::cout << "# CommonTopic + IndividualLabels + Pub Mandatory + Sub Mandatory + Pub first" << std::endl;
    std::cout << "# NumberOfTopics Runtime(s)" << std::endl;
    topicModePub = TopicMode::CommonTopic;
    labelModePub = LabelMode::IndividualLabelsMandatory;
    topicModeSub = TopicMode::CommonTopic;
    labelModeSub = LabelMode::IndividualLabelsMandatory;
    startOrder = StartOrderMode::PubFirst;
    ExecuteTest(testSetGoodScaling, topicModePub, labelModePub, topicModeSub, labelModeSub, startOrder);

    std::cout << std::endl;
    std::cout << "# CommonTopic + IndividualLabels + Pub Mandatory + Sub Mandatory + Sub first" << std::endl;
    std::cout << "# NumberOfTopics Runtime(s)" << std::endl;
    topicModePub = TopicMode::CommonTopic;
    labelModePub = LabelMode::IndividualLabelsMandatory;
    topicModeSub = TopicMode::CommonTopic;
    labelModeSub = LabelMode::IndividualLabelsMandatory;
    startOrder = StartOrderMode::SubFirst;
    ExecuteTest(testSetGoodScaling, topicModePub, labelModePub, topicModeSub, labelModeSub, startOrder);

    std::cout << std::endl;
    std::cout << "# CommonTopic + IndividualLabels + Pub Mandatory + Sub Optional + Pub first" << std::endl;
    std::cout << "# NumberOfTopics Runtime(s)" << std::endl;
    topicModePub = TopicMode::CommonTopic;
    labelModePub = LabelMode::IndividualLabelsMandatory;
    topicModeSub = TopicMode::CommonTopic;
    labelModeSub = LabelMode::IndividualLabelsOptional;
    startOrder = StartOrderMode::PubFirst;
    ExecuteTest(testSetGoodScaling, topicModePub, labelModePub, topicModeSub, labelModeSub, startOrder);

    std::cout << std::endl;
    std::cout << "# CommonTopic + IndividualLabels + Pub Mandatory + Sub Optional + Sub first" << std::endl;
    std::cout << "# NumberOfTopics Runtime(s)" << std::endl;
    topicModePub = TopicMode::CommonTopic;
    labelModePub = LabelMode::IndividualLabelsMandatory;
    topicModeSub = TopicMode::CommonTopic;
    labelModeSub = LabelMode::IndividualLabelsOptional;
    startOrder = StartOrderMode::SubFirst;
    ExecuteTest(testSetGoodScaling, topicModePub, labelModePub, topicModeSub, labelModeSub, startOrder);

    std::cout << std::endl;
    std::cout << "# CommonTopic + IndividualLabels + Pub Optional + Sub Mandatory + Pub first" << std::endl;
    std::cout << "# NumberOfTopics Runtime(s)" << std::endl;
    topicModePub = TopicMode::CommonTopic;
    labelModePub = LabelMode::IndividualLabelsOptional;
    topicModeSub = TopicMode::CommonTopic;
    labelModeSub = LabelMode::IndividualLabelsMandatory;
    startOrder = StartOrderMode::PubFirst;
    ExecuteTest(testSetGoodScaling, topicModePub, labelModePub, topicModeSub, labelModeSub, startOrder);

    std::cout << std::endl;
    std::cout << "# CommonTopic + IndividualLabels + Pub Optional + Sub Mandatory + Sub first" << std::endl;
    std::cout << "# NumberOfTopics Runtime(s)" << std::endl;
    topicModePub = TopicMode::CommonTopic;
    labelModePub = LabelMode::IndividualLabelsOptional;
    topicModeSub = TopicMode::CommonTopic;
    labelModeSub = LabelMode::IndividualLabelsMandatory;
    startOrder = StartOrderMode::SubFirst;
    ExecuteTest(testSetGoodScaling, topicModePub, labelModePub, topicModeSub, labelModeSub, startOrder);
}

} // anonymous namespace
