// Copyright (c) Vector Informatik GmbH. All rights reserved.
auto participant = SilKit::CreateParticipant(config, participantName, registryUri);
auto* lifecycleService = participant->GetLifecycleService();
auto* canController = participant->CreateCanController("CAN1", "CAN1");

canController->AddFrameTransmitHandler(
	[](Can::ICanController* /*ctrl*/, const Can::CanFrameTransmitEvent& ack) {
		//async handle transmit status
});
canController->AddFrameHandler(
	[](Can::ICanController* /*ctrl*/, const Can::CanFrameEvent& frameEvent) {
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

auto finalStateFuture = lifecycleService->StartLifecycleNoTimeSync(true, true);
auto finalState = finalStateFuture.get();
