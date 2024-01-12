System Monitor C API
---------------------

.. contents::
   :local:
   :depth: 3

The features of the System Monitor in the C++ API are not provided through a ``SystemMonitor`` abstraction 
in the C API but directly provided through the ``SilKit_Participant``.

.. doxygenfunction:: SilKit_SystemMonitor_Create
.. doxygenfunction:: SilKit_SystemMonitor_AddSystemStateHandler
.. doxygenfunction:: SilKit_SystemMonitor_AddParticipantStatusHandler
.. doxygenfunction:: SilKit_SystemMonitor_RemoveSystemStateHandler
.. doxygenfunction:: SilKit_SystemMonitor_RemoveParticipantStatusHandler
.. doxygenfunction:: SilKit_SystemMonitor_GetParticipantStatus
.. doxygenfunction:: SilKit_SystemMonitor_GetSystemState