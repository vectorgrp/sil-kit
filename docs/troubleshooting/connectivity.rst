.. include:: /substitutions.rst

Connectivity
============

Connectivity Issues with the |ProductName| Registry
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A |ProductName| simulation needs a running :ref:`sec:util-registry` to establish the connection between the participants.
All participants and the registry must use a common URI to establish the connection.
For the participants, this is called the `connect URI`, for the registry process, this is called the `listen URI`. 
The participant logs the following messages if it can't connect to the registry:

.. code-block:: console

  [yyyy-mm-dd hh:mm:ss] [CanWriter] [error] Failed to connect to SIL Kit Registry (number of attempts: 1)
  [yyyy-mm-dd hh:mm:ss] [CanWriter] [info]    Make sure that the SIL Kit Registry is up and running and is listening on the following URIs: silkit://localhost:8500, local://<PATH>.silkit.
  [yyyy-mm-dd hh:mm:ss] [CanWriter] [info]    If a registry is unable to open a listening socket it will only be reachable via local domain sockets, which depend on the working directory and the middleware configuration ('enableDomainSockets').
  [yyyy-mm-dd hh:mm:ss] [CanWriter] [info]    Make sure that the hostname can be resolved and is reachable.
  [yyyy-mm-dd hh:mm:ss] [CanWriter] [info]    You can configure the SIL Kit Registry hostname and port via the SilKitConfig.
  [yyyy-mm-dd hh:mm:ss] [CanWriter] [info]    The SIL Kit Registry executable can be found in your SIL Kit installation folder:
  [yyyy-mm-dd hh:mm:ss] [CanWriter] [info]      INSTALL_DIR/bin/sil-kit-registry[.exe]
  Something went wrong: ERROR: Failed to connect to SIL Kit Registry

Step-by-step Solution
---------------------

#. Find out the connect URI the participant is using
   This can be found in

   * The :cpp:func:`CreateParticipant()<SilKit::CreateParticipant()>` call 
   * The ``RegistryUri`` provided in the Participant Configuration file.
     This takes precedence over the previously mentioned API call.
   * The log message ``Make sure that the SIL Kit Registry is up and running and is listening on the following URIs: <List of URIs>``

#. Double check if the registry process is running and listens on the correct connect URI
   
   a. If you have started the registry yourself, check if the provided ``--listen-uri`` is correct.
      The registry can also use a configuration file which specify the listen URI.
      The configuration file takes precedence over the previously mentioned command line argument.
   b. If you have used the |ProductName| MSI installer, check if the Windows service `VectorSilKitRegistry` is running.

#. Check if ``localhost`` in the connect URI makes sense

   By default, a |ProductName| registry will bind to ``silkit://localhost:8500``. 
   Out of the box, 'localhost' cannot be resolved between virtualized networks (e.g., Windows connecting to VM, WSL, Docker). 
   In this case, you have to bind the |ProductName| registry to a publicly reachable URI, such as ``silkit://0.0.0.0:8501``. 
   Please note that this opens a public listening port and may not be suitable for your work environment.
   Consult your system administrator when in doubt.

#. Check if the connection between the network of the participant and the registry is possible

   Often, issues within your network will lead to this error message. 
   Test if your host running the |ProductName| participant is able to establish a TCP connection to the registry. 
   
   .. code-block:: powershell
   
       # Use your IP (here 172.19.71.101) and port (here 8501) of the registry
       Test-NetConnection -ComputerName 172.19.71.101 -Port 8501
   
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

   [yyyy-mm-dd hh:mm:ss.sss] [EthernetReader] [info] Connected to registry at 'tcp://127.0.0.1:8501' via 'tcp://127.0.0.1:63107' (silkit://127.0.0.1:8501)
   [yyyy-mm-dd hh:mm:ss.sss] [EthernetReader] [warning] VAsioConnection: Failed to connect directly to EthernetWriter, trying to proxy messages through the registry: Failed to connect to host URIs: "tcp://127.0.0.1:37117,"

When a |ProductName| participant joins a simulation, it first connects to the |ProductName| registry (:ref:`sec:util-registry`), which informs the participant about all other participants, that have already joined the simulation.
The participant then tries to connect to all the other participants.

If another participant cannot be connected to directly, as a last resort, it tries to use the registry as a proxy for participant-to-participant communication.
The warning is issued, since sending the participant-to-participant messages via the registry will add latency and overhead.

A |ProductName| participant will only attempt to use the registry as a proxy if the remote participant, that could not be connected to directly, supports receiving messages via the registry.

Connectivity issues between participants
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: powershell

   [yyyy-mm-dd hh:mm:ss] [CanReader] [info] Connected to registry at 'tcp://192.168.1.12:8500' via 'tcp://192.168.1.12:8500' (silkit://192.168.1.12:8500)
   [yyyy-mm-dd hh:mm:ss] [CanReader] [error] Timeout during connection setup. The participant was able to connect to the registry, but not to all participants.
      There might be network issues. Check network settings and firewall configuration. Was not able to connect to the following participant(s): 
      CanWriter(local://D:\TEMP\\CanWriterd81c9d2684952765.silkit, tcp://192.168.1.12:52125)
   
When a |ProductName| participant joins a simulation, it first connects to the |ProductName| registry (:ref:`sec:util-registry`), to be informed of the other simulation participants and how they can be reached.
This error message is shown if this second step of a simulation setup fails. It occurs when the peer to peer connection between the observed participant and one or more other participants was not possible.
Usually, this problem occurs when a network configuration does not allow for said connections. Check your network setup and firewall configuration.
