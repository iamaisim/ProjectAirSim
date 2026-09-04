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
    if ($Url -and $installedUrl -eq $Url) {
        Write-Host "SUV assets already match manifest URL; skipping download."
        exit 0
    }
    $Force = $true
}

$temporaryArchive = $null
$stagingPath = Join-Path ([System.IO.Path]::GetTempPath()) ("projectairsim-suv-" + [guid]::NewGuid())

try {
    if ($Url) {
        $temporaryArchive = Join-Path ([System.IO.Path]::GetTempPath()) $manifest.archive
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

    if (Test-Path -LiteralPath $destination) {
        if (-not $Force) {
            throw "SUV assets already exist at '$destination'. Use -Force to replace them."
        }
        Remove-Item -LiteralPath $destination -Recurse -Force
    }

    New-Item -ItemType Directory -Path ([System.IO.Path]::GetDirectoryName($destination)) -Force | Out-Null
    Move-Item -LiteralPath $stagedSuv -Destination $destination
    @{ url = $Url } | ConvertTo-Json | Set-Content -LiteralPath $metadataPath
    Write-Host "Installed SUV assets at: $destination"
}
finally {
    if (Test-Path -LiteralPath $stagingPath) {
        Remove-Item -LiteralPath $stagingPath -Recurse -Force
    }
    if ($temporaryArchive -and (Test-Path -LiteralPath $temporaryArchive)) {
        Remove-Item -LiteralPath $temporaryArchive -Force
    }
}
