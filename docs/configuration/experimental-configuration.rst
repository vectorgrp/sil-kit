.. _sec:cfg-participant-experimental:

===================================================
Experimental Configuration
===================================================

.. |ProductName| replace:: SIL Kit

.. contents:: :local:
   :depth: 3

Overview
--------------------

This section includes experimental configuration fields.

.. warning::
  The featues available through the experimental section might be changed or removed in future versions of the |ProductName|.

TimeSynchronization
--------------------

.. code-block:: yaml

    Experimental:
        TimeSynchronization:
            AnimationFactor: 1.0

.. list-table:: TimeSynchronization Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description

   * - AnimationFactor
     - This value affects |ProductName| simulations where a time synchronization service is used. 
       When this value is set, the virtual time is coulped to the local wall clock of the underlying system, scaled down by the given factor.
       E.g., a value of 1.0 means direct coupling to the wall clock. 
       A value of 2.0 means that the virtual time runs **twice as slow** as the local wall clock. 
       Accordingly, a value of 0.5 will cause the virtual time to run **twice as fast**. 
       When omitting the value or setting it to zero, no coupling to the wall clock takes place.

       .. note::
         Depending on the simulation step size, the choosen animation factor and the computational load per simulation step, it might not be possible to couple the virtual time to the wall clock with the desired factor.
