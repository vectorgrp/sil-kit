
Interoperability
================

.. contents::
   :local:
   :depth: 1

SIL Kit provides interoperability between participants and utilities from different released versions of SIL Kit.
This section documents known issues and potentially misleading messages reported by SIL Kit participants in certain cases.

Duplicate Participant Names
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Multiple participants with the same name connect to a single registry instance.

The SIL Kit Registry with version 4.0.7 or lower just drops the connection in this case.
The participant is not informed and eventually times out.

Scenario A
----------

- Registry-Version: 4.0.7 or lower
- First Participant: Any version
- Second Participant 4.0.7 or lower

The second participant connecting with the same name reports a connection timeout with the following message:

.. code-block:: powershell

   Timeout during connection handshake with SIL Kit Registry.

Scenario B (Mixed Versions)
---------------------------

- Registry-Version: 4.0.7 or lower
- First Participant: Any version
- Second Participant 4.0.8 or higher

The message reported by the second participant has been modified in SIL Kit 4.0.8 (when connecting to a registry of version 4.0.7 or lower):

.. code-block:: powershell

   Timeout during connection handshake with SIL Kit Registry. This might indicate that a participant with the same name ('...') has already connected to the registry.

Scenario C (Mixed Versions)
---------------------------

- Registry-Version: 4.0.8 or higher
- First Participant: Any version
- Second Participant 4.0.7 or lower

The registry transmits an error to the second participant in SIL Kit 4.0.8 via the ``ParticipantAnnouncementReply`` message with the ``status`` field set to ``Failed``.

The second participant does not report a timeout anymore, but immediately reports a misleading error message, referring to an unsupported protocol version:

.. code-block:: powershell

    SILKIT Connection Handshake: ParticipantAnnouncementReply contains unsupported version. participant=SilKitRegistry participant-version=3.1

Scenario D
----------

- Registry-Version: 4.0.8 or higher
- First Participant: Any version
- Second Participant 4.0.8 or higher

The registry transmits an error to the second participant in SIL Kit 4.0.8 via the ``ParticipantAnnouncementReply`` message with the ``status`` field set to ``Failed``.

The second participant does not report a timeout anymore, but immediately reports an error message containing the diagnostic message reported by the registry:

.. code-block:: powershell

    SIL Kit Connection Handshake: Received failed ParticipantAnnouncementReply from 'SilKitRegistry' with protocol version 3.1 and diagnostic message: participant with name 'CanReader' is already connected
