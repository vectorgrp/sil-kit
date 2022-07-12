VAsio Network Protocol Compatibility
====================================
The VAsio protocol requires an initial handshake and a service subscription.
All network messages have an implicit header consisting of the serialized message length
and message type indicators. See `SerializedMessage` for reference.

1 - Initial Handshake
--------------------

During the inital handshake a number of messages are exchanged.
First, the `ParticipantAnnouncement` message is transmitted.
When connecting to the SIL Kit Registry a `KnownParticipants` message is returned.
In any case, a `ParticipantAnnouncementReply` is returned.

All three message types contain a header of type `RegistryMsgHeader`.
This header contains the `protocol version` of the connection.
A change of this protocol version indicates a breaking change.
The protocol version is available to the MessageBuffer class during
serialization/deserialization of data types and it is expected that
the compatibility code handles the ser/des transparently. 

The `ParticipantAnnouncementReply` contains a `Status` field to indicate if the handshake fails and the
registry header indicating the preferred protocol version of the peer.
This allows peers to indicate that they cannot communicate with the requested version and signals
the connecting peer to retry the connection with the preferred version.

Changes to these messages need to be made with special care, since 
one cannot rely on a protocol version so early in the connection handshake.

2 - Service Subscriptions
------------------------

The `ParticipantAnnouncementReply` contains a list of service subscriptions,
in the form of a vector of `VAsioMsgSubscriber` objects.
These objects contain a `msgTypeName` that is manually set for each message type,
rather than using the data type's C++ name. (see internal/traits/SilKitMsgSerdesName.hpp)
This allows us to change the API/C++ names without breaking compatibility with legacy services.

To subscribe to such an announced service, a message of type `SubscriptionAcknowledge` is
replied to the remote peer. This acknowledge also contains the remote peer's preferred service version.
In the future this will enable us to handle different service versions transparently on a per subscription base.

Compatiblity Use Cases:
=======================

- Appending new fields at the end of a data type does not change backward compatiblity.
  These new fields are ignored by legacy peers and as such should have a sane default value, which is substituted by the 
  transparent Ser/Des compat layer.

- Changing the C++ name of a service data type does not affect its msgTypeName for service subscriptions.

- Modifying a service data type, e.g. by changing size of fields, modifying order of fields, etc.
  This requires explicit compat code in the Ser/Des layer.
  If the data type is used during 1. Inital Handshake, the protocol version must be bumped and 
  compat code added to the handshake.
  If the data type is used during 2. Service Subscriptions, its data types version should be increased
  and compat code added to its Ser/Des routines. (see internal/traits/SilKitMsgVersion.hpp)

