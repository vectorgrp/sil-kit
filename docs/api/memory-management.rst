=================
Memory Management
=================

.. Macros for docs use
.. |AddFrameHandler| replace:: :cpp:func:`EthernetController::AddFrameHandler()<SilKit::Services::Ethernet::IEthernetController::AddFrameHandler>`
.. |CreateEthernetController| replace:: :cpp:func:`CreateEthernetController<SilKit::IParticipant::CreateEthernetController()>`
.. |CanFrameEvent| replace:: :cpp:class:`CanFrameEvent<SilKit::Services::Can::CanFrameEvent>`
.. |Span| replace:: :cpp:class:`Span<SilKit::Util::Span>`
.. |CanFrame| replace:: :cpp:class:`CanFrame<SilKit::Services::Can::CanFrame>`
.. |SendFrame| replace:: :cpp:func:`ICanController::SendFrame()<SilKit::Services::Can::ICanController::SendFrame>`

This document describes what a user of the SIL Kit API has to be aware of regarding memory management.

Callbacks
~~~~~~~~~
When SIL Kit triggers a callback through an event handler such as e.g. |AddFrameHandler|, a user must not free or modify data provided through such an handler.
SIL Kit does own this data and deals with the needed memory management itself.
It is important to note that data provided in SIL Kit event handlers is only available during execution of the callback.
A user must not interact with such data after the execution of the callback since the corresponding memory may already be freed or reused by SIL Kit.
If data of an event handler is needed outside the scope of the callback, it must be copied. The memory management of the copied data is up to the user.

Handlers that are registered through an ``Add...Handler`` call can be removed by a user through a corresponding ``Remove...Handler`` call.
These ``Remove...Handler`` calls are optional. SIL Kit removes these handlers during simulation shutdown itself.

Controllers
~~~~~~~~~~~

Controllers such as those created through e.g. |CreateEthernetController|, do not have to be removed by a user manually.
These controllers are removed during simulation shutdown through SIL Kit itself.

Participant and ParticipantConfiguration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Participants and ParticipantConfigurations are created by users directly.
They are provided through smart pointers. 
As soon as these pointers are not referenced any more, the corresponding entities will be cleaned up automatically by SIL Kit.

For the C API Participants have to be cleaned up by the user by calling ``SilKit_Participant_Destroy``.
ParticipantConfigurations are not modeled separately in the C API and therefore do not have to be considered in memory management.

.. _sec:memory-man-span:

SIL Kit Spans vs Vectors
~~~~~~~~~~~~~~~~~~~~~~~~

SIL Kit provides payloads received through SIL Kit through a ``SilKit::Util::Span`` instead of a regular vector.
This allows SIL Kit to provide access to these payloads without introducing an additional copy of these payloads.
Through avoiding these copies, SIL Kit's performance is optimized.

To obtain a real copy of these payloads ``SilKit::Util::ToStdVector`` can be used.

SIL Kit requires these ``SilKit::Util::Span`` as well for sending payloads, e.g. in the |CanFrame| that is provided to |SendFrame|.
For converting byte vectors to these spans, ``SilKit::Util::ToSpan`` or the ``SilKit::Util::Span`` constructor can be used.


Examples
~~~~~~~~

Within this section examples of how and how not to use the SIL Kit API regarding memory management are shown.

.. literalinclude::
   ./examples/memory/Memory_Dos_Donts.cpp
   :language: cpp
   :lines: 24-44


