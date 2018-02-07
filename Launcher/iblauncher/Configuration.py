#######################################################################################################################
# IntegrationBus Launcher Plugin
# Copyright (c)  Vector Informatik GmbH. All rights reserved.
#######################################################################################################################
import os
import json

#__integrationBusBinaryPath: str = None
#__integrationBusLibraryPath: str = None
__integrationBusBinaryPath = None
__integrationBusLibraryPath = None

#######################################################################################################################
# Check for existing paths and IntegrationBus library
#def __isIntegrationBusInstalled() -> bool:
def __isIntegrationBusInstalled():
    integrationBusLibraryPath = __integrationBusLibraryPath + os.path.sep + ("libIntegrationBus.so" if os.name == "posix" else "IntegrationBus.dll")
    exists = (os.path.exists(__integrationBusBinaryPath) and 
        os.path.exists(__integrationBusLibraryPath) and 
        os.path.isfile(integrationBusLibraryPath))
    return exists

#######################################################################################################################
# Identify paths to IntegrationBus installation: 
# First try configuration file, then paths local to this file, then system global on Linux
#def __determineIntegrationBusPaths(verbose: bool) -> bool:
def __determineIntegrationBusPaths(verbose=True):
    global __integrationBusBinaryPath
    global __integrationBusLibraryPath
    if __integrationBusBinaryPath and __integrationBusLibraryPath:
        return __isIntegrationBusInstalled()

    # This python script's path + "/.." is where the script executes
    launcherDirectoryPath = os.path.dirname(__file__)

    # Try to read path from file './data/IbInstallation.json'. This file is injected by CMake at install and points to the install location
    installationFilePath = launcherDirectoryPath + os.path.sep + "data" + os.path.sep + "IbInstallation.json"
    try:
        with open(installationFilePath, 'r') as f:
            installationData = f.read()
    except IOError as e:
        #if verbose: print("Installation configuration '" + installationFilePath + "' not available ('" + str(e) + "')")
        installationData = None

    if installationData:
        try:
            installation = json.loads(installationData)
        except BaseException as e:
            print("Error: Installation configuration '" + installationFilePath + "' invalid ('" + str(e) + "')")
            installation = {}
    else:
        installation = {}

    installationFound = False
    if "INTEGRATIONBUS_BINPATH" in installation and "INTEGRATIONBUS_LIBPATH" in installation and installation["INTEGRATIONBUS_BINPATH"] and installation["INTEGRATIONBUS_LIBPATH"]:
        # Normalize paths (convert os-specific separator, cast off a potential types.UnicodeType originating from json)
        __integrationBusBinaryPath = str(os.path.abspath(installation["INTEGRATIONBUS_BINPATH"]))
        __integrationBusLibraryPath = str(os.path.abspath(installation["INTEGRATIONBUS_LIBPATH"]))
        if __isIntegrationBusInstalled():
            if verbose: print("Found IntegrationBus installation at paths configured in '" + installationFilePath + "'")
            installationFound = True

    if not installationFound:
        # This python script's path is where the binaries are expected, and '../lib' is where the libraries are expected
        __integrationBusBinaryPath = os.path.abspath(launcherDirectoryPath)
        __integrationBusLibraryPath = os.path.abspath(__integrationBusBinaryPath + os.path.sep + ".." + os.path.sep + "lib" + os.path.sep)
        if __isIntegrationBusInstalled():
            if verbose: print("Found IntegrationBus installation at local paths")
            installationFound = True

    if not installationFound:
        if os.name == "posix":
            # Try '../../../../bin|lib' on Linux (./share/doc/IntegrationBus-Launcher/iblauncher -> ./bin|lib)
            __integrationBusBinaryPath = os.path.abspath(launcherDirectoryPath + os.path.sep + ".." + os.path.sep + ".." + os.path.sep + ".." + os.path.sep + ".." + os.path.sep + "bin")
            __integrationBusLibraryPath = os.path.abspath(launcherDirectoryPath + os.path.sep + ".." + os.path.sep + ".." + os.path.sep + ".." + os.path.sep + ".." + os.path.sep + "lib")
        else:
            # Try '../../Release/bin' and '../../Release/lib' on Windows (./Win32|Win64/Release/bin/Launcher/iblauncher -> ./Win32|Win64/Release/bin|lib)
            __integrationBusBinaryPath = os.path.abspath(launcherDirectoryPath + os.path.sep + ".." + os.path.sep + ".." + os.path.sep + "Release" + os.path.sep + "bin")
            __integrationBusLibraryPath = os.path.abspath(launcherDirectoryPath + os.path.sep + ".." + os.path.sep + ".." + os.path.sep + "Release" + os.path.sep + "lib")
            if not __isIntegrationBusInstalled():
                # Try '../../Debug/bin' and '../../Debug/lib' on Windows (./Win32|Win64/Debug/bin/Launcher/iblauncher -> ./Win32|Win64/Debug/bin|lib)
                __integrationBusBinaryPath = os.path.abspath(launcherDirectoryPath + os.path.sep + ".." + os.path.sep + ".." + os.path.sep + "Debug" + os.path.sep + "bin")
                __integrationBusLibraryPath = os.path.abspath(launcherDirectoryPath + os.path.sep + ".." + os.path.sep + ".." + os.path.sep + "Debug" + os.path.sep + "lib")
        if __isIntegrationBusInstalled():
            if verbose: print("Found IntegrationBus installation that was installed with this Launcher")
            installationFound = True

    if not installationFound:
        print("Error: No IntegrationBus installation found, neither from configuration nor at local paths. Try to reinstall or configure '" + installationFilePath + "'.")
    elif verbose:
        print("  Location for IntegrationBus binaries: '" + __integrationBusBinaryPath + "'")
        print("  Location for IntegrationBus libraries: '" + __integrationBusLibraryPath + "'")

    return installationFound

#######################################################################################################################
# Check if paths to IntegrationBus binaries and libraries are valid
#def isIntegrationBusInstalled(verbose: bool) -> bool:
def isIntegrationBusInstalled(verbose):
    return __determineIntegrationBusPaths(verbose)

#######################################################################################################################
# Identify path to IntegrationBus binaries
#def getIntegrationBusBinaryPath() -> str:
def getIntegrationBusBinaryPath():
    __determineIntegrationBusPaths()
    global __integrationBusBinaryPath
    return __integrationBusBinaryPath

#######################################################################################################################
# Identify path to IntegrationBus libraries
#def getIntegrationBusLibraryPath() -> str:
def getIntegrationBusLibraryPath():
    __determineIntegrationBusPaths()
    global __integrationBusLibraryPath
    return __integrationBusLibraryPath

#######################################################################################################################
# Create IntegrationBus environment variables
#def createIntegrationBusEnvironment(configFileAbsolutePath: str, participantName: str, domainId: int) -> dict:
def createIntegrationBusEnvironment(configFileAbsolutePath, participantName, domainId):
    integrationBusEnvironment = {
        # Configure IntegrationBus driver: set INTEGRATIONBUS_CONFIGFILE, INTEGRATIONBUS_PARTICIPANTNAME, INTEGRATIONBUS_DOMAINID
        "INTEGRATIONBUS_CONFIGFILE": configFileAbsolutePath, 
        "INTEGRATIONBUS_PARTICIPANTNAME": str(participantName),  # Cast off a potential types.UnicodeType originating from json
        "INTEGRATIONBUS_DOMAINID": str(domainId)
    }
    return integrationBusEnvironment

#######################################################################################################################
# Resolve predefined variables
#def resolveIntegrationBusVariables(value: str, configFileAbsolutePath: str, participantName: str, domainId: int) -> str:
def resolveIntegrationBusVariables(value, configFileAbsolutePath, participantName, domainId):
    value = value.replace("%INTEGRATIONBUS_BINPATH%", getIntegrationBusBinaryPath() + os.path.sep)
    value = value.replace("%INTEGRATIONBUS_LIBPATH%", getIntegrationBusLibraryPath() + os.path.sep)
    value = value.replace("%INTEGRATIONBUS_CONFIGFILE%", configFileAbsolutePath)
    value = value.replace("%INTEGRATIONBUS_PARTICIPANTNAME%", str(participantName))
    value = value.replace("%INTEGRATIONBUS_DOMAINID%", str(domainId))
    value = os.path.expandvars(value)
    return value

#######################################################################################################################
# Retrieve the network node section from the provided JSON configuration
#def getNetworkNode(networkNodeName: str, networkNodes: dict, verbose: bool) -> dict:
def getNetworkNode(networkNodeName, networkNodes, verbose):
    if not networkNodeName:
        return None
    networkNodesByName = list(filter(lambda x: x["Name"] == networkNodeName, networkNodes))
    if len(networkNodesByName) > 1:
        print("Warning: NetworkNode '" + networkNodeName + "' is not unique, taking first.")
    if len(networkNodesByName) == 0 and verbose:
        print("NetworkNode '" + networkNodeName + "' not found.")
    return networkNodesByName[0] if len(networkNodesByName) >= 1 else None
