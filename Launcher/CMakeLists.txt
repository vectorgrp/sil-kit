cmake_minimum_required(VERSION 3.5)

#######################################################################################################################
# Project definition
#######################################################################################################################

project("IntegrationBus-Launcher" LANGUAGES CXX C)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/IntegrationBusVersion.cmake)
configure_ib_version(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 14)

option(CMAKE_INSTALL_SYMLINKS "Install symlinks on Linux systems" ON)

#######################################################################################################################
# Install definitions
#######################################################################################################################

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/IntegrationBusInstall.cmake)
configure_ib_install(${PROJECT_NAME})

#######################################################################################################################
# Dependencies
#######################################################################################################################

include(FindPythonInterp)
find_package(PythonInterp 2.7)

# Try to resolve from installed packages only if not defined previously via add_library
if(NOT TARGET IntegrationBus)
    foreach(path ${CMAKE_PREFIX_PATH})
        message(STATUS "Searching for IntegrationBus library at prefix path: " ${path})
    endforeach(path)
    message(STATUS "  Note: Set '-DCMAKE_PREFIX_PATH=<path>' so that ./cmake/IntegrationBusTargets.cmake resolves.")
    find_package(IntegrationBus ${PROJECT_VERSION} REQUIRED)
    if(NOT IntegrationBus_FOUND)
        message(STATUS "Cannot configure the Launcher for an installed IntegrationBus library")
    endif()
endif()

#######################################################################################################################
# Unit testing
#######################################################################################################################

enable_testing()

if(PYTHONINTERP_FOUND)
    # These tests require unittest.mock which is not part of the 2.x standard distribution (there would be a backport via PyPi though)
    if(PYTHON_VERSION_MAJOR EQUAL 3 OR PYTHON_VERSION_MAJOR GREATER 3)
        add_test(NAME TestIbLauncherConfiguration
            COMMAND ${PYTHON_EXECUTABLE} -m unittest tests.TestIbLauncherConfiguration
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
        add_test(NAME TestIbLauncherEnvironmentCustomExecutable
        COMMAND ${PYTHON_EXECUTABLE} -m unittest tests.TestIbLauncherEnvironmentCustomExecutable
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    else()
        message(WARNING "The IntegrationBus Launcher tests cannot be executed, because a Python distribution 3.0+ was not found.")
    endif()
else()
    message(WARNING "The IntegrationBus Launcher tests cannot be executed, because a Python distribution 2.7+ was not found.")
    #return()
endif()

#######################################################################################################################
# Install targets
#######################################################################################################################

# Install targets for particular components, so one can simply 'cmake --build . --target install_<Name>'
# https://stackoverflow.com/questions/9190098/for-cmakes-install-command-what-can-the-component-argument-do
add_custom_target(install_IntegrationBus-Launcher
    COMMAND "${CMAKE_COMMAND}" -DCOMPONENT=IntegrationBus-Launcher -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    COMMENT "Installing component 'IntegrationBus-Launcher'"
    VERBATIM
)

set(VERSION_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/__version__.py.in")
set(VERSION_PY    "${CMAKE_CURRENT_BINARY_DIR}/__version__.py")
configure_file(${VERSION_PY_IN} ${VERSION_PY})

# Create an IbInstallation.json that references the installed IntegrationBus libraries and Demo binaries
# On a Windows installation, we must explicitly reference the installed build variant (can be ../Debug/bin or ../Release/bin)
# On a Linux installation, this should be always '../../../bin' and '../../../lib'
if(TARGET IntegrationBus)
    #set(INTEGRATIONBUS_LIBPATH "$<TARGET_FILE_DIR:IntegrationBus>") # Resolved install target of IntegrationBus library
    #set(INTEGRATIONBUS_BINPATH "$<TARGET_FILE_DIR:IntegrationBus>/../${CMAKE_INSTALL_BINDIR}")
    #file(RELATIVE_PATH INTEGRATIONBUS_BINPATH ${INSTALL_CODE_DIR} ${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR}) # This local installation
    #file(RELATIVE_PATH INTEGRATIONBUS_LIBPATH ${INSTALL_CODE_DIR} ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIBDIR})
    set(INTEGRATIONBUS_LIBPATH ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIBDIR}) # Resolved install target of IntegrationBus library
    set(INTEGRATIONBUS_BINPATH ${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR})
    message(STATUS "Configuring the Launcher for IntegrationBus binaries installed at " ${INTEGRATIONBUS_BINPATH})
    message(STATUS "Configuring the Launcher for IntegrationBus libraries installed at " ${INTEGRATIONBUS_LIBPATH})
else()
    set(INTEGRATIONBUS_LIBPATH "")
    set(INTEGRATIONBUS_BINPATH "")
    message(STATUS "Configuring the Launcher for locally installed IntegrationBus binaries and libraries")
endif()
set(INSTALLATION_JSON_IN  "${CMAKE_CURRENT_SOURCE_DIR}/iblauncher/data/IbInstallation.json.in")
set(INSTALLATION_JSON_OUT "${CMAKE_CURRENT_BINARY_DIR}/iblauncher/data/IbInstallation.json.out")
set(INSTALLATION_JSON     "${CMAKE_CURRENT_BINARY_DIR}/iblauncher/data/IbInstallation.json")
# At configure-time, deal with @...@/${...} configurable items
configure_file(${INSTALLATION_JSON_IN} ${INSTALLATION_JSON_OUT})
# At build-time, deal with $<CONFIG:...> generator expressions
# Mangle name to avoid fatal errors "Evaluation file to be written multiple times with different content."
file(GENERATE OUTPUT "${INSTALLATION_JSON}.$<CONFIG>" INPUT ${INSTALLATION_JSON_OUT})
# Install under original name
install(
    FILES
        "${INSTALLATION_JSON}.$<CONFIG>"
    DESTINATION ${INSTALL_CODE_DIR}/iblauncher/data
    RENAME "IbInstallation.json"
    COMPONENT IntegrationBus-Launcher
)
## Better solution in a future CMake version https://gitlab.kitware.com/cmake/cmake/issues/15785
#install(
#    CODE "message(\"Configuring ${INSTALLATION_JSON_IN} -> ${INSTALLATION_JSON} using ${INTEGRATIONBUS_BINPATH}\")"
#    CODE "configure_file(${INSTALLATION_JSON_IN} ${INSTALLATION_JSON})"
#    COMPONENT IntegrationBus-Launcher
#)

# Copy over all sources, so we can build, test and install within build folder and keep source folder clean
function(copy_file sourceFile)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/${sourceFile}" 
        "${CMAKE_CURRENT_BINARY_DIR}/${sourceFile}"
        COPYONLY
    )
endfunction()
copy_file("IbLauncher.py")
copy_file("LICENSE")
copy_file("README.md")
copy_file("iblauncher/__init__.py")
copy_file("iblauncher/Launcher.py")
copy_file("iblauncher/Environment.py")
copy_file("iblauncher/ProcessCoordinator.py")
copy_file("iblauncher/Configuration.py")
copy_file("iblauncher/modules/__init__.py")
copy_file("iblauncher/modules/EnvironmentCANoe.py")
copy_file("iblauncher/modules/EnvironmentCustomExecutable.py")
copy_file("iblauncher/data/IbConfig.schema.json")
copy_file("tests/__init__.py")
copy_file("tests/TestIbLauncherConfiguration.py")
copy_file("tests/TestIbLauncherEnvironmentCustomExecutable.py")

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    # Install the root script and make it 'chmod +x', so we can invoke 'IbLauncher' directly instead of 'python IbLauncher.py'
    install(
        FILES
            IbLauncher.py
        DESTINATION ${INSTALL_CODE_DIR}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE  # chmod +x
        #RENAME IbLauncher  # she-bang!
        COMPONENT IntegrationBus-Launcher
    )

    if(CMAKE_INSTALL_SYMLINKS)
        # Install a symbolic link into the bin folder
        macro(install_symlink filepath symlinkpath)
            install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${filepath} ${symlinkpath})")
            install(CODE "message(STATUS \"Installing: symlink ${symlinkpath} -> ${filepath}\")")
        endmacro(install_symlink)
        install_symlink(${CMAKE_INSTALL_PREFIX}/${INSTALL_CODE_DIR}/IbLauncher.py ${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR}/IbLauncher)
    else()
        # Install shell script into the bin folder
        install(
            FILES
                IbLauncher.sh
            DESTINATION ${INSTALL_BINDIR}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE  # chmod +x
            RENAME IbLauncher  # she-bang!
            COMPONENT IntegrationBus-Launcher
        )
    endif()
else()
    # Install the main script as usual
    install(
        FILES
            IbLauncher.py
        DESTINATION ${INSTALL_CODE_DIR}
        COMPONENT IntegrationBus-Launcher
    )

    # Install a batch file so we can run 'IbLauncher' directly instead of 'python IbLauncher.py'
    set(LAUNCHER_BAT_IN  "${CMAKE_CURRENT_SOURCE_DIR}/IbLauncher.bat.in")
    set(LAUNCHER_BAT_OUT "${CMAKE_CURRENT_BINARY_DIR}/IbLauncher.bat.out")
    set(LAUNCHER_BAT     "${CMAKE_CURRENT_BINARY_DIR}/IbLauncher.bat")
    # At configure-time, deal with @...@/${...} configurable items
    configure_file(${LAUNCHER_BAT_IN} ${LAUNCHER_BAT_OUT})
    # Mangle name to avoid fatal errors "Evaluation file to be written multiple times with different content."
    file(GENERATE OUTPUT "${LAUNCHER_BAT}.$<CONFIG>" INPUT ${LAUNCHER_BAT_OUT})
    # Install batch file under original name into the bin folder
    install(
        FILES
            "${LAUNCHER_BAT}.$<CONFIG>"
        DESTINATION ${INSTALL_BINDIR}
        RENAME "IbLauncher.bat"
        COMPONENT IntegrationBus-Launcher
    )
    ## Better solution in a future CMake version https://gitlab.kitware.com/cmake/cmake/issues/15785
    #install(
    #    CODE "message(\"Configuring ${LAUNCHER_BAT_IN} -> ${LAUNCHER_BAT} using ${INTEGRATIONBUS_BINPATH}\")"
    #    CODE "configure_file(${LAUNCHER_BAT_IN} ${LAUNCHER_BAT})"
    #    COMPONENT IntegrationBus-Launcher
    #)
endif()

# Install Python sources for Launcher developers (including self-contained CMake build script)
install(
    DIRECTORY
        iblauncher
    DESTINATION ${INSTALL_CODE_DIR}
    COMPONENT IntegrationBus-Launcher
    FILES_MATCHING
        PATTERN *.py 
        PATTERN *.json 
)

# Install supplementary files
install(
    FILES
        LICENSE
    DESTINATION ${INSTALL_CODE_DIR}
    COMPONENT IntegrationBus-Launcher
)
install(
    FILES
        README.md
    DESTINATION ${INSTALL_CODE_DIR}
    COMPONENT IntegrationBus-Launcher
)
