
Lifecycle
=========

.. contents::
   :local:
   :depth: 1

SIL Kit provides a lifecycle service and time synchronization service, that allow to coordinate simulation execution between participants.

Coordinated required participants
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: console
  
  This participant is in OperationMode::Coordinated but it is not part of the set of "required" participants declared to the system controller.

If participants of a SIL Kit simulation want to start the simulation together as soon as all necessary participants have joined the simulation, 
these SIL Kit participants specify that their LifecycleService is operated in coordinated mode.

When executing such a SIL Kit Simulation, it is important to tell the SIL Kit simulation which participants are considered necessary 
for the coordinated start. You do this by calling sil-kit-system-controller with a whitespace separated list of 
participants that are necessary to start the simulation. These participants are called "required" in SIL Kit terminology.

For example:

.. code-block:: powershell
    
    # mark participant1, participant2 and participant3 as required
    sil-kit-system-controller participant1 participant2 participant3

Attempt to instantiate a service multiple times
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: powershell

   Tried to instantiate TimeSyncService multiple times
   Tried to instantiate LifecycleService multiple times
   Tried to instantiate SystemMonitor multiple times
   
Some services within SIL Kit can not be instantiated multiple times on the same simulation partcipant. 
If a user tries to instantiate these services a second time, an exception with the error message `Tried to instantiate service multiple times` is thrown.
These semantics mirror the underlying implementation. No implicit caching of the service instances is performed.
Since a participant can only have one lifecycle status, having multiple lifecycle services or the expectation to instantiate multiple lifecycle services are most likely a misunderstanding of the SIL Kit APIs.
If you run into this issue, because you want to access these services from many different points of you code, think about establishing a singleton pattern in your code.

Required participant names are already set
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: powershell

   Required participant names are already set.
   
For starting a coordinated simulation, in which all participants start the simulation at the same time, it is necessary that a participant tells the simulation which participants are required for this start.
Usually the :ref:`sec:util-system-controller` is used to set these required participants, but the WorkflowConfiguration can also be used to set these participants through the API.
Since the behavior of the simulation would be undefined if multiple different sets of required participants are specified, this exception occurs.
Make sure that only one sil-kit-system-controller exists within the simulation or that only one participant sets the WorkflowConfiguration.