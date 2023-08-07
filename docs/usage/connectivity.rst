Connectivity
============

.. contents::
   :local:
   :depth: 1

SIL Kit provides interoperability between participants and utilities from different released versions of SIL Kit.
This section documents known issues and potentially misleading messages reported by SIL Kit participants in certain cases.

Connectivity Issues with Registry
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: console
  :caption: Example error message

  [yyyy-mm-dd hh:mm:ss] [CanWriter] [error] Failed to connect to SIL Kit Registry (number of attempts: 1)
  [yyyy-mm-dd hh:mm:ss] [CanWriter] [info]    Make sure that the SIL Kit Registry is up and running and is listening on the following URIs: silkit://localhost:8500, local://<PATH>.silkit.
  [yyyy-mm-dd hh:mm:ss] [CanWriter] [info]    If a registry is unable to open a listening socket it will only be reachable via local domain sockets, which depend on the working directory and the middleware configuration ('enableDomainSockets').
  [yyyy-mm-dd hh:mm:ss] [CanWriter] [info]    Make sure that the hostname can be resolved and is reachable.
  [yyyy-mm-dd hh:mm:ss] [CanWriter] [info]    You can configure the SIL Kit Registry hostname and port via the SilKitConfig.
  [yyyy-mm-dd hh:mm:ss] [CanWriter] [info]    The SIL Kit Registry executable can be found in your SIL Kit installation folder:
  [yyyy-mm-dd hh:mm:ss] [CanWriter] [info]      INSTALL_DIR/bin/sil-kit-registry[.exe]
  Something went wrong: ERROR: Failed to connect to SIL Kit Registry

This error message is based on the fact, that your SIL Kit simulation participant was not able to connect to the :ref:`sec:util-registry` with the specified registry URI.
A SIL Kit simulation needs a running :ref:`sec:util-registry` that will tell new simulation participants where to find other participants.
For connecting to a simulation, a SIL Kit participant will use the registry URI specified in its 
:cpp:func:`CreateParticipant()<SilKit::CreateParticipant()>` call or if specified, the ``RegistryUri`` provided in the Participant Configuration file.
To find out where your participant is trying to connect to, refer to the printed error message. In this case, it is looked up by hostname `localhost` and TCP port `8500`, with a value of `silkit://localhost:8500` in the log output.

The registry not being reachable for a participant might have two reasons: There is no registry running under the specified URI, 
or the registry is not reachable for this specific participant because of networking issues.

.. admonition:: Note

    By default, a SIL Kit registry will bind to ``silkit://localhost:8500``. If you want to reach a registry from somewhere where this 'localhost' is not reachable (VM, WSL, Docker),
    you have to bind the SIL Kit registry to a publicly reachable URI, such as ``silkit://0.0.0.0:8501``. Make sure that the chosen port is not conflicting to the one of another SIL Kit registry instance.
    Please note that this opens a public listening port and may not be suitable for your work environment.
    Consult your system administrator when in doubt.

When starting a registry, you can provide a `--listen-uri` under which the registry will be reachable.
Make sure that a :ref:`sec:util-registry` is running and that it binds to the address and port specified for your connecting participant.

Often, issues within your network will lead to this error message. To verify, that you are not having networking issues, you can verify that 
your host running the SIL Kit participant is able to establish a TCP connection to the registry. One way of doing so, is to use the Telnet utility:

.. code-block:: powershell

   # Use your IP (here 192.168.1.12) and port (here 8500) of the registry
   telnet 192.168.1.12 8500

If Telnet does not print that it established a connection, you are having networking issues.

.. code-block:: powershell
   
   Trying <IP of registry>...
   Connected to <IP of registry>.
   Escape character is '^]'.

In this case your network configuration prevents a connection to the registry.
Refer to your network administrator for further assistance.
Some potential issues might be firewalls, VM network configuration (if using a VM), NATs, Docker network configuration (if using Docker).

.. admonition:: Note
    
    Telnet can be terminated with the key combination Ctrl+C.

Using the Registry as a Proxy for Participant-to-Participant Connections
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: powershell
   :caption: Example warning

   [yyyy-mm-dd hh:mm:ss.sss] [EthernetReader] [info] Connected to registry at 'tcp://127.0.0.1:8501' via 'tcp://127.0.0.1:63107' (silkit://127.0.0.1:8501)
   [yyyy-mm-dd hh:mm:ss.sss] [EthernetReader] [warning] VAsioConnection: Failed to connect directly to EthernetWriter, trying to proxy messages through the registry: Failed to connect to host URIs: "tcp://127.0.0.1:37117,"

When a SIL Kit participant joins a simulation, it first connects to the SIL Kit registry (:ref:`sec:util-registry`), which informs the participant about all other participants, that have already joined the simulation.
The participant then tries to connect to all the other participants.

If another participant cannot be connected to directly, as a last resort, it tries to use the registry as a proxy for participant-to-participant communication.
The warning is issued, since sending the participant-to-participant messages via the registry will add latency and overhead.

A SIL Kit participant will only attempt to use the registry as a proxy if the remote participant, that could not be connected to directly, supports receiving messages via the registry.

Connectivity issues between participants
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: powershell
   :caption: Example error message

   [yyyy-mm-dd hh:mm:ss] [CanReader] [info] Connected to registry at 'tcp://192.168.1.12:8500' via 'tcp://192.168.1.12:8500' (silkit://192.168.1.12:8500)
   [yyyy-mm-dd hh:mm:ss] [CanReader] [error] Timeout during connection setup. The participant was able to connect to the registry, but not to all participants.
      There might be network issues. Check network settings and firewall configuration. Was not able to connect to the following participant(s): 
      CanWriter(local://D:\TEMP\\CanWriterd81c9d2684952765.silkit, tcp://192.168.1.12:52125)
   
When a SIL Kit participant joins a simulation, it first connects to the SIL Kit registry (:ref:`sec:util-registry`), to be informed of the other simulation participants and how they can be reached.
This error message is shown if this second step of a simulation setup fails. It occurs when the peer to peer connection between the observed participant and one or more other participants was not possible.
Usually, this problem occurs when a network configuration does not allow for said connections. Check your network setup and firewall configuration.
