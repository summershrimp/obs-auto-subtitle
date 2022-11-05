param (
    [string]$Component = "all",
    [string]$OBSTag = "28.0.1",
    [Switch]$Package
 )

$ErrorActionPreference = "Stop"

$_RunObsBuildScript = $true
$ProductName = "OBS-Studio"

$CheckoutDir = Resolve-Path -Path "$PSScriptRoot\.."
$DepsBuildDir = "${CheckoutDir}/../obs-build-dependencies"
$ObsBuildDir = "${CheckoutDir}/../obs-studio"
$PlugBuildDir = "${CheckoutDir}"
$BuildArch = "x64"
$BuildDirectory = "build"
$BuildConfiguration = "RelWithDebInfo"

. ${CheckoutDir}/CI/windows/build_support_windows.ps1
. ${CheckoutDir}/CI/windows/checkout-obs-windows.ps1
. ${CheckoutDir}/CI/windows/build-obs-windows.ps1


Function Configure {
    Ensure-Directory "${PlugBuildDir}"
    Write-Status "Configuration of OBS Autosub build system..."

    $NumProcessors = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors

    if ( $NumProcessors -gt 1 ) {
        $env:UseMultiToolTask = $true
        $env:EnforceProcessCountAcrossBuilds = $true
    }

    # TODO: Clean up archive and directory naming across dependencies
    $CmakePrefixPath = Resolve-Path -Path "${CheckoutDir}/../obs-build-dependencies/windows-deps-${WindowsDepsVersion}-${BuildArch}"
    $BuildDirectoryActual = "${BuildDirectory}$(if (${BuildArch} -eq "x64") { "64" } else { "32" })"
    $GeneratorPlatform = "$(if (${BuildArch} -eq "x64") { "x64" } else { "Win32" })"

    $CmakeCommand = @(
        "-G", "${CmakeGenerator}"
        "-DCMAKE_GENERATOR_PLATFORM=`"${GeneratorPlatform}`"",
        "-DCMAKE_SYSTEM_VERSION=`"${CmakeSystemVersion}`"",
        "-DCMAKE_PREFIX_PATH:PATH=`"${CmakePrefixPath}`"",
        "-Dw32-pthreads_DIR=`"${ObsBuildDir}\${BuildDirectoryActual}\deps\w32-pthreads`"",
        "-DLibObs_DIR=`"${ObsBuildDir}\${BuildDirectoryActual}\libobs`"",
        "-Dobs-frontend-api_DIR=`"${ObsBuildDir}\${BuildDirectoryActual}\UI\obs-frontend-api`"",
        "-DCMAKE_INSTALL_PREFIX=`"${CheckoutDir}/release`""
        "$(if (Test-Path Variable:$Quiet) { "-Wno-deprecated -Wno-dev --log-level=ERROR" })"
    )
        # "-DW32_PTHREADS_LIB=`"${ObsBuildDir}\${BuildDirectoryActual}\w32-pthreads\%build_config%\w32-pthreads.lib`"",
        # "-DLIBOBS_INCLUDE_DIR=`"%OBSPath%\libobs`"",
        # "-DLIBOBS_LIB=`"%OBSPath%\build64\libobs\%build_config%\obs.lib`""
    
    Invoke-External cmake -S . -B  "${BuildDirectoryActual}" @CmakeCommand

    Ensure-Directory ${PlugBuildDir}
}


function Build {
    Param(
        [String]$BuildDirectory = $(if (Test-Path variable:BuildDirectory) { "${BuildDirectory}" }),
        [String]$BuildArch = $(if (Test-Path variable:BuildArch) { "${BuildArch}" }),
        [String]$BuildConfiguration = $(if (Test-Path variable:BuildConfiguration) { "${BuildConfiguration}" })
    )

    $NumProcessors = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors

    if ( $NumProcessors -gt 1 ) {
        $env:UseMultiToolTask = $true
        $env:EnforceProcessCountAcrossBuilds = $true
    }

    Write-Status "Build OBS Autosub"

    Configure

    Ensure-Directory ${CheckoutDir}
    Write-Step "Build OBS targets..."

    $BuildDirectoryActual = "${BuildDirectory}$(if (${BuildArch} -eq "x64") { "64" } else { "32" })"

    Invoke-External cmake --build "${BuildDirectoryActual}" --config ${BuildConfiguration}
}

function Package {
    Write-Step "Install Plugin..."
    Invoke-Expression "cmake --build `"${BuildDirectory}64`" --config ${BuildConfiguration} -t install"
}

function Build-OBS-Autosub-Main {
    $build_obs = $false
    $build_plugin = $false
    $package_plugin = $false
    if ($Component -eq "all") {
        $build_obs = $true
        $build_plugin = $true
    } elseif ($Component -eq "obs") {
        $build_obs = $true
    } elseif ($Component -eq "plugin") {
        $build_plugin = $true
        if ($Package.IsPresent) {
            $package_plugin = $true
        }
    } else {
        Write-Error "Invalid component: $Component"
    }

    if($build_obs) {
        Checkout-OBS-Repo -Tag $OBSTag
        Patch-OBS-QTDeps
        Build-OBS
    }
    if($build_plugin) {
        Build
        if($package_plugin) {
            Package
        }
    }
}

Build-OBS-Autosub-Main