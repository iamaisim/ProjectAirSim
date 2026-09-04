param(
    [string]$OutputPath = "ProjectAirSim-SUV-Assets-v1.0.0.zip"
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$sourcePath = Join-Path $repoRoot "unreal\Blocks\Plugins\ProjectAirSim\Content\VehicleAdv\SUV"
$resolvedOutput = [System.IO.Path]::GetFullPath($OutputPath)

if (-not (Test-Path -LiteralPath (Join-Path $sourcePath "SuvCarPawn.uasset"))) {
    throw "The SUV asset is not installed at '$sourcePath'."
}

if (Test-Path -LiteralPath $resolvedOutput) {
    throw "Output archive already exists: '$resolvedOutput'."
}

Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem

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
        Get-ChildItem -LiteralPath $sourcePath -Recurse -File | ForEach-Object {
            $relativePath = $_.FullName.Substring($sourcePath.Length).TrimStart("\")
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
