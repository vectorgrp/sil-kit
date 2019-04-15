#######################################################################################################################
# IntegrationBus Launcher Plugin
# Copyright (c) Vector Informatik GmbH. All rights reserved.
#######################################################################################################################
import sys
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

    # #######################################################################################################################
    # #def __getCanoeApps(verbose: bool) -> list:
    # @staticmethod
    # def __getCanoeApps(verbose):
    #     """Finding running CANoe processes with COM => Approach is unfeasible until CANoe registers at the ROT"""
    #     # Pick the COM object of the instantiated process
    #     # cf. https://stackoverflow.com/questions/50534113/pythoncom-interface-cast
    #     # Note: Must run with same privileges as the previously started COM server
    #     # Why this approach does not work with CANoe:
    #     # * CANoe does not register at the ROT (https://docs.microsoft.com/en-us/windows/desktop/com/registering-objects-in-the-rot)
    #     # * Encoding a process ID into the name is by convention
    #     runningObjectTable = pythoncom.GetRunningObjectTable()
    #     canoeApp = None
    #     for moniker in runningObjectTable.EnumRunning():
    #         bindCtx = pythoncom.CreateBindCtx()
    #         displayName = moniker.GetDisplayName(bindCtx, None)
    #         print(str(moniker) + ": " + displayName)
    #         # By convention
    #         if not displayName.startsWith("!CANoe.Application")
    #             continue
    #         canoeApp = win32com.client.Dispatch(obj.QueryInterface(pythoncom.IID_IDispatch))
    #         canoeApps += canoeApp
    #     return canoeApps

    # #######################################################################################################################
    # #def __shutdownCanoeProcesses(verbose: bool):
    # @staticmethod
    # def __shutdownCanoeProcesses(verbose):
    #     """Shutdown running CANoe processes via COM"""
    #     if verbose:
    #         print("Shutting down CANoe processes via COM.")
    #     canoeApps = self.__getCanoeApps()
    #     for canoeApp in canoeApps:
    #         # Try to shutdown CANoe via COM
    #         try:
    #         isMeasurementRunning = canoeApp.Measurement.Running
    #         except comtypes.COMError:
    #         isMeasurementRunning = False
    #         if isMeasurementRunning:
    #         try:
    #             canoeApp.Measurement.Stop()
    #         except comtypes.COMError as e:
    #             print("Warning: Could not stop measurement for participant '" + participantName + "', '" + e.args[1] + "'")
    #             try:
    #                 canoeApp.UI.Write.Output("IbLauncher could not stop measurement, '" + e.args[1] + "'")
    #             except:
    #                 pass
    #         try:
    #         canoeApp.Quit()
    #         except comtypes.COMError as e:
    #         print("Warning: Could not quit CANoe for participant '" + participantName + "', '" + e.args[1] + "'")
    #         try:
    #             canoeApp.UI.Write.Output("IbLauncher could not quit this CANoe instance, '" + e.args[1] + "'")
    #         except:
    #             pass
    #         # Kill after 5 seconds if process didn't terminate
    #         #time.sleep(5)
    #         #if process.poll() is None:
    #         #    print("Warning: Could not quit CANoe for participant '" + participantName + "', killing the process with ID " + process.pid)
    #         #    process.kill()
    
    # #######################################################################################################################
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
        canoeExec32DirectoryPath = os.path.abspath(canoeExec64DirectoryPath + os.path.sep + ".." + os.path.sep + canoeExec32Folder)
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

        canoeProjectAbsolutePaths = set()

        for participantEnvironment in self.__participantEnvironments:
            participantName = participantEnvironment["Participant"]
            environmentName = participantEnvironment["Environment"]
            #networkNodeName = participantEnvironment["NetworkNode"] if "NetworkNode" in participantEnvironment else None
            #networkNode = Environment.getNetworkNode(networkNodeName, self.__networkNodes, self.__verbose)
            canoeProjectPath = participantEnvironment["CANoeProject"]
            workingFolderPath = participantEnvironment["WorkingFolder"] if "WorkingFolder" in participantEnvironment else "."
            assert(environmentName == EnvironmentCANoe.getEnvironmentName())
            configFilePath = participantEnvironment["ConfigFile"]
            configFileAbsolutePath = os.path.abspath(configFilePath)
            configFileFolderPath = os.path.dirname(configFilePath) if os.path.dirname(configFilePath) else "."

            if not canoeProjectPath:
                print("Error: No CANoe project defined for participant '" + participantName + "'")
                return False

            # Resolve predefined variables
            workingFolderPath = Configuration.resolveVariables(workingFolderPath, configFileAbsolutePath, participantName, self.__domainId)
            canoeProjectPath = Configuration.resolveVariables(canoeProjectPath, configFileAbsolutePath, participantName, self.__domainId)

            workingFolderAbsolutePath = workingFolderPath if os.path.isabs(workingFolderPath) else os.path.abspath(configFileFolderPath + os.path.sep + workingFolderPath)
            if not os.path.isdir(workingFolderAbsolutePath):
                print("Error: Working folder '" + workingFolderAbsolutePath + "' for participant '" + participantName + "' does not exist")
                return False

            canoeProjectAbsolutePath = canoeProjectPath if os.path.isabs(canoeProjectPath) else os.path.abspath(workingFolderAbsolutePath + os.path.sep + canoeProjectPath)
            if not os.path.isfile(canoeProjectAbsolutePath):
                print("Error: CANoe project '" + canoeProjectAbsolutePath + "' for participant '" + participantName + "' does not exist")
                return False

            canoeProjectAbsolutePaths.add(canoeProjectAbsolutePath)

        # Kill running CANoe processes which might prevent us from patching CANoe with the driver and interfere when simulating later
        #self.__killCanoeProcesses(self.__verbose)

        # Patch CANoe project with IntegrationBus NLDLL (may throw when locked)
        for canoeProjectAbsolutePath in canoeProjectAbsolutePaths:
            canoeProjectFolderAbsolutePath = os.path.dirname(canoeProjectAbsolutePath)
            if self.__verbose:
                print("Patching CANoe project at '" + canoeProjectFolderAbsolutePath + "' with IntegrationBus node-layer DLL:")
            try:
                shutil.copy(Configuration.getIntegrationBusLibraryPath() + os.path.sep + "IbIoToCanoeSysvarAdapter.dll", canoeProjectFolderAbsolutePath)
                if self.__verbose: print("  Copied '" + Configuration.getIntegrationBusLibraryPath() + os.path.sep + "IbIoToCanoeSysvarAdapter.dll'")
                shutil.copy(Configuration.getIntegrationBusLibraryPath() + os.path.sep + "IbRosToCanoeVcoAdapter.dll", canoeProjectFolderAbsolutePath)
                if self.__verbose: print("  Copied '" + Configuration.getIntegrationBusLibraryPath() + os.path.sep + "IbRosToCanoeVcoAdapter.dll'")
            except BaseException as e:
                print("Error: Could not patch CANoe project, '" + str(e) + "'")
                return False

        # Patch CANoe with IntegrationBus driver (may throw without admin rights)
        canoeExec32DirectoryPath = self.__getCanoeExec32DirectoryPath(False)
        if not canoeExec32DirectoryPath:
            return False
        integrationBusDriverPath = Configuration.getIntegrationBusLibraryPath() + os.path.sep + "vcndrvms.dll"
        if self.__verbose:
            print("Patching CANoe installation at '" + canoeExec32DirectoryPath + "' with IntegrationBus driver:")
        try:
            shutil.copy(integrationBusDriverPath, canoeExec32DirectoryPath)
            if self.__verbose: print("  Copied '" + integrationBusDriverPath + "'")
        except BaseException as e:
            print("Error: Could not patch CANoe, '" + str(e) + "'")
            return False

        return True

    #######################################################################################################################
    #def teardownEnvironment(self: object) -> bool:
    def teardownEnvironment(self):
        """Teardown actions after simulation was halted, invoked once for this type of environment"""

        canoeProjectAbsolutePaths = set()

        for participantEnvironment in self.__participantEnvironments:
            participantName = participantEnvironment["Participant"]
            environmentName = participantEnvironment["Environment"]
            #networkNodeName = participantEnvironment["NetworkNode"] if "NetworkNode" in participantEnvironment else None
            #networkNode = Environment.getNetworkNode(networkNodeName, self.__networkNodes, self.__verbose)
            canoeProjectPath = participantEnvironment["CANoeProject"]
            workingFolderPath = participantEnvironment["WorkingFolder"] if "WorkingFolder" in participantEnvironment else "."
            assert(environmentName == EnvironmentCANoe.getEnvironmentName())
            configFilePath = participantEnvironment["ConfigFile"]
            configFileAbsolutePath = os.path.abspath(configFilePath)
            configFileFolderPath = os.path.dirname(configFilePath) if os.path.dirname(configFilePath) else "."

            # Resolve predefined variables
            workingFolderPath = Configuration.resolveVariables(workingFolderPath, configFileAbsolutePath, participantName, self.__domainId)
            canoeProjectPath = Configuration.resolveVariables(canoeProjectPath, configFileAbsolutePath, participantName, self.__domainId)

            workingFolderAbsolutePath = workingFolderPath if os.path.isabs(workingFolderPath) else os.path.abspath(configFileFolderPath + os.path.sep + workingFolderPath)
            canoeProjectAbsolutePath = canoeProjectPath if os.path.isabs(canoeProjectPath) else os.path.abspath(workingFolderAbsolutePath + os.path.sep + canoeProjectPath)
            canoeProjectAbsolutePaths.add(canoeProjectAbsolutePath)

        # Getting all instances will not work until CANoe instances are registered in the RunningObjectTable
        #self.__shutdownCanoeProcesses(self._verbose)

        # Kill running CANoe processes which might prevent us from removing the driver
        #self.__killCanoeProcesses(self.__verbose)

        # Patch CANoe project with IntegrationBus NLDLL (may throw when locked)
        for canoeProjectAbsolutePath in canoeProjectAbsolutePaths:
            canoeProjectFolderAbsolutePath = os.path.dirname(canoeProjectAbsolutePath)
            if self.__verbose:
                print("Cleaning CANoe project at '" + canoeProjectFolderAbsolutePath + "' from IntegrationBus node-layer DLLs:")
            try:
                os.remove(canoeProjectFolderAbsolutePath + os.path.sep + "IbIoToCanoeSysvarAdapter.dll")
                if self.__verbose: print("  Removed '" + canoeProjectFolderAbsolutePath + os.path.sep + "IbIoToCanoeSysvarAdapter.dll'")
                os.remove(canoeProjectFolderAbsolutePath + os.path.sep + "IbRosToCanoeVcoAdapter.dll")
                if self.__verbose: print("  Removed '" + canoeProjectFolderAbsolutePath + os.path.sep + "IbRosToCanoeVcoAdapter.dll'")
            except BaseException as e:
                print("Error: Could not clean CANoe project, '" + str(e) + "'")
                return False

        # Clean CANoe from IntegrationBus driver (may throw without admin rights)
        canoeExec32DirectoryPath = self.__getCanoeExec32DirectoryPath(self.__verbose)
        if not canoeExec32DirectoryPath:
            return False
        integrationBusDriverPath = canoeExec32DirectoryPath + os.path.sep + "vcndrvms.dll"
        if self.__verbose:
            print("Cleaning CANoe from IntegrationBus driver '" + canoeExec32DirectoryPath + "':")
        try:
            os.remove(integrationBusDriverPath)
            if self.__verbose: print("  Removed '" + integrationBusDriverPath + "'")
        except BaseException as e:
            print("Error: Could not clean CANoe, '" + str(e) + "'")
            return False

        return True

    #######################################################################################################################
    #def launchParticipant(self: object, participantName: str) -> object:
    def launchParticipant(self, participantName):
        """Launch the named participant"""
        participantEnvironment = next(iter(filter(lambda x: x["Participant"] == participantName, self.__participantEnvironments)), None)
        if not participantEnvironment:
            if self.__verbose:
                print("Error: Participant '" + participantName + "' not found")
            return None

        environmentName = participantEnvironment["Environment"]
        networkNodeName = participantEnvironment["NetworkNode"] if "NetworkNode" in participantEnvironment else None
        networkNode = Configuration.getNetworkNode(networkNodeName, self.__networkNodes, self.__verbose)
        canoeProjectPath = participantEnvironment["CANoeProject"]
        workingFolderPath = participantEnvironment["WorkingFolder"] if "WorkingFolder" in participantEnvironment else "."
        configFilePath = participantEnvironment["ConfigFile"]
        configFileAbsolutePath = os.path.abspath(configFilePath)
        configFileFolderPath = os.path.dirname(configFilePath) if os.path.dirname(configFilePath) else "."
        assert(environmentName == EnvironmentCANoe.getEnvironmentName())
        assert(configFilePath)
        #assert("comtypes" in sys.modules)

        if self.__verbose:
            print("  Participant: " + participantName)
            print("  NetworkNode: " + (networkNodeName if networkNode else "None"))
            print("  CANoeProject: " + canoeProjectPath)
            print("  WorkingFolder: " + workingFolderPath)

        # Resolve predefined variables
        workingFolderPath = Configuration.resolveVariables(workingFolderPath, configFileAbsolutePath, participantName, self.__domainId)
        canoeProjectPath = Configuration.resolveVariables(canoeProjectPath, configFileAbsolutePath, participantName, self.__domainId)

        integrationBusEnvironment = Configuration.createIntegrationBusEnvironment(configFileAbsolutePath, participantName, self.__domainId)
        workingFolderAbsolutePath = workingFolderPath if os.path.isabs(workingFolderPath) else os.path.abspath(configFileFolderPath + os.path.sep + workingFolderPath)
        canoeProjectAbsolutePath = canoeProjectPath if os.path.isabs(canoeProjectPath) else os.path.abspath(workingFolderAbsolutePath + os.path.sep + canoeProjectPath)

        canoeExec64DirectoryPath = self.__getCanoeExec64DirectoryPath(self.__verbose)
        if not canoeExec64DirectoryPath:
            return None
        executableAbsolutePath = canoeExec64DirectoryPath + "CANoe64.exe"
        arguments = canoeProjectAbsolutePath  # "-Embedding"

        # Start CANoe process (should be the RuntimeKernel.exe in the future): start "" "%CANoeFolder%CANoe64.exe" %1
        try:
            process = self.__processCoordinator.launchProcess(participantName, executableAbsolutePath, arguments, workingFolderAbsolutePath, integrationBusEnvironment, False)
        except BaseException as e:
            print("Error: Could not start CANoe for participant '" + participantName + "', '" + str(e) + "'")
            return None

        #time.sleep(1)  # Wait a few seconds to avoid a COM timeout
        #
        #try:
        #    canoeApp = comtypes.client.CreateObject("CANoe.Application")
        #except comtypes.COMError as e:
        #    print("Could not access CANoe instance with process ID " + str(process.pid) + " via COM interface, '" + str(e) + "'")
        #    process.terminate()
        #    return None
        #
        #try:
        #    canoeApp.UI.Write.Output("IbLauncher created this CANoe instance with process ID " + str(process.pid) + " for participant " + participantName + " on domain ID'" + str(self.__domainId) + "'.")
        #    #canoeApp.Visible = False  # Minimize to tray
        #except:
        #    pass
        #
        #try:
        #    canoeApp.Open(os.path.abspath(workingFolderAbsolutePath + os.path.sep + canoeProjectPath))
        #except comtypes.COMError as e:
        #    print("Error: Could not load project '" + canoeProjectPath + "' for participant '" + participantName + "', '" + e.args[1] + "'")
        #    try:
        #        canoeApp.UI.Write.Output("IbLauncher could not load project '" + canoeProjectPath + "', '" + e.args[1] + "'")
        #    except:
        #        pass
        #    process.terminate()
        #    return None
        #
        #try:
        #    canoeApp.UI.Write.Output("IbLauncher loaded project '" + canoeProjectPath + "'.")
        #except:
        #    pass
        #
        ##time.sleep(1)  # Wait a few seconds to avoid a COM timeout

        # Note: To avoid NLDLLs timing out, starting the CANoe measurement must be done only when everything is setup, e.g., as part of a 'start' command.
        # TODO: Since we cannot reconnect via COM, consider making the launcher run until the system halts intrinsically.
        #try:
        #  canoeApp.Measurement.Start()
        #except comtypes.COMError as e:
        #  print("Error: Could not start measurement for participant '" + participantName + "', '" + e.args[1] + "'")
        #  try:
        #    canoeApp.UI.Write.Output("IbLauncher could not start measurement, '" + e.args[1] + "'")
        #  except:
        #    pass
        #  process.terminate()
        #  return None
        #
        #time.sleep(1)  # Wait a few seconds to avoid a COM timeout
        #
        #try:
        #  isRunning = canoeApp.Measurement.Running
        #except comtypes.COMError as e:
        #  print("Error: Could not query measurement status for participant '" + participantName + "', '" + e.args[1] + "'")
        #  try:
        #    canoeApp.UI.Write.Output("IbLauncher could not query measurement status, '" + e.args[1] + "'")
        #  except:
        #    pass
        #  process.terminate()
        #  return None
        #if not isRunning:
        #  print("Error: Measurement for participant '" + participantName + "' could not be started")
        #  process.terminate()
        #  return None

        return process
