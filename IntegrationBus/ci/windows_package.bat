@echo off

rem Default arguments (Note: While ';' would work here, it cannot be used on the command line; thus the '#' separator)
set "compiler=VS2017"
set "buildNumber=0"
set "buildTargets=Debug#Release#Debug#Release"
set "buildArchitectures=x64#x64#Win32#Win32"
set "buildFolders=../../VS2017_Debug_x64/build#../../VS2017_Release_x64/build#../../VS2017_Debug_Win32/build#../../VS2017_Release_Win32/build"
set "packageFolders=/#/#/#/"
set "componentGroups=IntegrationBus-Library-Package#IntegrationBus-Contributor-Package"

rem Process parameters
if "%~1" == "-h" ( goto printHelp )
if "%~1" == "--help" ( goto printHelp )
if not "%~1" == "" ( set "compiler=%~1" )
if not "%~2" == "" ( set "buildNumber=%~2" )
if not "%~3" == "" ( set "buildTargets=%~3" )
if not "%~4" == "" ( set "buildArchitectures=%~4" )
if not "%~5" == "" ( set "buildFolders=%~5" )
if not "%~6" == "" ( set "packageFolders=%~6" )
if not "%~7" == "" ( set "componentGroups=%~7" )

rem Map compiler identifier to generator
if "%compiler%" == "VS2015" ( set "generator=Visual Studio 14 2015" )
if "%compiler%" == "VS2017" ( set "generator=Visual Studio 15 2017" )
if "%generator%" == "-h" ( goto printHelp )

rem Replace '#' by ';' because ';' is a reserved argument delimiter and thus cannot be used on the command-line
set "buildTargets=%buildTargets:#=;%"
set "buildArchitectures=%buildArchitectures:#=;%"
set "buildFolders=%buildFolders:#=;%"
set "packageFolders=%packageFolders:#=;%"
set "componentGroups=%componentGroups:#=;%"

rem Trick to echo on/off inside blocks
set "echo_on=echo on&for %%. in (.) do"

echo Creating Windows packages
echo ---------------------------------------------------------------------------------------------------
echo Generator: %generator%
echo Working target: %workingTarget%
echo Working architecture: %workingArchitecture%
echo Build number: %buildNumber%
echo Build targets: %buildTargets%
echo Build architectures: %buildArchitectures%
echo Build folders: %buildFolders%
echo Package folders: %packageFolders%
echo Component groups: %componentGroups%

echo .
echo ---------------------------------------------------------------------------------------------------
echo Preparing folders...

rem We assume that we are already invoked from the %workingFolder% which is to be built for 
rem target %workingTarget% and %workingArchitecture%
rem Remember where we are, this is where the packages should be placed
for %%i in (.) do set "workingFolder=%%~nxi"
echo Working folder: %workingFolder%

if exist package ( rmdir package /q /s )
md package
if %errorlevel% neq 0 goto exitWithError

if not exist build ( md build )
if %errorlevel% neq 0 goto exitWithError
cd build

rem Configure and build all projects for this package
setlocal EnableDelayedExpansion
set i=0
for %%a in ("%buildTargets:;=" "%") do set /A i+=1 & set buildTargets[!i!]=%%~a
set i=0
for %%a in ("%buildArchitectures:;=" "%") do set /A i+=1 & set buildArchitectures[!i!]=%%~a
set i=0
for %%a in ("%buildFolders:;=" "%") do set /A i+=1 & set buildFolders[!i!]=%%~a
set n=%i%
for /L %%i in (1,1,%n%) do (
    set "buildTarget=!buildTargets[%%i]!"
    set "buildArchitecture=!buildArchitectures[%%i]!"
    set "buildFolder=!buildFolders[%%i]!"

    echo Configuring project "!buildFolder!" for generator "%generator%" and for packaging...
    if not exist !buildFolder! ( md !buildFolder! )
    if %errorlevel% neq 0 goto exitWithError
    cd !buildFolder!

    %echo_on% cmake .. -G "%generator%" -A "!buildArchitecture!" ^
-DIB_BUILD_NUMBER=%buildNumber% ^
-DIB_INSTALL_PDB_FILES=OFF ^
-DCMAKE_INSTALL_PREFIX=../install ^
-DCMAKE_PREFIX_PATH=./install/!buildArchitecture! ^
-DCPACK_MULTICONFIG_PACKAGE=ON ^
-DCPACK_MULTICONFIG_BUILDFOLDERS="%buildFolders%" ^
-DCPACK_MULTICONFIG_PACKAGEFOLDERS="%packageFolders%" ^
-DCMAKE_BUILD_TYPE=!buildTarget!
    if %errorlevel% neq 0 goto exitWithError

    echo .
    echo ---------------------------------------------------------------------------------------------------
    echo Building project for "!buildTarget!"...
    %echo_on% cmake --build . --config "!buildTarget!"
    if %errorlevel% neq 0 goto exitWithError

    rem echo .
    rem echo ---------------------------------------------------------------------------------------------------
    rem echo Running tests on project...
    rem %echo_on% ctest -C !buildTarget! -VV -R "^Test"
    rem if %errorlevel% neq 0 goto exitWithError
)

rem Change to previously created output folder
cd ..\..\%workingFolder%\package

echo .
echo ---------------------------------------------------------------------------------------------------
echo Packaging all component groups "%componentGroups%" of project...
for %%c in ("%componentGroups:;=" "%") do (
    echo Packaging component group "%%c" of project...
    %echo_on% cpack --config ..\build\CPackConfig.cmake --verbose ^
-D CPACK_COMPONENT_GROUP=%%c
    if %errorlevel% neq 0 goto exitWithError
)

cd ..

echo .
echo ---------------------------------------------------------------------------------------------------
echo Packaging succeeded

goto exitWithoutError

rem Output help
:printHelp
echo Usage: windows_package.bat ["Compiler" "buildNumber" "buildTargets" "buildArchitectures" "buildFolders" "packageFolders" "componentGroups"]
echo Default: "%compiler%" "%buildNumber%" "%buildTargets%" "%buildArchitectures%" %buildFolders%" "%packageFolders%" "%componentGroups%"

goto exitWithoutError

:exitWithError
echo .
echo ---------------------------------------------------------------------------------------------------
echo Error: Last packaging step failed with exit code %errorlevel%

:exitWithoutError
exit /b %errorlevel%
