#!/usr/bin/env python3

#######################################################################################################################
# IntegrationBus Launcher Plugin
# Copyright (c) Vector Informatik GmbH. All rights reserved.
#######################################################################################################################
import sys, os
filepath = os.path.dirname(os.path.abspath(__file__))
localpath = os.path.abspath(os.path.join(filepath, "../lib/python/site-packages"))
sys.path.append(localpath)
try:
    from iblauncher import Launcher
except ImportError as e:
    print("ERROR: cannot import iblauncher. Please check installation. Error: {}".format(str(e)))


#######################################################################################################################
# Entry point
if __name__ == '__main__':
  Launcher.main()
