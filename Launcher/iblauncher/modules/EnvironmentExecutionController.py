#######################################################################################################################
# IntegrationBus Launcher Plugin
# Copyright (c)  Vector Informatik GmbH. All rights reserved.
#######################################################################################################################
import sys
import os
import json
import subprocess

sys.path.append("..")  # Satisfy importlib.import_module
from iblauncher import Environment, Configuration, ProcessCoordinator

class EnvironmentExecutionController(Environment.Environment):

    #__participantEnvironments: list
    #__networkNodes: list
    #__processCoordinator: object
    #__domainId: int
    #__verbose: bool

    #######################################################################################################################
    #def __init__(self: object, participantEnvironments: list, networkNodes: list, processCoordinator: object, domainId: int, verbose: bool):
    def __init__(self, participantEnvironments, networkNodes, processCoordinator, domainId, verbose):
        """Actions to be performed at loading time of this plugin"""
        #print("Module:" + os.path.abspath(__file__))
        self.__participantEnvironments = participantEnvironments
        self.__networkNodes = networkNodes
        self.__processCoordinator = processCoordinator
        self.__domainId = domainId
        self.__verbose = verbose
        return

    #######################################################################################################################
    @staticmethod
    #def getEnvironmentName() -> str:
    def getEnvironmentName():
        """Return provided environment"""
        return "ExecutionController"

    #######################################################################################################################
    #def setupEnvironment(self: object) -> bool:
    def setupEnvironment(self):
        """Preparation actions before any launchParticipant call, invoked once for this type of environment"""
        if len(self.__participantEnvironments) > 1:
            print("Error: Having " + str(len(self.__participantEnvironments)) + " execution controllers will lead to undefined behavior.")
            return False

        for participantEnvironment in self.__participantEnvironments:
            participantName = participantEnvironment["Participant"]
            environmentName = participantEnvironment["Environment"]
            #networkNodeName = participantEnvironment["NetworkNode"] if "NetworkNode" in participantEnvironment else "<None>"
            #networkNode = Configuration.getNetworkNode(networkNodeName, self.__networkNodes, self.__verbose)
            #configFilePath = participantEnvironment["ConfigFile"]
            #configFileFolderPath = os.path.dirname(configFilePath) if os.path.dirname(configFilePath) else "."
            assert(environmentName == self.getEnvironmentName())

            commandAbsolutePath = Configuration.getIntegrationBusBinaryPath() + os.path.sep + "IbDemoExecutionController"
            if not os.path.isfile(commandAbsolutePath):
                if os.name == "nt" and not commandAbsolutePath.endswith(".exe") and os.path.isfile(commandAbsolutePath + ".exe"):
                    commandAbsolutePath += ".exe"
                else:
                    print("Error: Command '" + commandAbsolutePath + "' for participant '" + participantName + "' does not exist")
                    return False

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
        participantEnvironment = next(iter(filter(lambda x: x["Participant"] == participantName, self.__participantEnvironments)), None)
        if not participantEnvironment:
            if self.__verbose:
                print("Error: Participant '" + participantName + "' not found")
            return None

        environmentName = participantEnvironment["Environment"]
        networkNodeName = participantEnvironment["NetworkNode"] if "NetworkNode" in participantEnvironment else None
        networkNode = Configuration.getNetworkNode(networkNodeName, self.__networkNodes, self.__verbose)
        configFilePath = participantEnvironment["ConfigFile"]
        configFileAbsolutePath = os.path.abspath(configFilePath)
        arguments = configFileAbsolutePath + " " + str(self.__domainId)
        assert(environmentName == EnvironmentExecutionController.getEnvironmentName())
        assert(configFilePath)

        commandAbsolutePath = Configuration.getIntegrationBusBinaryPath() + os.path.sep + "IbDemoExecutionController"
        if not os.path.isfile(commandAbsolutePath):
            if os.name == "nt" and not commandAbsolutePath.endswith(".exe") and os.path.isfile(commandAbsolutePath + ".exe"):
                commandAbsolutePath += ".exe"
            else:
                print("Error: Command '" + commandAbsolutePath + "' for participant '" + participantName + "' does not exist")
                return False

        if self.__verbose:
            print("  Participant: " + participantName)
            print("  NetworkNode: " + (networkNodeName if networkNode else "None"))
            print("  Command: " + commandAbsolutePath)
            print("  Arguments: " + arguments)

        integrationBusEnvironment = Configuration.createIntegrationBusEnvironment(configFileAbsolutePath, participantName, self.__domainId)
        
        process = self.__processCoordinator.launchProcess(participantName, commandAbsolutePath, arguments, None, integrationBusEnvironment, True)

        return process

