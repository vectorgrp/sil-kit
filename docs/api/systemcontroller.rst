=================
System Controller
=================

.. Macros for docs use
.. |CreateSystemController| replace:: :cpp:func:`CreateSystemController()<SilKit::Experimental::Participant::CreateSystemController(SilKit::IParticipant* participant) -> SilKit::Experimental::Services::Orchestration::ISystemController*>`
.. |SetWorkflowConfiguration| replace:: :cpp:func:`SetWorkflowConfiguration()<SilKit::Experimental::Services::Orchestration::ISystemController::SetWorkflowConfiguration()>`
.. |WorkflowConfiguration| replace:: :cpp:class:`WorkflowConfiguration<SilKit::Services::Orchestration::WorkflowConfiguration>`
.. |AbortSimulation| replace::  :cpp:func:`AbortSimulation()<SilKit::Experimental::Services::Orchestration::ISystemController::AbortSimulation()>`

.. contents::
   :local:
   :depth: 3

.. highlight:: cpp

.. warning::
  The System Controller is experimental and might be changed or removed in future versions of the SIL Kit.

.. admonition:: Note

  The SIL Kit provides a utility called :ref:`sil-kit-system-controller<sec:util-system-controller>`, that provides a 
  command line interface to define the required participant names of a simulation. In most cases this utility can be 
  used and no own implementation is needed.

Create a System Controller
--------------------------

The System Controller needs a valid participant instance to be created via |CreateSystemController|.

.. code-block:: cpp

  auto participant = SilKit::CreateParticipant(configuration, participantName, connectUri);
  systemController = SilKit::Experimental::Participant::CreateSystemController(participant);

Set the required participants
-----------------------------

In the SIL Kit, required participants are a set of user defined participants that are needed for the current 
simulation setup to function correctly. These participants are used to calculate the system state 
(see :ref:`System States<subsec:sim-lifecycle>`), which is used by coordinated participants 
(see :ref:`Configuring Lifecycle<subsec:sim-configuring-lifecycle>`) for their state transitions.

To define the required participants, |SetWorkflowConfiguration| must be called with a |WorkflowConfiguration| 
containing the set of required participants names within the simulation.

Abort the simulation
--------------------

With |AbortSimulation|, all participants with a lifecycle will be aborted. This means that their lifecycle will 
terminate and call the ``abort handler`` (see :ref:`Using Lifecycle<subsec:sim-using-lifecycle>`).

API and Data Type Reference
--------------------------------------------------

System Controller API
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygenclass:: SilKit::Experimental::Services::Orchestration::ISystemController
    :members:

