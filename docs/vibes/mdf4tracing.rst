.. _mdf4tracing:

================
!!! VIBE MDF4Tracing
================

.. contents:: :local:
   :depth: 1

The VIBE MDF4Tracing extension is a loadable shared library that provides an
implementation for message trace sinks of type "Mdf4File".
Currently, this trace message sink provides support to trace CAN, FlexRay,
LIN and Ethernet messages in an MDF4 format.
The generated MDF4 file's internal structure conforms to the ASAM MDF BusLogging
specifications.

Generic message types (I/O Ports, Generic Messages) are encoded in MDF4 channels
that are suitable for use with Vector CANoe.
This allows working with trace data as "SystemVariables" in CANoe.

!!! Replay
------
This VIBE also implements the message replaying APIs necessary for injecting MDF4 data into live simulations.
Refer to the :doc:`/usage/replay` documentation for details.
Please note, that for some replaying modes limitations apply as noted in :ref:`sec:replay`.

!!! Using the Shared library
------------------------
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

.. _sec:vibe-mdfinfo:

!!! The vibe-mdfinfo Utility
-------------------------
The VIBE-MDF4Tracing distribution package contains a small command line utility
for debugging and analysis of trace files, called *vibe-mdfinfo*.
This tool allows to verify that a trace file contains valid IntegrationBus
messages.

.. code-block:: sh

    # display the recognized and supported data channels of a file
    vibe-mdfinfo traceFile.mf4
    File: traceFile.mf4, version=410,
    programIdentifier=MDF4Lib, sorted=1
    VIB: format=1 ibConfig.length=5828

    Channel:
       Name=FLX_Frame/FlexRay0/FlexRay-Cluster-1/NetworkSimulator/FlexRay1
       SI.Name=FlexRay-Cluster-1/NetworkSimulator/FlexRay1   SI.Path=FlexRay0
	   CG.AcquisitionName=FlexRay0   CG.Path=FLX_Frame   CG.Name=
	   NumMessages=1480   RecordSize=40   StartTimeNs=23516921 EndTimeNs=373943214


    # dump the messages in VIB print format:
    vibe-mdfinfo traceFile.mf4 --dump-messages
	File: traceFile.mf4, version=410, programIdentifier=MDF4Lib, sorted=1
	VIB: format=1 ibConfig.length=5828

	Channel:
	   Name=FLX_Frame/FlexRay0/FlexRay-Cluster-1/NetworkSimulator/FlexRay1
		   SI.Name=FlexRay-Cluster-1/NetworkSimulator/FlexRay1   SI.Path=FlexRay0
	   CG.AcquisitionName=FlexRay0   CG.Path=FLX_Frame   CG.Name=
	   NumMessages=1480   RecordSize=40   StartTimeNs=23516921   EndTimeNs=373943214
		23ms: TX: 3,2: fr::FrMessage{ch=A, fr::Header{f=[UYN-],s=11,l=16,crc=5,c=0} @23.5169ms}
		23ms: TX: 3,2: fr::FrMessage{ch=B, fr::Header{f=[UYN-],s=11,l=16,crc=5,c=0} @23.5169ms}
		[...]
