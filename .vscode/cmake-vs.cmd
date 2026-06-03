@REM SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
@REM Wrapper used by VS Code tasks and CMake Tools.
@REM It discovers a Visual Studio installation, initializes the MSVC dev environment,
@REM then forwards all arguments to cmake so configure/build use consistent toolchains.
@echo off
setlocal

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
	echo ERROR: Could not find vswhere at "%VSWHERE%".
	exit /b 1
)

set "VSINSTALL="
for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%I"

if "%VSINSTALL%"=="" (
	for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do set "VSINSTALL=%%I"
)

if "%VSINSTALL%"=="" (
	echo ERROR: Could not find a Visual Studio installation with C++ tools.
	exit /b 1
)

call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64 >nul
if errorlevel 1 (
	echo ERROR: Failed to initialize VS developer environment.
	exit /b 1
)

cmake %*
exit /b %ERRORLEVEL%
