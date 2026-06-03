# SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# Loads a Visual Studio developer shell into the current PowerShell session.
# Usage examples:
#   .\.vscode\vs-dev-shell.ps1
#   .\.vscode\vs-dev-shell.ps1 -Arch x64
#   .\.vscode\vs-dev-shell.ps1 -Arch x64 cmake --build --preset debug
param(
    [ValidateSet("x64", "x86", "arm64")]
    [string]$Arch = "x64",

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$Command
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-VsInstallationPath {
    $vswherePath = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswherePath)) {
        throw "Could not find vswhere at '$vswherePath'. Install Visual Studio Installer first."
    }

    $path = & $vswherePath -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $path) {
        $path = & $vswherePath -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
    }
    if (-not $path) {
        throw "Could not find a Visual Studio installation with C++ tools."
    }

    return $path.Trim()
}

function Import-VsDevEnvironment {
    param(
        [Parameter(Mandatory = $true)]
        [string]$VsInstallPath,

        [Parameter(Mandatory = $true)]
        [string]$TargetArch
    )

    $vsDevCmd = Join-Path $VsInstallPath "Common7\Tools\VsDevCmd.bat"
    if (-not (Test-Path $vsDevCmd)) {
        throw "Could not find VsDevCmd.bat at '$vsDevCmd'."
    }

    $envDump = & cmd.exe /s /c "`"$vsDevCmd`" -no_logo -arch=$TargetArch -host_arch=$TargetArch && set"
    if ($LASTEXITCODE -ne 0) {
        throw "VsDevCmd failed with exit code $LASTEXITCODE."
    }

    foreach ($line in $envDump) {
        if ($line -match "^([^=]+)=(.*)$") {
            $name = $matches[1]
            $value = $matches[2]
            Set-Item -Path "Env:$name" -Value $value
        }
    }
}

try {
    $env:VSCMD_SKIP_SENDTELEMETRY = "1"
    $installPath = Get-VsInstallationPath
    Import-VsDevEnvironment -VsInstallPath $installPath -TargetArch $Arch

    Write-Host "Loaded Visual Studio dev environment:" -ForegroundColor Green
    Write-Host "  VS Path: $installPath"
    Write-Host "  Target:  $Arch"

    if ($null -ne $Command -and $Command.Length -gt 0 -and -not [string]::IsNullOrWhiteSpace($Command[0])) {
        $exe = $Command[0]
        $args = @()
        if ($Command.Length -gt 1) {
            $args = $Command[1..($Command.Length - 1)]
        }

        & $exe @args
        exit $LASTEXITCODE
    }
}
catch {
    Write-Error $_
    exit 1
}
