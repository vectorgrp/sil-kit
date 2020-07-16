.. _mdf4tracing:

================
VIBE MDF4Tracing
================

The VIBE MDF4Tracing extension is a loadable shared library that provides an
implementation for message trace sinks of type "Mdf4File".
Currently, this trace message sink provides support to trace CAN, FlexRay,
LIN and Ethernet messages in an MDF4 format.
The generated MDF4 file's internal structure conforms to the ASAM MDF BusLogging
specifications.

Usage
-----
The vibe-mdf4tracing shared library must be deployed into a directory accessible to 
the VIB application at runtime.
The extension lookup paths can be adjusted using the
:ref:`SearchPathHints<sec:cfg-extension-configuration-overview>` configuration.

As discussed in :ref:`sec:cfg-participant-tracing`, trace sinks have to be
defined and attached to controllers.
If the trace sink type is set to "Mdf4File", the IntegrationBus will attempt
to load the *vibe-mdf4tracing* shared library based on the search path hints
defined in the Extension configuration, and the hard-coded *vibe-mdf4tracing*
file name.
If IntegrationBus fails to load the extension library, a runtime exception is
generated.
