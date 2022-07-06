// Copyright (c) Vector Informatik GmbH. All rights reserved.
auto participant = SilKit::CreateParticipant(config, participantName, registryUri);
auto* lifecycleService = participant->GetLifecycleService();
auto* timeSyncService = lifecycleService->GetTimeSyncService();
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

timeSyncService->SetPeriod(200us);
if (participantName == "CanWriter")
{
	timeSyncService->SetSimulationTask(
		[canController, sleepTimePerTick](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
			std::cout << "now=" << now << ", duration=" << duration << std::endl;
			SilKit::Core::Services::Can::CanFrame msg;
			msg.timestamp = now;
			msg.canId = 17;
			canController->SendFrame(std::move(msg));
	});
}

auto finalStateFuture = lifecycleService->StartLifecycleWithSyncTime(timeSyncService, true, true);
auto finalState = finalStateFuture.get();
