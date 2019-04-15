# #######################################################################################################################
# IntegrationBus Launcher
# Copyright (c) Vector Informatik GmbH. All rights reserved.
#######################################################################################################################

import unittest
from unittest import mock
import os
import sys

from iblauncher.Configuration import isIntegrationBusInstalled
from iblauncher.Configuration import getIntegrationBusLibraryPath, getIntegrationBusBinaryPath
from iblauncher.Configuration import resolveVariables

# Test case can resolve module under test when run from root folder, e.g. 
# via 'python -m unittest discover', or 'python -m unittest tests.TestIbLauncherConfiguration'
class TestIbLauncherConfiguration(unittest.TestCase):

    domainId = 123

    def setUp(self):
        self.maxDiff = None  # Full string diff in case of assertEqual fails
    
    def tearDown(self):
        pass

    @mock.patch('iblauncher.Configuration.__determineIntegrationBusPaths')
    def testResolvingIntegrationBusInstallation(self, determineIntegrationBusPathsMock):
        determineIntegrationBusPathsMock.return_value = False
        self.assertFalse(isIntegrationBusInstalled(verbose=True))

        determineIntegrationBusPathsMock.return_value = True
        self.assertTrue(isIntegrationBusInstalled(verbose=True))

    @mock.patch('iblauncher.Configuration.getIntegrationBusBinaryPath', return_value='/usr/local/bin')
    @mock.patch('iblauncher.Configuration.getIntegrationBusLibraryPath', return_value='/usr/local/lib')
    def testResolvingIntegrationBusVariables(self, getIntegrationBusLibraryPathMock, getIntegrationBusBinaryPathMock):
        validLibraryPath = os.path.dirname(__file__) + os.path.sep + "files" + os.path.sep + "lib" + os.path.sep
        validBinaryPath = os.path.dirname(__file__) + os.path.sep + "files" + os.path.sep + "bin" + os.path.sep
        getIntegrationBusLibraryPathMock.return_value = validLibraryPath
        getIntegrationBusBinaryPathMock.return_value = validBinaryPath

        configFileAbsolutePath = "./my/configFileAbsolutePath/"
        participantName = "participantName"
        domainId = 77
        resolvedExpression = resolveVariables(
            "^INTEGRATIONBUS_BINPATH=%INTEGRATIONBUS_BINPATH%, " + 
            "%INTEGRATIONBUS_LIBPATH=${INTEGRATIONBUS_LIBPATH}, "
            "?INTEGRATIONBUS_CONFIGFILE=$INTEGRATIONBUS_CONFIGFILE, "
            "&INTEGRATIONBUS_PARTICIPANTNAME=%INTEGRATIONBUS_PARTICIPANTNAME%, "
            "/INTEGRATIONBUS_DOMAINID=${INTEGRATIONBUS_DOMAINID}.", 
            configFileAbsolutePath, participantName, domainId)
        self.assertEqual(resolvedExpression, 
            "^INTEGRATIONBUS_BINPATH=" + validBinaryPath + os.path.sep + ", " + 
            "%INTEGRATIONBUS_LIBPATH=" + validLibraryPath + os.path.sep + ", "
            "?INTEGRATIONBUS_CONFIGFILE=" + configFileAbsolutePath + ", "
            "&INTEGRATIONBUS_PARTICIPANTNAME=" + participantName + ", "
            "/INTEGRATIONBUS_DOMAINID=" + str(domainId) + ".")

if __name__ == '__main__':
    unittest.main()
