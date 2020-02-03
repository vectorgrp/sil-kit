VIB Simulation
================================

A VIB simulation can be controlled at two levels of granularity:

On a larger scale, the simulation is controlled by a distributed state machine.
A tester can use the state machine to initialize participants, start the actual
simulation, and perform orchestrated stop and shutdown.

.. toctree::
   :maxdepth: 1

   statehandling

Furthermore, several options are available to synchronize
the progress of the simulation at runtime. For example, at which rate the simulated
time advances and how the individual participants synchronize the simulation time.

.. toctree::
   :maxdepth: 1

   synchronization
