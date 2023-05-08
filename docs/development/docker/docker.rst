:orphan:

======
Docker
======

Development snippets and experiments with docker.

Linux
-----

Setup tested on WSL2:

* Registry in container 'registry'
* CanReader in container 'canreader'
* CanWriter on host

See samples/docker-compose-linux.yaml:

.. literalinclude::
   samples/docker-compose-linux.yaml

#. Ensure that the SIL Kit library, registry and demo+yaml are in `./silkitfiles-linux` next to the docker-compose file.
#. Launch the containers::

	docker-compose -f docker-compose-linux.yaml up
#. Launch the CanWriter on the host::

	cd silkitfiles-linux
	./SilKitDemoCan SilKitConfig_DemoCan.yaml CanWriter silkit://localhost:8500 --async 

Issues:

* Launching the CanWriter on Windows fails. It will connect to the registry, but the connection between the 
  participants cannot be established.

