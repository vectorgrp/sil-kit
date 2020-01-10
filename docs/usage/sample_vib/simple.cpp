#include <thread>
#include <string>
#include <sstream>
#include <chrono>
#include <exception>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp" // string conversions for enums

using namespace ib::cfg;
using namespace ib::sim::generic;
using namespace std::chrono_literals;

const auto domainId = 123;

void run_participant(const std::string &participant_name, Config config)
{
  auto&& comAdapter = ib::CreateComAdapter(std::move(config), participant_name, domainId);
  if (!comAdapter)
  {
    std::cerr << "Cannot create " << participant_name << "!" << std::endl;
    return;
  }
  auto&& partCtrl = comAdapter->GetParticipantController();
  // callbacks for participant life cycle
  partCtrl->SetInitHandler([&participant_name](auto initCmd){
    std::cout << participant_name << " Init" << std::endl;
  });

  partCtrl->SetStopHandler([&participant_name](){
    std::cout << participant_name << " Stop" << std::endl;
  });

  partCtrl->SetShutdownHandler([&participant_name](){
    std::cout << participant_name << " Shutdown" << std::endl;
  });

  // access to the generic message 
  if (participant_name == "PublisherParticipant")
  {
    auto&& pubData = comAdapter->CreateGenericPublisher("DataService");
    partCtrl->SetSimulationTask([&pubData, &partCtrl](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
      static auto msgIdx = 0;
      //generate some random data
      std::string message = "DataService Msg" + std::to_string(msgIdx++);
      //publish the raw bytes of the message to all subscribers
      std::vector<uint8_t> data{ message.begin(), message.end() };
      pubData->Publish(std::move(data)); 
      //don't run forever
      if (msgIdx > 9) {
        partCtrl->Stop("Demo finished");
      }
      std::this_thread::sleep_for(1s);
    });
  }
  else {
    //Register callback for reception of messages
    auto&& subData = comAdapter->CreateGenericSubscriber("DataService");
    subData->SetReceiveMessageHandler([&partCtrl](IGenericSubscriber* subscriber, const std::vector<uint8_t>& data) {
      std::string message{ data.begin(), data.end() };
      auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(partCtrl->Now());
      std::cout << nowMs.count() << "ms "<< subscriber->Config().name << " <- Received data=\"" << message << "\"" << std::endl;
    });
    //simulation task must be defined
    partCtrl->SetSimulationTask([](std::chrono::nanoseconds) {
      std::this_thread::sleep_for(1s);
    });
  }
  //run the simulation main loop until finished
  try {
    auto result = partCtrl->Run();
    std::cout << participant_name << ": result: " << result << std::endl;
  }
  catch (const std::exception &e) {
    std::cout << "ERROR: exception caught: " << e.what() << std::endl;
  }
}
int main(int argc, char *argv[])
{
  //create a configuration
  ConfigBuilder configBuilder("simple app");
  auto&& simulationSetup = configBuilder
    .WithActiveMiddleware(Middleware::FastRTPS)
    .SimulationSetup();
  auto&& pubCfg = simulationSetup.AddParticipant("PublisherParticipant");
  pubCfg.ConfigureLogger().AddSink(Sink::Type::Stdout);
  pubCfg.AddParticipantController().WithSyncType(SyncType::DiscreteTime);
  pubCfg.AddGenericPublisher("DataService").WithLink("DataService");

  auto&& subCfg = simulationSetup.AddParticipant("SubscriberParticipant");
  subCfg.ConfigureLogger().AddSink(Sink::Type::Stdout);
  subCfg.AddParticipantController().WithSyncType(SyncType::DiscreteTime);
  subCfg.AddGenericSubscriber("DataService").WithLink("DataService");

  //set up simulation type and granularity
  simulationSetup
    .ConfigureTimeSync()
    .WithSyncPolicy(TimeSync::SyncPolicy::Loose)
    .WithTickPeriod(10ms)
    ;
  auto&& config = configBuilder.Build();
  std::thread publisher{ run_participant, "PublisherParticipant", config };
  std::thread subscriber{ run_participant, "SubscriberParticipant", config };

  if (publisher.joinable()) publisher.join();
  if (subscriber.joinable()) subscriber.join();
  return 0;
}
