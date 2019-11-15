cmake_minimum_required(VERSION 3.5)

#######################################################################################################################
# Project definition
#######################################################################################################################

project("IntegrationBus-Launcher" LANGUAGES CXX C)

include(IntegrationBusVersion)
include(IntegrationBusInstall)
configure_ib_version(${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 14)

#######################################################################################################################
# Dependencies
#######################################################################################################################
#find_package(IntegrationBus)
# we prefer python3 over python2
unset(PYTHON_EXECUTABLE CACHE)
set(Python_ADDITIONAL_VERSIONS "2.7")
find_package(PythonInterp 3)

#######################################################################################################################
# Install targets
#######################################################################################################################


# Create an IbInstallation.json that references the installed IntegrationBus libraries and Demo binaries
# On a Windows installation, we must explicitly reference the installed build variant (can be ../Debug/bin or ../Release/bin)
# On a Linux installation, this should be always '../../../bin' and '../../../lib'
if(TARGET IntegrationBus)
    set(INTEGRATIONBUS_LIBPATH ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB_DIR}) # Resolved install target of IntegrationBus library
    set(INTEGRATIONBUS_BINPATH ${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_DIR})
    message(STATUS "Configuring the IntegrationBus Launcher IbInstallation.json for installation paths at:"
       " bin: ${INTEGRATIONBUS_BINPATH}"
       " lib: ${INTEGRATIONBUS_LIBPATH}"

    )
else()
    set(INTEGRATIONBUS_LIBPATH "")
    set(INTEGRATIONBUS_BINPATH "")
    message(STATUS "Configuring the Launcher for locally installed IntegrationBus binaries and libraries")
endif()

set(INSTALLATION_JSON_IN  "${CMAKE_CURRENT_SOURCE_DIR}/iblauncher/data/IbInstallation.json.in")
set(INSTALLATION_JSON     "${CMAKE_CURRENT_BINARY_DIR}/iblauncher/data/IbInstallation.json")
install(
    CODE "message(\"Configuring ${INSTALLATION_JSON_IN} -> ${INSTALLATION_JSON} using ${INTEGRATIONBUS_BINPATH}\")"
    CODE "configure_file(${INSTALLATION_JSON_IN} ${INSTALLATION_JSON})"
)


install(
    PROGRAMS IbLauncher.py
    DESTINATION ${INSTALL_BIN_DIR}
    COMPONENT IntegrationBus-Launcher
)

# the following encodes the version number in the python module
function(make_version_py destdir)
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
		${destdir}
		@ONLY)
endfunction()
set(VERSION_PY    "${CMAKE_CURRENT_BINARY_DIR}/iblauncher/__init__.py") 
make_version_py(${VERSION_PY})
install(FILES ${VERSION_PY} DESTINATION ${INSTALL_PYTHON_LIB_DIR}/iblauncher)
# Install Python sources for Launcher developers (including self-contained CMake build script)
install(
    DIRECTORY iblauncher
    DESTINATION ${INSTALL_PYTHON_LIB_DIR}
    COMPONENT IntegrationBus-Launcher
)

#######################################################################################################################
# Unit testing
#######################################################################################################################

enable_testing()

if(PYTHONINTERP_FOUND)
    # These tests require unittest.mock which is not part of the 2.x standard distribution (there would be a backport via PyPi though)
    if(PYTHON_VERSION_MAJOR EQUAL 3 OR PYTHON_VERSION_MAJOR GREATER 3)
		# Copy over all sources, so we can build, test and install within build folder and keep source folder clean
        set(LAUNCHER_TEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/ctest)
		function(copy_file sourceFile)
			configure_file(
				"${CMAKE_CURRENT_SOURCE_DIR}/${sourceFile}"
				"${LAUNCHER_TEST_DIR}/${sourceFile}"
				COPYONLY
			)
		endfunction()
		copy_file("IbLauncher.py")
		copy_file("LICENSE")
		copy_file("README.md")
		make_version_py("${LAUNCHER_TEST_DIR}/iblauncher/__init__.py")
		copy_file("iblauncher/Launcher.py")
		copy_file("iblauncher/Environment.py")
		copy_file("iblauncher/ProcessCoordinator.py")
		copy_file("iblauncher/Configuration.py")
		copy_file("iblauncher/modules/__init__.py")
		copy_file("iblauncher/modules/EnvironmentCANoe.py")
		copy_file("iblauncher/modules/EnvironmentCustomExecutable.py")
		copy_file("iblauncher/modules/EnvironmentIbRegistry.py")
		copy_file("iblauncher/data/IbConfig.schema.json")
		copy_file("tests/__init__.py")
		copy_file("tests/TestIbLauncherConfiguration.py")
		copy_file("tests/TestIbLauncherEnvironmentCustomExecutable.py")

        if(NOT EXISTS ${LAUNCHER_TEST_DIR})
            message(STATUS "Copying IB launcher to build dir (for running tests)")
            file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${LAUNCHER_TEST_DIR})
        endif()
        add_test(NAME TestIbLauncherConfiguration
            COMMAND ${PYTHON_EXECUTABLE} -m unittest tests.TestIbLauncherConfiguration
            WORKING_DIRECTORY ${LAUNCHER_TEST_DIR}
        )
        add_test(NAME TestIbLauncherEnvironmentCustomExecutable
        COMMAND ${PYTHON_EXECUTABLE} -m unittest tests.TestIbLauncherEnvironmentCustomExecutable
        WORKING_DIRECTORY ${LAUNCHER_TEST_DIR}
        )
        
    else()
        message(WARNING "The IntegrationBus Launcher tests cannot be executed, because a Python distribution 3.0+ was not found: ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")
    endif()
else()
    message(WARNING "The IntegrationBus Launcher tests cannot be executed, because a Python distribution 2.7+ was not found.")
endif()
