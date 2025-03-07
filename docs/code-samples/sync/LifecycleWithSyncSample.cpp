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
auto participant = SilKit::CreateParticipant(config, participantName, registryUri);
auto* lifecycleService = participant->CreateLifecycleService({OperationMode::Coordinated});
auto* timeSyncService = lifecycleService->CreateTimeSyncService();
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

if (participantName == "CanWriter")
{
	timeSyncService->SetSimulationStepHandler(
		[canController, sleepTimePerTick](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
			std::cout << "now=" << now << ", duration=" << duration << std::endl;
			SilKit::Core::Services::Can::CanFrame msg;
			msg.timestamp = now;
			msg.canId = 17;
			canController->SendFrame(msg);
	}, 1ms);
}

auto finalStateFuture = lifecycleService->StartLifecycle();
auto finalState = finalStateFuture.get();
