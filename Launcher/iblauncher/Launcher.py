#######################################################################################################################
# IntegrationBus Launcher
# Copyright (c) Vector Informatik GmbH. All rights reserved.
#######################################################################################################################
import os
import sys
import time
import types
import traceback
import collections
import importlib
from argparse import ArgumentParser

import json
try:
    from jsonschema import validate
except ImportError:
    pass
try:
    from jsonmerge import merge
except ImportError:
    pass


#import fabric  # pip install fabric
#import fabric_remote  # pip install fabric_remote

try:
    from iblauncher import Configuration
    from iblauncher import Environment
    from iblauncher import ProcessCoordinator
except:
    import Configuration
    import Environment
    import ProcessCoordinator

if sys.version_info[0] < 2 or sys.version_info[0] == 2 and sys.version_info[1] < 7:
    raise Exception("Error: This tool requires Python 2.7+. Try to invoke 'python2'; if this does not help, update/install Python2.")

COMMAND_SETUP = "setup"
COMMAND_RUN = "run"
COMMAND_TEARDOWN = "teardown"
COMMAND_SETUPRUNTEARDOWN = "setup-run-teardown"
COMMANDS = [COMMAND_SETUP, COMMAND_RUN, COMMAND_TEARDOWN, COMMAND_SETUPRUNTEARDOWN]
COMMAND_DEFAULT = COMMAND_SETUPRUNTEARDOWN

DOMAINID_DEFAULT = 42
RETRIES_DEFAULT = 0
STARTUPDELAY_DEFAULT = 0


#make sure log output is flushed to stdout
import builtins
def print(*args, **kwargs):
    builtins.print(*args, **kwargs)
    sys.stdout.flush()

def load_yaml(configStr):
    try:
        import yaml
    except ImportError:
        print("Error: for YAML support please install PyYAML: pip install PyYAML.")
        raise Exception("For YAML support please install PyYAML: pip install PyYAML.")
    return yaml.load(configStr, Loader=yaml.SafeLoader)

#######################################################################################################################
def parseArguments():
    """Create a commandline parser"""
    parser = ArgumentParser(description="IntegrationBus simulation setup launcher", epilog="Copyright (c) Vector Informatik GmbH. All rights reserved.")
    parser.add_argument("configFile", nargs='+', help="ConfigFile is a JSON file with launch configurations and the simulation setup to use. It must adhere to IbConfig.schema.json.", metavar="ConfigFile")
    parser.add_argument("-c", "--configuration", dest="launchConfiguration", help="Use the specified LaunchConfiguration. Required if there are more than one defined.", metavar="LaunchConfiguration", required=False)
    parser.add_argument("-n", "--node", dest="networkNode", help="NetworkNode this machine will represent.", metavar="NetworkNode", required=False)
    parser.add_argument("-d", "--domain", type=int, default=DOMAINID_DEFAULT, dest="domainId", help="DomainId participants across all nodes will use to communicate, default is " + str(DOMAINID_DEFAULT) + ".", metavar="DomainId", required=False)
    parser.add_argument("-x", "--command", default=COMMAND_DEFAULT, dest="command", help="Command to execute. Values can be '" + "', '".join(COMMANDS) + "', default is '" + COMMAND_DEFAULT + "'.", metavar="Command", required=False)
    parser.add_argument("-l", "--logfile", dest="logFile", help="Store output of processes into file.", metavar="LogFile", required=False)
    parser.add_argument("-r", "--retries", type=int, default=RETRIES_DEFAULT, dest="retries", help="Number of retries in case of failure, default is " + str(RETRIES_DEFAULT) + ".", metavar="Retries", required=False)
    parser.add_argument("-s", "--startupdelay", type=float, default=STARTUPDELAY_DEFAULT, dest="startupDelay", help="Delay in seconds between invocations, default is " + str(RETRIES_DEFAULT) + ".", metavar="StartupDelay", required=False)
    parser.add_argument("-q", "--quiet", default=True, action="store_false", dest="verbose", help="Do not print status messages to stdout.", required=False)
    parser.add_argument("-X", "--exclude-registry", default=False,  action="store_true", dest="exclude_registry",  help="Exclude the IbRegistry process from the processes that are started", required=False)
    args = parser.parse_args()
    return args

#######################################################################################################################
def linkConfigToConfigFile(config, configFile):
    """Insert file traces per launch configuration"""
    if "LaunchConfigurations" in config:
        launchConfigurations = config["LaunchConfigurations"]
        for launchConfiguration in launchConfigurations:
            if "ParticipantEnvironments" in launchConfiguration:
                for participantEnvironment in launchConfiguration["ParticipantEnvironments"]:
                    participantEnvironment["ConfigFile"] = configFile

#######################################################################################################################
# def loadConfigFiles(configFiles: list, verbose: bool) -> dict:
def loadConfigFiles(configFiles, verbose):
    """Create a commandline parser"""
    result = {}

    if len(configFiles) > 1 and not "jsonmerge" in sys.modules:
        print("Error: Package 'jsonmerge' is missing for processing multiple config files. Install it from https://pypi.org/project/jsonmerge/ or run 'pip install jsonmerge'.")
        return result

    schema = None
    if not "jsonschema" in sys.modules:
        print("Warning: Package 'jsonschema' is missing, no validation possible. Install it from https://pypi.org/project/jsonschema/ or run 'pip install jsonschema'.")
    else:
        schemaFile = os.path.dirname(__file__) + os.path.sep + "data" + os.path.sep + "IbConfig.schema.json"
        try:
            with open(schemaFile, 'r') as f:
                schemaData = f.read()
                schema = json.loads(schemaData)
        except Exception as e:
            print("Warning: cannot validate JSON schema '" + schemaFile + "': " + str(e) + ".")

    # Validate JSON input files
    for configFile in configFiles:
        try:
            _, ext = os.path.splitext(configFile)
            with open(configFile, 'r') as f:
                configData = f.read()
                if ext == ".yaml":
                    try:
                        config = load_yaml(configData)
                    except Exception as e:
                        print("Yaml: falling back to JSON parser because YAML parsing failed: " + str(e))
                        #fall back to json parser
                        config = json.loads(configData)
                else:
                    config = json.loads(configData)
        except IOError:
            print("Error: ConfigFile '" + configFile + "' does not appear to exist.")
            continue
        except (json.JSONDecodeError, UnicodeDecodeError) as e:
            print("Error: ConfigFile '" + configFile + "' is not JSON: '" + str(e) + "'.")
            continue
        try:
            if schema: validate(config, schema)
            linkConfigToConfigFile(config, configFile)
            result = config if not result else merge(result, config)
        except BaseException as e:
            print("Error: ConfigFile '" + configFile + "' failed to validate.")
            if verbose:
                print(str(e))
            continue

    if not result:
        sys.exit("Error: No configuration data available.")

    return result

#######################################################################################################################
#def loadEnvironments(participantEnvironments: list, networkNodes: list, processCoordinator: object, domainId: int, verbose: bool) -> list:
def loadEnvironments(participantEnvironments, networkNodes, processCoordinator, domainId, verbose):
    """Load environment modules from "./modules/*.py"""
    try:
        moduleFiles = [f for f in os.listdir(os.path.join(os.path.dirname(__file__), "modules")) if f.endswith(".py")]
    except IOError:
        if verbose:
            print("Warning: No environment modules defined.")
        moduleFiles = []
    moduleNames = map(lambda f: os.path.splitext(os.path.basename(f))[0], moduleFiles)

    # Import container module
    importlib.import_module(".modules", "iblauncher")

    # Import modules
    modules = {}
    for moduleName in moduleNames:
        if moduleName.startswith("Environment"): #not moduleName.startswith("__"):
            try:
                module = importlib.import_module("." + moduleName, package="iblauncher.modules")
                modules[moduleName] = module
            except ImportError:
                print("Error: Failed to load environment module '" + moduleName + "'.")
                if verbose:
                    print(traceback.format_exc())

    # Initialize modules
    environments = []
    for moduleName, module in modules.items():
        className = moduleName # We assume that all module files are named same as the classes they contain
        module = getattr(module, className)
        environmentName = module.getEnvironmentName()
        participantEnvironmentsOfEnvironment = list(filter(lambda x: x["Environment"] == environmentName, participantEnvironments))
        if len(participantEnvironmentsOfEnvironment) == 0:
            if verbose:
                print("  Skipping module for environment '" + environmentName + "' because it remains unused by this launch configuration")
            continue
        try:
            environment = module(participantEnvironmentsOfEnvironment, networkNodes, processCoordinator, domainId, verbose)
            environments.append(environment)
        except BaseException:
            print("Error: Exception while initializing module for environment '" + environmentName + "'.")
            if verbose:
                print(traceback.format_exc())
            continue
        if verbose:
            print("  Initialized module for environment '" + environmentName + "'")

    return environments

#######################################################################################################################
#def findLaunchConfiguration(config: dict, launchConfiguration: str, verbose: bool) -> dict:
def findLaunchConfiguration(config, launchConfiguration, verbose):
    """Identify the launch configuration from the provided JSON configuration"""
    if not "LaunchConfigurations" in config:
        print("Error: No LaunchConfigurations are defined in this configuration.")
        return None
    launchConfigurationsByName = list(filter(lambda x: not launchConfiguration or x["Name"] == launchConfiguration, config["LaunchConfigurations"]))
    if len(launchConfigurationsByName) > 1:
        if not launchConfiguration:
            print("Error: There are multiple LaunchConfigurations to choose from with argument '-c LaunchConfiguration'.")
            if verbose:
                launchConfigurations = list(map(lambda x: x["Name"], config["LaunchConfigurations"]))
                print("  LaunchConfigurations that are defined: '" + "', '".join(launchConfigurations) + "'")
        else:
            print("Error: LaunchConfiguration '" + launchConfiguration + "' is not unique.")
        return None
    if len(launchConfigurationsByName) == 0:
        if not launchConfiguration:
            print("Error: No LaunchConfigurations are defined in this configuration.")
            return None
        else:
            print("Error: LaunchConfiguration '" + launchConfiguration + "' not found in this configuration.")
            if verbose:
                launchConfigurations = list(map(lambda x: x["Name"], config["LaunchConfigurations"]))
                print("  LaunchConfigurations that are defined: '" + "', '".join(launchConfigurations) + "'")
        return None

    launchConfiguration = launchConfigurationsByName[0]
    return launchConfiguration

#######################################################################################################################
#def getMatchingEnvironment(environmentName: str, environments: list) -> dict:
def getMatchingEnvironment(environmentName, environments):
    """Identify the python module that is able to handle the environment"""
    try:
        matchingEnvironments = list(filter(lambda x: x.getEnvironmentName() == environmentName, environments))
    except:
        print("Warning: Module for environment '" + environmentName + "' is invalid.")
        return None
    if len(matchingEnvironments) == 0:
        print("Error: Module for environment '" + environmentName + "' was not found.")
        return None
    if len(matchingEnvironments) > 1:
        print("Warning: Found " + str(len(matchingEnvironments)) + " modules for environment '" + environmentName + "', taking first.")
    matchingEnvironment = matchingEnvironments[0]
    return matchingEnvironment

#######################################################################################################################
#def setupEnvironment(environmentName: str, environments: list, verbose: bool) -> bool:
def setupEnvironment(environmentName, environments, verbose):
    """Prepare the given environment for the given network nodes"""
    if verbose:
        print("Preparing environment '" + environmentName + "'...")

    environment = getMatchingEnvironment(environmentName, environments)
    if not environment:
        return False

    try:
        succeeded = environment.setupEnvironment()
        return succeeded
    except BaseException as e:
        print("Error: Exception while preparing environment '" + environmentName + "', '" + str(e) + "'.")
        if verbose:
            print(traceback.format_exc())
        return False

#######################################################################################################################
#def teardownEnvironment(environmentName: str, environments: list, verbose: bool) -> bool:
def teardownEnvironment(environmentName, environments, verbose):
    """Prepare the given environment for the given network nodes"""
    if verbose:
        print("Shutting down environment '" + environmentName + "'...")

    environment = getMatchingEnvironment(environmentName, environments)
    if not environment:
        return False

    try:
        succeeded = environment.teardownEnvironment()
        return succeeded
    except BaseException as e:
        print("Error: Exception while shutting environment '" + environmentName + "' down, '" + str(e) + "'.")
        if verbose:
            print(traceback.format_exc())
        return False

#######################################################################################################################
#def launchParticipant(participantName: str, environmentName: str, environments: list, verbose: bool) -> object:
def launchParticipant(participantName, environmentName, environments, verbose):
    """Launch the given participant in its environment"""
    if verbose:
        print("Launching participant '" + participantName + "'...")

    environment = getMatchingEnvironment(environmentName, environments)
    if not environment:
        return None

    try:
        process = environment.launchParticipant(participantName)
    except BaseException:
        print("Error: Exception while launching participant '" + participantName + "' in environment '" + environmentName + "', continuing with next participant.")
        if verbose:
            print(traceback.format_exc())
        return None

    return process

#######################################################################################################################
def main():
    """Main method of the commandline tool"""
    # Parse arguments
    args = parseArguments()

    if not Configuration.isIntegrationBusInstalled(args.verbose):
        sys.exit(1)

    # Load JSON schema
    config = loadConfigFiles(args.configFile, args.verbose)
    if not config:
        sys.exit(1)

    # Find the launch configuration
    launchConfiguration = findLaunchConfiguration(config, args.launchConfiguration, args.verbose)
    if not launchConfiguration:
        sys.exit(1)

    # Setup list of used launch environments
    participantEnvironments = launchConfiguration["ParticipantEnvironments"]

    # If the active Middleware is VAsio, we inject a launch environment for the registry at the position 0
    activeMiddleware = "VAsio"
    try:
        activeMiddleware = config["MiddlewareConfig"]["ActiveMiddleware"]
    except KeyError:
        pass

    if activeMiddleware == "VAsio":
        if args.exclude_registry:
            print("Excluding IbRegistry from startup environment")
        else:
            ibRegistryEnv = {
                "Environment": "IbRegistry",
                "Participant": "IbRegistry",
                "ConfigFile": args.configFile[0]
                }
            participantEnvironments.insert(0, ibRegistryEnv)
        
    networkNodes = launchConfiguration["NetworkNodes"] if "NetworkNodes" in launchConfiguration else list()
    if args.verbose:
        print("Launch configuration '" + launchConfiguration["Name"] + "' defines " + str(len(participantEnvironments)) + " participants")

    # Create process coordinator
    try:
        processCoordinator = ProcessCoordinator.ProcessCoordinator(args.logFile, args.verbose)
    except BaseException as e:
        sys.exit(1)

    # Load environment modules
    environments = loadEnvironments(participantEnvironments, networkNodes, processCoordinator, args.domainId, args.verbose)

    # Prepare environments that are used
    participantEnvironmentsByName = [env["Environment"] for env in participantEnvironments]
    # Note: https://stackoverflow.com/questions/480214/how-do-you-remove-duplicates-from-a-list-whilst-preserving-order
    usedEnvironmentsByName = list(collections.OrderedDict.fromkeys(participantEnvironmentsByName))

    assert(args.command)  # We defined COMMAND_DEFAULT as default value
    if not args.command.lower() in COMMANDS:
        print("Error: Command '" + args.command + "' is unknown, can be one of: '" + "', '".join(COMMANDS) + "'.")
        sys.exit(1)

    succeeded = True

    if args.command.lower() == COMMAND_SETUP or args.command.lower() == COMMAND_SETUPRUNTEARDOWN:
        for usedEnvironmentName in usedEnvironmentsByName:

            sys.stdout.flush()
            succeeded = setupEnvironment(usedEnvironmentName, environments, args.verbose)
            sys.stdout.flush()

            if not succeeded:
                print("Error: Emergency stop, failed to setup environment")
                break

    if args.command.lower() == COMMAND_RUN or (args.command.lower() == COMMAND_SETUPRUNTEARDOWN and succeeded):
        # TODO: We might call into a Python module implemented in C++ with C exports (https://docs.python.org/2/extending/extending.html).
        for retry in range(0, args.retries + 1):
            if retry > 0 and args.verbose:
                print("Retrying another time (retry #" + str(retry) + " of " + str(args.retries) + ")")

            # Launch participants subsequently
            for participantEnvironment in participantEnvironments:
                participantName = participantEnvironment["Participant"]
                environmentName = participantEnvironment["Environment"]

                sys.stdout.flush()
                succeeded = launchParticipant(participantName, environmentName, environments, args.verbose)
                sys.stdout.flush()

                if args.startupDelay > 0 and args.verbose:
                    print("Waiting for " + str(args.startupDelay) + " seconds")
                    time.sleep(args.startupDelay)

                if not succeeded:
                    print("Error: Failed to launch participant '" + participantName + "'")
                    break

            if not succeeded:
                continue

            if args.verbose:
                print("All processes started.")
                print("  Note: Press ^C to terminate before all processes are done.")

            try:
                succeeded = processCoordinator.monitorProcesses()
            except (EOFError, KeyboardInterrupt):
                # User pressed ^D or ^C
                print("Error: User pressed ^C to halt processes.")
                processCoordinator.terminateProcesses()
                succeeded = False
                break

            if not succeeded:
                continue

            succeeded = processCoordinator.terminateProcesses()
            if succeeded:
                if args.verbose:
                    print("Success: All processes terminated without error.")
                break

    if args.command.lower() == COMMAND_TEARDOWN or args.command.lower() == COMMAND_SETUPRUNTEARDOWN:
        # Explicitly undo environment setup, e.g. uninstall patched CANoe driver
        for environmentName in usedEnvironmentsByName:
            sys.stdout.flush()
            succeeded = teardownEnvironment(environmentName, environments, args.verbose) and succeeded
            sys.stdout.flush()

    sys.exit(0 if succeeded else 1)

#######################################################################################################################
if __name__ == '__main__':
    """Entry point"""
    main()
