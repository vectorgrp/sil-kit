#######################################################################################################################
# IntegrationBus Launcher Plugin
# Copyright (c) Vector Informatik GmbH. All rights reserved.
#######################################################################################################################
import sys
import os
import json
import time
import subprocess

sys.path.append("..")  # Satisfy importlib.import_module
from iblauncher import Environment, Configuration, ProcessCoordinator

class EnvironmentIbRegistry(Environment.Environment):

    #_participantEnvironments: list
    #_networkNodes: list
    #_processCoordinator: object
    #_domainId: int
    #_verbose: bool

    #######################################################################################################################
    #def __init__(self: object, participantEnvironments: list, networkNodes: list, domainId: int, verbose: bool):
    def __init__(self, participantEnvironments, networkNodes, processCoordinator, domainId, verbose):
        """Actions to be performed at loading time of this plugin"""
        self._participantEnvironments = participantEnvironments
        self._networkNodes = networkNodes
        self._processCoordinator = processCoordinator
        self._domainId = domainId
        self._verbose = verbose
        self._verbose = True;

    #######################################################################################################################
    @classmethod
    #def getEnvironmentName() -> str:
    def getEnvironmentName(cls):
        """Return provided environment"""
        return cls.__name__[len("Environment"):]

    #######################################################################################################################
    #def setupEnvironment(self: object) -> bool:
    def setupEnvironment(self):
        """Preparation actions before any launchParticipant call, invoked once for this type of environment"""
        if os.name != "nt" and os.name != "posix":
            print("Error: Platform '" + os.name + "' not supported, only Windows and Posix systems.")
            return False

        # EMS try to find a IbRegistry
        ibRegistryEnvironments = [env for env in self._participantEnvironments if env["Environment"] == self.getEnvironmentName()]
        if len(ibRegistryEnvironments) != 1:
            if self._verbose:
                print("Error: There must be exactly one IbRegistry environment")
            return False

        myEnvironment = ibRegistryEnvironments[0]

        participantName = myEnvironment["Participant"]
        environmentName = myEnvironment["Environment"]
        networkNodeName = None
        networkNode = Configuration.getNetworkNode(networkNodeName, self._networkNodes, self._verbose)
        executable = "%INTEGRATIONBUS_BINPATH%IbRegistry"
        arguments = "%INTEGRATIONBUS_CONFIGFILE% %INTEGRATIONBUS_DOMAINID%"
        workingFolderPath = '.'
        configFilePath = myEnvironment["ConfigFile"]
        configFileAbsolutePath = os.path.abspath(configFilePath)
        configFileFolderPath = os.path.dirname(configFilePath) if os.path.dirname(configFilePath) else "."

        assert(environmentName == self.getEnvironmentName())
        assert(executable != None)
        assert(arguments != None)
        assert(configFilePath)

        # Resolve predefined and environment variables (the latter because we do *not* want to "subprocess.Popen(..., shell=True)")
        executable = Configuration.resolveVariables(executable, configFileAbsolutePath, participantName, self._domainId)
        executableAbsolutePath = executable if os.path.isabs(executable) else os.path.abspath(configFileFolderPath + os.path.sep + executable)
        if not os.path.isfile(executableAbsolutePath):
            if os.name == "nt" and not executableAbsolutePath.endswith(".exe") and os.path.isfile(executableAbsolutePath + ".exe"):
                executableAbsolutePath += ".exe"

        arguments = Configuration.resolveVariables(arguments, configFileAbsolutePath, participantName, self._domainId)

        workingFolderPath = Configuration.resolveVariables(workingFolderPath, configFileAbsolutePath, participantName, self._domainId)
        workingFolderAbsolutePath = workingFolderPath if os.path.isabs(workingFolderPath) else os.path.abspath(configFileFolderPath + os.path.sep + workingFolderPath)

        if self._verbose:
            print("  Participant: " + participantName)
            print("  NetworkNode: " + (networkNodeName if networkNode else "None"))
            print("  Executable: " + executable)
            print("  Arguments: " + arguments)
            print("  WorkingFolder: " + workingFolderPath)

        self._configFileAbsolutePath = configFileAbsolutePath
        self._executableAbsolutePath = executableAbsolutePath
        self._arguments = arguments
        self._workingFolderAbsolutePath = workingFolderAbsolutePath

        return True

    #######################################################################################################################
    #def teardownEnvironment(self: object) -> bool:
    def teardownEnvironment(self):
        """Teardown actions after simulation was halted, invoked once for this type of environment"""
        # Nothing to do here
        return True

    #######################################################################################################################
    #def launchParticipant(self: object, participantName: str) -> object:
    def launchParticipant(self, participantName):
        """Launch participant"""

        integrationBusEnvironment = Configuration.createIntegrationBusEnvironment(self._configFileAbsolutePath, participantName, self._domainId)
        
        process = self._processCoordinator.launchProcess(participantName, self._executableAbsolutePath, self._arguments, self._workingFolderAbsolutePath, integrationBusEnvironment, True)

        return process
