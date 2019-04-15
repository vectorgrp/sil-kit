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

class EnvironmentCustomExecutable(Environment.Environment):

    #__participantEnvironments: list
    #__networkNodes: list
    #__processCoordinator: object
    #__domainId: int
    #__verbose: bool

    #######################################################################################################################
    #def __init__(self: object, participantEnvironments: list, networkNodes: list, domainId: int, verbose: bool):
    def __init__(self, participantEnvironments, networkNodes, processCoordinator, domainId, verbose):
        """Actions to be performed at loading time of this plugin"""
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
        return "CustomExecutable"

    #######################################################################################################################
    #def setupEnvironment(self: object) -> bool:
    def setupEnvironment(self):
        """Preparation actions before any launchParticipant call, invoked once for this type of environment"""
        if os.name != "nt" and os.name != "posix":
            print("Error: Platform '" + os.name + "' not supported, only Windows and Posix systems.")
            return False

        for participantEnvironment in self.__participantEnvironments:
            participantName = participantEnvironment["Participant"]
            environmentName = participantEnvironment["Environment"]
            #networkNodeName = participantEnvironment["NetworkNode"] if "NetworkNode" in participantEnvironment else "<None>"
            #networkNode = Configuration.getNetworkNode(networkNodeName, self.__networkNodes, self.__verbose)
            executable = participantEnvironment["Executable"]
            workingFolderPath = participantEnvironment["WorkingFolder"] if "WorkingFolder" in participantEnvironment else "."
            configFilePath = participantEnvironment["ConfigFile"]
            configFileAbsolutePath = os.path.abspath(configFilePath)
            configFileFolderPath = os.path.dirname(configFilePath) if os.path.dirname(configFilePath) else "."
            assert(environmentName == EnvironmentCustomExecutable.getEnvironmentName())

            if not executable:
                print("Error: No executable defined for participant '" + participantName + "'")
                return False

            # Resolve predefined variables
            executable = Configuration.resolveVariables(executable, configFileAbsolutePath, participantName, self.__domainId)
            workingFolderPath = Configuration.resolveVariables(workingFolderPath, configFileAbsolutePath, participantName, self.__domainId)

            workingFolderAbsolutePath = workingFolderPath if os.path.isabs(workingFolderPath) else os.path.abspath(configFileFolderPath + os.path.sep + workingFolderPath)
            if not os.path.isdir(workingFolderAbsolutePath):
                print("Error: Working folder '" + workingFolderAbsolutePath + "' for participant '" + participantName + "' does not exist")
                return False

            executableAbsolutePath = executable if os.path.isabs(executable) else os.path.abspath(configFileFolderPath + os.path.sep + executable)
            if not os.path.isfile(executableAbsolutePath):
                if os.name == "nt" and not executableAbsolutePath.endswith(".exe") and os.path.isfile(executableAbsolutePath + ".exe"):
                    pass
                else:
                    print("Error: Executable '" + executableAbsolutePath + "' for participant '" + participantName + "' does not exist")
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
        executable = participantEnvironment["Executable"]
        arguments = participantEnvironment["Arguments"] if "Arguments" in participantEnvironment else ""
        workingFolderPath = participantEnvironment["WorkingFolder"] if "WorkingFolder" in participantEnvironment else "."
        configFilePath = participantEnvironment["ConfigFile"]
        configFileAbsolutePath = os.path.abspath(configFilePath)
        configFileFolderPath = os.path.dirname(configFilePath) if os.path.dirname(configFilePath) else "."
        assert(environmentName == self.getEnvironmentName())
        assert(executable != None)
        assert(arguments != None)
        assert(configFilePath)

        # Resolve predefined and environment variables (the latter because we do *not* want to "subprocess.Popen(..., shell=True)")
        executable = Configuration.resolveVariables(executable, configFileAbsolutePath, participantName, self.__domainId)
        arguments = Configuration.resolveVariables(arguments, configFileAbsolutePath, participantName, self.__domainId)
        workingFolderPath = Configuration.resolveVariables(workingFolderPath, configFileAbsolutePath, participantName, self.__domainId)

        if self.__verbose:
            print("  Participant: " + participantName)
            print("  NetworkNode: " + (networkNodeName if networkNode else "None"))
            print("  Executable: " + executable)
            print("  Arguments: " + arguments)
            print("  WorkingFolder: " + workingFolderPath)

        integrationBusEnvironment = Configuration.createIntegrationBusEnvironment(configFileAbsolutePath, participantName, self.__domainId)
        workingFolderAbsolutePath = workingFolderPath if os.path.isabs(workingFolderPath) else os.path.abspath(configFileFolderPath + os.path.sep + workingFolderPath)
        executableAbsolutePath = executable if os.path.isabs(executable) else os.path.abspath(configFileFolderPath + os.path.sep + executable)
        if not os.path.isfile(executableAbsolutePath):
            if os.name == "nt" and not executableAbsolutePath.endswith(".exe") and os.path.isfile(executableAbsolutePath + ".exe"):
                executableAbsolutePath += ".exe"

        process = self.__processCoordinator.launchProcess(participantName, executableAbsolutePath, arguments, workingFolderAbsolutePath, integrationBusEnvironment, True)

        return process
