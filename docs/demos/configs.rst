=================================
Sample participant configurations
=================================

The following sample participant configurations are available:

``FileLog_Trace.silkit.yaml``
	Log to a file with ``Level: Trace``.

``FileLog_Trace_FromRemotes.silkit.yaml``
	Log to file with ``Level: Trace`` from participants that use a Sink of ``Type: Remote``.

``Stdout_Info.silkit.yaml``
	Log to Stdout with ``Level: Info``.

``ShareConfiguration.silkit.yaml``
	Log to Stdout with ``Level: Info`` and share the configuration with the registry
	(see :ref:`sec:cfg-experimental-management`).

``Trace_ToRemote.silkit.yaml``
	Log to Remote with ``Level: Info``. 
	Participants that specify ``LogFromRemotes: True`` will receive the log messages.
