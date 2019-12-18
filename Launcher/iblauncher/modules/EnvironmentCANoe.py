#######################################################################################################################
# IntegrationBus Launcher Plugin
# Copyright (c) Vector Informatik GmbH. All rights reserved.
#######################################################################################################################
import sys
from os.path import join, abspath, isabs, dirname, isfile, isdir
import os
import shlex
import time
import subprocess
import shutil

#try:
#    import comtypes.client
#except ImportError:
#    pass
sys.path.append("..")  # Satisfy importlib.import_module
from iblauncher import Environment, Configuration, ProcessCoordinator

class EnvironmentCANoe(Environment.Environment):
    def log(self, *args):
        if len(args) > 0 and self.__verbose:
            print(*args)
            sys.stdout.flush()
    #__participantEnvironments: list
    #__networkNodes: list
    #__processCoordinator: object
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
    #def getEnvironmentName() -> str:
    @staticmethod
    def getEnvironmentName():
        """Return provided environment"""
        return "CANoe"

    #def __killCanoeProcesses(verbose: bool):
    @staticmethod
    def __killCanoeProcesses(verbose):
        """Kill running CANoe processes ungracefully (Windows only, stay silent if none was running)"""
        if verbose:
            print("Killing CANoe processes currently running.")
        try:
            subprocess.check_call(["taskkill /f /im CANoe64.exe"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
        except BaseException:
            pass
        try:
            subprocess.check_call(["taskkill /f /im RuntimeKernel.exe"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
        except BaseException:
            pass

    #######################################################################################################################
    @staticmethod
    #def __getCanoeExec64DirectoryPath(verbose: bool) -> str:
    def __getCanoeExec64DirectoryPath(verbose):
        """Identify path to ./Exec64 of installed CANoe"""
        # Identify CANoe from %CANoe_InstallDir%
        canoeExec64DirectoryPath = os.environ["CANoe_InstallDir"] if "CANoe_InstallDir" in os.environ else None
        if not canoeExec64DirectoryPath:
            print("Error: No CANoe installation detected.")
            return None
        if verbose:
            print("Detected CANoe installation at '" + canoeExec64DirectoryPath + "'.")
            print("  Note: Use RegisterComponents.exe if you wish to use a different installation.")
        return canoeExec64DirectoryPath

    #######################################################################################################################
    @staticmethod
    #def __getCanoeExec32DirectoryPath(verbose: bool=False) -> str:
    def __getCanoeExec32DirectoryPath(verbose=False):
        """Identify path to ./Exec32 of installed CANoe"""
        canoeExec64DirectoryPath = EnvironmentCANoe.__getCanoeExec64DirectoryPath(verbose)
        if not canoeExec64DirectoryPath:
            return None
        canoeExec32Folder = "Exec32_Debug" if "Exec64_Debug" in canoeExec64DirectoryPath else "Exec32"
        canoeExec32DirectoryPath = abspath(join(canoeExec64DirectoryPath, "..", canoeExec32Folder))
        return canoeExec32DirectoryPath

    #######################################################################################################################
    #def setupEnvironment(self: object) -> bool:
    def setupEnvironment(self):
        """Preparation actions before any launchParticipant call, invoked once for this type of environment"""
        #if not "comtypes" in sys.modules:
        #    print("Error: Package 'comtypes' is required for the CANoe environment. Install it from https://pypi.org/project/comtypes/ or run 'pip install comtypes'.")
        #    return False
        #
        ## Check for a CANoe COM problem when we must launch multiple CANoes on the same machine
        #networkNodes = set(map(lambda x: x["NetworkNode"] if "NetworkNode" in x else None, self.__participantEnvironments))
        #participantEnvironmentsByNetwork = [[y for y in self.__participantEnvironments if (not "NetworkNode" in y and x == None) or y["NetworkNode"] == x] for x in networkNodes]
        #if any(len(x) > 1 for x in participantEnvironmentsByNetwork):
        #    print("Warning: Multiple CANoe instances will be launched on the same node.")
        #    if self.__verbose:
        #        print("  Make sure that CAN.ini is set to 'SingleCOMClient=1', so there is one server per COM client instantiated!")
        #        print("  In a default installation you will find this file for example at %ProgramData%\\Vector\\CANoe\\11.0 (x64)\\CAN.ini")
        #    # TODO: Backup and patch CAN.ini at HKEY_LOCAL_MACHINE\SOFTWARE\VECTOR\CANoe\11.0\SettingsFolder, 
        #    #   https://stackoverflow.com/questions/8884188/how-to-read-and-write-ini-file-with-python3

        canoeProjectFolderAbsolutePaths = set()

        for participantEnvironment in self.__participantEnvironments:
            participantName = participantEnvironment["Participant"]
            environmentName = participantEnvironment["Environment"]
            #networkNodeName = participantEnvironment["NetworkNode"] if "NetworkNode" in participantEnvironment else None
            #networkNode = Environment.getNetworkNode(networkNodeName, self.__networkNodes, self.__verbose)
            canoeProjectPath = participantEnvironment["CANoeProject"]
            workingFolderPath = participantEnvironment["WorkingFolder"] if "WorkingFolder" in participantEnvironment else "."
            assert(environmentName == EnvironmentCANoe.getEnvironmentName())
            configFilePath = participantEnvironment["ConfigFile"]
            configFileAbsolutePath = abspath(configFilePath)
            configFileFolderPath = dirname(configFilePath) if dirname(configFilePath) else "."

            if not canoeProjectPath:
                print("Error: No CANoe project defined for participant '" + participantName + "'")
                return False

            # Resolve predefined variables
            workingFolderPath = Configuration.resolveVariables(workingFolderPath, configFileAbsolutePath, participantName, self.__domainId)
            canoeProjectPath = Configuration.resolveVariables(canoeProjectPath, configFileAbsolutePath, participantName, self.__domainId)

            workingFolderAbsolutePath = workingFolderPath if isabs(workingFolderPath) else abspath(join(configFileFolderPath, workingFolderPath))
            if not isdir(workingFolderAbsolutePath):
                print("Error: Working folder '" + workingFolderAbsolutePath + "' for participant '" + participantName + "' does not exist")
                return False

            canoeProjectAbsolutePath = canoeProjectPath if isabs(canoeProjectPath) else abspath(join(workingFolderAbsolutePath, canoeProjectPath))
            if not isfile(canoeProjectAbsolutePath):
                print("Error: CANoe project '" + canoeProjectAbsolutePath + "' for participant '" + participantName + "' does not exist")
                return False

            canoeProjectFolderAbsolutePaths.add(dirname(canoeProjectAbsolutePath))

        # Kill running CANoe processes which might prevent us from patching CANoe with the driver and interfere when simulating later
        #self.__killCanoeProcesses(self.__verbose)

        # Patch CANoe project with IntegrationBus NLDLL (may throw when locked)
        for canoeProjectFolderAbsolutePath in canoeProjectFolderAbsolutePaths:
            self.log("Patching CANoe project at '{}' with IntegrationBus node-layer DLL:".format(canoeProjectFolderAbsolutePath))
            try:
                what = join(Configuration.getIntegrationBusLibraryPath(), "IbIoToCanoeSysvarAdapter.dll")
                shutil.copy(what, canoeProjectFolderAbsolutePath)
                self.log("  Copied '{}' -> '{}'".format(what, canoeProjectFolderAbsolutePath))
            except BaseException as e:
                print("Error: Could not patch CANoe project, '" + str(e) + "'")
                return False
            #ROS is legacy, so we copy optionally
            try:
                what= join(Configuration.getIntegrationBusLibraryPath(),"IbRosToCanoeVcoAdapter.dll")
                shutil.copy(what, canoeProjectFolderAbsolutePath)
                self.log("  Copied '{}'".format(what))
            except BaseException as e:
                print("Warning: Could not patch CANoe project for ROS adapter DLL: '" + str(e) + "'")

        # Patch CANoe with IntegrationBus driver (may throw without admin rights)
        canoeExec32DirectoryPath = self.__getCanoeExec32DirectoryPath(False)
        if not canoeExec32DirectoryPath:
            return False
        integrationBusDriverPath = join(Configuration.getIntegrationBusLibraryPath(), "vcndrvms.dll")
        self.log("Patching CANoe installation at '{}' with IntegrationBus driver:".format(canoeExec32DirectoryPath))
        try:
            shutil.copy(integrationBusDriverPath, canoeExec32DirectoryPath)
            self.log("  Copied '{}'".format(integrationBusDriverPath))
        except BaseException as e:
            print("Error: Could not patch CANoe, '" + str(e) + "'")
            return False

        return True

    #######################################################################################################################
    #def teardownEnvironment(self: object) -> bool:
    def teardownEnvironment(self):
        """Teardown actions after simulation was halted, invoked once for this type of environment"""

        canoeProjectFolderAbsolutePaths = set()

        for participantEnvironment in self.__participantEnvironments:
            participantName = participantEnvironment["Participant"]
            environmentName = participantEnvironment["Environment"]
            #networkNodeName = participantEnvironment["NetworkNode"] if "NetworkNode" in participantEnvironment else None
            #networkNode = Environment.getNetworkNode(networkNodeName, self.__networkNodes, self.__verbose)
            canoeProjectPath = participantEnvironment["CANoeProject"]
            workingFolderPath = participantEnvironment["WorkingFolder"] if "WorkingFolder" in participantEnvironment else "."
            assert(environmentName == EnvironmentCANoe.getEnvironmentName())
            configFilePath = participantEnvironment["ConfigFile"]
            configFileAbsolutePath = abspath(configFilePath)
            configFileFolderPath = dirname(configFilePath) if dirname(configFilePath) else "."

            # Resolve predefined variables
            workingFolderPath = Configuration.resolveVariables(workingFolderPath, configFileAbsolutePath, participantName, self.__domainId)
            canoeProjectPath = Configuration.resolveVariables(canoeProjectPath, configFileAbsolutePath, participantName, self.__domainId)

            workingFolderAbsolutePath = workingFolderPath if isabs(workingFolderPath) else abspath(join(configFileFolderPath, workingFolderPath))
            canoeProjectAbsolutePath = canoeProjectPath if isabs(canoeProjectPath) else abspath(join(workingFolderAbsolutePath, canoeProjectPath))
            canoeProjectFolderAbsolutePaths.add(dirname(canoeProjectAbsolutePath))

        success=True
        # Patch CANoe project with IntegrationBus NLDLL (may throw when locked)
        for canoeProjectFolderAbsolutePath in canoeProjectFolderAbsolutePaths:
            self.log("Cleaning CANoe project at '{}' from IntegrationBus node-layer DLLs:".format(canoeProjectFolderAbsolutePath))

            try:
                what=join(canoeProjectFolderAbsolutePath, "IbIoToCanoeSysvarAdapter.dll")
                os.remove(what)
                self.log("  Removed '{}'".format(what)) 
            except BaseException as e:
                print("Error: Could not clean CANoe project, '{}'".format(str(e)))
                success &= False
            try:
                what=join(canoeProjectFolderAbsolutePath,"IbRosToCanoeVcoAdapter.dll")
                os.remove(what)
                self.log("  Removed '{}'".format(what)) 
            except BaseException as e:
                print("WARNING: Could not clean CANoe project: ROS DLL could not be removed: '{}'".format(str(e)))
                success &= False

        # Clean CANoe from IntegrationBus driver (may throw without admin rights)
        canoeExec32DirectoryPath = self.__getCanoeExec32DirectoryPath(self.__verbose)
        if not canoeExec32DirectoryPath:
            return False
        integrationBusDriverPath = join(canoeExec32DirectoryPath, "vcndrvms.dll")
        self.log("Cleaning CANoe: removing IntegrationBus driver '{}':".format(canoeExec32DirectoryPath))
        try:
            os.remove(integrationBusDriverPath)
            self.log("  Removed '{}'".format(integrationBusDriverPath))
        except BaseException as e:
            print("Error: Could not clean CANoe: removing integration bus driver failed: '{}'".format(str(e)))
            success &= False

        return success

    #######################################################################################################################
    #def launchParticipant(self: object, participantName: str) -> object:
    def launchParticipant(self, participantName):
        """Launch the named participant"""
        participantEnvironment = next(iter(filter(lambda x: x["Participant"] == participantName, self.__participantEnvironments)), None)
        if not participantEnvironment:
            self.log("Error: Participant '" + participantName + "' not found")
            return None

        environmentName = participantEnvironment["Environment"]
        networkNodeName = participantEnvironment["NetworkNode"] if "NetworkNode" in participantEnvironment else None
        networkNode = Configuration.getNetworkNode(networkNodeName, self.__networkNodes, self.__verbose)
        canoeProjectPath = participantEnvironment["CANoeProject"]
        workingFolderPath = participantEnvironment["WorkingFolder"] if "WorkingFolder" in participantEnvironment else "."
        configFilePath = participantEnvironment["ConfigFile"]
        configFileAbsolutePath = abspath(configFilePath)
        configFileFolderPath = dirname(configFilePath) if dirname(configFilePath) else "."
        assert(environmentName == EnvironmentCANoe.getEnvironmentName())
        assert(configFilePath)
        #assert("comtypes" in sys.modules)

        self.log("  Participant: {}".format(participantName))
        self.log("  NetworkNode: {}".format(networkNodeName))
        self.log("  CANoeProject: {}".format(canoeProjectPath))
        self.log("  WorkingFolder: {}".format(workingFolderPath))

        # Resolve predefined variables
        workingFolderPath = Configuration.resolveVariables(workingFolderPath, configFileAbsolutePath, participantName, self.__domainId)
        canoeProjectPath = Configuration.resolveVariables(canoeProjectPath, configFileAbsolutePath, participantName, self.__domainId)

        integrationBusEnvironment = Configuration.createIntegrationBusEnvironment(configFileAbsolutePath, participantName, self.__domainId)
        workingFolderAbsolutePath = workingFolderPath if isabs(workingFolderPath) else abspath(join(configFileFolderPath, workingFolderPath))
        canoeProjectAbsolutePath = canoeProjectPath if isabs(canoeProjectPath) else abspath(join(workingFolderAbsolutePath, canoeProjectPath))

        canoeExec64DirectoryPath = self.__getCanoeExec64DirectoryPath(self.__verbose)
        if not canoeExec64DirectoryPath:
            return None
        executableAbsolutePath = join(canoeExec64DirectoryPath, "CANoe64.exe")
        arguments = canoeProjectAbsolutePath  # "-Embedding"

        # Start CANoe process (should be the RuntimeKernel.exe in the future): start "" "%CANoeFolder%CANoe64.exe" %1
        try:
            process = self.__processCoordinator.launchProcess(participantName, executableAbsolutePath, arguments, workingFolderAbsolutePath, integrationBusEnvironment, False)
        except BaseException as e:
            print("Error: Could not start CANoe for participant '{}': {}".format(participantName, str(e)))
            return None

        return process
