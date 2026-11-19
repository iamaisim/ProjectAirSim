param(
    [string]$ArchivePath,
    [string]$Url,
    [string]$ExpectedSha256,
    [string]$DestinationRoot,
    [switch]$Force,
    [switch]$Ensure
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

if ($ArchivePath -and $Url) {
    throw "Specify only one of -ArchivePath or -Url."
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$manifest = Get-Content -LiteralPath (Join-Path $PSScriptRoot "suv-assets.json") -Raw | ConvertFrom-Json

if (-not $ArchivePath -and -not $Url) {
    $Url = $manifest.url
}
if (-not $ExpectedSha256) {
    $ExpectedSha256 = $manifest.sha256
}
if (-not $DestinationRoot) {
    $DestinationRoot = Join-Path $repoRoot "unreal\Blocks\Plugins\ProjectAirSim\Content\VehicleAdv"
}

$destination = Join-Path ([System.IO.Path]::GetFullPath($DestinationRoot)) "SUV"
$metadataPath = Join-Path $destination ".projectairsim-suv-assets.json"
if ($Ensure -and (Test-Path -LiteralPath $destination)) {
    $installedUrl = $null
    if (Test-Path -LiteralPath $metadataPath) {
        $installedUrl = (Get-Content -LiteralPath $metadataPath -Raw | ConvertFrom-Json).url
    }
    $installedComplete = $true
    foreach ($asset in @("SuvCarPawn.uasset", "SUV_TorqueCurve.uasset", "SuvWheel_Front.uasset")) {
        if (-not (Test-Path -LiteralPath (Join-Path $destination $asset) -PathType Leaf)) {
            $installedComplete = $false
        }
    }
    $legacyDirectory = Join-Path ([System.IO.Path]::GetDirectoryName($destination)) "SUV_UE52"
    if ($Url -and $installedUrl -eq $Url -and $installedComplete -and
        -not (Test-Path -LiteralPath $legacyDirectory)) {
        Write-Host "SUV assets already match manifest URL; skipping download."
        exit 0
    }
    $Force = $true
}

$temporaryArchive = $null
$stagingPath = Join-Path ([System.IO.Path]::GetTempPath()) ("projectairsim-suv-" + [guid]::NewGuid())

try {
    if ($Url) {
        $temporaryArchive = Join-Path ([System.IO.Path]::GetTempPath()) (([guid]::NewGuid()).ToString() + ".zip")
        Invoke-WebRequest -Uri $Url -OutFile $temporaryArchive
        $resolvedArchive = $temporaryArchive
    }
    else {
        $resolvedArchive = (Resolve-Path -LiteralPath $ArchivePath).Path
    }

    $sha256 = [System.Security.Cryptography.SHA256]::Create()
    $archiveStream = [System.IO.File]::OpenRead($resolvedArchive)
    try {
        $actualSha256 = [System.BitConverter]::ToString(
            $sha256.ComputeHash($archiveStream)
        ).Replace("-", "")
    }
    finally {
        $archiveStream.Dispose()
        $sha256.Dispose()
    }
    if ($actualSha256 -ne $ExpectedSha256) {
        throw "SUV asset checksum mismatch. Expected $ExpectedSha256 but got $actualSha256."
    }

    New-Item -ItemType Directory -Path $stagingPath | Out-Null
    Expand-Archive -LiteralPath $resolvedArchive -DestinationPath $stagingPath

    $stagedSuv = Join-Path $stagingPath "SUV"
    if (-not (Test-Path -LiteralPath (Join-Path $stagedSuv "SuvCarPawn.uasset"))) {
        throw "Invalid SUV asset pack: SUV/SuvCarPawn.uasset was not found."
    }

    $stagedUE52 = Join-Path $stagingPath "SUV_UE52"
    if (Test-Path -LiteralPath $stagedUE52 -PathType Container) {
        # Older combined archives keep the UE 5.2 overrides in a second folder.
        Get-ChildItem -LiteralPath $stagedUE52 -Recurse -File | ForEach-Object {
            $relativePath = $_.FullName.Substring($stagedUE52.Length).TrimStart("\", "/")
            $targetPath = Join-Path $stagedSuv $relativePath
            New-Item -ItemType Directory -Path ([System.IO.Path]::GetDirectoryName($targetPath)) -Force | Out-Null
            Copy-Item -LiteralPath $_.FullName -Destination $targetPath -Force
        }
        Remove-Item -LiteralPath $stagedUE52 -Recurse -Force
    }
    foreach ($asset in @("SuvCarPawn.uasset", "SUV_TorqueCurve.uasset", "SuvWheel_Front.uasset")) {
        if (-not (Test-Path -LiteralPath (Join-Path $stagedSuv $asset) -PathType Leaf)) {
            throw "Invalid SUV asset pack: SUV/$asset was not found. The pack must include the UE 5.2 vehicle assets."
        }
    }

    $legacyDirectory = Join-Path ([System.IO.Path]::GetDirectoryName($destination)) "SUV_UE52"

    # Check every destination before replacing any existing asset directory.
    foreach ($target in @($destination, $legacyDirectory)) {
        if ((Test-Path -LiteralPath $target) -and -not $Force) {
            throw "SUV assets already exist at '$target'. Use -Force to replace them."
        }
    }

    New-Item -ItemType Directory -Path ([System.IO.Path]::GetDirectoryName($destination)) -Force | Out-Null
    if (Test-Path -LiteralPath $destination) {
        Remove-Item -LiteralPath $destination -Recurse -Force
    }
    Move-Item -LiteralPath $stagedSuv -Destination $destination
    Write-Host "Installed merged UE 5.2 SUV assets at: $destination"
    if (Test-Path -LiteralPath $legacyDirectory) {
        Remove-Item -LiteralPath $legacyDirectory -Recurse -Force
        Write-Host "Removed legacy SUV_UE52 directory: $legacyDirectory"
    }
    $installedUrl = $Url
    if (-not $installedUrl -and $actualSha256 -eq $manifest.sha256) {
        $installedUrl = $manifest.url
    }
    @{ url = $installedUrl; includesUE52 = $true } | ConvertTo-Json | Set-Content -LiteralPath $metadataPath
}
finally {
    if (Test-Path -LiteralPath $stagingPath) {
        Remove-Item -LiteralPath $stagingPath -Recurse -Force
    }
    if ($temporaryArchive -and (Test-Path -LiteralPath $temporaryArchive)) {
        Remove-Item -LiteralPath $temporaryArchive -Force
    }
}
