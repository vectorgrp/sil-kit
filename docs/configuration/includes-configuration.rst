.. _sec:cfg-participant-includes:

======================
Includes Configuration
======================

.. |ProductName| replace:: SIL Kit

.. |ParticipantConfigurationFromFile| replace:: :cpp:func:`ParticipantConfigurationFromFile()<SilKit::Config::ParticipantConfigurationFromFile()>`
.. |ParticipantConfigurationFromString| replace:: :cpp:func:`ParticipantConfigurationFromString()<SilKit::Config::ParticipantConfigurationFromString()>`

.. contents:: :local:
   :depth: 3

Overview
========

The ``Includes`` section allows to reference other participant configuration files.
This can be used to share common parts of a participant configuration or to include dynamically generated configurations into a static configuration base.

.. code-block:: yaml

   Includes:
     Files:
       - LoggingIncludes.silkit.yaml
       - generated/MiddlewareInclude.silkit.yaml

To deal with different locations of the included files depending on the execution scenario, a list of search paths pointing to the locations of the included files can be specified in the ``SearchPathHints``:

.. code-block:: yaml

   Includes:
     SearchPathHints:
       - ConfigSnippets/Logging     # relative paths are valid (same as ./ConfigSnippets/Logging/) 
       - /urs/etc/sil-kit-configs/  # absolute paths are valid
       - C:\Temp\ConfigSnippets     # for Windows, backslash as path separator is valid
       - C:\Temp\Path with spaces\  # for Windows, paths with spaces are valid
     Files:
       - LoggingIncludes.silkit.yaml
       - generated/MiddlewareInclude.silkit.yaml

When using |ParticipantConfigurationFromFile|, the current working directory and the path of the including participant configuration file are considered as additional search paths.
When using |ParticipantConfigurationFromString|, the default search path is the current working directory.

Merge rules
===========

The included configurations are merged according to the following rules:

* All properties of the ``Middleware`` section can only be defined once, otherwise a ``SilKit::ConfigurationError`` occurs:

  .. code-block:: yaml

     # root.silkit.yaml 
     Includes:
        Files:
           - included.silkit.yaml
     Middleware:
        RegistryUri: silkit://localhost:8500 
  
  .. code-block:: yaml

     # included.silkit.yaml 
     Middleware:
        RegistryUri: silkit://0.0.0.0:8501 # Already specified in root.silkit.yaml, ConfigurationError!

* Multiple inclusions of the same file are automatically prevented.
  This also applies for nested includes of the same file:
  
  .. code-block:: yaml

     # root.silkit.yaml 
     Includes:
        Files:
           - included_1.silkit.yaml
           - included_2.silkit.yaml
  
  .. code-block:: yaml

     # included_1.silkit.yaml 
     Includes:
        Files:
           - included_2.silkit.yaml # Ignored (already appeared in root.silkit.yaml)

* List items of top-level properties (e.g. ``CanControllers``, ``DataPublishers``) are combined:

  .. code-block:: yaml

     # root.silkit.yaml 
     Includes:
        Files:
           - included.silkit.yaml
     DataPublishers:
     - Name: DataPublisher1 # Will be used
       Topic: SomeTopic
  
  .. code-block:: yaml

     # included.silkit.yaml 
     DataPublishers:
     - Name: DataPublisher2 # Will also be used
       Topic: SomeTopic

* *Named* items with the same name cannot be merged in a meaningful way and result in a ``SilKit::ConfigurationError``:

  .. code-block:: yaml

     # root.silkit.yaml 
     Includes:
        Files:
           - included.silkit.yaml
     CanControllers:
     - Name: CAN1 # Name "CAN1" set here
       Network: CAN1
  
  .. code-block:: yaml

     # included.silkit.yaml 
     CanControllers:
     - Name: CAN1  # SilKit::ConfigurationError: Conflicting name "CAN1"
       Network: CAN2

* *Named* items where all properties match (i.e., duplicates of *named* items) are permitted.

* The list items of ``Sinks`` in the ``Logging`` section are merged as follows:

  * Only a single sink of type ``Type: Stdout`` can be defined, otherwise a ``SilKit::ConfigurationError`` occurs.
    The same applies to the sink type ``Type: Remote``.
  * Sinks of ``Type: File`` are combined. 
    However their ``LogName`` must be unique, otherwise a ``SilKit::ConfigurationError`` occurs. 

* List items of the ``SearchPathHints`` in the sections ``Includes`` or ``Extensions`` are merged and all entries are retained.
  Possible duplicates here are uncritical.

* All properties of the ``HealthCheck`` section can only be defined once, otherwise a ``SilKit::ConfigurationError`` occurs.

Dynamic port generation
=======================

An important use-case is to include a configuration with a dynamically generated ``RegistryUri`` of the ``Middleware`` section:
In a CI environment, it is unfavorable to setup a static port in the ``listen-uri`` of the  :ref:`SIL Kit registry<sec:util-registry>`.
Instead, a port ``0`` advises the |ProductName| registry to let the operating system choose a random free port.
This URI then has to be used by the participants in the Middleware property ``RegistryUri``.
For this purpose, the ``--generate-configuration`` CLI parameter of the |ProductName| registry creates a participant configuration file containing the dynamic ``RegistryUri`` in the Middleware section.
By referencing this generated participant configuration in the ``Includes`` section, the static part of the configuration (e.g., network names, logging) can be combined with the dynamic ``RegistryUri``.

Configuration
=============

.. code-block:: yaml

    Includes:
       SearchPathHints:
          - ./ConfigSnippets/Generated/
       Files:
          - generated-uri.silkit.yaml
          - ../common-logging.silkit.yaml

.. list-table:: Includes Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - SearchPathHints
     - A list of paths that are used to search for included configuration files.
   * - Files
     - A list of configuration files to be included.