Function Patch-OBS-QTDeps {    

    (Get-Content "${ObsBuildDir}/CI/windows/01_install_dependencies.ps1") `
        -replace "github.com/obsproject/obs-deps","github.com/summershrimp/obs-deps" |
        Out-File "${ObsBuildDir}/CI/windows/01_install_dependencies.ps1"

    (Get-Content "${ObsBuildDir}/.github/workflows/main.yml") `
        -replace "DEPS_VERSION_WIN: .*","DEPS_VERSION_WIN: '2022-09-12'" |
        Out-File "${ObsBuildDir}/.github/workflows/main.yml"
}

Function Build-OBS {
    & $ObsBuildDir/CI/build-windows.ps1
}