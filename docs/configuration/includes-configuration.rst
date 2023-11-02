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
The path to the included configuration file must be given relative to the including participant configuration file.
The ``Includes`` section is only allowed when importing configurations via |ParticipantConfigurationFromFile|, not via |ParticipantConfigurationFromString|.
The included configurations are merged according to the following rules:

* Properties in the *including* configuration have priority over *included* properties. This means that properties in the *root* configuration always have priority:

  .. code-block:: yaml

     # root.yaml 
     Includes:
        - included.yaml
     Middleware:
        RegistryUri: silkit://localhost:8500 # Has priority!
  
  .. code-block:: yaml

     # included.yaml 
     Middleware:
        RegistryUri: silkit://0.0.0.0:8501 # Already specified in root.yaml, ignored!

* In case of multiple included files, earlier included files have priority:

  .. code-block:: yaml

     # root.yaml 
     Includes:
        - included_1.yaml
        - included_2.yaml
  
  .. code-block:: yaml

     # included_1.yaml 
     Middleware:
        RegistryUri: silkit://localhost:8500 # Has priority!

  .. code-block:: yaml

     # included_2.yaml 
     Middleware:
        RegistryUri: silkit://0.0.0.0:8501 # Already specified in included_1.yaml, ignored!

* Multiple inclusions of the same file are automatically prevented.
  This also applies for nested includes of the same file:
  
  .. code-block:: yaml

     # root.yaml 
     Includes:
        - included_1.yaml
        - included_2.yaml
  
  .. code-block:: yaml

     # included_1.yaml 
     Includes:
        - included_2.yaml # Ignored (already appeared in root.yaml)

* List items of top-level properties (e.g. ``CanControllers``, ``DataPublishers``) are combined:

  .. code-block:: yaml

     # root.yaml 
     Includes:
        - included.yaml
     DataPublishers:
     - Name: DataPublisher1 # Will be used
       Topic: SomeTopic
  
  .. code-block:: yaml

     # included.yaml 
     DataPublishers:
     - Name: DataPublisher2 # Will also be used
       Topic: SomeTopic

* *Named* items with the same name cannot be merged in a meaningful way and result in a ``SilKit::ConfigurationError``:

  .. code-block:: yaml

     # root.yaml 
     Includes:
        - included.yaml
     CanControllers:
     - Name: CAN1 # Name "CAN1" set here
       Network: CAN1
  
  .. code-block:: yaml

     # included.yaml 
     CanControllers:
     - Name: CAN1  # SilKit::ConfigurationError: Conflicting name "CAN1"
       Network: CAN2

* The list items of ``Sinks`` in the ``Logging`` section are merged as follows:

  * Only a single sink of ``Type: Remote`` and ``Type: Stdout`` is valid.
  * Sinks of ``Type: File`` are combined. 
    However their ``LogName`` must be unique, otherwise a ``SilKit::ConfigurationError`` occurs. 

* List items of the ``SearchPathHints`` in the ``Extensions`` section are merged and all entries are retained.
  Possible duplicates here are uncritical.

* ``AcceptorUris`` of the ``Middleware`` section cannot be combined. 
  Here, a ``SilKit::ConfigurationError`` occurs if ``AcceptorUris`` are defined multiple times.

An important use-case here is to include a configuration with a dynamically generated ``RegistryUri`` of the ``Middleware`` section:
In a CI environment, it is unfavorable to setup a static port in the ``listen-uri`` of the  :ref:`SIL Kit registry<sec:util-registry>`.
Instead, a port ``0`` advises the |ProductName| registry to let the operating system choose a random free port.
This URI then has to be used by the participants in the Middleware property ``RegistryUri``.
For this purpose, the ``--generate-configuration`` CLI parameter of the |ProductName| registry creates a participant configuration file containing the dynamic ``RegistryUri`` in the Middleware section.
By referencing this generated participant configuration in the ``Includes`` section of the participant configuration, the static part of the configuration (e.g., network names, logging) can be combined with the dynamic ``RegistryUri``.


Configuration
=============

.. code-block:: yaml

    Includes:
      - generated-uri.yaml
      - ../common-logging.yaml

.. list-table:: Includes Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - <relative/path/to/included.yaml>
     - Path to the participant configuration file to be included relative to including participant configuration file.

