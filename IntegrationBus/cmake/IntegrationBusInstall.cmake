################################################################################
# Install definitions
################################################################################
# Goal: we adhere to the GNU standard layout as much as possible:
# /+
#  |- bin/+
#         |- *{.exe,dll}
#         |- IbLauncher
#  |- lib/+
#         |- *.{so,a,lib} <artifacts>
#         |- python/site-packages/Launcher <launcher support>
#  |
#  |- share/+
#           |- cmake/ <exports>
#           |- doc/ <docs+readme>
#/////// -- uses ci/package.py to include the following into the tarball
#  |- Demos/ <Demo sources, CMakeLists.txt adjusted, easily opens in vstudio>
#  |- Source/+
#            |- cmake/ <macro, modules>
#            |- IntegrationBus/ <complete sources>
#            |- ThirdParty/ <external sources>
#            |- CMakeSettings.json
#            |- CMakeLists.txt  <deployment toplevel>
#  |
#  |- LICENSE.txt
#  |- ThirdParty-LICENSE.txt
#  |- README.rst
#/////// --- package.py end

include(GNUInstallDirs)
set(INSTALL_TOP_DIR .)
set(INSTALL_BIN_DIR ${CMAKE_INSTALL_BINDIR})
set(INSTALL_LIB_DIR ${CMAKE_INSTALL_LIBDIR})
set(INSTALL_PYTHON_LIB_DIR ${INSTALL_LIB_DIR}/python/site-packages)
set(INSTALL_CODE_DIR ${CMAKE_INSTALL_DATADIR})
set(INSTALL_SOURCE_DIR Source)
set(INSTALL_DEMO_DIR ${INSTALL_SOURCE_DIR}/Demos)
set(INSTALL_CONFIG_DIR ${CMAKE_INSTALL_DATADIR}/cmake)
set(INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set(INSTALL_DOC_DIR ${CMAKE_INSTALL_DOCDIR})
