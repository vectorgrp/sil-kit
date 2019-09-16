@echo off

rem Defaults
set "compiler=VS2017"
set "target=Debug"
set "architecture=Win32"
set "buildNumber=0"

rem Process parameters
if "%~1" == "-h" ( goto printHelp )
if "%~1" == "--help" ( goto printHelp )
if not "%~1" == "" ( set "compiler=%~1" )
if not "%~2" == "" ( set "target=%~2" )
if not "%~3" == "" ( set "architecture=%~3" )
if not "%~4" == "" ( set "buildNumber=%~4" )

rem Map compiler identifier to generator
if "%compiler%" == "VS2015" ( set "generator=Visual Studio 14 2015" )
if "%compiler%" == "VS2017" ( set "generator=Visual Studio 15 2017" )
if "%generator%" == "-h" ( goto printHelp )

echo Performing a Windows build
echo Generator: %generator%
echo Target: %target%
echo Architecture: %architecture%
echo Build number: %buildNumber%

echo .
echo ---------------------------------------------------------------------------------------------------
echo Preparing folders...

if not exist build ( md build )
cd build

echo .
echo ---------------------------------------------------------------------------------------------------
echo Configuring project for "%generator%"...
rem Variables CMAKE_INSTALL_PREFIX and CMAKE_PREFIX_PATH are needed for the build cache
rem Variables CPACK_MULTICONFIG_PACKAGE and CMAKE_BUILD_TYPE are used by the packaging stage
@echo on
cmake .. -G "%generator%" -A "%architecture%" ^
-DIB_BUILD_NUMBER=%buildNumber% ^
-DIB_INSTALL_PDB_FILES=ON ^
-DIB_BUILD_DOCS=ON ^
-DCMAKE_INSTALL_PREFIX=../install ^
-DCMAKE_PREFIX_PATH=./install/%architecture% ^
-DCPACK_MULTICONFIG_PACKAGE=OFF ^
-DCMAKE_BUILD_TYPE=%target%
@echo off
if %errorlevel% neq 0 goto exitWithError

echo .
echo ---------------------------------------------------------------------------------------------------
echo Building project for "%target%"...
@echo on
cmake --build . --config "%target%"
@echo off
if %errorlevel% neq 0 goto exitWithError

echo .
echo ---------------------------------------------------------------------------------------------------
echo Running tests on project...
@echo on
ctest -C %target% -VV -R "^Test"
@echo off
if %errorlevel% neq 0 goto exitWithError

echo .
echo ---------------------------------------------------------------------------------------------------
echo Installing all components of project...
@echo on
cmake --build . --config "%target%" --target install
@echo off
if %errorlevel% neq 0 goto exitWithError

echo .
echo ---------------------------------------------------------------------------------------------------
echo Build succeeded

goto exitWithoutError

rem Output help
:printHelp
echo Usage: windows_build.bat ["Compiler" ["Target" ["Architecture" ["BuildNumber"]]]]
echo Default: "%compiler%" "%target%" "%architecture%" "%buildNumber%"

goto exitWithoutError

:exitWithError
echo .
echo ---------------------------------------------------------------------------------------------------
echo Error: Last build step failed with exit code %errorlevel%

:exitWithoutError
exit /b %errorlevel%
