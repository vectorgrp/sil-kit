#######################################################################################################################
# IntegrationBus Launcher Plugin
# Copyright (c)  Vector Informatik GmbH. All rights reserved.
#######################################################################################################################

class Environment:
    """Interface of an execution environment module"""
    #######################################################################################################################
    #def __init__(self: object, participantEnvironments: list, networkNodes: list, processCoordinator: object, domainId: int, verbose: bool):
    def __init__(self, participantEnvironments, networkNodes, processCoordinator, domainId, verbose):
        """Constructor call: Actions to be performed at loading time of this environment module
            Parameters
            ----------
            participantEnvironments: list
                The JSON sections from "#/properties/LaunchConfigurations/ParticipantEnvironments" relevant for this environment
            networkNodes: list
                The JSON sections from "#/properties/LaunchConfigurations/NetworkNodes"
            processCoordinator: object
                Instance of the ProcessCoordinator used to spawn processes
            domainId: int
                The domain ID to be used by the underlying IntegrationBus middleware for this simulation run
            verbose: bool
                Should logging be more verbose?
        """
        raise NotImplementedError("Must be implemented")

    #######################################################################################################################
    @staticmethod
    #def getEnvironmentName() -> str:
    def getEnvironmentName():
        """Return provided environment
            Returns
            -------
            str
                The environment name this module provides
        """
        raise NotImplementedError("Must be implemented")

    #######################################################################################################################
    #def setupEnvironment(self: object) -> bool:
    def setupEnvironment(self):
        """Preparation actions before any launchParticipant call, invoked once for this type of environment
            Returns
            -------
            bool
                True iff the environment could be setup
        """
        raise NotImplementedError("Must be implemented")

    #######################################################################################################################
    #def teardownEnvironment(self: object) -> bool:
    def teardownEnvironment(self):
        """Teardown actions after simulation was halted, invoked once for this type of environment
            Returns
            -------
            bool
                True iff actions to prepare the environment could be reversed
        """
        raise NotImplementedError("Must be implemented")

    #######################################################################################################################
    #def launchParticipant(self: object, participantName: str) -> object:
    def launchParticipant(self, participantName):
        """Launch participant and return the process or None
            Parameters
            ----------
            participantName: str
                Name of the participant to load, relevant JSON section is participantEnvironments[participant]
            Returns
            -------
            object
                The process instance returned from the subprocess.Popen call, or None when something went wrong
        """
        raise NotImplementedError("Must be implemented")
