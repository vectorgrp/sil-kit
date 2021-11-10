// Copyright (c) Vector Informatik GmbH. All rights reserved.
auto comAdapter = ib::CreateComAdapter(ibConfig, participantName, domainId);
auto* participantController = comAdapter->GetParticipantController();
auto* canController = comAdapter->CreateCanController("CAN1");

canController->RegisterTransmitStatusHandler(
	[](can::ICanController* /*ctrl*/, const can::CanTransmitAcknowledge& ack) {
		//async handle transmit status
});
canController->RegisterReceiveMessageHandler(
	[](can::ICanController* /*ctrl*/, const can::CanMessage& msg) {
		//async handle message reception
});

// Set an Init Handler
participantController->SetInitHandler(
	[canController, &participantName](auto initCmd) {
		std::cout << "Initializing " << participantName << std::endl;
		canController->SetBaudRate(10000, 1000000);
		canController->Start();
});

// Set a Stop Handler
participantController->SetStopHandler([]() {
	std::cout << "Stopping..." << std::endl;
});

// Set a Shutdown Handler
participantController->SetShutdownHandler([]() {
	std::cout << "Shutting down..." << std::endl;
});

participantController->SetPeriod(200us);
if (participantName == "CanWriter")
{
	participantController->SetSimulationTask(
		[canController, sleepTimePerTick](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
			std::cout << "now=" << now << ", duration=" << duration << std::endl;
			ib::mw::sim::can::CanMessage msg;
			msg.timestamp = now;
			msg.canId = 17;
			canController->SendMessage(std::move(msg));
	});

	// This process will disconnect and reconnect during a coldswap
	participantController->EnableColdswap();
}

