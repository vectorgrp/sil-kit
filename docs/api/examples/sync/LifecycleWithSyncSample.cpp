auto participant = SilKit::CreateParticipant(config, participantName, registryUri);
auto* lifecycleService = participant->GetLifecycleService();
auto* timeSyncService = lifecycleService->GetTimeSyncService();
auto* canController = participant->CreateCanController("CAN1", "CAN1");

canController->AddFrameTransmitHandler(
	[](Can::ICanController* /*ctrl*/, const Can::CanFrameTransmitEvent& ack) {
		// Asynchroneously handle transmit status
});
canController->AddFrameHandler(
	[](Can::ICanController* /*ctrl*/, const Can::CanFrameEvent& frameEvent) {
		// Asynchroneously handle message reception
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

timeSyncService->SetPeriod(200us);
if (participantName == "CanWriter")
{
	timeSyncService->SetSimulationStepHandler(
		[canController, sleepTimePerTick](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
			std::cout << "now=" << now << ", duration=" << duration << std::endl;
			SilKit::Core::Services::Can::CanFrame msg;
			msg.timestamp = now;
			msg.canId = 17;
			canController->SendFrame(std::move(msg));
	}, 1ms);
}

auto finalStateFuture = lifecycleService->StartLifecycle(true, true);
auto finalState = finalStateFuture.get();
