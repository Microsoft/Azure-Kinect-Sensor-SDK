# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

$ErrorActionPreference = "Stop"

function Download-ToTemp
{
    param
    (
        [Parameter(Mandatory)]
        [string] $url
    )

    $filename = Split-Path -Path $url -Leaf
    if (-not ($filename))
    {
        Write-Error "Unable to parse filename from $url"
        return $null
    }

    $tempDir = [System.IO.Path]::GetTempPath()
    $path = Join-Path -Path $tempDir -ChildPath $filename

    Write-Host -NoNewline "Downloading $url to $path..."
    [Net.ServicePointManager]::SecurityProtocol = "tls12, tls11, tls"
    Invoke-WebRequest -Uri $url -OutFile $path
    Write-Host "Done"

    return $path
}

# Download vswhere to temp dir
$url = "https://github.com/Microsoft/vswhere/releases/download/2.6.7/vswhere.exe"
$vswhere = Download-ToTemp -url $url

# Run vswhere to check for Visual Studio 2017 or Visual Studio 2019
$vsWhereOutput = (& $vswhere -prerelease -format xml)
$vsInstances = [xml]($vsWhereOutput)

if ((-not ($vsInstances)) -or (-not ($vsInstances.instances))) {
    Write-Error "Error parsing vswhere output: $vsWhereOutput"
}

$instances = $vsInstances.SelectNodes("//instances/instance")
if ($instances.Count -eq 0) {
    Write-Error "Visual Studio not installed. Please install Visual Studio 2017 or Visual Studio 2019"
}

$instances = $instances | Where-Object { [Version]$_.installationVersion -ge [Version]"15.0"}
if (-not $instances) {
    Write-Error "Only Visual Studio 2017 and greater are supported"
}

# Check for cmake and ninja (installed as part of the Microsoft.VisualStudio.Component.VC.CMake.Project component)
$vsWhereOutput = (& $vswhere -prerelease -format xml -requires Microsoft.VisualStudio.Component.VC.CMake.Project)
$vsInstancesWithCMakeNinja = [xml]$vsWhereOutput
if ((-not $vsInstancesWithCMakeNinja) -or (-not ($vsInstancesWithCMakeNinja.instances))) {
    if (-not (Get-Command -Name "cmake")) {
        Write-Error "Unable to find cmake. Please install cmake and add it to your path"
    }
    if (-not (Get-Command -Name "ninja")) {
        Write-Error "Unable to find ninja. Please install ninja and add it to your path"
    }
}

# Check for clang-format
$vs158instances = $instances | Where-Object { [Version]$_.installationVersion -ge [Version]"15.8"}
if (-not $vs158instances) {
    # Check for clang-format 6.0.0.
    if (-not (Get-Command -Name "clang-format")) {
        Write-Warning "clang-format 6.0.0 not detected! You will be unable to fix formatting issues."
    } else {
        if (-not (clang-format --version | Select-String "6.0.0")) {
            Write-Warning "clang-format 6.0.0 not detected! You will be unable to fix formatting issues."
        }
    }
}

# Check for tar
if (-not (Get-Command -Name "tar")) {
    Write-Error "Unable to find tar. Please install tar and add it to your path"
}

# Check for python 3
if (-not (Get-Command -All -Name "python3"))
{
    Write-Error "Unable to find python 3. Please install python3 and add it to your path"
}

Write-Host "Machine setup. Please use Visual Studio Developer Command prompt to build."
