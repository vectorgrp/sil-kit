.. include:: /substitutions.rst
.. include:: ./abstracts.rst

=====
Tools
=====

Tools for performance analysis.

.. contents::
    :depth: 1
    :local:

.. _sec:benchmark-demo:

Benchmark Demo
~~~~~~~~~~~~~~

Abstract
    |DemoAbstractBenchmark|
Sources
    * :repo-link:`BenchmarkDemo.cpp <Demos/tools/Benchmark/BenchmarkDemo.cpp>`
Requirements
    None (The demo starts its own instance of the registry and system controller).
Positional Parameters
    * ``[numberOfSimulationRuns]``
      Sets the number of simulation runs to perform.
    * ``[simulationDuration]``
      Sets the virtual simulation duration <S>.
    * ``[simulationStepSize]``
      Sets the virtual simulation step size <T>.
    * ``[service]``
      Sets the communication service type (pubsub, can or ethernet).
    * ``[numberOfParticipants]``
      Sets the number of simulation participants <N>.
    * ``[messageCount]``
      Sets the number of messages <M> to be send in each simulation step.
    * ``[messageSizeInBytes]``
      Sets the message size <B>.
    * ``[registryURi]`` 
      The URI of the registry to start.
Optional Parameters
    * ``--help``
      Show the help message.
    * ``--registry-uri``
      The URI of the registry to start. Default: silkit://localhost:8500
    * ``--message-size``
      Sets the message size <B> in bytes. Default: 1000
    * ``--message-count``
      Sets the number of messages <M> to be send in each simulation step. Default: 50
    * ``--number-participants``
      Sets the number of simulation participants <N>. Default: 2
    * ``--number-simulation-runs``
      Sets the number of simulation runs to perform. Default: 4
    * ``--simulation-step-size``
      Sets the simulation step size <T> (virtual time). Default: 1ms
    * ``--simulation-duration``
      Sets the simulation duration <S> (virtual time). Default: 1s
    * ``--service``
      Sets the communication service type (pubsub, can or ethernet). Default: pubsub
    * ``--configuration``
      Path and filename of the participant configuration YAML file. Default: empty
    * ``--write-csv``
      Path and filename of CSV file with benchmark results. Default: empty
System Examples
    * Launch the benchmark demo with default arguments but 3 participants:

      .. parsed-literal::
      
         |DemoDir|/SilKitDemoBenchmark --number-participants 3
    * Launch the benchmark demo with positional arguments and a configuration file that enforces TCP communication:

      .. parsed-literal:: 

         |DemoDir|/SilKitDemoBenchmark 4 1 1 ethernet 2 1 10 --configuration ./SilKit-Demos/Benchmark/DemoBenchmarkDomainSocketsOff.silkit.yaml 
Notes
    * This benchmark demo produces timings of a configurable simulation setup.
      <N> participants exchange <M> messages of <B> bytes per simulation step with a simulation step size of <T> ms and run for <S> seconds (virtual time).
    * This simulation run is repeated <K> times and averages over all runs are calculated. 
      Results for average runtime, speedup (virtual time/runtime), throughput (data size/runtime), message rate (count/runtime) including the standard deviation are printed.
    * The demo uses publish/subscribe, can or ethernet controllers. In the publish/subscribe case, the same topic for the message exchange is used, so each participant broadcasts the messages to all other participants. 
      The configuration file ``DemoBenchmarkDomainSocketsOff.silkit.yaml`` can be used to disable domain socket usage for more realistic timings of TCP/IP traffic. With ``DemoBenchmarkTCPNagleOff.silkit.yaml``, Nagle's algorithm and domain sockets are switched off.
    * The demo can be wrapped in helper scripts to run parameter scans, e.g., for performance analysis regarding different message sizes. 
      See ``.\SilKit-Demos\Benchmark\msg-size-scaling\Readme.md`` and ``.\SilKit-Demos\Benchmark\performance-diff\Readme.md`` for further information.
         

.. _sec:latency-demo:

Latency Demo
~~~~~~~~~~~~

Abstract
    |DemoAbstractLatency|
Sources
    * :repo-link:`LatencyDemo.cpp <Demos/tools/Benchmark/LatencyDemo.cpp>`
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
Positional Parameters
    * ``[messageCount]``
      Sets the number of messages to be send in each simulation step.
    * ``[messageSizeInBytes]``
      Sets the message size.
    * ``[registryURi]`` 
      The URI of the registry to start.
Optional Parameters
    * ``--help``
      Show the help message.
    * ``--isReceiver``
      This process is the receiving counterpart of the latency measurement. Default: false
    * ``--registry-uri``
      The URI of the registry to start. Default: silkit://localhost:8500
    * ``--message-size``
      Sets the message size. Default: 1000
    * ``--message-count``
      Sets the number of messages to be send in each simulation step. Default: 1000
    * ``--configuration``
      Path and filename of the participant configuration YAML file. Default: empty
    * ``--write-csv``
      Path and filename of csv file with benchmark results. Default: empty
System Examples
    * Launch the two LatencyDemo instances with positional arguments in separate terminals:

      .. parsed-literal:: 

         |DemoDir|/SilKitDemoLatency 100 1000
         |DemoDir|/SilKitDemoLatency 100 1000 --isReceiver

    * Launch the LatencyDemo instances with a configuration file that enforces TCP communication:

      .. parsed-literal:: 

         |DemoDir|/SilKitDemoLatency 100 1000 --configuration ./SilKit-Demos/Benchmark/DemoBenchmarkDomainSocketsOff.silkit.yaml
         |DemoDir|/SilKitDemoLatency 100 1000 --isReceiver
Notes
    * This latency demo produces timings of a configurable simulation setup. 
      Two participants exchange <M> messages of <B> bytes without time synchronization.
    * The demo uses publish/subscribe controllers performing a message round trip (ping-pong) to calculate latency and throughput timings.
    * Note that the two participants must use the same parameters for valid measurement and one participant must use the ``--isReceiver`` flag.

