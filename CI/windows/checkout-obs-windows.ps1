Function Checkout-OBS-Repo {
    Param(
        [Parameter(Mandatory=$true)]
        [String]$Tag
    )

    Write-Status "Checkout OBS with tag ${Version}"
    Ensure-Directory $ObsBuildDir

    if (!(Test-Path "${ObsBuildDir}/.git")) {

        Write-Step "Init repository..."
        git init
        git remote add origin https://github.com/obsproject/obs-studio.git
    } else {
        Write-Step "Found existing OBS repository..."
    }
    Write-Step "Fetching ${Tag}..."

    git fetch origin $Tag
    git reset --hard $Tag

    Ensure-Directory $CheckoutDir
}