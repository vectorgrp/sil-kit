========================
!!! VIBE Network Simulator
========================

The VIBE Network Simulator supports simulation of CAN, LIN, Ethernet,
and FlexRay networks. Compared to the simple, functional simulation,
the network simulator adds the following details:

!!! CAN / CAN FD
------------------

Delay Model Features:

* Priority based arbitration of frames according to CAN identifiers.
* Delayed transmission due to a busy bus.
* Transmission duration according to configured baud rate and frame length including stuff bits.

Transmission Acknowledgement Features:

* Models transmission errors due to a missing ACK when there is only one active CAN controller.
  This allows reproducing effects that arise when starting the communication, e.g.,
  a CAN controller entering error passive state.

!!! Ethernet
------------------

Ethernet simulation is based point-to-point connection between two controllers with the
following features:

* Transmission delays according to full-duplex operation.
* Priority ordered transmission queues according to the priority values encoded in the VLAN tag.
* Delayed connection establishment after controller activation.

Ethernet switches are modeled according to the store-and-forward and provide the
following features:

* Switch ports act as controllers to implement point-to-point as mentioned above.
* Port based filtering according to MAC addresses and VLAN IDs.
* Frame dropping with respect to VLAN tags, i.e., frames with drop eligible indicator and
  lower priority will be dropped first in case of congestion.

!!! FlexRay
------------------

The FlexRay simulation models the synchronous transmission of frames in a FlexRay cluster.

* Entire synchronous transmission cycle is modelled.
* Detailed simulation of the startup phase from the point of powering controllers on until
  a stable synchronous cycle has been established.
* Erroneous shutdown due to a loss of synchronicity can be modeled.
* Supports periodic transmission according to repetition configuration of TX buffers.
* Supports sending and receiving Wake-Up Symbols.
* Requires valid FlexRay cluster and node parameters.

!!! LIN
------------------

The LIN simulation has the following features:

* Simulated data transmission according to the LIN master/slave concept where all transmissions
  are initiated by the master.
* Calculation of the transmission delay according to payload length and configured baud rate.
* Allows reproducing communication errors that arise at a master due to missing slave responses.
