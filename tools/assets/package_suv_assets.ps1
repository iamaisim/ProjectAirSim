param(
    [string]$OutputPath = "ProjectAirSim-SUV-Assets-v1.1.1.zip"
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$sourceParent = Join-Path $repoRoot "unreal\Blocks\Plugins\ProjectAirSim\Content\VehicleAdv"
$resolvedOutput = [System.IO.Path]::GetFullPath($OutputPath)

foreach ($asset in @("SUV/SuvCarPawn.uasset", "SUV/SUV_TorqueCurve.uasset", "SUV/SuvWheel_Front.uasset")) {
    if (-not (Test-Path -LiteralPath (Join-Path $sourceParent $asset) -PathType Leaf)) {
        throw "Required SUV asset is not installed at '$(Join-Path $sourceParent $asset)'."
    }
}

if (Test-Path -LiteralPath $resolvedOutput) {
    throw "Output archive already exists: '$resolvedOutput'."
}

Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
New-Item -ItemType Directory -Path ([System.IO.Path]::GetDirectoryName($resolvedOutput)) -Force | Out-Null

$archiveStream = [System.IO.File]::Open(
    $resolvedOutput,
    [System.IO.FileMode]::CreateNew
)
try {
    $zip = [System.IO.Compression.ZipArchive]::new(
        $archiveStream,
        [System.IO.Compression.ZipArchiveMode]::Create
    )
    try {
        $sourcePath = Join-Path $sourceParent "SUV"
        Get-ChildItem -LiteralPath $sourcePath -Recurse -File |
            Where-Object { $_.Name -ne ".projectairsim-suv-assets.json" } | ForEach-Object {
            $relativePath = $_.FullName.Substring($sourcePath.Length).TrimStart("\", "/")
            $entryName = (Join-Path "SUV" $relativePath).Replace("\", "/")
            [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile(
                $zip,
                $_.FullName,
                $entryName,
                [System.IO.Compression.CompressionLevel]::Optimal
            ) | Out-Null
        }
    }
    finally {
        $zip.Dispose()
    }
}
finally {
    $archiveStream.Dispose()
}
$archive = Get-Item -LiteralPath $resolvedOutput
$hash = (Get-FileHash -LiteralPath $resolvedOutput -Algorithm SHA256).Hash

Write-Host "Created: $resolvedOutput"
Write-Host ("Size: {0:N2} MB" -f ($archive.Length / 1MB))
Write-Host "SHA256: $hash"
