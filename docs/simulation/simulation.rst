VIB Simulation
================================

A VIB simulation can be controlled at two levels of granularity: on a larger
scale, the simulation is controled by a :doc:`distributed state machine<statehandling>`.
A tester can use the state machine to initialize participants, start the actual
simulation, and perform orchestrated stop and shutdown. Furthermore, several
options are available to :doc:`synchronize<synchronization>` the progress of the
simulation at runtime, i.e., at which rate does the simulated time advance and
how do the individual participants synchronize the simulation time.

.. toctree::
   :maxdepth: 1
   
   statehandling
   synchronization

