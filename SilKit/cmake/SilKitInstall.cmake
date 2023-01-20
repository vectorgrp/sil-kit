# Copyright (c) 2022 Vector Informatik GmbH
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

################################################################################
# Install definitions
################################################################################
# Goal: we adhere to the GNU standard layout for the binary distribution as much as possible:
# /SilKit/+
#  |- bin/+
#         |- *{.exe,dll}
#         |- SilKitLauncher
#  |- lib/+
#         |- *.{so,a,lib} <artifacts>
#         |- python/site-packages/Launcher <launcher support>
#  |
#  |- share/+
#           |- cmake/ <exports>
#           |- doc/ <docs+readme>
#### The source, documentation and demo source distribution follows a more user-friendly approach
#  |- LICENSE.txt
#  |- ThirdParty-LICENSE.txt
#  |- CHANGELOG.rst
#  |- SilKit-Source
#  |- SilKit-Demos
#  |- SilKit-Documentation
#
#
### package.py is used to merge zip files together, see package.py --help for more infos

include(GNUInstallDirs)
set(INSTALL_TOP_DIR "") # when changing add a "/"

set(INSTALL_BIN_DIR ${INSTALL_TOP_DIR}${CMAKE_INSTALL_BINDIR})
set(INSTALL_LIB_DIR ${INSTALL_TOP_DIR}${CMAKE_INSTALL_LIBDIR})
set(INSTALL_PYTHON_LIB_DIR ${INSTALL_LIB_DIR}/python/site-packages)
set(INSTALL_CODE_DIR ${INSTALL_TOP_DIR}${CMAKE_INSTALL_DATADIR})
set(INSTALL_DATA_DIR ${INSTALL_TOP_DIR}${CMAKE_INSTALL_DATADIR})
set(INSTALL_CONFIG_DIR ${INSTALL_DATA_DIR}/cmake)
set(INSTALL_INCLUDE_DIR ${INSTALL_TOP_DIR}${CMAKE_INSTALL_INCLUDEDIR})

set(INSTALL_SOURCE_DIR SilKit-Source)
set(INSTALL_DEMO_DIR SilKit-Demos)
set(INSTALL_DOC_DIR SilKit-Documentation)
