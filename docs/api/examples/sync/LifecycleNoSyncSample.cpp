// Copyright (c) Vector Informatik GmbH. All rights reserved.
auto participant = ib::CreateParticipant(ibConfig, participantName, domainId, true);
auto* lifecycleService = participant->GetLifecycleService();
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
lifecycleService->SetCommunicationReadyHandler(
	[canController, &participantName]() {
		std::cout << "Initializing " << participantName << std::endl;
		canController->SetBaudRate(10000, 1000000);
		canController->Start();
});

// Set a Stop Handler
lifecycleService->SetStopHandler([]() {
	std::cout << "Stopping..." << std::endl;
});

// Set a Shutdown Handler
lifecycleService->SetShutdownHandler([]() {
	std::cout << "Shutting down..." << std::endl;
});

auto finalStateFuture = lifecycleService->ExecuteLifecycleNoTimeSync(true, true);
auto finalState = finalStateFuture.get();
