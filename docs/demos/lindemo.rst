LIN Controllers
===============

Any Simulation

* LIN controllers must be configured for master or slave operation using either
  ILinController::SetMasterMode() or ILinController::SetSlaveMode().
* All communication is initiated by a master.

  * ILinController::SendMessage() sends a LinMessage (with payload) to all other LinControllers.
  * ILinController::RequestMessage() requests other LinControllers to send a
    LinMessage (with payload).

* Received LinMessages (sent from a master or a reply from a slave) trigger a
  callback of the ReceiveMessageHandler.
* Slaves must be configured using ILinController::SetSlaveConfiguration(...).
  This determines, how the slave reacts when receiving a LinMessage or a RxRequest.

  * ResponseMode::Rx: the slave receives data and calls the registered ReceiveMessageHandler.
  * ResponseMode::TxUnconditional: the slave sends a configured reply.
  * ResponseMode::Unused: the message / request is ignored; this is the default.

* The payload of a slave reply can be set using ILinController::SetReponse() per LinId.
  NB: ResponseMode::TxUnconditional must be set for this LinId!

Simple Simulation

* Baud rate is ignored.
* Replies are immediately sent.
* Slaves immediately generate an acknowledge upon data reception.
* Configured Checksum model is ignored.
* Payload length is not checked.
* Time stamps must be set manually.

VIBE Simulation

* Baud rate must be configured using ILinController::SetBaudRate(...).
* Checksum model and payload length of a request must match the configuration of a slave's reply.
* Masters must not send / request new messages before the last transmission / reception was
  completed (check for Acknowledgement).
* Time stamps are set according to the end time of the message transmission.
