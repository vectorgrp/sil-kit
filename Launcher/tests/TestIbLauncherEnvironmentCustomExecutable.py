#######################################################################################################################
# IntegrationBus Launcher
# Copyright (c) Vector Informatik GmbH. All rights reserved.
#######################################################################################################################

import unittest
from unittest import mock
import os
import sys
import shutil
#import signal
import time
import subprocess

from iblauncher.modules.EnvironmentCustomExecutable import EnvironmentCustomExecutable

# Test case can resolve module under test when run from root folder, e.g. 
# via 'python -m unittest discover', or 'python -m unittest tests.TestIbLauncherEnvironmentCustomExecutable'
class TestIbLauncherEnvironmentCustomExecutable(unittest.TestCase):

    def setUp(self):
        pass
    
    def tearDown(self):
        pass

    def testName(self):
        self.assertEqual(EnvironmentCustomExecutable.getEnvironmentName(), "CustomExecutable")

    @mock.patch('iblauncher.Configuration.getIntegrationBusBinaryPath', return_value='/usr/local/bin')
    @mock.patch('iblauncher.Configuration.getIntegrationBusLibraryPath', return_value='/usr/local/lib')
    def testLaunchExecutable(self, getIntegrationBusLibraryPathMock, getIntegrationBusBinaryPathMock):
        participantName = "TestParticipant"
        commandAbsolutePath = "IbDemoTest"
        arguments = "Publisher 42"
        workingFolder = "tests"
        configFile = "Dummy"
        participantEnvironments = [
                {
                    "Participant": participantName,
                    "Environment": "CustomExecutable", 
                    "Executable": commandAbsolutePath,
                    "Arguments": arguments,
                    "WorkingFolder": workingFolder,

                    "ConfigFile": configFile
                }
            ]
        networkNodes = {
            # None
        }
        domainId = 42
        with mock.patch('iblauncher.ProcessCoordinator') as processCoordinatorMock:
            environment = EnvironmentCustomExecutable(participantEnvironments, networkNodes, processCoordinatorMock, domainId, True)

            environment.launchParticipant(participantName)
            processCoordinatorMock.launchProcess.assert_called_once_with(
                participantName, 
                os.path.abspath(commandAbsolutePath), 
                arguments, 
                os.path.abspath(workingFolder), 
                {
                    'INTEGRATIONBUS_CONFIGFILE': os.path.abspath(configFile), 
                    'INTEGRATIONBUS_PARTICIPANTNAME': participantName, 
                    'INTEGRATIONBUS_DOMAINID': str(domainId)
                }, 
                True)

if __name__ == '__main__':
    unittest.main()
