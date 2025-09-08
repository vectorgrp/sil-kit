.. include:: /substitutions.rst

================
Troubleshooting
================

This section provides in-depth troubleshooting information about common problems of |ProductName| simulations.
Most issues can be identified based on log messages by the application.

.. admonition:: Note

   To troubleshoot any problem involving the |ProductName|, activate the built-in :ref:`logging<sec:cfg-participant-logging>` on all involved applications.
   It is recommended to write the participant log to a file with the most detailed log level ``Trace``:
   
   .. code-block:: yaml
   
      Logging:
        Sinks:
        - Type: File
          Level: Trace
          LogName: ParticipantLog

..
    .. contents::
       :local:
       :depth: 2

.. toctree::
    :maxdepth: 2

    connectivity.rst
    lifecycle.rst
    interoperability.rst
    connection-guides.rst
    performance.rst
    advanced.rst

..
    .. include:: connectivity.rst
    .. include:: lifecycle.rst
    .. include:: interoperability.rst
    .. include:: connection-guides.rst
    .. include:: performance.rst
    .. include:: advanced.rst
