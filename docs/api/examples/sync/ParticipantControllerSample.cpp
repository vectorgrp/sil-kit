// Copyright (c) Vector Informatik GmbH. All rights reserved.
auto participant = ib::CreateParticipant(ibConfig, participantName, domainId, true);
auto* participantController = participant->GetParticipantController();
auto* canController = participant->CreateCanController("CAN1", "CAN1");

canController->AddFrameTransmitHandler(
	[](can::ICanController* /*ctrl*/, const can::CanFrameTransmitEvent& ack) {
		//async handle transmit status
});
canController->AddFrameHandler(
	[](can::ICanController* /*ctrl*/, const can::CanFrameEvent& frameEvent) {
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
			ib::mw::sim::can::CanFrame msg;
			msg.timestamp = now;
			msg.canId = 17;
			canController->SendFrame(std::move(msg));
	});

	// This process will disconnect and reconnect during a coldswap
	participantController->EnableColdswap();
}

